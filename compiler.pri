
# c++20 support
#
win32 {
    CONFIG += c++20
}
unix {
    CONFIG -= c++17
	QMAKE_CXXFLAGS += -std=c++20	# now using 'CONFIG += c++20' leads to -std=gnu++2a
}

# Optimization flags
#
CONFIG(release, debug|release) {
    CONFIG += optimize_speed
	CONFIG += ltcg					# LTO
}
