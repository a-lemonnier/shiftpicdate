cmake_minimum_required(VERSION 3.0)

project(shiftpicdate)

set(COMPILER gcc)

if (COMPILER STREQUAL "gcc")
  set(CMAKE_CC_COMPILER "gcc")
  set(CMAKE_CXX_COMPILER "g++")
  message("CC: clang")
  message("CXX: clang++")
elseif (COMPILER STREQUAL "clang")
  set(CMAKE_CC_COMPILER "clang")
  set(CMAKE_CXX_COMPILER "clang++")
  message("CC: gcc")
  message("CXX: g++")
elseif (COMPILER STREQUAL "intel") #does not work atm
  set(INTEL_DIR  /opt/intel/sw_dev_tools/bin)
  set(CMAKE_CC_COMPILER ${INTEL_DIR}/icc)
  set(CMAKE_CXX_COMPILER ${INTEL_DIR}/icpc)
  message("CC: icc")
  message("CXX: icpc")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(shiftpicdate main.cpp)

set_property(TARGET shiftpicdate PROPERTY CXX_STANDARD 17)
set_property(TARGET shiftpicdate PROPERTY CXX_STANDARD_REQUIRED ON)


find_package( Boost 1.40 COMPONENTS program_options 
                         REQUIRED timer filesystem unit_test_framework)

find_package(Exiv2 REQUIRED)
                         
target_link_libraries(shiftpicdate LINK_PUBLIC ${Boost_LIBRARIES} -lpthread -lstdc++fs -lexiv2)




install(TARGETS shiftpicdate RUNTIME DESTINATION bin)
