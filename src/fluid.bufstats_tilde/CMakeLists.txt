cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/pd-pretarget.cmake)

add_library(
	${PROJECT_NAME}
	SHARED
	${PROJECT_NAME}.cpp
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/pd-posttarget.cmake)
