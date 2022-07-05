cmake_minimum_required(VERSION 3.15)

project(vtex2 CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project settings

# Set to enable Qt for this project
set(USING_QT OFF)

# Global flags, mainly for UNIX. Use $ORIGIN rpath & -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Sources
set(CLI_SRC
		src/cli/main.cpp)

add_executable(vtex2 ${CLI_SRC})

# Qt support
if (USING_QT)
	include(cmake_scripts/Qt.cmake)
	find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)
	target_link_libraries(${PROJECTNAME} PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)
	target_include_directories(${PROJECTNAME} PRIVATE ${QT_INCLUDE} ${QT_INCLUDE}/QtWidgets ${QT_INCLUDE}/QtGui ${QT_INCLUDE}/QtCore)
endif()


# Set up the debugger so it can run the program without copying a million dlls
if(WIN32)
	set_target_properties(vtex2 PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${QT_BASEDIR}/bin;")
endif()

# Build vtflib as static lib
set(VTFLIB_STATIC ON)
add_subdirectory(external/vtflib)

target_link_libraries(vtex2 vtflib)
