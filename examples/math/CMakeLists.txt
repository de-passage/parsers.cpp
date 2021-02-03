add_executable(math_parser ./main.cpp)
target_include_directories(math_parser PUBLIC ${LIBRARY_INCLUDE_DIRECTORY})

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  target_compile_options(math_parser PRIVATE /W3 /WX)
else ()
  target_compile_options(math_parser PRIVATE -Wall -Wextra -pedantic -Wconversion)
endif(MSVC)
