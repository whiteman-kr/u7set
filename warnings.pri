#win32:QMAKE_CXXFLAGS += /analyze

gcc:CONFIG += warn_off

win32:CONFIG -= warn_on				# warn_on is level 3 warnings
#win32:CONFIG += warn_on				# The compiler should output as many warnings as possible. If warn_off is also specified, the last one takes effect.
win32:QMAKE_CXXFLAGS += /W4			# CONFIG += warn_on is just W3 level, so set level 4
win32:QMAKE_CXXFLAGS += /wd4201		# Disable warning: C4201: nonstandard extension used: nameless struct/union
win32:QMAKE_CXXFLAGS += /wd4458		# Disable warning: C4458: declaration of 'selectionPen' hides class member
win32:QMAKE_CXXFLAGS += /wd4275		# Disable warning: C4275: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
win32:QMAKE_CXXFLAGS += /wd4251
win32:QMAKE_CXXFLAGS += /wd5054		# Disable warning: C5054: operator '&': deprecated between enumerations of different types
win32:QMAKE_CXXFLAGS += /wd4828		# Disable warning: C4828: The file contains a character starting at offset 0xvalue that is illegal in the current source character set (codepage number). Sources have russian commenrs

win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

