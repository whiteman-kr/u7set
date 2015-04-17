QT = core gui sql
TEMPLATE = app
TARGET = CustomDatasourceDemo
NCREPORT_LIBPATH = $$OUT_PWD/../../../lib
include($$PWD/../demos.pri)
include($$PWD/../../ncreport_library.pri)

HEADERS += \
$$PWD/../../runner/ncrtestform.h \
$$PWD/../../runner/testdatasource.h

SOURCES += \ 
$$PWD/main.cpp \
$$PWD/../../runner/ncrtestform.cpp \
$$PWD/../../runner/testdatasource.cpp

FORMS += \
$$PWD/../../runner/ncrtestform.ui

