
# c++20 support
#
CONFIG += c++20

# Optimization flags
#
CONFIG(release, debug|release) {
    CONFIG += optimize_speed
	CONFIG += ltcg					# LTO
}
