cmake_minimum_required(VERSION 3.15)
project(gres C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable(gres main.c gres.c)
target_link_libraries(gres PRIVATE PkgConfig::GTK4)
