# Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
# Copyright 2017-2019 University of Huddersfield.
# Licensed under the BSD-3 License.
# See license.md file in the project root for full license information.
# This project has received funding from the European Research Council (ERC)
# under the European Union’s Horizon 2020 research and innovation programme
# (grant agreement No 725899).

cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

set(PD_PATH "" CACHE PATH "The top level of a Pure Data installation")
if(NOT PD_PATH)
  message(FATAL_ERROR "Pure Data source or application path is not set. Please pass with with -DPD_PATH=<path> or via ccmake / CMake gui. On Mac OS, you can use pd.app/Contents/Resources")
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/release-packaging")
set(PD_INSTALL_PREFIX "." CACHE PATH "Prefix for assembling PD packages")
set(FLUID_PACKAGE_NAME FluidCorpusManipulation CACHE STRING "Name for published package")
set(PD_PACKAGE_ROOT ${PD_INSTALL_PREFIX}/${FLUID_PACKAGE_NAME})

set(FLUID_PATH "" CACHE PATH "Optional top level of the flucoma-core repo; will download if absent")

option(DOCS "Generate HTML documentation" OFF)
set(FLUID_DOCS_PATH "" CACHE PATH "Optional path to flucoma-docs (needed for docs); will download if absent")

if (APPLE)
  set(CMAKE_XCODE_GENERATE_SCHEME ON)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "")
  # A consequence of targetting 10.8. Needs to be set globally from 10.15 onwards in order for the test program to compile successfully during configure 
  string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
endif()

project (flucoma-pd LANGUAGES CXX)

set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  flucoma-core
  GIT_REPOSITORY https://github.com/flucoma/flucoma-core.git
  GIT_PROGRESS TRUE
  GIT_TAG origin/main
)

FetchContent_Declare(
  flucoma-docs
  GIT_REPOSITORY https://github.com/flucoma/flucoma-docs.git
  GIT_PROGRESS TRUE
  GIT_TAG origin/main
)

if(FLUID_PATH)
  get_filename_component(
    FETCHCONTENT_SOURCE_DIR_FLUCOMA-CORE ${FLUID_PATH} ABSOLUTE
  )
endif()

FetchContent_GetProperties(flucoma-core)
if(NOT flucoma-core_POPULATED)
  FetchContent_Populate(flucoma-core)
  add_subdirectory(${flucoma-core_SOURCE_DIR} ${flucoma-core_BINARY_DIR})
  include(flucoma_version)
  include(flucoma-buildtools)
  include(flucoma-buildtype)
endif()

set_if_toplevel(VAR CMAKE_LIBRARY_OUTPUT_DIRECTORY
                TOPLEVEL "${CMAKE_CURRENT_SOURCE_DIR}/pd_objects"
              SUPERBUILD "${CMAKE_SOURCE_DIR}/pd_objects/${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}")


if(DOCS)
  
  if(FLUID_DOCS_PATH)
    get_filename_component(
      FETCHCONTENT_SOURCE_DIR_FLUCOMA-DOCS ${FLUID_DOCS_PATH} ABSOLUTE
    )
  endif()

  FetchContent_GetProperties(flucoma-docs)
  if(NOT flucoma-docs_POPULATED)
    FetchContent_Populate(flucoma-docs)
    file(GLOB_RECURSE DOC_SOURCE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${flucoma-docs_SOURCE_DIR}/**/*.cpp" )
    source_group("\\HTMLs Doc Gen" FILES ${DOC_SOURCE})
    add_subdirectory(${flucoma-docs_SOURCE_DIR} ${flucoma-docs_BINARY_DIR})
    include(FlucomaDocs)
    set(PD_DOC_OUT "${CMAKE_BINARY_DIR}/pd_ref")
    add_ref_target(pd "Making PD docs")
  endif()

  add_custom_target(PD_MAKE_DOCS ALL DEPENDS MAKE_PD_REF)
endif()

set(LOCAL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)

get_filename_component(PD_ABS_PATH ${PD_PATH} ABSOLUTE)
set(PD_LIB ${PD_ABS_PATH}/bin/pd.lib)

if(NOT(EXISTS ${PD_LIB}) AND WIN32)
  message(FATAL_ERROR "Can't find pd.lib at ${PD_LIB}/bin. This is needed to build on Windows: you need a Windows build of PD for this to be available")
endif()

add_library( FLUID_PD INTERFACE )

target_include_directories (
  FLUID_PD INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  SYSTEM INTERFACE
  "${PD_ABS_PATH}/src" # <= include m_pd.h
)

target_sources(
	FLUID_PD INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include/FluidPDWrapper.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/PDBufferAdaptor.hpp"
)

if(MSVC)
  target_compile_options(FLUID_PD INTERFACE -D_USE_MATH_DEFINES)
endif()

include(MakePDSources)

# generate targets for standalone clients
get_client_group(NONE client_list)
foreach(client ${client_list}) 
  get_core_client_header(${client} header)   
  get_core_client_class(${client} class)   
  generate_pd_source(
    CLIENTS ${client} 
    HEADERS ${header} 
    CLASSES ${class} 
  )
endforeach()

#generate target for MANIPULATION group 
get_client_group(MANIPULATION client_list)
foreach(client ${client_list})  
  get_core_client_header(${client} header)  
  get_core_client_class(${client} class)  
  list(APPEND MANIPULATION_CLIENTS ${client})
  list(APPEND MANIPULATION_HEADERS ${header})
  list(APPEND MANIPULATION_CLASSES ${class})
endforeach()

generate_pd_source(
  CLIENTS ${MANIPULATION_CLIENTS}
  HEADERS ${MANIPULATION_HEADERS}
  CLASSES ${MANIPULATION_CLASSES}
  FILENAME fluid_libmanipulation 
)

# make a working tree for devs 

add_custom_target(MAKE_DEV_FOlDER
  COMMAND ${CMAKE_COMMAND} -DROOT="${CMAKE_CURRENT_SOURCE_DIR}" -DCORE_SRC="${flucoma-core_SOURCE_DIR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/scripts/MakeWorkingFolder.cmake"
  COMMENT "synthesising working copy at ${CMAKE_CURRENT_SOURCE_DIR}/dev"
)

#install steps (externals have their own install target)
install(DIRECTORY help/ 
        DESTINATION ${PD_PACKAGE_ROOT} 
        FILES_MATCHING PATTERN "*.pd")
install(DIRECTORY abstractions/
        DESTINATION ${PD_PACKAGE_ROOT}
        FILES_MATCHING PATTERN "*.pd")
install(DIRECTORY examples 
        DESTINATION ${PD_PACKAGE_ROOT})
install(DIRECTORY "${flucoma-core_SOURCE_DIR}/Resources/AudioFiles/" 
        DESTINATION "${PD_PACKAGE_ROOT}/media")
install(DIRECTORY "${flucoma-core_SOURCE_DIR}/Resources/Data/"
        DESTINATION "${PD_PACKAGE_ROOT}/data")
install(FILES icon.png QuickStart.md 
        DESTINATION ${PD_PACKAGE_ROOT})
install(FILES ${flucoma-core_SOURCE_DIR}/distribution.lic 
        DESTINATION ${PD_PACKAGE_ROOT} 
        RENAME LICENSE.md)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/FluidCorpusManipulationOverview.pd 
        DESTINATION ${PD_PACKAGE_ROOT})
if(DOCS)
  install(DIRECTORY "${PD_DOC_OUT}/" 
          DESTINATION "${PD_PACKAGE_ROOT}/docs" 
          FILES_MATCHING REGEX "\\.(html|css)")
endif()
