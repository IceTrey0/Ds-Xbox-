cmake_minimum_required(VERSION 3.20)
project(melonxbox C CXX)
set(CMAKE_CXX_STANDARD 17)

option(MXB_WITH_MELONDS "Build melonDS glue (needs extern/melonDS)" OFF)

add_library(mxb_core STATIC src/SaveCompat.cpp src/SaveStates.cpp)
target_include_directories(mxb_core PUBLIC src)

enable_testing()
add_executable(test_savecompat tests/test_savecompat.cpp)
target_link_libraries(test_savecompat mxb_core)
add_test(NAME savecompat COMMAND test_savecompat)

if(MXB_WITH_MELONDS)
  set(BUILD_QT_SDL OFF CACHE BOOL "" FORCE)
  set(ENABLE_OGLRENDERER OFF CACHE BOOL "" FORCE)
  set(ENABLE_GDBSTUB OFF CACHE BOOL "" FORCE)
  set(ENABLE_JIT OFF CACHE BOOL "" FORCE)
  set(ENABLE_LTO OFF CACHE BOOL "" FORCE)

  # Set as global flags BEFORE add_subdirectory so melonDS inherits them.
  set(MXB_DEFS "/D_USE_MATH_DEFINES /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /Dstrncasecmp=_strnicmp /Dstrcasecmp=_stricmp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MXB_DEFS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MXB_DEFS} /DLITTLE_ENDIAN=1234 /DBIG_ENDIAN=4321 /DBYTE_ORDER=1234")

  include_directories(${CMAKE_SOURCE_DIR}/shim)

  add_subdirectory(extern/melonDS EXCLUDE_FROM_ALL)

  add_library(mxb_melon STATIC src/glue/MelonGlue.cpp)
  target_include_directories(mxb_melon PUBLIC extern/melonDS/src)
  target_link_libraries(mxb_melon PUBLIC mxb_core core)
endif()
