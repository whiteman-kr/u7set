IF NOT EXIST ..\bin\release\docs (
	mkdir ..\bin\release\docs
)

FOR /F "delims=|" %%d  IN (SvnDocList.txt) DO (

	svn checkout "https://delo:8443/svn//RadICS_Platform/trunk/Docs/FSC Documents/FSC Safety Manual/%%d" ..\bin\release\docs\%%d --depth empty --non-interactive --trust-server-cert --username=gitlab --password gitl@b

	PUSHD .
	
	cd ..\bin\release\docs\%%d

	svn list --non-interactive --trust-server-cert --recursive "https://delo:8443/svn//RadICS_Platform/trunk/Docs/FSC Documents/FSC Safety Manual/%%d" | find /I ".pdf" > filelist.txt

	FOR /F "delims=|" %%i  IN (filelist.txt) DO (

	 	svn update --non-interactive --trust-server-cert --parents "%%i"
	)

	xcopy *.pdf .. /sy

	POPD

	rem rmdir /S /Q ..\bin\release\docs\%%d
)

IF NOT EXIST ..\bin\release\docs\D11.5_AFBL_RM.pdf goto SvnError
IF NOT EXIST ..\bin\release\docs\D11.6_RPCT-UM.pdf goto SvnError
IF NOT EXIST "..\bin\release\docs\Appendixes\D11.6 RPCT User Manual Appendix A Warnings and Errors List.pdf"  goto SvnError
IF NOT EXIST ..\bin\release\docs\D11.8_FSC_MATS_User_Manual.pdf goto SvnError
IF NOT EXIST ..\bin\release\docs\D11.9_FSC_Tuning_User_Manual.pdf  goto SvnError

:SvnSuccess
echo All files were successfully received from SVN.
cmd /c exit 0
echo Errorlevel = %errorlevel%
goto Done

:SvnError
echo ERROR - some files were not received from SVN!
cmd /c exit 1
echo Errorlevel = %errorlevel%

:Done