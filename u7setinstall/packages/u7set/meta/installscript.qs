var ComponentSelectionPage = null;

function Component()
{

	if (installer.isInstaller()) {

		component.loaded.connect(this, Component.prototype.installerLoaded);

		ComponentSelectionPage = gui.pageById(QInstaller.ComponentSelection);
		if (ComponentSelectionPage == null)
		{
			QMessageBox.critical("vcRedist.install", "RPCT Setup", "ComponentSelectionPage not found!", QMessageBox.OK);
		}
	
		//

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
	
		var page = gui.pageById(QInstaller.StartMenuSelection); // get the introduction wizard page
		if (page != null)
		{
		       	page.entered.connect(changeTargetPath);
		}
	}
}

Component.prototype.installerLoaded = function () 
{	
	if (installer.addWizardPage(component, "InstallationWidget", QInstaller.ComponentSelection)) 
	{
		var widget = gui.pageWidgetByObjectName("DynamicInstallationWidget");
	    if (widget != null) 
		{
			widget.developInstall.toggled.connect(this, Component.prototype.developInstallToggled);
			widget.serverInstall.toggled.connect(this, Component.prototype.serverInstallToggled);
			widget.clientInstall.toggled.connect(this, Component.prototype.clientInstallToggled);
			widget.customInstall.toggled.connect(this, Component.prototype.customInstallToggled);
	
	        widget.developInstall.checked = true;
	        widget.windowTitle = "Select Installation Type";
			
			installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
	    }
		else
		{
			QMessageBox.critical("vcRedist.install", "RPCT Setup", "DynamicInstallationWidget not found!", QMessageBox.OK);
		}
	}
	else
	{
		QMessageBox.critical("vcRedist.install", "RPCT Setup", "installer.addWizardPage failed!", QMessageBox.OK);
	}

}

Component.prototype.developInstallToggled = function (checked) {
    if (checked) {
		//QMessageBox.critical("vcRedist.install", "RPCT Setup", "developInstallToggled", QMessageBox.OK);
        if (ComponentSelectionPage != null)
		{
			ComponentSelectionPage.deselectAll();
			ComponentSelectionPage.selectComponent("u7set.develop.rpct");
		}
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    }
}

Component.prototype.serverInstallToggled = function (checked) {
    if (checked) 
	{
		//QMessageBox.critical("vcRedist.install", "RPCT Setup", "serverInstallToggled", QMessageBox.OK);
        if (ComponentSelectionPage != null)
		{
			ComponentSelectionPage.deselectAll();
			ComponentSelectionPage.selectComponent("u7set.mats.cfgsrv");
			ComponentSelectionPage.selectComponent("u7set.mats.appdatasrv");
			ComponentSelectionPage.selectComponent("u7set.mats.archsrv");
			ComponentSelectionPage.selectComponent("u7set.mats.tunsrv");
			ComponentSelectionPage.selectComponent("u7set.mats.scm");
			ComponentSelectionPage.selectComponent("u7set.mats.monitor");
			ComponentSelectionPage.selectComponent("u7set.mats.tuningclient");
		}
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    }
}

Component.prototype.clientInstallToggled = function (checked) {
    if (checked) 
	{
		//QMessageBox.critical("vcRedist.install", "RPCT Setup", "clientInstallToggled", QMessageBox.OK);
        if (ComponentSelectionPage != null)
		{
			ComponentSelectionPage.deselectAll();
			ComponentSelectionPage.selectComponent("u7set.mats.monitor");
			ComponentSelectionPage.selectComponent("u7set.mats.tuningclient");
		}
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    }
}

Component.prototype.customInstallToggled = function (checked) {
    if (checked) 
	{
		//QMessageBox.critical("vcRedist.install", "RPCT Setup", "customInstallationToggled", QMessageBox.OK);
        if (ComponentSelectionPage != null)
		{
            ComponentSelectionPage.selectAll();
		}
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, true);
    }
}


changeTargetPath = function()
{
	var page = gui.pageById(QInstaller.StartMenuSelection); // get the introduction wizard page
	if (page != null)
	{
		var text = page.StartMenuPathLineEdit.text;
		if (text.endsWith(")") == false)
		{
			page.StartMenuPathLineEdit.setText(text + " (" + installer.value("Version") + ")");
		}
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
