cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)


if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -mavx -msse -msse2 -msse3 -msse4")

# We shouldn't have to do this, bah:
#TODO find way of guranteeing libc++ on OS X CLI (hence >=C++11) without brute forcing CMake like this
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++)
SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "Minimum OS X deployment version")
endif(APPLE)

project (flucoma_pd LANGUAGES CXX)

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

set(FLUID_PATH '' CACHE PATH "The top level of the fluid_decomposition repo")
set(PD_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "The top level of a Pure Data installation")
set(LOCAL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)

get_filename_component(FLUID_ABS_PATH ${FLUID_PATH} ABSOLUTE)
message(${FLUID_ABS_PATH})
# add_subdirectory("${FLUID_ABS_PATH}" build)
# if (NOT DEFINED ${FLUID_DECOMP_PATH})
# 	message(FATAL_ERROR "Please set the path to the fluid_decomposition sources with -DFLUID_DECOMP_PATH=<path>")
# endif()

if (NOT (EXISTS "${FLUID_ABS_PATH}/build/fluid_decomposition-exports.cmake"))
	message(FATAL_ERROR "Can't find the fluid_decomposition CMake targets file at ${FLUID_ABS_PATH}/build/fluid_decomposition-exports.cmake. Please go to ${FLUID_ABS_PATH}/build and run CMake")
endif()

get_filename_component(PD_ABS_PATH ${PD_PATH} ABSOLUTE)
set(PD_LIB ${PD_ABS_PATH}/bin/pd.lib)

if(NOT(EXISTS ${PD_LIB}) AND WIN32)
  message(FATAL_ERROR "Can't find pd.lib at ${PD_LIB}. This is needed to build on Windows")
endif()

include("${FLUID_ABS_PATH}/build/fluid_decomposition-exports.cmake")

get_property(FFT_SOURCES TARGET HISSTools_FFT  PROPERTY INTERFACE_SOURCES)
get_property(FFT_LINK TARGET HISSTools_FFT  PROPERTY INTERFACE_LINK_LIBRARIES)

add_library(FFTLIB STATIC ${FFT_SOURCES})

if (UNIX AND NOT APPLE)
target_compile_options( FFTLIB PUBLIC -fPIC)
endif()

target_link_libraries(
  FFTLIB PRIVATE ${FFT_LINK}
)

if(MSVC)
target_compile_options(
   FFTLIB PRIVATE $<$<NOT:$<CONFIG:DEBUG>>: /arch:AVX>
)
else(MSVC)
target_compile_options(
   FFTLIB PRIVATE $<$<NOT:$<CONFIG:DEBUG>>: -mavx -msse -msse2 -msse3 -msse4>
)
endif(MSVC)

add_library(FLUID_PD_WRAPPER INTERFACE)

target_include_directories (FLUID_PD_WRAPPER INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(FLUID_PD_WRAPPER
  INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include/FluidPDWrapper.hpp"
)

add_library( FLUID_PD INTERFACE )
target_sources(
	FLUID_PD INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include/FluidPDWrapper.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/PDBufferAdaptor.hpp"
)

if(MSVC)
target_compile_options(FLUID_PD INTERFACE -D_USE_MATH_DEFINES)
endif(MSVC)

SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach (project_dir ${PROJECT_DIRS})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir}/CMakeLists.txt")
		message("Generating: ${project_dir}")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir})
	endif ()
endforeach ()
