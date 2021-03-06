cmake_minimum_required(VERSION 3.12.0)
project(parsers VERSION 0.1.0)

include(CTest)
enable_testing()

set(INCLUDE_DIRECTORY category_theory)
set(HEADER_FILES )

option(BUILD_INDIVIDUAL_TEST_FILES "Provide targets for individual test files" OFF)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CPack)

###############
# Google Test #
###############

# Download and unpack googletest at configure time
configure_file(gtest.CMakeLists.txt googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
include_directories("${gtest_SOURCE_DIR}/include")

set (TEST_SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/)
set (EXAMPLE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/)
set (LIBRARY_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set (TEST_SRC_FILES 
  build.cpp
  bind.cpp
  discard.cpp
  ascii.cpp
  map.cpp
  parsers.cpp 
  matcher.cpp 
  dsl.cpp 
  object_parser.cpp 
  range_parser.cpp 
  sequence.cpp 
  alternative.cpp
  recursive_dsl.cpp
  additional_descriptions.cpp
  recursive_map.cpp)

if (${BUILD_INDIVIDUAL_TEST_FILES})
foreach(FILE IN LISTS TEST_SRC_FILES)

add_executable(${FILE} ${TEST_SRC_DIRECTORY}${FILE})
target_link_libraries(${FILE} gtest_main)
target_include_directories(${FILE} PUBLIC ${LIBRARY_INCLUDE_DIRECTORY})
  
endforeach(FILE)
endif()


list (TRANSFORM TEST_SRC_FILES PREPEND ${TEST_SRC_DIRECTORY})
add_executable(tests ${TEST_SRC_FILES})
target_link_libraries(tests gtest_main)
target_include_directories(tests PUBLIC ${LIBRARY_INCLUDE_DIRECTORY})
add_test(NAME gtests COMMAND tests)

add_subdirectory(${EXAMPLE_DIRECTORY})

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  target_compile_options(tests PRIVATE /W3 /WX)
endif(MSVC)