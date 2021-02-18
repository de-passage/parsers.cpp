function(set_target_options TARGET_NAME) 

target_include_directories(${TARGET_NAME} PUBLIC ${LIBRARY_INCLUDE_DIRECTORY})

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  target_compile_options(${TARGET_NAME} PRIVATE /W3 /WX)
else ()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic -Wconversion)
endif(MSVC)

endfunction()

add_subdirectory(math)
add_subdirectory(cli_options)