cmake_minimum_required(VERSION 3.9)
project(propcalc VERSION 0.0.1 DESCRIPTION "All-in-one, FFIable propositional calculus package")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(propcalc SHARED
	core/variable.cpp
	core/assignment.cpp
	core/ast.cpp
	core/formula.cpp
)

target_include_directories(propcalc PRIVATE include)
set_target_properties(propcalc PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/propcalc.hpp)

#configure_file(libpropcalc.pc.in libpropcalc.pc @ONLY)

add_executable(cpptest cpptest.cpp)
add_dependencies(cpptest propcalc)
target_include_directories(cpptest PRIVATE include)
target_link_libraries(cpptest PRIVATE propcalc)
