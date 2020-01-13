function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/Metrology.exe", "@StartMenuDir@/Metrology.lnk",
            "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut", "@TargetDir@/Metrology.exe", "@DesktopDir@/RPCT (@Version@)/Metrology.lnk",
            "workingDirectory=@TargetDir@");
    }
}