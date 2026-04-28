option(ENABLE_TEST "test version" OFF)
option(ENABLE_CPACK "Enable CPack" OFF)
option(ELOG_BUFFER_ENABLE "Enable log to buffer" ON)
option(ELOG_FILE_ENABLE "Enable log to file" ON)

set(CMAKE_BUILD_TYPE ${BUILD_TYPE})
set(CMAKE_CXX_FLAGS_DEBUG
    "$ENV{CXXFLAGS} -rpath  -O0 -Wall -g3 -ggdb -rdynamic")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -rpath -O2  -Wall")
message(STATUS "build_type: " ${CMAKE_BUILD_TYPE})

if("${PLAT}" STREQUAL "0")
  message(STATUS "use desk toolchain")
  set(CMAKE_TOOLCHAIN_FILE ${ROOT_DIR}/scripts/cmake/desk.cmake)
  set(LIBRARY_DIR ${ROOT_DIR}/lib/desk)
else()
  message(STATUS "use T113 toolchain")
  set(CMAKE_TOOLCHAIN_FILE ${ROOT_DIR}/scripts/cmake/T113.cmake)
  set(LIBRARY_DIR ${ROOT_DIR}/lib/T113)
endif()


message(STATUS "BINARY_DIR: " ${BINARY_DIR})
message(STATUS "LIBRARY_DIR: " ${LIBRARY_DIR})
message(STATUS "CMAKE_TOOLCHAIN_FILE:" ${CMAKE_TOOLCHAIN_FILE})
include(${CMAKE_TOOLCHAIN_FILE})
