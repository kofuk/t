cmake_minimum_required(VERSION 3.15)
project(c C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Cairo REQUIRED IMPORTED_TARGET cairo)

add_executable(c main.c)
target_link_libraries(c PRIVATE PkgConfig::Cairo)
