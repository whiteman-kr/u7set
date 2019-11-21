function Component()
{
	if(systemInfo.currentCpuArchitecture.search("64") < 0) 
	{
		//x86
	        QMessageBox.critical("vcRedist.install", "RPCT Setup", "The application runs on 64-bit architecture.", QMessageBox.OK);
	
	        installer.setValue("FinishedText", "<font color='red' size=3>The installation was aborted.</font>");
	        installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
	        installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
        	installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
	        installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);
	        installer.setDefaultPageVisible(QInstaller.PerformInstallation, false);
        	installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
	        gui.clickButton(buttons.NextButton);
		return;
	} else 
	{
		//x64
	}

	installer.installationFinished.connect(this, Component.prototype.installVCRedist);

	var widget = gui.pageById(QInstaller.StartMenuSelection); // get the introduction wizard page
	if (widget != null)
	{
	       	widget.entered.connect(changeTargetPath);
	}
}

changeTargetPath = function()
{
	var page = gui.pageById(QInstaller.StartMenuSelection); // get the introduction wizard page
	if (page != null)
	{
		var text = page.StartMenuPathLineEdit.text;
                page.StartMenuPathLineEdit.setText(text + " (" + installer.value("Version") + ")");
	}

}

Component.prototype.installVCRedist = function()
{
    var registryVC2017x64 = installer.execute("reg", new Array("QUERY", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64", "/v", "Installed"))[0];
    var doInstall = false;
    if (!registryVC2017x64) 
    {
        doInstall = true;
    }
    else
    {
        var bld = installer.execute("reg", new Array("QUERY", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64", "/v", "Bld"))[0];

        var elements = bld.split(" ");

        bld = parseInt(elements[elements.length-1]);
        if (bld < 27024)
        {
            doInstall = true;
        }
    }


    if (doInstall)
    {
        var vcPath = installer.value("TargetDir") + "/vcredist_x64.exe";
        installer.execute(vcPath, "/norestart", "/passive");
    }
}