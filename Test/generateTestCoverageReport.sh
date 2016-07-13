#!/bin/bash

echo "=========================";
echo "Executting tests";
echo "=========================";

if [ -d "bin_unix/debug/" ]; then
	cd bin_unix/debug;
else
	echo "Can not find test executor";
	echo "Have you placed this script in the build folder?";
	exit 1;
fi

if ! ./u7databasetests; then
        echo "Can not execute tests binary";
        exit 2;
fi

if [ -d "../../Test" ]; then
        cd ../../Test;
else
        echo "Can not find object folder";
        echo "Have you placed this script in the build folder?";
        exit 3;
fi


echo "=========================";
echo "Cleaning old data";
echo "=========================";

if [ -d "testCoverage/" ]; then
	rm -r testCoverage/;
	echo "Old data folder wiped!";
fi

if [ -f "coverage.info" ]; then
	rm coverage.info;
	echo "Old coverage.info wiped";
fi

if [ -f "coverageDbController.info" ]; then
	rm coverageDbController.info;
	echo "Old coverageDbController.info wiped";
fi

if [ -f "coverageDbWorker.info" ]; then
        rm coverageDbWorker.info;
        echo "Old coverageDbWorker.info wiped";
fi

echo "=========================";
echo "Generating Coverage files";
echo "-------------------------";
echo "Generating coverageDbController.info...";


if ! lcov -c -d "u7databaseTests/DbController.gcda" -o coverageDbController.info; then
	echo "Can not generate coverageDbController.info. Err code $?";
	echo "This script must be placed in project build dir.";
	exit 4;
fi

echo "Generating coverageDbWorker.info...";

if ! lcov -c -d "u7databaseTests/DbWorker.gcda" -o coverageDbWorker.info; then
	echo "Can not generate coverageDbWorker.info. Err code $?";
	echo "This script must be placed in project build dir.";
        exit 5;
fi

echo "Generating coverageDbStruct.info...";

if ! lcov -c -d "u7databaseTests/DbStruct.gcda" -o coverageDbStruct.info; then
	echo "Can not generate coverageDbStruct.info. Err code $?";
	echo "This script must be placed in project build dir.";
        exit 5;
fi

echo "Generating coveragePropertyObject.info...";

if ! lcov -c -d "u7databaseTests/PropertyObject.gcda" -o coveragepropertyObject.info; then
        echo "Can not generate coveragePropertyObject.info. Err code $?";
        echo "This script must be placed in project build dir.";
        exit 5;
fi


echo "Merging coverage files into coverage.info...";

if ! lcov --add-tracefile coverageDbController.info -a coverageDbWorker.info -a coverageDbStruct.info -a coveragePropertyObject.info -o coverage.info; then
	echo "Can not merge files to coverage.info. Err code $?";
        exit 6;
fi

echo "Cleaning waste information from coverage.info...";

if ! lcov -r coverage.info "/usr/*" "/home/$USER/Qt*" "lib/*.h" -o coverage.info; then
	echo "Can not cleanup waste data from coverage.info. Err code $?";
        exit 7;
fi

echo "=========================";
echo "Converting result to html...";
echo "=========================";

if ! genhtml coverage.info --output-directory testCoverage/; then
	echo "Can not generate html! Err code: $?";
	exit 8;
fi

echo "========================";
echo "Try to open with default system browser";
echo "========================";

if ! xdg-open 'testCoverage/index.html'; then
	echo "Can not open file testCoverage/index.html with your default browser. Try to open it by yourself";
	exit 9;
fi

echo "Done";
