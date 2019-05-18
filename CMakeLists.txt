####### original SC Cmake file starts here
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -mavx -msse -msse2 -msse3 -msse4")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "Minimum OS X deployment version")

project (fluid_decomposition_cli LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

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

set(FLUID_PATH "" CACHE PATH "The top level of the fluid_decomposition repo")
set(LOCAL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)

get_filename_component(FLUID_ABS_PATH ${FLUID_PATH} ABSOLUTE)
message(${FLUID_ABS_PATH})
# add_subdirectory("${FLUID_ABS_PATH}" build)
# if (NOT DEFINED ${FLUID_DECOMP_PATH})
# 	message(FATAL_ERROR "Please set the path to the fluid_decomposition sources with -DFLUID_DECOMP_PATH=<path>")
# endif()

if (NOT (EXISTS "${FLUID_ABS_PATH}/build/fluid_decomposition-exports.cmake"))
	message(FATAL_ERROR "Can't find the fluid_decomposition CMake targets file at ${FLUID_ABS_PATH}/build/fluid_decomposition-expors.cmake. Please go to ${FLUID_ABS_PATH}/build and run CMake")
endif()

include("${FLUID_ABS_PATH}/build/fluid_decomposition-exports.cmake")

get_property(FFT_SOURCES TARGET HISSTools_FFT  PROPERTY INTERFACE_SOURCES)
get_property(FFT_LINK TARGET HISSTools_FFT  PROPERTY INTERFACE_LINK_LIBRARIES)

add_library(FFTLIB STATIC ${FFT_SOURCES})
target_link_libraries(
  FFTLIB PRIVATE ${FFT_LINK}
)
# target_compile_options(
#   FFTLIB PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>: -mavx -msse -msse2 -msse3 -msse4>"
# )

get_property(AUFIOFILE_SOURCES TARGET HISSTools_AudioFile PROPERTY INTERFACE_SOURCES)
add_library(AUDIOFILELIB STATIC ${AUFIOFILE_SOURCES})

# target_compile_options(
#   AUDIOFILELIB PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>: -mavx -msse -msse2 -msse3 -msse4>"
# )


add_library(FLUID_CLI_WRAPPER INTERFACE)

target_include_directories (FLUID_CLI_WRAPPER INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(FLUID_CLI_WRAPPER
  INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include/FluidCLIWrapper.hpp"
)

SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach (project_dir ${PROJECT_DIRS})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir}/CMakeLists.txt")
		message("Generating: ${project_dir}")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir})
	endif ()
endforeach ()
