cmake_minimum_required(VERSION 3.11)

set(PD_PATH "" CACHE PATH "The top level of a Pure Data installation")
if(NOT PD_PATH)
  message(FATAL_ERROR "Pure Data source or application path is not set. Please pass with with -DPD_PATH=<path> or via ccmake / CMake gui. On Mac OS, you can use pd.app/Contents/Resources")
endif()

set(FLUID_PATH "" CACHE PATH "Optional top level of the fluid_decomposition repo; will download if absent")

option(DOCS "Generate HTML documentation" OFF)
set(FLUID_PARAMDUMP_PATH "" CACHE PATH "Optional path to fluid_paramdump (needed for docs); will download if absent")

if (APPLE)
  set(CMAKE_XCODE_GENERATE_SCHEME ON)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)
endif()

project (flucoma_pd LANGUAGES CXX)
set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  fluid_decomposition
  GIT_REPOSITORY https://bitbucket.org/flucoma/fluid_decomposition.git
  GIT_PROGRESS TRUE
)

FetchContent_Declare(
  fluid_paramdump
  GIT_REPOSITORY https://bitbucket.org/flucoma/fluid_paramdump.git
  GIT_PROGRESS TRUE
)

if(FLUID_PATH)
  get_filename_component(
    FETCHCONTENT_SOURCE_DIR_FLUID_DECOMPOSITION ${FLUID_PATH} ABSOLUTE
  )
endif()

FetchContent_GetProperties(fluid_decomposition)
if(NOT fluid_decomposition_POPULATED)
  FetchContent_Populate(fluid_decomposition)
  add_subdirectory(${fluid_decomposition_SOURCE_DIR} ${fluid_decomposition_BINARY_DIR})
endif()

if(DOCS)
  if(FLUID_PARAMDUMP_PATH)
    get_filename_component(
      FETCHCONTENT_SOURCE_DIR_FLUID_PARAMDUMP ${FLUID_PARAMDUMP_PATH} ABSOLUTE
    )
  endif()

  FetchContent_GetProperties(fluid_paramdump)
  if(NOT fluid_paramdump_POPULATED)
    FetchContent_Populate(fluid_paramdump)
    file(GLOB_RECURSE DOC_SOURCE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${fluid_paramdump_SOURCE_DIR}/**/*.cpp" )
    source_group("\\HTMLs Doc Gen" FILES ${DOC_SOURCE})
    add_subdirectory(${fluid_paramdump_SOURCE_DIR} ${fluid_paramdump_BINARY_DIR})
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

SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach (project_dir ${PROJECT_DIRS})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir}/CMakeLists.txt")
		message("Generating: ${project_dir}")
		add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir}")
    source_group("" FILES "${project_dir}.cpp")
	endif ()
endforeach ()

#install 
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/release-packaging")
set(PD_INSTALL_PREFIX "." CACHE PATH "Prefix for assembling PD packages")
set(FLUID_PACKAGE_NAME FluidCorpusManipulation CACHE STRING "Name for published package")
set(PD_PACKAGE_ROOT ${PD_INSTALL_PREFIX}/${FLUID_PACKAGE_NAME})

#slashes at the end of the directories copy just the files, rather than the hierarchy; so don't remove them
install(DIRECTORY pd_objects/ DESTINATION ${PD_PACKAGE_ROOT} FILES_MATCHING REGEX "\\.(pd_darwin|pd_linux|dll)")
install(DIRECTORY help/ DESTINATION ${PD_PACKAGE_ROOT} FILES_MATCHING PATTERN "*.pd")
install(DIRECTORY examples DESTINATION ${PD_PACKAGE_ROOT})

# install(CODE [[ message(STATUS "${fluid_decomposition_SOURCE_DIR}") ]])
install(DIRECTORY "${fluid_decomposition_SOURCE_DIR}/AudioFiles/" DESTINATION "${PD_PACKAGE_ROOT}/media")
install(FILES license.md icon.png QuickStart.md DESTINATION ${PD_PACKAGE_ROOT})

if(DOCS)
  install(DIRECTORY ${FLUID_PD_REF_PATH} DESTINATION "${PD_PACKAGE_ROOT}/docs" FILES_MATCHING REGEX "\\.(html|css)")
endif()
