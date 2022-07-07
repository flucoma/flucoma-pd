# Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
# Copyright 2017-2019 University of Huddersfield.
# Licensed under the BSD-3 License.
# See license.md file in the project root for full license information.
# This project has received funding from the European Research Council (ERC)
# under the European Union’s Horizon 2020 research and innovation programme
# (grant agreement No 725899).

cmake_minimum_required(VERSION 3.18)

include(FluidClientStub)


# MSVC_RUNTIME_LIBRARY
# if(MSVC)
#   foreach(flag_var
#       CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
#       CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
#     if(${flag_var} MATCHES "/MD")
#       string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
#     endif()
#   endforeach()
# endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

function(make_external_name client header var)  
  string(FIND ${header} "clients/rt" is_rtclient)
  string(FIND ${client}  "Buf" is_bufclient)  
  string(TOLOWER ${client} lc_client) 
  string(PREPEND lc_client "fluid.")
  
  get_property(NO_TILDE GLOBAL PROPERTY FLUID_CORE_CLIENTS_${client}_KR_IN SET)
  
  if(${is_rtclient} GREATER -1 AND ${is_bufclient} EQUAL -1 AND NOT ${NO_TILDE})
    string(APPEND lc_client "~")
  endif()
  set(${var} ${lc_client} PARENT_SCOPE)
endfunction()

function (add_pd_external external source)

  # Define the supported set of keywords
  set(noValues NOINSTALL)
  set(singleValues "")
  set(multiValues "")
  # Process the arguments passed in
  include(CMakeParseArguments)
  cmake_parse_arguments(ARG
  "${noValues}"
  "${singleValues}"
  "${multiValues}"
  ${ARGN})  

  string(REPLACE "~" "_tilde" safe_name ${external})
  
  add_library(${safe_name} SHARED ${source})

  target_link_libraries(${safe_name}
      PRIVATE 
      FLUID_DECOMPOSITION FLUID_PD
  )

  target_include_directories(
    ${safe_name}
    PRIVATE 
    "${FLUID_VERSION_PATH}"
  )
  
  if(APPLE)
      file (STRINGS "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pd-linker-flags.txt" PD_SYM_LINKER_FLAGS)
    target_link_options(${safe_name} PRIVATE ${PD_SYM_LINKER_FLAGS})
  endif()

  set_target_properties(${safe_name} 
    PROPERTIES 
    OUTPUT_NAME "${external}"
  )
      
  if(WIN32)
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      target_compile_definitions(${safe_name} 
        PRIVATE 
        PD_LONGINTTYPE=intptr_t
      )
    endif()
  endif()

  if(MSVC)
    target_compile_options(${safe_name} PRIVATE /W3 /bigobj)
  	target_link_libraries(${safe_name} PRIVATE ${PD_LIB})
  else()
    target_compile_options(${safe_name} PRIVATE -Wall -Wextra -Wpedantic -Wreturn-type -Wconversion -Wno-c++11-narrowing)
  endif()

  #set AVX, or whatever
  # if(DEFINED FLUID_ARCH)
  #   target_compile_options(${PROJECT_NAME} PRIVATE ${FLUID_ARCH})
  # endif()

  get_property(HEADERS TARGET FLUID_DECOMPOSITION PROPERTY INTERFACE_SOURCES)
  source_group(TREE "${flucoma-core_SOURCE_DIR}/include" FILES ${HEADERS})

  get_property(HEADERS TARGET FLUID_PD PROPERTY INTERFACE_SOURCES)
  source_group("PD Wrapper" FILES ${HEADERS})
  source_group("" FILES "${source}")

  if(MSVC)
    target_compile_definitions(${safe_name} PRIVATE USE_MATH_DEFINES)
  endif(MSVC)

  ### Output ###

  if (APPLE)

  	set_target_properties(${safe_name} PROPERTIES
  		SUFFIX ".pd_darwin"
      PREFIX ""
  		XCODE_ATTRIBUTE_MACH_O_TYPE mh_dylib
  		XCODE_ATTRIBUTE_EXECUTABLE_PREFIX ""
  		XCODE_ATTRIBUTE_EXECUTABLE_EXTENSION "pd_darwin"
      # OSX_DEPLOYMENT_TARGET "10.8"
  	) 
    #targeting <= 10.9, need to explicitly set libc++
    target_compile_options(${safe_name} PRIVATE -stdlib=libc++)
    
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    set_target_properties(${safe_name} PROPERTIES
      SUFFIX ".pd_linux"
      PREFIX ""
      POSITION_INDEPENDENT_CODE ON
    )

  elseif (MSVC)
  	set_target_properties(${safe_name} PROPERTIES
      SUFFIX ".dll"
      WINDOWS_EXPORT_ALL_SYMBOLS ON
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
      RUNTIME_OUTPUT_DIRECTORY_RelWithDebInfo ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

  	# warning about constexpr not being const in c++14
  	set_target_properties(${safe_name} 
      PROPERTIES
  		COMPILE_FLAGS "/wd4814"
    )
  endif()
  
  if(NOT ARG_NOINSTALL)    
    install(
      TARGETS ${safe_name} 
      LIBRARY DESTINATION ${PD_PACKAGE_ROOT}
      RUNTIME DESTINATION ${PD_PACKAGE_ROOT}
    )
  endif() 
  
endfunction()

function(generate_pd_source)  
  # # Define the supported set of keywords
  set(noValues "")
  set(singleValues FILENAME EXTERNALS_OUT FILE_OUT)
  set(multiValues CLIENTS HEADERS CLASSES)
  # # Process the arguments passed in
  include(CMakeParseArguments)
  cmake_parse_arguments(ARG
  "${noValues}"
  "${singleValues}"
  "${multiValues}"
  ${ARGN})  
  
  if(ARG_FILENAME)
    set(external_name ${ARG_FILENAME})
  else()
    list(GET ARG_CLIENTS 0 client_name)
    list(GET ARG_HEADERS 0 header)
    make_external_name(${client_name} ${header} external_name)
  endif()
  
  string(REPLACE "~" "_tilde" safe_name ${external_name})
  string(REPLACE "." "0x2e" munge_name ${safe_name})
  
  #for reasons unclear to me, PD expects the string 'setup' to be glued to *different ends* of the external name depending on whether or not some characters have been replaced by hex strings (see s_loader.c)
  if(${safe_name} STREQUAL ${munge_name}) #no replacement
    set(ENTRY_POINT "extern \"C\" void ${munge_name}_setup(void)")
  else()    
    set(ENTRY_POINT "extern \"C\" void setup_${munge_name}(void)")
  endif()
  
  set(WRAPPER_TEMPLATE [=[makePDWrapper<${class}>("${external}");]=])
  set(CCE_WRAPPER "#include \"FluidPDWrapper.hpp\"")
  generate_source(${ARGN} EXTERNALS_OUT external FILE_OUT outfile)
  

  set(INSTALLFLAG "")
  if(ARG_CLIENTS AND NOT ARG_FILENAME)
    list(GET ARG_CLIENTS 0 client)    
    get_property(no_install GLOBAL PROPERTY FLUID_CORE_CLIENTS_${client}_INSTALL)
    if(no_install) 
      set(INSTALLFLAG NOINSTALL)
    endif()
  endif()

  message(STATUS "Generating: ${external_name}")
  add_pd_external(${external_name} ${outfile} ${INSTALLFLAG})
endfunction()
