@echo off
for %%F in (*.xml) do (
    echo checking file %%F...
	rem ..\..\Tools\xmlstarlet-1.6.1-win32\xml.exe val -e %%F 
	..\..\Tools\xmlstarlet-1.6.1-win32\xml.exe val %%F | find "invalid" > NUL & if errorlevel 1 (
		echo %%F has valid xml format 
	) ELSE (
		echo %%F has INVALID/ERROR xml format 
		..\..\Tools\xmlstarlet-1.6.1-win32\xml.exe val -e %%F
		exit /B 1
	)
)

exit /B 0