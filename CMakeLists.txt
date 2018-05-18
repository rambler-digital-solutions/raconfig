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
find_package(Boost COMPONENTS unit_test_framework)

set(CXX_STD $ENV{CXX_STD})
if (NOT CXX_STD)
    set(CXX_STD "-std=c++11")
endif()

set(CMAKE_CXX_FLAGS "${CXX_STD} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g ${CMAKE_CXX_FLAGS_RELEASE}")

set(HEADER_LIB raconfig)
add_library(${HEADER_LIB} INTERFACE)
target_link_libraries(${HEADER_LIB} INTERFACE ${Boost_PROGRAM_OPTIONS_LIBRARY})
target_include_directories(${HEADER_LIB} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

set(STATIC_LIB raconfig_static)
add_library(${STATIC_LIB} STATIC src/raconfig.cpp)
target_link_libraries(${STATIC_LIB} ${HEADER_LIB})

set(DEMO_TARGET raconfig-demo)
add_executable(${DEMO_TARGET} demo/main.cpp)
target_link_libraries(${DEMO_TARGET} ${STATIC_LIB})
target_compile_definitions(${DEMO_TARGET} PRIVATE RACONFIG_LIB "RACONFIG_VERSION_STRING=\"${DEMO_TARGET} 1.0\"")

if (Boost_UNIT_TEST_FRAMEWORK_FOUND)
    set(TEST_TARGET raconfig-test)
    add_executable(${TEST_TARGET} test/raconfig.cpp)
    target_link_libraries(${TEST_TARGET} ${HEADER_LIB} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    enable_testing()
    add_test(NAME raconfig COMMAND ${TEST_TARGET})
else()
    message(WARNING "Boost.Test not found, unit tests disabled")
endif()

install(TARGETS ${STATIC_LIB} ARCHIVE DESTINATION lib)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/raconfig" DESTINATION include)
