cmake --graphviz=test.dot .

@echo off

REM Enable delayed environment variable expansion
setlocal enabledelayedexpansion

REM Create the LibraryDependencies directory if it doesn't exist
if not exist "LibraryDependencies" (
    mkdir "LibraryDependencies"
)

REM First loop for files matching "test.dot.*"
for %%f in (test.dot.*) do (
    REM Check if the file exists and is not a directory
    if exist "%%f" if not exist "%%f\" (
        REM Set the output file name by replacing the original extension with .svg
        set "output_file=LibraryDependencies\%%f.svg"
        
        REM Run the dot command to generate the SVG file
        dot -Tsvg "%%f" -o "!output_file!"
        
        REM Optionally, print a message to indicate progress
        echo Generated !output_file!
    )
)

REM Second loop for files matching "test.dot.*.dependers"
for %%f in (test.dot.*.dependers) do (
    REM Check if the file exists and is not a directory
    if exist "%%f" if not exist "%%f\" (
        REM Set the output file name by replacing the original extension with .svg
        set "output_file=LibraryDependencies\%%f.svg"
        
        REM Run the dot command to generate the SVG file
        dot -Tsvg "%%f" -o "!output_file!"
        
        REM Optionally, print a message to indicate progress
        echo Generated !output_file!
    )
)

endlocal
