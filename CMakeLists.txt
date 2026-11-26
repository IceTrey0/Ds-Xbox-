cmake_minimum_required(VERSION 3.20)
project(melonxbox CXX)
set(CMAKE_CXX_STANDARD 17)

option(MXB_WITH_MELONDS "Build melonDS glue (needs extern/melonDS)" OFF)

add_library(mxb_core STATIC src/SaveCompat.cpp src/SaveStates.cpp)
target_include_directories(mxb_core PUBLIC src)

enable_testing()
add_executable(test_savecompat tests/test_savecompat.cpp)
target_link_libraries(test_savecompat mxb_core)
add_test(NAME savecompat COMMAND test_savecompat)

if(MXB_WITH_MELONDS)
  set(BUILD_QT_SDL OFF CACHE BOOL "" FORCE)   # VERIFY option name
  add_subdirectory(extern/melonDS EXCLUDE_FROM_ALL)
  add_library(mxb_melon STATIC src/glue/MelonGlue.cpp)
  target_include_directories(mxb_melon PUBLIC extern/melonDS/src)
  target_link_libraries(mxb_melon PUBLIC mxb_core core)  # VERIFY target "core"
endif()
