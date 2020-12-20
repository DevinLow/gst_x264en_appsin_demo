cmake_minimum_required(VERSION 3.14)
project(optflow)

set(CMAKE_CXX_STANDARD 14)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

find_package(PkgConfig)
pkg_search_module(GST REQUIRED gstreamer-1.0 gstreamer-base-1.0 gstapp-1.0)

include_directories("${GST_INCLUDE_DIRS}")
link_libraries(${GST_LIBRARIES})

set(ENV{PKG_CONFIG_PATH})

add_executable(optflow  main.cpp)
target_link_libraries(optflow ${GST_LIBRARIES} libgstapp-1.0.so)