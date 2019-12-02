function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/TuningClient.exe", "@StartMenuDir@/TuningClient.lnk",
            "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut", "@TargetDir@/TuningClient.exe", "@DesktopDir@/RPCT (@Version@)/TuningClient.lnk",
            "workingDirectory=@TargetDir@");
    }
}