# CMakeLists.txt for qwt library
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project(qwt6 LANGUAGES CXX)

# Qt5/Qt6 compatible include directories
if(VICUS_QT_VERSION EQUAL 6)
	include_directories(
		${PROJECT_SOURCE_DIR}/src		# needed so that ui-generated header files find our own headers
		${Qt6Core_INCLUDE_DIRS}
		${Qt6Gui_INCLUDE_DIRS}
		${Qt6Widgets_INCLUDE_DIRS}
		${Qt6PrintSupport_INCLUDE_DIRS}
		${Qt6Svg_INCLUDE_DIRS}
		${Qt6Concurrent_INCLUDE_DIRS}
	)
else()
	include_directories(
		${PROJECT_SOURCE_DIR}/src		# needed so that ui-generated header files find our own headers
		${Qt5Core_INCLUDE_DIRS}
		${Qt5Gui_INCLUDE_DIRS}
		${Qt5Widgets_INCLUDE_DIRS}
		${Qt5PrintSupport_INCLUDE_DIRS}
		${Qt5Svg_INCLUDE_DIRS}
		${Qt5Concurrent_INCLUDE_DIRS}
	)
endif()

# Collect source files
file(GLOB LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Collect header files
file(GLOB LIB_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# Don't build OpenGL plot canvas for now
list(REMOVE_ITEM LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/qwt_plot_glcanvas.cpp)
list(REMOVE_ITEM LIB_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/src/qwt_plot_glcanvas.h)
list(REMOVE_ITEM LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/qwt_plot_opengl_canvas.cpp)
list(REMOVE_ITEM LIB_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/src/qwt_plot_opengl_canvas.h)

# run rcc, uic and moc compilers automatically as needed
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

# add build target for library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
	${LIB_HDRS}
)


