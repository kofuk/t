cmake_minimum_required(VERSION 3.15)
project(g CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(gtkmm REQUIRED IMPORTED_TARGET gtkmm-3.0)

add_executable(g main.cc)
target_link_libraries(g PkgConfig::gtkmm)
