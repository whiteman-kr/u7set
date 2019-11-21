function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/Monitor.exe", "@StartMenuDir@/Monitor.lnk",
            "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut", "@TargetDir@/Monitor.exe", "@DesktopDir@/RPCT (@Version@)/Monitor.lnk",
            "workingDirectory=@TargetDir@");
    }
}