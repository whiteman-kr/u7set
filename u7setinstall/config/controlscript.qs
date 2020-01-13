function Controller() {

    if (installer.isUninstaller()) 
    {
	installer.uninstallationFinished.connect(this, Controller.prototype.uninstallationFinished);
    }

}

Controller.prototype.uninstallationFinished = function()
{
	installer.performOperation("Rmdir", "@DesktopDir@/RPCT (@Version@)");
}