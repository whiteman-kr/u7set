function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
       component.addOperation("CreateShortcut", "@TargetDir@/mconf.exe", "@StartMenuDir@/mconf.lnk",
            "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut", "@TargetDir@/mconf.exe", "@DesktopDir@/RPCT (@Version@)/mconf.lnk",
            "workingDirectory=@TargetDir@");
    }
}