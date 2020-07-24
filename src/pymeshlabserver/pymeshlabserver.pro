# disables the lib prefix
CONFIG += no_plugin_name_prefix
# disable symlinks & versioning
CONFIG += plugin

TEMPLATE = lib
DESTDIR = $$PYMESHLAB_DISTRIB_DIRECTORY

CONFIG += c++11
CONFIG += qt
QT += core opengl xml qml

win32:TARGET_NAME = $$system(python.exe $$PYMESHLAB_BASE_DIRECTORY/install/windows/python_config.py --extension-suffix)
!win32:TARGET_NAME = $$system(python3-config --extension-suffix | cut -f 2 -d '.')
TARGET = pymeshlabserver.$$TARGET_NAME

win32 {
	PYTHON_INCLUDES = $$system(python.exe $$PYMESHLAB_BASE_DIRECTORY/install/windows/python_config.py --includes)
	PYTHON_LIBS = $$system(python.exe $$PYMESHLAB_BASE_DIRECTORY/install/windows/python_config.py --libs)

#	CONFIG += dll
	LIBS += \
		$$PYTHON_LIBS \
		-L$$PYMESHLAB_DISTRIB_DIRECTORY/lib -lmeshlab-common -lopengl32 -lGLU32


	QMAKE_CXXFLAGS += $$PYTHON_INCLUDES #includepath python lib

#	QMAKE_LFLAGS_PLUGIN -= -dynamiclib
#	QMAKE_LFLAGS_PLUGIN += -bundle
	QMAKE_EXTENSION_SHLIB = pyd
} #win32

macx {
	PYTHON_INCLUDES = $$system(python3-config --includes)

	#needs to be a .so also on macos!
	QMAKE_LFLAGS_PLUGIN -= -dynamiclib
	QMAKE_LFLAGS_PLUGIN += -bundle
	QMAKE_EXTENSION_SHLIB = so

	#needed in order to do not link pythonlib
	QMAKE_LFLAGS += -Wl,-undefined,dynamic_lookup

	LIBS += \
		$$PYMESHLAB_DISTRIB_DIRECTORY/lib/libmeshlab-common.dylib

	QMAKE_CXXFLAGS += $$PYTHON_INCLUDES #includepath python lib


	QMAKE_POST_LINK += "\
		install_name_tool -change libmeshlab-common.1.dylib @loader_path/lib/libmeshlab-common.1.dylib $$PYMESHLAB_DISTRIB_DIRECTORY/$${TARGET}.so; \
		install_name_tool -change @rpath/QtOpenGL.framework/Versions/5/QtOpenGL @loader_path/lib/meshlab/Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL $$PYMESHLAB_DISTRIB_DIRECTORY/$${TARGET}.so; \
		install_name_tool -change @rpath/QtWidgets.framework/Versions/5/QtWidgets @loader_path/lib/meshlab/Frameworks/QtWidgets.framework/Versions/5/QtWidgets $$PYMESHLAB_DISTRIB_DIRECTORY/$${TARGET}.so; \
		install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @loader_path/lib/meshlab/Frameworks/QtGui.framework/Versions/5/QtGui $$PYMESHLAB_DISTRIB_DIRECTORY/$${TARGET}.so; \
		install_name_tool -change @rpath/QtXml.framework/Versions/5/QtXml @loader_path/lib/meshlab/Frameworks/QtXml.framework/Versions/5/QtXml $$PYMESHLAB_DISTRIB_DIRECTORY/$${TARGET}.so; \
		install_name_tool -change @rpath/QtQml.framework/Versions/5/QtQml @loader_path/lib/meshlab/Frameworks/QtQml.framework/Versions/5/QtQml $$PYMESHLAB_DISTRIB_DIRECTORY/$${TARGET}.so; \
		install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @loader_path/lib/meshlab/Frameworks/QtCore.framework/Versions/5/QtCore $$PYMESHLAB_DISTRIB_DIRECTORY/$${TARGET}.so; \
		"
} # macx

linux {
	PYTHON_INCLUDES = $$system(python3-config --includes)

	LIBS += \
		-L$$PYMESHLAB_DISTRIB_DIRECTORY/lib -lmeshlab-common -lGLU

	QMAKE_CXXFLAGS += $$PYTHON_INCLUDES #includepath python lib

	QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/lib
} #linux

DEFINES += MESHLAB_SCALAR=float

include($$PYMESHLAB_BASE_DIRECTORY/meshlab/src/find_system_libs.pri)

#Eigen and glew dirs
CONFIG(system_eigen3): EIGENDIR = /usr/include/eigen3
!CONFIG(system_eigen3):EIGENDIR = $$PYMESHLAB_BASE_DIRECTORY/meshlab/vcglib/eigenlib
GLEWDIR = $$PYMESHLAB_BASE_DIRECTORY/meshlab/src/external/glew-2.1.0

INCLUDEPATH += \
	$$PYMESHLAB_BASE_DIRECTORY/pybind11/include/ \ #pybind11
	$$EIGENDIR \                                   #eigen
	$$GLEWDIR/include/ \                           #glew
	$$PYMESHLAB_BASE_DIRECTORY/meshlab/vcglib \    #vcg
	$$PYMESHLAB_BASE_DIRECTORY/meshlab/src/common  #meshlab_common

# defining pymeshlab version
exists($$PYMESHLAB_BASE_DIRECTORY/PYML_VERSION){
	PYMESHLAB_VERSION = $$cat($$PYMESHLAB_BASE_DIRECTORY/PYML_VERSION)
	message(PyMeshLab Version: $$PYMESHLAB_VERSION)
	DEFINES += "PYMESHLAB_VERSION=$$PYMESHLAB_VERSION"
}

#uncomment this only when you need to debug c++ code on the module
#be sure to have a lib compiled with debug symbols:
#use script build_meshlab_debug.sh
#CONFIG += debug_mode

debug_mode {
	DEFINES+=PYMESHLAB_DEBUG
	TEMPLATE = app
	SOURCES += debug_main.cpp
	!win32:PYTHON_LIBS = $$system(python3-config --ldflags) -lpython3.8
	!win32:LIBS += $$PYTHON_LIBS #includepath python lib
}

SOURCES += \
	extendedmeshdocument.cpp \
	main.cpp \
	pyboundingbox.cpp \
	pymesh.cpp \
	pymeshdocument.cpp \
	pymeshlabcommon.cpp \
	pypoint.cpp

HEADERS += \
	extendedmeshdocument.h \
	pyboundingbox.h \
	pymesh.h \
	pymeshdocument.h \
	pymeshlabcommon.h \
	pypoint.h

