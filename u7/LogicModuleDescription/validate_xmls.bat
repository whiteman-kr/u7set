@echo off
@echo %1\*.xml
for %%F in (%1\*.xml) do (
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