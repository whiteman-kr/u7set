TEMPLATE=subdirs
CONFIG += ordered
include(common.pri)
qtservice-uselib:SUBDIRS=buildlib

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

