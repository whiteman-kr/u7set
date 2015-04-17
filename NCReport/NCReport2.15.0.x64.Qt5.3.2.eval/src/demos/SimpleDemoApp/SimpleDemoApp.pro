QT = core gui sql
TEMPLATE = app
TARGET = SimpleDemoApp
NCREPORT_LIBPATH = $$OUT_PWD/../../../lib
include($$PWD/../demos.pri)
include($$PWD/../../ncreport_library.pri)

HEADERS += $$PWD/ncreportdemo.h
SOURCES += $$PWD/main.cpp $$PWD/ncreportdemo.cpp
