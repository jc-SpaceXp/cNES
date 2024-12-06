cmake_minimum_required(VERSION 3.21)
project(cNES LANGUAGES C)
enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(LIBCHECK REQUIRED check)

set(CMAKE_C_STANDARD 99)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_SIZE_UTIL size)


set(SRCTARGET cnes)
set(TSTTARGET test_all)


set(SRCDIR ${CMAKE_SOURCE_DIR}/src)
set(COREDIR ${SRCDIR}/core)
set(UTILDIR ${SRCDIR}/util)
set(INCDIR ${CMAKE_SOURCE_DIR}/include)
set(INCCOREDIR ${INCDIR}/core)
set(INCUTILDIR ${INCDIR}/util)
set(LIBDIR lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBDIR})

set(COMMON_CFLAGS "-Wall;-Wextra")


set(UTILSRCS "${UTILDIR}/bits_and_bytes.c")
set(CORESRCS "${COREDIR}/cart.c")
string(APPEND CORESRCS ";${COREDIR}/cpu.c")
string(APPEND CORESRCS ";${COREDIR}/emu.c")
string(APPEND CORESRCS ";${COREDIR}/gui.c")
string(APPEND CORESRCS ";${COREDIR}/mappers.c")
string(APPEND CORESRCS ";${COREDIR}/ppu.c")
string(APPEND CORESRCS ";${COREDIR}/cpu_ppu_interface.c")
string(APPEND CORESRCS ";${COREDIR}/cpu_mapper_interface.c")

set(TSTDIR ${CMAKE_SOURCE_DIR}/tests)
set(TSTSRCS "${TSTDIR}/test_all.c")
string(APPEND TSTSRCS ";${TSTDIR}/cpu_tests.c")
string(APPEND TSTSRCS ";${TSTDIR}/mappers_tests.c")
string(APPEND TSTSRCS ";${TSTDIR}/ppu_tests.c")
string(APPEND TSTSRCS ";${TSTDIR}/cpu_ppu_interface_tests.c")
string(APPEND TSTSRCS ";${TSTDIR}/util_tests.c")
if (DEFINED ASAN)
	unset(ASAN)
	set(ASAN_FLAGS -fsanitize=address,undefined)
endif()

# Sanity checks
if (${DEBUG_CMAKE})
	message("DEBUG_VARS (DIRS):\n"
		"SRCDIR ${SRCDIR}\n"
		"COREDIR ${COREDIR}\n"
		"UTILDIR ${COREDIR}\n"
		"TSTDIR ${TSTDIR}\n"
		"INCDIR ${INCDIR}\n"
		"INCCOREDIR ${INCCOREDIR}\n"
		"INCUTILDIR ${INCUTILDIR}\n"
		"ARDIR ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}\n"
		"LIBDIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\n"
		"SLD2_INC_DIR ${SDL2_INCLUDE_DIRS}\n"
	)

	message("DEBUG_VARS (COMPILER FLAGS):\n"
		"COMMON_CFLAGS ${COMMON_CFLAGS}\n"
		"SDL2_CFLAGS ${SDL2_CFLAGS_OTHER}\n"
		"SDL2_LDFLAGS -l${SDL2_LIBRARIES}\n"
		"LIBCHECK_CFLAGS ${LIBCHECK_CFLAGS_OTHER}\n"
		"LIBCHECK_LDFLAGS -l${LIBCHECK_LIBRARIES}\n"
	)

	message("DEBUG_VAR (FILES): \n"
		"CORESRCS ${CORESRCS}\n"
		"UTILSRCS ${UTILSRCS}\n"
		"TSTSRCS ${TSTSRCS}\n"
	)
endif()


# cNES
add_library(common INTERFACE)
target_compile_options(common INTERFACE
	${COMMON_CFLAGS}
	"$<$<CONFIG:DEBUG>:-g>"
)
target_compile_definitions(common INTERFACE "$<$<CONFIG:DEBUG>:__DEBUG__>")


add_library(utilsrcs ${UTILSRCS})
target_include_directories(utilsrcs PRIVATE
	${INCUTILDIR}
)

add_library(coresrcs ${CORESRCS})
target_include_directories(coresrcs PRIVATE
	${INCCOREDIR}
	${INCUTILDIR}
)
target_compile_options(coresrcs PRIVATE ${SDL2_CFLAGS_OTHER})


add_executable(${SRCTARGET} ${CORESRCS})
target_include_directories(${SRCTARGET} PRIVATE
	${INCDIR}
	${INCUTILDIR}
	${INCCOREDIR}
	${SDL2_INCLUDE_DIRS}
)

target_link_libraries(utilsrcs PRIVATE common)
target_link_libraries(coresrcs PRIVATE common ${SDL2_LIBRARIES})
target_link_libraries(${SRCTARGET} PRIVATE common utilsrcs coresrcs)

add_custom_command(TARGET ${SRCTARGET} POST_BUILD
	COMMAND ${CMAKE_SIZE_UTIL} "$<TARGET_FILE_NAME:${SRCTARGET}>"
	COMMENT "Executable size"
)


# Unit test section
add_executable(${TSTTARGET} ${TSTSRCS})
add_test(NAME ${TSTTARGET} COMMAND ${TSTTARGET})

target_compile_options(${TSTTARGET} PRIVATE
	${COMMON_CFLAGS}
	"$<$<CONFIG:DEBUG>:-g>"
	${LIBCHECK_CFLAGS_OTHER}
	${ASAN_FLAGS}
)

target_link_options(${TSTTARGET} PRIVATE ${ASAN_FLAGS})

target_include_directories(${TSTTARGET} PRIVATE
	${INCCOREDIR}
	${INCUTILDIR}
)

target_link_libraries(${TSTTARGET} PRIVATE ${LIBCHECK_LIBRARIES} coresrcs utilsrcs)

add_custom_command(TARGET ${TSTTARGET} POST_BUILD
	COMMAND ${CMAKE_SIZE_UTIL} "$<TARGET_FILE_NAME:${TSTTARGET}>"
	COMMENT "Executable size (unit tests)"
)

add_custom_command(TARGET ${TSTTARGET} POST_BUILD
	COMMAND ctest --test-dir "${CMAKE_BINARY_DIR}"
	COMMENT "Execute tests"
)

unset(DEBUG_CMAKE CACHE)
