TEMPLATE=subdirs
CONFIG += ordered
include(common.pri)
qtservice-uselib:SUBDIRS=buildlib

# c++20 support
#
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest

