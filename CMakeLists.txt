cmake_minimum_required(VERSION 3.9)
project(propcalc
	VERSION 0.0.1
	DESCRIPTION "All-in-one, FFIable propositional calculus package"
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(
	-Wall -Wextra -Werror
)

add_library(propcalc SHARED
	core/variable.cpp
	core/assignment.cpp
	core/ast.cpp
	core/formula.cpp
)

configure_file(libpropcalc.pc.in libpropcalc.pc @ONLY)
configure_file(include/config.hpp.in include/propcalc/config.hpp @ONLY)

file(GLOB PROPCALC_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/propcalc/*.hpp"
	"${CMAKE_CURRENT_BINARY_DIR}/include/propcalc/config.hpp"
)

target_include_directories(propcalc
	PUBLIC include
    		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    		$<INSTALL_INTERFACE:include>
)

set_target_properties(propcalc PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "${PROPCALC_HEADERS}"
)

add_executable(cpptest cpptest.cpp)
add_dependencies(cpptest propcalc)

target_include_directories(cpptest
	PRIVATE include
    		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    		$<INSTALL_INTERFACE:include>
)

target_link_libraries(cpptest PRIVATE propcalc)

install(TARGETS propcalc
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/propcalc/
)

install(FILES ${CMAKE_BINARY_DIR}/libpropcalc.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)
