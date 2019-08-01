
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
add_dependencies (${PROJECT_NAME} FLUID_DECOMPOSITION)
target_link_libraries(${PROJECT_NAME}
PUBLIC FLUID_DECOMPOSITION FLUID_PD
PRIVATE FFTLIB
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../include"
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W3)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${PD_LIB})
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wreturn-type -Wconversion)
endif()

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>: /arch:AVX>)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>: -mavx -msse -msse2 -msse3 -msse4>)
endif()



get_property(HEADERS TARGET FLUID_DECOMPOSITION PROPERTY INTERFACE_SOURCES)
source_group(TREE "${FLUID_PATH}/include" FILES ${HEADERS})

if ("${PROJECT_NAME}" MATCHES ".*_tilde")
	string(REGEX REPLACE "_tilde" "~" EXTERN_OUTPUT_NAME "${PROJECT_NAME}")
else ()
    set(EXTERN_OUTPUT_NAME "${PROJECT_NAME}")
endif ()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${EXTERN_OUTPUT_NAME}")

message(${EXTERN_OUTPUT_NAME})


if(MSVC)
  target_compile_definitions( ${PROJECT_NAME} PUBLIC USE_MATH_DEFINES)
endif(MSVC)

### Output ###

if (APPLE)

	set_target_properties(${PROJECT_NAME} PROPERTIES
		SUFFIX ".pd_darwin"
		XCODE_ATTRIBUTE_MACH_O_TYPE mh_dylib
		XCODE_ATTRIBUTE_EXECUTABLE_PREFIX ""
		XCODE_ATTRIBUTE_EXECUTABLE_EXTENSION "pd_darwin"
	)

elseif(UNIX AND NOT APPLE)
		set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pd_linux")
elseif (MSVC)

	set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RelWithDebInfo ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  )

	# warning about constexpr not being const in c++14
	set_target_properties(${PROJECT_NAME} PROPERTIES
		COMPILE_FLAGS "/wd4814")

endif (APPLE)

### Post Build ###
#if (WIN32)
#	add_custom_command(
#		TARGET ${PROJECT_NAME}
#		POST_BUILD
#		COMMAND rm "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${EXTERN_OUTPUT_NAME}.ilk"
#		COMMENT "ilk file cleanup"
#	)
#endif ()
