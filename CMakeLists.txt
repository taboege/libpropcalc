cmake_minimum_required(VERSION 3.9)

##### libpropcalc.so ###########################################################

project(propcalc
	VERSION 0.0.1
	DESCRIPTION "All-in-one, FFIable propositional calculus package"
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(
	-Wall -Wextra -Werror
)

add_library(propcalc SHARED
	core/variable.cpp
	core/assignment.cpp
	core/ast.cpp
	core/formula.cpp
	core/tseitin.cpp
	core/cnf.cpp
	core/dimacs.cpp
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

install(TARGETS propcalc
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/propcalc/
)

install(FILES ${CMAKE_BINARY_DIR}/libpropcalc.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

##### tests ####################################################################

# We do not use enable_testing() and ctest. Our test driver is called prove.
# This will also allow us to have `test` and `test-memcheck` targets, which
# run the same tests, except the latter does it through valgrind memcheck.
#
# Since memcheck is slower and valgrind may not be available, we usually
# want to run `test`, but sometimes also `test-memcheck`, but never both.
# This is easier to accomplish without ctest it seems.

file(GLOB files "t/*.t.cpp")
foreach(file ${files})
	get_filename_component(testname ${file} NAME_WLE)

	add_executable(${testname} EXCLUDE_FROM_ALL ${file})
	add_dependencies(${testname} propcalc)
	# We may not use all functions imported from the tappp.hpp header
	target_compile_options(${testname} PRIVATE -Wno-unused-function)
	target_include_directories(${testname}
		PRIVATE include t
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
			$<INSTALL_INTERFACE:include>
	)
	target_link_libraries(${testname} PRIVATE propcalc)

	list(APPEND testnames ${testname})
	list(APPEND tests $<TARGET_FILE:${testname}>)
endforeach()

add_custom_target(test
	COMMAND prove -e '' ${tests}
	DEPENDS propcalc ${testnames}
)

##### valgrind tests ###########################################################

find_program(VALGRIND NAMES valgrind)
if(VALGRIND)
	add_custom_target(test-memcheck
		COMMAND prove -e '${VALGRIND} --quiet --error-exitcode=111 --exit-on-first-error=yes --leak-resolution=low --leak-check=full --errors-for-leak-kinds=all' ${tests}
		DEPENDS propcalc ${testnames}
	)
endif()

##### cpptest (soon to be removed) #############################################

add_executable(cpptest cpptest.cpp)
add_dependencies(cpptest propcalc)

target_include_directories(cpptest
	PRIVATE include
    		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    		$<INSTALL_INTERFACE:include>
)

target_link_libraries(cpptest PRIVATE propcalc)
