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

if [ "$1" == "-exec" ]; then
	if ! ./u7databasetests; then
        	echo "Can not execute tests binary";
	        exit 2;
	fi
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
else
	echo "Nothing to clean";
fi

echo "=========================";
echo "Generating Coverage files";
echo "-------------------------";

file="../coverageConfig.files";
mergeFiles="";
while IFS= read -r line
do
	if [ -n $line ]; then
		if [ -f "$line.info" ]; then
        		rm $line.info;
        		echo "Old $line.info wiped";
		fi

		echo "Generating $line.info...";

		if ! lcov -c -d "u7databaseTests/$line.gcda" -o $line.info; then
			echo "Can not generate $line.info. Err code $?";
			echo "This script must be placed in project build dir.";
			echo "If it is already done, try to delete build dir, and rebuild u7setest";
			exit 4;
		fi
	fi

	mergeFiles="$mergeFiles -a $line.info";

done < $file;

echo "Merging coverage files into coverage.info...";

if ! lcov $mergeFiles -o coverage.info; then
	echo "Can not merge files to coverage.info. Err code $?";
        exit 6;
fi

echo "Cleaning waste information from coverage.info...";

if ! lcov -r coverage.info "/usr/*" "/home/$USER/Qt*" "lib/*.h" "Proto/*" "*Qt*" -o coverage.info; then
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
