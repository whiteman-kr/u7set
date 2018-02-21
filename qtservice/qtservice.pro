TEMPLATE=subdirs
CONFIG += ordered
include(common.pri)
qtservice-uselib:SUBDIRS=buildlib

win32:QMAKE_CXXFLAGS += /std:c++17
