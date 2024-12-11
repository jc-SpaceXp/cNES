cmake_minimum_required(VERSION 3.24)
project(cNES LANGUAGES C)
enable_testing()
include(CTest)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/deps)

set(sdl2_version 2.30.9)
set(sdl2_release_link "https://github.com/libsdl-org/SDL/releases/download/")
string(APPEND sdl2_release_link "release-${sdl2_version}/SDL2-${sdl2_version}.tar.gz")

FetchContent_Declare(
	SDL2
	URL ${sdl2_release_link}
	URL_HASH MD5=57393a5e1a46dd19ae40968a301c70a3
	DOWNLOAD_EXTRACT_TIMESTAMP True
	FIND_PACKAGE_ARGS
)

FetchContent_Declare(
	LIBCHECK
	GIT_REPOSITORY https://github.com/libcheck/check.git
	# Latest master commit
	GIT_TAG 455005dc29dc6727de7ee36fee4b49a13b39f73f
	FIND_PACKAGE_ARGS NAMES check
)

FetchContent_MakeAvailable(SDL2 LIBCHECK)


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
if (DEFINED ASAN)
	unset(ASAN)
	set(ASAN_FLAGS -fsanitize=address,undefined)
endif()

if (DEFINED COVERAGE)
	set(COVERAGE_FLAGS "-coverage;-O0")
endif()


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
	${ASAN_FLAGS}
	${COVERAGE_FLAGS}
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
	"$<$<BOOL:${sdl2_BINARY_DIR}>:${sdl2_BINARY_DIR}/include>"
)
target_compile_options(coresrcs PRIVATE)


add_executable(${SRCTARGET} ${CORESRCS})
target_include_directories(${SRCTARGET} PRIVATE
	${INCDIR}
	${INCUTILDIR}
	${INCCOREDIR}
	"$<$<BOOL:${sdl2_BINARY_DIR}>:${sdl2_BINARY_DIR}/include>"
	"$<$<BOOL:${sdl2_SOURCE_DIR}>:${sdl2_SOURCE_DIR}/include>"
)

target_link_libraries(utilsrcs PRIVATE common ${COVERAGE_FLAGS})
target_link_libraries(coresrcs PRIVATE common SDL2::SDL2 ${COVERAGE_FLAGS})
target_link_libraries(${SRCTARGET} PRIVATE common utilsrcs coresrcs)
target_link_options(${SRCTARGET} PRIVATE ${ASAN_FLAGS})

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
	${ASAN_FLAGS}
	"$<$<BOOL:${TSTSRCCOVERAGE}>:${COVERAGE_FLAGS}>"
)

target_link_options(${TSTTARGET} PRIVATE
	${ASAN_FLAGS}
	"$<$<BOOL:${TSTSRCCOVERAGE}>:${COVERAGE_FLAGS}>"
)

target_include_directories(${TSTTARGET} PRIVATE
	${INCCOREDIR}
	${INCUTILDIR}
	${libcheck_BINARY_DIR}
)

target_link_libraries(${TSTTARGET} PRIVATE check coresrcs utilsrcs)

add_custom_command(TARGET ${TSTTARGET} POST_BUILD
	COMMAND ${CMAKE_SIZE_UTIL} "$<TARGET_FILE_NAME:${TSTTARGET}>"
	COMMENT "Executable size (unit tests)"
)

# if custom_command() fails test executable is deleted using custom_target() instead
add_custom_target(runtests
	DEPENDS ${TSTTARGET}
	COMMAND ctest --test-dir "${CMAKE_BINARY_DIR}" --output-on-failure
	COMMENT "Execute tests"
)

if (DEFINED COVERAGE)
	unset(COVERAGE)

	add_custom_target(runcoveragedata
		DEPENDS ${TSTTARGET}
		COMMAND ctest -T Coverage --test-dir "${CMAKE_BINARY_DIR}"
		COMMENT "Coverage of tests"
	)
endif()

unset(DEBUG_CMAKE CACHE)
