# CMakeLists.txt file for SciChart library
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( SciChart )

# Qt5/Qt6 compatible include directories
if(VICUS_QT_VERSION EQUAL 6)
	include_directories(
		${PROJECT_SOURCE_DIR}/src		# needed so that ui-generated header files find our own headers
		${PROJECT_SOURCE_DIR}/../QtExt/src
		${PROJECT_SOURCE_DIR}/../EmfEngine/src
		${PROJECT_SOURCE_DIR}/../IBK/src
		${PROJECT_SOURCE_DIR}/../DataIO/src
		${PROJECT_SOURCE_DIR}/../qwt/src
		${PROJECT_SOURCE_DIR}/../TiCPP/src
		${Qt6Core_INCLUDE_DIRS}
		${Qt6Gui_INCLUDE_DIRS}
		${Qt6Widgets_INCLUDE_DIRS}
		${Qt6PrintSupport_INCLUDE_DIRS}
		${Qt6Svg_INCLUDE_DIRS}
	)
else()
	include_directories(
		${PROJECT_SOURCE_DIR}/src		# needed so that ui-generated header files find our own headers
		${PROJECT_SOURCE_DIR}/../QtExt/src
		${PROJECT_SOURCE_DIR}/../EmfEngine/src
		${PROJECT_SOURCE_DIR}/../IBK/src
		${PROJECT_SOURCE_DIR}/../DataIO/src
		${PROJECT_SOURCE_DIR}/../qwt/src
		${PROJECT_SOURCE_DIR}/../TiCPP/src
		${Qt5Core_INCLUDE_DIRS}
		${Qt5Gui_INCLUDE_DIRS}
		${Qt5Widgets_INCLUDE_DIRS}
		${Qt5PrintSupport_INCLUDE_DIRS}
		${Qt5Svg_INCLUDE_DIRS}
	)
endif()

if (DEFINED MSVC AND SCICHART_HAS_EMFENGINE)
	message("Enabling EmfEngine support in SciChart")
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D\"SCICHART_HAS_EMFENGINE\"" )
endif (DEFINED MSVC AND SCICHART_HAS_EMFENGINE)

# collect a list of all source files
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB LIB_HDRS ${PROJECT_SOURCE_DIR}/src/*.h )

# collect a list of all form files
file( GLOB LIB_UIS  ${PROJECT_SOURCE_DIR}/src/*.ui )

# set resource files
set( LIB_QRCS       ${PROJECT_SOURCE_DIR}/resources/gfx/SciChart.qrc )

# run rcc, uic and moc compilers automatically as needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# add build target for library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
	${LIB_HDRS}
	${LIB_UIS}
	${LIB_QRCS}
)


# precompiled headers
target_precompile_headers(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/Sci_headers.h)
