add_executable(cli-options main.cpp)
target_include_directories(cli-options PRIVATE ${LIBRARY_INCLUDE_DIRECTORY})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    target_compile_options(cli-options PRIVATE /W3 /WX)
else() 
    target_compile_options(cli-options PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)