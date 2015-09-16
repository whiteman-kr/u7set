#-------------------------------------------------
#
# Project created by QtCreator 2015-06-08T16:19:06
#
#-------------------------------------------------

QT       += network testlib

QT       -= gui

TARGET = tst_File2pgsqlTestTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_File2pgsqlTestTest.cpp \
    ../file2pgsql/Convertor.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

OTHER_FILES += \
    TestFiles/Test0/test3.txt \
    TestFiles/Test1/test2.txt \
    TestFiles/Test1/test2.txt.files/test3.txt \
    TestFiles/Test1/test2.txt.files/test3.txt.files/test4.txt \
    TestFiles/Test2/test.txt \
    TestFiles/Test2/test1.txt \
    TestFiles/Test2/test.txt.files/dfsfdsfsdfdsfsdf.txt \
    TestFiles/Test2/test1.txt.files/dfsfsdfsdf.txt \
    TestFiles/Test2/test2/fsfdsfs \
    TestFiles/Test3/logic.preview.png \
    TestFiles/Test3/dfsdfdsf.txt \
    TestFiles/Test3/sdfsdfdsfs.txt

HEADERS += \
    ../file2pgsql/Convertor.h

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11
