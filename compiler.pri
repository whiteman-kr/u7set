
# c++20 support
#
win32 {
    CONFIG += c++20
}
unix {
	CONFIG += c++20
    #CONFIG -= c++17	
	#QMAKE_CXXFLAGS += -std=c++20	# now using 'CONFIG += c++20' leads to -std=gnu++2a
}

# Optimization flags
#
win32 {
	CONFIG(release, debug|release) {
		CONFIG += optimize_speed
		CONFIG += ltcg					# LTO
	}
}


unix {
	CONFIG(release, debug|release) {
		CONFIG += optimize_speed
#		CONFIG += ltcg					# LTO is disabled for faster build by CI/CD
	}
}
