#
# Copyright 2018 Rambler Digital Solutions
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.0)

project(raconfig LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS program_options)

set(HEADER_LIB raconfig)
add_library(${HEADER_LIB} INTERFACE)
target_link_libraries(${HEADER_LIB} INTERFACE ${Boost_PROGRAM_OPTIONS_LIBRARY})
target_include_directories(${HEADER_LIB} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/raconfig" DESTINATION include)
