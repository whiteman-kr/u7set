rem @echo off

IF NOT EXIST %BIN_OUT_DIR%\docs (
	mkdir %BIN_OUT_DIR%\docs
)

if DEFINED SKIP_DELO (
    goto :SkipDelo
)

FOR /F "delims=|" %%d  IN (SvnDocList.txt) DO (

	svn checkout "https://delo:8443/svn//RadICS_Platform/trunk/Docs/FSC Documents/FSC Safety Manual/%%d" %BIN_OUT_DIR%\docs\%%d --depth empty --non-interactive --trust-server-cert --username=gitlab --password gitl@b

	PUSHD .
	
	cd %BIN_OUT_DIR%\docs\%%d

	svn list --non-interactive --trust-server-cert --recursive "https://delo:8443/svn//RadICS_Platform/trunk/Docs/FSC Documents/FSC Safety Manual/%%d" | find /I ".pdf" > filelist.txt

	FOR /F "delims=|" %%i  IN (filelist.txt) DO (

	 	svn update --non-interactive --trust-server-cert --parents "%%i"
	)

	xcopy *.pdf .. /sy

	POPD

	rmdir /S /Q %BIN_OUT_DIR%\docs\%%d
)

IF NOT EXIST "%BIN_OUT_DIR%\docs\D11.5_AFBL_RM.pdf" goto SvnError
IF NOT EXIST "%BIN_OUT_DIR%\docs\D11.6_RPCT-UM.pdf" goto SvnError
IF NOT EXIST "%BIN_OUT_DIR%\docs\Appendixes\D11.6 RPCT User Manual Appendix A Warnings and Errors List.pdf"  goto SvnError
IF NOT EXIST "%BIN_OUT_DIR%\docs\Appendixes\D11.6 RPCT User Manual Appendix B Build Directory and Output Bitstream File Description.pdf"  goto SvnError
IF NOT EXIST "%BIN_OUT_DIR%\docs\D11.8_FSC_MATS_User_Manual.pdf" goto SvnError
IF NOT EXIST "%BIN_OUT_DIR%\docs\D11.9_FSC_Tuning_User_Manual.pdf"  goto SvnError
IF NOT EXIST "%BIN_OUT_DIR%\docs\Installing and configuring RPCT.pdf" goto SvnError
IF NOT EXIST "%BIN_OUT_DIR%\docs\RPCT Quick Start Guide.pdf" goto SvnError


:SvnSuccess
echo All files were successfully received from SVN.
cmd /c exit 0
echo Errorlevel = %errorlevel%
goto Done

:SkipDelo
echo Getting documents from the SVN server was skipped
cmd /c exit 0
echo Errorlevel = %errorlevel%
goto Done

:SvnError
echo ERROR - some files were not received from SVN!
cmd /c exit 1
echo Errorlevel = %errorlevel%

:Done