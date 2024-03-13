#!/bin/bash
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
set -x  # Echo on.
set -e  # Terminate script if any command returns error.

# Run StopServices when exit (any error or success).
#
trap StopServices EXIT  

function StopServices() {
    echo "StopServices()"

    # Stop services for functional tests.
    #
    pkill CfgSrv || true
    pkill AppDataSrv || true
    pkill TuningSrv || true
    pkill SimulatorConsol || true       # without last e, I assume there is a limitation to 15 symbols.
    sleep 1
}

# Stop if any service is running.
#
StopServices || true

# lcov exec arguments and output dir.
# 
OUTPUT_DIR="./CodeCoverage"
LCOV_CLEAR_ARGUMENTS="--no-external --capture --initial"
LCOV_COLLECT_ARGUMENTS="--rc lcov_branch_coverage=1 --capture"

# Create output dir.
#
rm -rvf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

# Init is not required, as we have an empty dir at start.
#
#lcov $LCOV_CLEAR_ARGUMENTS --output-file $OUTPUT_DIR/Simulator.info --directory ./Simulator/debug

# Build project u7_test_simulator.
#
rm -rvf /tmp/build/test_simulator
$CI_PROJECT_DIR/bin/debug/BuilderConsole $CI_PROJECT_DIR/Test/BuilderConsoleArgsCoverage.xml

# Start services for functional tests.
#
pushd $CI_PROJECT_DIR/bin/debug

cp /tmp/build/${SIMULATOR_PROJECT_NAME}/build/RunServiceScripts/Linux/linux_code_coverage_systemid_clienttest*.sh .
chmod +x *.sh

./linux_code_coverage_systemid_clienttest_ws01_cfgs.sh simulation < /dev/null > clienttest_ws01_cfgs.out 2>&1 &
sleep 5
./linux_code_coverage_systemid_clienttest_ws02_cfgs.sh simulation < /dev/null > clienttest_ws02_cfgs.out 2>&1 &
sleep 5
./linux_code_coverage_systemid_clienttest_ws01_ads.sh < /dev/null > clienttest_ws01_ads.out 2>&1 &
./linux_code_coverage_systemid_clienttest_ws02_ads.sh < /dev/null > clienttest_ws02_ads.out 2>&1 &
./linux_code_coverage_systemid_clienttest_ws01_tuns.sh < /dev/null > clienttest_ws01_tuns.out 2>&1 &
./linux_code_coverage_systemid_clienttest_ws02_tuns.sh < /dev/null > clienttest_ws02_tuns.out 2>&1 &
./linux_code_coverage_systemid_clienttest_ws04_tuns.sh < /dev/null > clienttest_ws04_tuns.out 2>&1 &
./SimulatorConsole -build=/tmp/build/${SIMULATOR_PROJECT_NAME}/build -profile=linux_code_coverage -enable_lan -script=$CI_PROJECT_DIR/Test/ClientTests/Scripts/TuningTests.js > SimulatorConsole.out 2>&1 &

sleep 5

# Run ClientTests, they are functional.
#
./ClientTests -build=/tmp/build/${SIMULATOR_PROJECT_NAME}/build -profile=linux_code_coverage

# Stop services after ClientTests (functional tests)
#
StopServices || true

# Run other tests, not services are required here.
#
./SimulatorTests
./MetrologyTests
./u7databasetests -config=$CI_PROJECT_DIR/Test/u7databasetestsArgsCoverage.xml

popd

# Get code coverage data.
#

# AppSignalLib
TEST_DIR="./build/AppSignalLib/CMakeFiles/AppSignalLib.dir"
TEST_OUTPUT_FILE="AppSignalLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# CommonLib
TEST_DIR="./build/CommonLib/CMakeFiles/CommonLib.dir"
TEST_OUTPUT_FILE="CommonLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# OnlineLib
TEST_DIR="./build/OnlineLib/CMakeFiles/OnlineLib.dir"
TEST_OUTPUT_FILE="OnlineLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# DbLib
TEST_DIR="./build/libs/DbLib/CMakeFiles/DbLib.dir"
TEST_OUTPUT_FILE="DbLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# HardwareLib
TEST_DIR="./build/libs/HardwareLib/CMakeFiles/HardwareLib.dir"
TEST_OUTPUT_FILE="HardwareLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Metrology
TEST_DIR="./build/Test/MetrologyTests/CMakeFiles/MetrologyTests.dir"
TEST_OUTPUT_FILE="MetrologyTests.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Builder
TEST_DIR="./build/Builder/CMakeFiles/Builder.dir"
TEST_OUTPUT_FILE="Builder.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Simulator
TEST_DIR="./build/Simulator/CMakeFiles/Simulator.dir"
TEST_OUTPUT_FILE="Simulator.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# TrendView
TEST_DIR="./build/TrendView/CMakeFiles/TrendView.dir"
TEST_OUTPUT_FILE="TrendView.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# UtilsLib
TEST_DIR="./build/UtilsLib/CMakeFiles/UtilsLib.dir"
TEST_OUTPUT_FILE="UtilsLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# ClientLib
TEST_DIR="./build/libs/ClientLib/CMakeFiles/ClientLib.dir"
TEST_OUTPUT_FILE="ClientLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# AppDataSrv -- Cannot collect .gcda as process is killed and not finished normally
#TEST_DIR="./AppDataService"
#TEST_OUTPUT_FILE="AppDataSrv.info"
#lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# CfgSrv -- Cannot collect .gcda as process is killed and not finished normally
#TEST_DIR="./ConfigurationService"
#TEST_OUTPUT_FILE="CfgSrv.info"
#lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR


# Combine results to a single file, result stored to $OUTPUT_DIR/u7set-dirty.info
#
lcov --output-file $OUTPUT_DIR/u7set-dirty.info \
    --add-tracefile $OUTPUT_DIR/AppSignalLib.info \
    --add-tracefile $OUTPUT_DIR/CommonLib.info \
    --add-tracefile $OUTPUT_DIR/OnlineLib.info \
    --add-tracefile $OUTPUT_DIR/DbLib.info \
    --add-tracefile $OUTPUT_DIR/HardwareLib.info \
    --add-tracefile $OUTPUT_DIR/MetrologyTests.info \
    --add-tracefile $OUTPUT_DIR/Builder.info \
    --add-tracefile $OUTPUT_DIR/Simulator.info \
    --add-tracefile $OUTPUT_DIR/UtilsLib.info \
    --add-tracefile $OUTPUT_DIR/ClientLib.info

#    --add-tracefile $OUTPUT_DIR/AppDataSrv.info \
#    --add-tracefile $OUTPUT_DIR/CfgSrv.info

# Filter combined file, result stored to $OUTPUT_DIR/u7set.info.
#
lcov -r $OUTPUT_DIR/u7set-dirty.info "*Qt*.framework*" "/usr/*" "*/Qt/*" "*/Proto*/*" "*.pb.*" "*/Test*/*" "*.moc" "*moc_*.cpp" "*/test/*" --output-file $OUTPUT_DIR/u7set.info

# Generate HTML report.
#
genhtml --legend --rc lcov_branch_coverage=1 --rc genhtml_med_limit=60 --rc genhtml_hi_limit=80 --output-directory $OUTPUT_DIR/Report $OUTPUT_DIR/u7set.info

# Generate cobertura XML (for GitLab CI).
# https://pypi.org/project/lcov-cobertura/
# sudo pip install lcov-cobertura
#
lcov_cobertura $OUTPUT_DIR/u7set.info --output $OUTPUT_DIR/coverage.xml
