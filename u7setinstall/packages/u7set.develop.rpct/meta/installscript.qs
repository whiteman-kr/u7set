function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/u7.exe", "@StartMenuDir@/u7.lnk",
            "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut", "@TargetDir@/u7.exe", "@DesktopDir@/RPCT (@Version@)/u7.lnk",
            "workingDirectory=@TargetDir@");
    }
}