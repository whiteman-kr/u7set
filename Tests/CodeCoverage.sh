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
    pkill -SIGINT CfgSrv || true
    pkill -SIGINT AppDataSrv || true
    pkill -SIGINT TuningSrv || true
    pkill -SIGINT SimulatorConsol || true  # without last e, I assume there is a limitation to 15 symbols.
    pkill -SIGINT GatewaySrv || true
    sleep 6
}

# Stop if any service is running.
#
StopServices || true

# lcov exec arguments and output dir.
# 
OUTPUT_DIR="./CodeCoverage"
LCOV_CLEAR_ARGUMENTS="--no-external --capture --initial"
LCOV_COLLECT_ARGUMENTS="--capture"

# Create output dir.
#
rm -rvf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

# Init is not required, as we have an empty dir at start.
#
#lcov $LCOV_CLEAR_ARGUMENTS --output-file $OUTPUT_DIR/Simulator.info --directory ./SimulatorLib/debug

# Build project u7_test_simulator.
#
rm -rvf /tmp/build/test_simulator
$CI_PROJECT_DIR/bin/debug/BuilderConsole $CI_PROJECT_DIR/Tests/BuilderConsoleArgsCoverage.xml

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
./linux_code_coverage_systemid_clienttest_ws01_tuns2.sh < /dev/null > clienttest_ws01_tuns2.out 2>&1 &
./linux_code_coverage_systemid_clienttest_ws02_tuns.sh < /dev/null > clienttest_ws02_tuns.out 2>&1 &
./linux_code_coverage_systemid_clienttest_ws04_tuns.sh < /dev/null > clienttest_ws04_tuns.out 2>&1 &

./SimulatorConsole -build=/tmp/build/${SIMULATOR_PROJECT_NAME}/build -profile=linux_code_coverage -enable_lan -script=$CI_PROJECT_DIR/Tests/ClientTests/Scripts/TuningTests.js -speed_factor=x0.5 -verbose > SimulatorConsole.out 2>&1 &

sleep 6

# Run ClientTests, they are functional.
#
./ClientTests -build=/tmp/build/${SIMULATOR_PROJECT_NAME}/build -profile=linux_code_coverage

# Run AdsBridgeTests, they are functional.
#
./AdsBridgeTests --config=/tmp/build/${SIMULATOR_PROJECT_NAME}/build/SYSTEMID_CLIENTTEST_WS03_ADSBRIDGE/Configuration.xml --profile=linux_code_coverage

# Run TestSuite Tests
#
export QT_QPA_PLATFORM=offscreen
./TestSuiteConsole -settings="../../Tests/TestSuiteConsoleCodeCoverage.xml" -nosecurity
./TestSuiteConsole -settings="../../Tests/TestSuiteConsoleCodeCoverage.xml" -scripts_path=/tmp/build/${SIMULATOR_PROJECT_NAME}/build/SYSTEMID_CLIENTTEST_WS01_TESTSUITE -nosecurity

# Stop services after ClientTests (functional tests)
#
StopServices || true

# ----------------------------------------------
#               Run AdsGateway tests
# ----------------------------------------------
pushd $CI_PROJECT_DIR/bin/debug
StopServices || true
sleep 5

./linux_code_coverage_systemid_clienttest_ws01_cfgs.sh simulation < /dev/null > clienttest_ws01_cfgs_ads_adsgwtest.out 2>&1 &
sleep 5

./linux_code_coverage_systemid_clienttest_ws01_gwslinuxcc.sh &
sleep 5

# Check that only Gateway and Config services are running.
#
ps -A | grep Srv

# First run tests that require no ADS connection.
#
$CI_PROJECT_DIR/bin/debug/GatewayTests --port=5567 --gtest_filter=AdsGatewayTestsNoAds.*

# Then start ADS for other tests.
#
./linux_code_coverage_systemid_clienttest_ws01_ads.sh < /dev/null > clienttest_ws01_ads_adsgwtest.out 2>&1 &
sleep 5

# Run other Adsgateway tests.
#
$CI_PROJECT_DIR/bin/debug/GatewayTests --port=5567 --gtest_filter=AdsGatewayTests.*
sleep 5

StopServices || true
sleep 5
popd

