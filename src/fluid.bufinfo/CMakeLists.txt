# Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
# Copyright University of Huddersfield.
# Licensed under the BSD-3 License.
# See license.md file in the project root for full license information.
# This project has received funding from the European Research Council (ERC)
# under the European Union’s Horizon 2020 research and innovation programme
# (grant agreement No 725899).

cmake_minimum_required(VERSION 3.11)




include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/pd-pretarget.cmake)

add_library(
	${PROJECT_NAME}
	SHARED
	${PROJECT_NAME}.cpp
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/pd-posttarget.cmake)
