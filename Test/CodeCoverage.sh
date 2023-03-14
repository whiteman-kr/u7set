#!/bin/sh
#
# Basic script to run unit ot functional tests with code coverage support
# and build HTML and XML reports with lcov.
# Run it from build root folder.
#
# Required tools:
#   sudo apt install lcov
#   sudo pip install python3
#   sudo pip install lcov-cobertura
#

set -x  # Echo on
set -e  # Terminate script if any command returns error

# lcov exec arguments and output dir.
# 
OUTPUT_DIR="./CodeCoverage"
LCOV_CLEAR_ARGUMENTS="--no-external --capture --initial"
LCOV_COLLECT_ARGUMENTS="--rc lcov_branch_coverage=1 --capture"

# Create output dir.
#
rm -rvf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

# Init is not required, as we have empty dir at start
#
#lcov $LCOV_CLEAR_ARGUMENTS --output-file $OUTPUT_DIR/Simulator.info --directory ./Simulator/debug

# Run tests
#
./bin_unix/debug/ClientTests
./bin_unix/debug/SimulatorTests
./bin_unix/debug/MetrologyTests
./bin_unix/debug/u7databasetests -config=$CI_PROJECT_DIR/Test/u7databasetestsArgsCoverage.xml

# Get code coverage data
#

# AppSignalLib
TEST_DIR="./AppSignalLib/debug"
TEST_OUTPUT_FILE="AppSignalLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Builder
TEST_DIR="./Builder/debug"
TEST_OUTPUT_FILE="Builder.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# CommonLib
TEST_DIR="./CommonLib/debug"
TEST_OUTPUT_FILE="CommonLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# DbLib
TEST_DIR="./DbLib/debug"
TEST_OUTPUT_FILE="DbLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# HardwareLib
TEST_DIR="./HardwareLib/debug"
TEST_OUTPUT_FILE="HardwareLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Metrology
TEST_DIR="./Test/MetrologyTests"
TEST_OUTPUT_FILE="MetrologyTests.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Simulator
TEST_DIR="./Simulator/debug"
TEST_OUTPUT_FILE="Simulator.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# TrendView
TEST_DIR="./TrendView"
TEST_OUTPUT_FILE="TrendView.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# UtilsLib
TEST_DIR="./UtilsLib/debug"
TEST_OUTPUT_FILE="UtilsLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# ClientLib
TEST_DIR="./ClientLib/debug"
TEST_OUTPUT_FILE="ClientLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Combine results to a single file, result stored to $OUTPUT_DIR/u7set-dirty.info
#
lcov --output-file $OUTPUT_DIR/u7set-dirty.info \
    --add-tracefile $OUTPUT_DIR/AppSignalLib.info \
    --add-tracefile $OUTPUT_DIR/CommonLib.info \
    --add-tracefile $OUTPUT_DIR/DbLib.info \
    --add-tracefile $OUTPUT_DIR/HardwareLib.info \
    --add-tracefile $OUTPUT_DIR/MetrologyTests.info \
    --add-tracefile $OUTPUT_DIR/Simulator.info \
    --add-tracefile $OUTPUT_DIR/UtilsLib.info \
    --add-tracefile $OUTPUT_DIR/ClientLib.info

# There is no test data for these files yet.
#
#--add-tracefile $OUTPUT_DIR/Builder.info             
#--add-tracefile $OUTPUT_DIR/TrendView.info

# Filter combined file, result stored to $OUTPUT_DIR/u7set.info
#
lcov -r $OUTPUT_DIR/u7set-dirty.info "*Qt*.framework*" "/usr/*" "*/Qt/*" "*/Proto*/*" "*.pb.*" "*/Test*/*" "*.moc" "*moc_*.cpp" "*/test/*" --output-file $OUTPUT_DIR/u7set.info

# Generate HTML report.
#
genhtml --legend --rc lcov_branch_coverage=1 --output-directory $OUTPUT_DIR/Report $OUTPUT_DIR/u7set.info

# Generate cobertura XML (for GitLab CI). 
# https://pypi.org/project/lcov-cobertura/
# sudo pip install lcov-cobertura
#
lcov_cobertura $OUTPUT_DIR/u7set.info --output $OUTPUT_DIR/coverage.xml