# Run other tests, not services are required here.
#
#./LicenseLibTests
./MetrologyTests
./SimulatorTests
./TestSuiteLibUnitTests
./u7databasetests -config=$CI_PROJECT_DIR/Tests/u7databasetestsArgsCoverage.xml

popd

# Give some time to flush .gcda files.
#
sleep 4

# Get code coverage data.
#

# AppSignalLib
TEST_DIR="./build/AppSignalLib/CMakeFiles/AppSignalLib.dir"
TEST_OUTPUT_FILE="AppSignalLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# CommonLib
TEST_DIR="./build/libs/CommonLib/CMakeFiles/CommonLib.dir"
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
TEST_DIR="./build/Tests/MetrologyTests/CMakeFiles/MetrologyTests.dir"
TEST_OUTPUT_FILE="MetrologyTests.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# LicenseLib
#TEST_DIR="./build/libs/LicenseLib/CMakeFiles/LicenseLib.dir"
#TEST_OUTPUT_FILE="LicenseLib.info"
#lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Builder
TEST_DIR="./build/Builder/CMakeFiles/Builder.dir"
TEST_OUTPUT_FILE="Builder.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# Simulator
TEST_DIR="./build/Simulator/SimulatorLib/CMakeFiles/SimulatorLib.dir"
TEST_OUTPUT_FILE="SimulatorLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# TrendView
TEST_DIR="./build/libs/TrendView/CMakeFiles/TrendView.dir"
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

# TestSuiteLib
TEST_DIR="./build/libs/TestSuiteLib/CMakeFiles/TestSuiteLib.dir"
TEST_OUTPUT_FILE="TestSuiteLib.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# AdsBridge
TEST_DIR="./build/libs/AdsBridge/CMakeFiles/AdsBridge.dir"
TEST_OUTPUT_FILE="AdsBridge.info"
lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# GatewayService
#TEST_DIR="./build/GatewayService/CMakeFiles/GatewaySrv.dir"
#TEST_OUTPUT_FILE="GatewaySrv.info"
#lcov --test-name "$TEST_OUTPUT_FILE" $LCOV_COLLECT_ARGUMENTS --output-file $OUTPUT_DIR/$TEST_OUTPUT_FILE --directory $TEST_DIR

# GatewayLib
TEST_DIR="./build/GatewayLib/CMakeFiles/GatewayLib.dir"
TEST_OUTPUT_FILE="GatewayLib.info"
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
    --add-tracefile $OUTPUT_DIR/SimulatorLib.info \
    --add-tracefile $OUTPUT_DIR/UtilsLib.info \
    --add-tracefile $OUTPUT_DIR/ClientLib.info \
    --add-tracefile $OUTPUT_DIR/TestSuiteLib.info \
    --add-tracefile $OUTPUT_DIR/AdsBridge.info \
    --add-tracefile $OUTPUT_DIR/GatewayLib.info

#    --add-tracefile $OUTPUT_DIR/GatewaySrv.info \ # Cannot collect .gcda -- process is killed in graceful, still no cgda files are NOT generated (((
#    --add-tracefile $OUTPUT_DIR/LicenseLib.info \
#    --add-tracefile $OUTPUT_DIR/AppDataSrv.info \
#    --add-tracefile $OUTPUT_DIR/CfgSrv.info

# Filter combined file, result stored to $OUTPUT_DIR/u7set.info.
#
lcov --ignore-errors unused --remove $OUTPUT_DIR/u7set-dirty.info \
   "*Qt*.framework*" \
   "/usr/*" \
   "*/Qt/*" \
   "*/Proto*/*" \
   "*.pb.*" \
   "*.moc" \
   "*moc_*.cpp" \
   "*/TestAppDataService/*" \
   "*/TestSuiteLibUnitTests/*" \
   "*/build/vcpkg_installed/*" \
   --output-file $OUTPUT_DIR/u7set.info

# Generate HTML report.
#
genhtml --legend \
    --rc branch_coverage=1 \
    --rc genhtml_med_limit=60 \
    --rc genhtml_hi_limit=80 \
    --output-directory $OUTPUT_DIR/Report \
    $OUTPUT_DIR/u7set.info

# Generate cobertura XML (for GitLab CI).
# https://pypi.org/project/lcov-cobertura/
# sudo pip install lcov-cobertura
#
# lcov_cobertura $OUTPUT_DIR/u7set.info --output $OUTPUT_DIR/coverage.xml
