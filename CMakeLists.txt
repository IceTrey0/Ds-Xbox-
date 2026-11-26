cmake_minimum_required(VERSION 3.20)
project(melonxbox CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(mxb_core STATIC src/SaveCompat.cpp src/SaveStates.cpp)
target_include_directories(mxb_core PUBLIC src)

enable_testing()
add_executable(test_savecompat tests/test_savecompat.cpp)
target_link_libraries(test_savecompat mxb_core)
add_test(NAME savecompat COMMAND test_savecompat)

# Next: add_subdirectory(extern/melonDS) and the UWP frontend target.
