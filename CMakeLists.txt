cmake_minimum_required(VERSION 3.15)

project(vtex2 CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Project settings
option(BUILD_GUI "Build the VTFViewer GUI" ON)
option(BUILD_TESTS "Build test binaries" OFF)

# Global flags, mainly for UNIX. Use $ORIGIN rpath & -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# MT/MTd specification for Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Build vtflib as static lib
add_subdirectory(external/vtflib)
add_subdirectory(external/fmtlib)

add_definitions(-DVTFLIB_STATIC=1)

##############################
# Setup gtest
##############################
if (BUILD_TESTS)
	include(FetchContent)
	FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG v1.15.2
	)
	FetchContent_MakeAvailable(googletest)
	enable_testing()
endif()

##############################
# Common code
##############################
set(COMMON_SRC
		src/common/image.cpp
		src/common/enums.cpp
		src/common/pack.cpp
		src/common/util.cpp
		src/common/vtftools.cpp)

add_library(com STATIC ${COMMON_SRC})

##############################
# CLI
##############################

# Sources
set(CLI_SRC
		src/cli/main.cpp
		src/cli/action_extract.cpp
		src/cli/action_info.cpp
		src/cli/action_convert.cpp
		src/cli/action_pack.cpp)

add_executable(vtex2 ${CLI_SRC})

##############################
# GUI
##############################

if (BUILD_GUI)
	include(cmake_scripts/Qt.cmake)

	set(VIEWER_SRC
			src/gui/main.cpp
			src/gui/viewer.cpp
			src/gui/document.cpp
			res/resource.qrc)

	add_executable(vtfview ${VIEWER_SRC})
endif ()

# Set up the debugger so it can run the program without copying a million dlls
if (WIN32)
	set_target_properties(vtfview PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${QT_BASEDIR}/bin;")
endif ()

target_link_libraries(vtex2 PRIVATE vtflib_static com fmt::fmt)
target_include_directories(vtex2 PRIVATE src external)
target_include_directories(com PRIVATE src external external/vtflib/lib)

if (UNIX)
	# Don't build vtex2 cli as a dynamic executable
	set_target_properties(vtex2 PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

if (BUILD_GUI)
	target_link_libraries(vtfview PRIVATE vtflib_static com fmt::fmt)
	target_include_directories(vtfview PRIVATE src external)

	find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Svg)
	target_link_libraries(vtfview PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::Svg)
	target_include_directories(vtfview PRIVATE ${QT_INCLUDE} ${QT_INCLUDE}/QtWidgets ${QT_INCLUDE}/QtGui ${QT_INCLUDE}/QtCore)
endif ()

include(GNUInstallDirs)
install(TARGETS vtex2)

if (BUILD_GUI)
	install(TARGETS vtfview)

	install(
		FILES ${CMAKE_BINARY_DIR}/vtfview.desktop
		DESTINATION share/applications
	)
	
	install(
		FILES res/icon.png
		DESTINATION share/icons
		RENAME vtfview.png
	)
endif()

##############################
# Tests
##############################

if (BUILD_TESTS)
	include(GoogleTest)

	add_executable(
		vtex2_tests

		src/tests/image_tests.cpp
	)

	target_link_libraries(
		vtex2_tests PRIVATE

		gtest_main
		vtflib_static
		com
	)
	
	target_include_directories(
		vtex2_tests PRIVATE

		src
	)

	gtest_discover_tests(vtex2_tests)
endif()

##############################
# Version header
##############################
find_package(Git)

include_directories("${CMAKE_BINARY_DIR}/src")

execute_process(
	COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*"
	WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
	OUTPUT_VARIABLE VTEX2_VERSION
	RESULT_VARIABLE GITD_RESULT
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (GITD_RESULT)
	set(VTEX2_VERSION "UNKNOWN")
endif()

configure_file(src/common/vtex2_version.h.in src/common/vtex2_version.h)

configure_file(install/vtfview.desktop.in ${CMAKE_BINARY_DIR}/vtfview.desktop)
