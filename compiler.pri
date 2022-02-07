
# c++20 support
#
win32 {
    CONFIG -= c++17
    CONFIG += c++latest
}
unix {
    CONFIG -= c++17
    #CONFIG += c++20
	QMAKE_CXXFLAGS += -std=c++20
}

# Optimization flags
#
win32 {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

