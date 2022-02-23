cmake_minimum_required(VERSION 3.1)

project(spice VERSION 0.1 DESCRIPTION "Spice SNN Simulator")

add_executable(spice main.cpp)
set_property(TARGET spice PROPERTY CXX_STANDARD 20)
target_compile_options(spice PRIVATE
-O2
-march=native
-Werror
-Wall
-Wextra
-Wshadow
-Wnon-virtual-dtor
-pedantic
-Wpedantic
-Wold-style-cast
-Wcast-align
-Wunused
-Woverloaded-virtual
-Wconversion
-Wsign-conversion
-Wnull-dereference
-Wuseless-cast)