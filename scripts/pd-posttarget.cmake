# Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
# Copyright 2017-2019 University of Huddersfield.
# Licensed under the BSD-3 License.
# See license.md file in the project root for full license information.
# This project has received funding from the European Research Council (ERC)
# under the European Union’s Horizon 2020 research and innovation programme
# (grant agreement No 725899).

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    FLUID_DECOMPOSITION FLUID_PD
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE 
  "${FLUID_VERSION_PATH}"
)


if ("${PROJECT_NAME}" MATCHES ".*_tilde")
	string(REGEX REPLACE "_tilde" "~" EXTERN_OUTPUT_NAME "${PROJECT_NAME}")
else ()
    set(EXTERN_OUTPUT_NAME "${PROJECT_NAME}")
endif ()

set_target_properties(${PROJECT_NAME} 
  PROPERTIES 
  OUTPUT_NAME "${EXTERN_OUTPUT_NAME}"
)
    
if(WIN32)
  if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    target_compile_definitions(${PROJECT_NAME} 
      PRIVATE 
      PD_LONGINTTYPE=intptr_t
    )
  endif()
endif()

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W3)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${PD_LIB})
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wreturn-type -Wconversion -Wno-c++11-narrowing)
endif()

#set AVX, or whatever
if(DEFINED FLUID_ARCH)
  target_compile_options(${PROJECT_NAME} PRIVATE ${FLUID_ARCH})
endif()

get_property(HEADERS TARGET FLUID_DECOMPOSITION PROPERTY INTERFACE_SOURCES)
source_group(TREE "${flucoma-core_SOURCE_DIR}/include" FILES ${HEADERS})

get_property(HEADERS TARGET FLUID_PD PROPERTY INTERFACE_SOURCES)
source_group("PD Wrapper" FILES ${HEADERS})
source_group("" FILES "${PROJECT_NAME}.cpp")

if(MSVC)
  target_compile_definitions( ${PROJECT_NAME} PRIVATE USE_MATH_DEFINES)
endif(MSVC)

### Output ###

if (APPLE)

	set_target_properties(${PROJECT_NAME} PROPERTIES
		SUFFIX ".pd_darwin"
    PREFIX ""
		XCODE_ATTRIBUTE_MACH_O_TYPE mh_dylib
		XCODE_ATTRIBUTE_EXECUTABLE_PREFIX ""
		XCODE_ATTRIBUTE_EXECUTABLE_EXTENSION "pd_darwin"
    # OSX_DEPLOYMENT_TARGET "10.8"
	) 
  #targeting <= 10.9, need to explicitly set libc++
  target_compile_options(${PROJECT_NAME} PRIVATE -stdlib=libc++)
  
elseif(UNIX AND NOT APPLE)

  set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".pd_linux"
    PREFIX ""
    POSITION_INDEPENDENT_CODE ON
  )

elseif (MSVC)

	set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".dll"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RelWithDebInfo ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  )

	# warning about constexpr not being const in c++14
	set_target_properties(${PROJECT_NAME} 
    PROPERTIES
		COMPILE_FLAGS "/wd4814"
  )

endif()
