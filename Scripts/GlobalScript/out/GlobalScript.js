"use strict";
var E;
(function (E) {
    let Channel;
    (function (Channel) {
        Channel[Channel["A"] = 0] = "A";
        Channel[Channel["B"] = 1] = "B";
        Channel[Channel["C"] = 2] = "C";
        Channel[Channel["D"] = 3] = "D"; /**< Channel D = 3 */
    })(Channel = E.Channel || (E.Channel = {}));
    let AnalogFormat;
    (function (AnalogFormat) {
        AnalogFormat["e_9e"] = "e";
        AnalogFormat["E_9E"] = "E";
        AnalogFormat["f_9"] = "f";
        AnalogFormat["g_9_or_9e"] = "g";
        AnalogFormat["G_9_or_9E"] = "G"; /**< G_9_or_9E = 'G' (0x47/71) Use E or f format, whichever is the most concise*/
    })(AnalogFormat = E.AnalogFormat || (E.AnalogFormat = {}));
    let ColumnData;
    (function (ColumnData) {
        ColumnData[ColumnData["AppSignalID"] = 0] = "AppSignalID";
        ColumnData[ColumnData["CustomSignalID"] = 1] = "CustomSignalID";
        ColumnData[ColumnData["Caption"] = 2] = "Caption";
        ColumnData[ColumnData["State"] = 3] = "State";
        ColumnData[ColumnData["ImpactAppSignalID"] = 32] = "ImpactAppSignalID";
        ColumnData[ColumnData["ImpactCustomSignalID"] = 33] = "ImpactCustomSignalID";
        ColumnData[ColumnData["ImpactCaption"] = 34] = "ImpactCaption";
        ColumnData[ColumnData["ImpactState"] = 35] = "ImpactState";
        ColumnData[ColumnData["CustomText"] = 64] = "CustomText"; /**< CustomText = 64*/
    })(ColumnData = E.ColumnData || (E.ColumnData = {}));
    let LineStyle;
    (function (LineStyle) {
        LineStyle[LineStyle["NoPen"] = 0] = "NoPen";
        LineStyle[LineStyle["SolidLine"] = 1] = "SolidLine";
        LineStyle[LineStyle["DashLine"] = 2] = "DashLine";
        LineStyle[LineStyle["DotLine"] = 3] = "DotLine";
        LineStyle[LineStyle["DashDotLine"] = 4] = "DashDotLine";
        LineStyle[LineStyle["DashDotDotLine"] = 5] = "DashDotDotLine"; /**< DashDotDotLine = 5. One dash, two dots, one dash, two dots.*/
    })(LineStyle = E.LineStyle || (E.LineStyle = {}));
    let TextFormat;
    (function (TextFormat) {
        TextFormat[TextFormat["PlainText"] = 0] = "PlainText";
        TextFormat[TextFormat["Markdown"] = 1] = "Markdown";
        TextFormat[TextFormat["HtmlSubset"] = 2] = "HtmlSubset"; /**< HtmlSubset = 2. HTML-formatted text in the html string. Support of the limited HTML Subset.*/
    })(TextFormat = E.TextFormat || (E.TextFormat = {}));
    let HorzAlign;
    (function (HorzAlign) {
        HorzAlign[HorzAlign["AlignLeft"] = 1] = "AlignLeft";
        HorzAlign[HorzAlign["AlignRight"] = 2] = "AlignRight";
        HorzAlign[HorzAlign["AlignHCenter"] = 4] = "AlignHCenter";
        HorzAlign[HorzAlign["AlignJustify"] = 8] = "AlignJustify"; /**< AlignJustify = 0x08*/
    })(HorzAlign = E.HorzAlign || (E.HorzAlign = {}));
    let VertAlign;
    (function (VertAlign) {
        VertAlign[VertAlign["AlignTop"] = 32] = "AlignTop";
        VertAlign[VertAlign["AlignBottom"] = 64] = "AlignBottom";
        VertAlign[VertAlign["AlignVCenter"] = 128] = "AlignVCenter";
        VertAlign[VertAlign["AlignBaseline"] = 256] = "AlignBaseline"; /**< AlignBaseline = 0x100*/
    })(VertAlign = E.VertAlign || (E.VertAlign = {}));
    let Alignment;
    (function (Alignment) {
        Alignment[Alignment["AlignLeft"] = 1] = "AlignLeft";
        Alignment[Alignment["AlignRight"] = 2] = "AlignRight";
        Alignment[Alignment["AlignHCenter"] = 4] = "AlignHCenter";
        Alignment[Alignment["AlignJustify"] = 8] = "AlignJustify";
        Alignment[Alignment["AlignAbsolute"] = 16] = "AlignAbsolute";
        Alignment[Alignment["AlignTop"] = 32] = "AlignTop";
        Alignment[Alignment["AlignBottom"] = 64] = "AlignBottom";
        Alignment[Alignment["AlignVCenter"] = 128] = "AlignVCenter";
        Alignment[Alignment["AlignBaseline"] = 256] = "AlignBaseline"; /**< AlignBaseline = 0x100*/
    })(Alignment = E.Alignment || (E.Alignment = {}));
    let SoftwareType;
    (function (SoftwareType) {
        SoftwareType[SoftwareType["Unknown"] = 8000] = "Unknown";
        SoftwareType[SoftwareType["BaseService"] = 8999] = "BaseService";
        SoftwareType[SoftwareType["Monitor"] = 9000] = "Monitor";
        SoftwareType[SoftwareType["ConfigurationService"] = 9001] = "ConfigurationService";
        SoftwareType[SoftwareType["AppDataService"] = 9002] = "AppDataService";
        SoftwareType[SoftwareType["ArchiveService"] = 9003] = "ArchiveService";
        SoftwareType[SoftwareType["TuningService"] = 9004] = "TuningService";
        SoftwareType[SoftwareType["DiagDataService"] = 9005] = "DiagDataService";
        SoftwareType[SoftwareType["TuningClient"] = 9006] = "TuningClient";
        SoftwareType[SoftwareType["Metrology"] = 9007] = "Metrology";
        SoftwareType[SoftwareType["ServiceControlManager"] = 9008] = "ServiceControlManager";
        SoftwareType[SoftwareType["TestClient"] = 9009] = "TestClient";
        SoftwareType[SoftwareType["TestSuite"] = 9010] = "TestSuite";
        SoftwareType[SoftwareType["GatewayService"] = 9011] = "GatewayService";
        SoftwareType[SoftwareType["Diagnostics"] = 9012] = "Diagnostics";
    })(SoftwareType = E.SoftwareType || (E.SoftwareType = {}));
    ;
    let DeviceType;
    (function (DeviceType) {
        DeviceType[DeviceType["Root"] = 0] = "Root";
        DeviceType[DeviceType["System"] = 1] = "System";
        DeviceType[DeviceType["Rack"] = 2] = "Rack";
        DeviceType[DeviceType["Chassis"] = 3] = "Chassis";
        DeviceType[DeviceType["Module"] = 4] = "Module";
        DeviceType[DeviceType["Workstation"] = 5] = "Workstation";
        DeviceType[DeviceType["Software"] = 6] = "Software";
        DeviceType[DeviceType["Controller"] = 7] = "Controller";
        DeviceType[DeviceType["AppSignal"] = 8] = "AppSignal";
        DeviceType[DeviceType["DiagSignal"] = 9] = "DiagSignal";
    })(DeviceType = E.DeviceType || (E.DeviceType = {}));
    ;
})(E || (E = {}));
//declare var app: App.ScriptMonitorApplication;		// For Monitor script
//declare var equipment: Hardware.ScriptEquipment;		// For Monitor script
//declare var app: App.ScriptTuningClientApplication;	// For TuningClient script
//declare var app: App.ScriptTestSuiteApplication;	// For TestSuite script
// Global functions
function showSignalValue(schemaItem, signalId) {
    schemaItem.alignHorz = E.HorzAlign.AlignHCenter;
    schemaItem.alignVert = E.VertAlign.AlignVCenter;
    if (signals.signalExists(signalId) == false) {
        schemaItem.text = "Signal not found!";
        return;
    }
    let state = signals.signalState(signalId);
    if (state == null) {
        schemaItem.text = "State not found!";
        return;
    }
    if (state.valid == true) {
        schemaItem.text = state.value.toString();
    }
    else {
        schemaItem.text = "???";
    }
    return;
}
var Hardware;
(function (Hardware) {
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
})(Hardware || (Hardware = {}));
var Qt;
(function (Qt) {
    let Orientation;
    (function (Orientation) {
        Orientation[Orientation["Horizontal"] = 1] = "Horizontal";
        Orientation[Orientation["Vertical"] = 2] = "Vertical";
    })(Orientation = Qt.Orientation || (Qt.Orientation = {}));
})(Qt || (Qt = {}));
var QMessageBox;
(function (QMessageBox) {
    let Icon;
    (function (Icon) {
        Icon[Icon["NoIcon"] = 0] = "NoIcon";
        Icon[Icon["Information"] = 1] = "Information";
        Icon[Icon["Warning"] = 2] = "Warning";
        Icon[Icon["Critical"] = 3] = "Critical";
        Icon[Icon["Question"] = 4] = "Question";
    })(Icon = QMessageBox.Icon || (QMessageBox.Icon = {}));
    let StandardButton;
    (function (StandardButton) {
        StandardButton[StandardButton["NoButton"] = 0] = "NoButton";
        StandardButton[StandardButton["Ok"] = 1024] = "Ok";
        StandardButton[StandardButton["Save"] = 2048] = "Save";
        StandardButton[StandardButton["SaveAll"] = 4096] = "SaveAll";
        StandardButton[StandardButton["Open"] = 8192] = "Open";
        StandardButton[StandardButton["Yes"] = 16384] = "Yes";
        StandardButton[StandardButton["YesToAll"] = 32768] = "YesToAll";
        StandardButton[StandardButton["No"] = 65536] = "No";
        StandardButton[StandardButton["NoToAll"] = 131072] = "NoToAll";
        StandardButton[StandardButton["Abort"] = 262144] = "Abort";
        StandardButton[StandardButton["Retry"] = 524288] = "Retry";
        StandardButton[StandardButton["Ignore"] = 1048576] = "Ignore";
        StandardButton[StandardButton["Close"] = 2097152] = "Close";
        StandardButton[StandardButton["Cancel"] = 4194304] = "Cancel";
        StandardButton[StandardButton["Discard"] = 8388608] = "Discard";
        StandardButton[StandardButton["Help"] = 16777216] = "Help";
        StandardButton[StandardButton["Apply"] = 33554432] = "Apply";
        StandardButton[StandardButton["Reset"] = 67108864] = "Reset";
        StandardButton[StandardButton["RestoreDefaults"] = 134217728] = "RestoreDefaults";
    })(StandardButton = QMessageBox.StandardButton || (QMessageBox.StandardButton = {}));
})(QMessageBox || (QMessageBox = {}));
var QSlider;
(function (QSlider) {
    let TickPosition;
    (function (TickPosition) {
        TickPosition[TickPosition["NoTicks"] = 0] = "NoTicks";
        TickPosition[TickPosition["TicksBothSides"] = 3] = "TicksBothSides";
        TickPosition[TickPosition["TicksAbove"] = 1] = "TicksAbove";
        TickPosition[TickPosition["TicksBelow"] = 2] = "TicksBelow";
        TickPosition[TickPosition["TicksLeft"] = 1] = "TicksLeft";
        TickPosition[TickPosition["TicksRight"] = 2] = "TicksRight"; /**< Draw tick marks to the right of the (vertical) slider.*/
    })(TickPosition = QSlider.TickPosition || (QSlider.TickPosition = {}));
})(QSlider || (QSlider = {}));
