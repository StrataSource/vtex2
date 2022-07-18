cmake_minimum_required(VERSION 3.15)

project(vtex2 CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Project settings
option(BUILD_GUI "Build the VTFViewer GUI" ON)

# Global flags, mainly for UNIX. Use $ORIGIN rpath & -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Build vtflib as static lib
set(VTFLIB_STATIC ON)
add_subdirectory(external/vtflib)
add_subdirectory(external/fmtlib)

##############################
# Common code
##############################
set(COMMON_SRC
	src/common/image.cpp
	src/common/enums.cpp)

add_library(libcom STATIC ${COMMON_SRC})

##############################
# CLI
##############################

# Sources
set(CLI_SRC
		src/cli/main.cpp
		src/cli/action_extract.cpp
		src/cli/action_info.cpp)

add_executable(vtex2 ${CLI_SRC})

##############################
# GUI
##############################

if (BUILD_GUI)
	include(cmake_scripts/Qt.cmake)

	set(VIEWER_SRC
			src/gui/main.cpp
			src/gui/viewer.cpp)
			
	add_executable(vtfview ${VIEWER_SRC})
endif()

# Set up the debugger so it can run the program without copying a million dlls
if(WIN32)
	set_target_properties(vtfview PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${QT_BASEDIR}/bin;")
endif()

target_link_libraries(vtex2 PRIVATE vtflib libcom fmt::fmt)
target_include_directories(vtex2 PRIVATE src external)
target_include_directories(libcom PRIVATE src external external/vtflib/lib)

if (BUILD_GUI)
	target_link_libraries(vtfview PRIVATE vtflib libcom fmt::fmt)
	target_include_directories(vtfview PRIVATE src external)

	find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)
	target_link_libraries(vtfview PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)
	target_include_directories(vtfview PRIVATE ${QT_INCLUDE} ${QT_INCLUDE}/QtWidgets ${QT_INCLUDE}/QtGui ${QT_INCLUDE}/QtCore)
endif()
