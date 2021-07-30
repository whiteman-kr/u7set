// Non-strict mode part
//
var DeviceObjectType;
(function (DeviceObjectType) {
    DeviceObjectType[DeviceObjectType["Root"] = 0] = "Root";
    DeviceObjectType[DeviceObjectType["System"] = 1] = "System";
    DeviceObjectType[DeviceObjectType["Rack"] = 2] = "Rack";
    DeviceObjectType[DeviceObjectType["Chassis"] = 3] = "Chassis";
    DeviceObjectType[DeviceObjectType["Module"] = 4] = "Module";
    DeviceObjectType[DeviceObjectType["Controller"] = 5] = "Controller";
    DeviceObjectType[DeviceObjectType["Workstation"] = 6] = "Workstation";
    DeviceObjectType[DeviceObjectType["Software"] = 7] = "Software";
    DeviceObjectType[DeviceObjectType["AppSignal"] = 8] = "AppSignal";
})(DeviceObjectType || (DeviceObjectType = {}));
var SoftwareType;
(function (SoftwareType) {
    SoftwareType[SoftwareType["Monitor"] = 9000] = "Monitor";
    SoftwareType[SoftwareType["ConfigurationService"] = 9001] = "ConfigurationService";
    SoftwareType[SoftwareType["AppDataService"] = 9002] = "AppDataService";
    SoftwareType[SoftwareType["ArchiveService"] = 9003] = "ArchiveService";
    SoftwareType[SoftwareType["TuningService"] = 9004] = "TuningService";
    SoftwareType[SoftwareType["DiagDataService"] = 9005] = "DiagDataService";
    SoftwareType[SoftwareType["TuningClient"] = 9006] = "TuningClient";
    SoftwareType[SoftwareType["Metrology"] = 9007] = "Metrology";
    SoftwareType[SoftwareType["ServiceControlManager"] = 9008] = "ServiceControlManager";
})(SoftwareType || (SoftwareType = {}));
var ElectricUnit;
(function (ElectricUnit) {
    ElectricUnit[ElectricUnit["NoUnit"] = 0] = "NoUnit";
    ElectricUnit[ElectricUnit["mA"] = 1] = "mA";
    ElectricUnit[ElectricUnit["mV"] = 2] = "mV";
    ElectricUnit[ElectricUnit["Ohm"] = 3] = "Ohm";
    ElectricUnit[ElectricUnit["V"] = 4] = "V";
})(ElectricUnit || (ElectricUnit = {}));
var SensorType;
(function (SensorType) {
    SensorType[SensorType["NoSensor"] = 0] = "NoSensor";
    SensorType[SensorType["Ohm_Pt50_W1391"] = 1] = "Ohm_Pt50_W1391";
    SensorType[SensorType["Ohm_Pt100_W1391"] = 2] = "Ohm_Pt100_W1391";
    SensorType[SensorType["Ohm_Pt50_W1385"] = 3] = "Ohm_Pt50_W1385";
    SensorType[SensorType["Ohm_Pt100_W1385"] = 4] = "Ohm_Pt100_W1385";
    SensorType[SensorType["Ohm_Cu_50_W1428"] = 5] = "Ohm_Cu_50_W1428";
    SensorType[SensorType["Ohm_Cu_100_W1428"] = 6] = "Ohm_Cu_100_W1428";
    SensorType[SensorType["Ohm_Cu_50_W1426"] = 7] = "Ohm_Cu_50_W1426";
    SensorType[SensorType["Ohm_Cu_100_W1426"] = 8] = "Ohm_Cu_100_W1426";
    SensorType[SensorType["Ohm_Pt21"] = 9] = "Ohm_Pt21";
    SensorType[SensorType["Ohm_Cu23"] = 10] = "Ohm_Cu23";
    SensorType[SensorType["mV_K_TXA"] = 11] = "mV_K_TXA";
    SensorType[SensorType["mV_L_TXK"] = 12] = "mV_L_TXK";
    SensorType[SensorType["mV_N_THH"] = 13] = "mV_N_THH";
    //
    SensorType[SensorType["mV_Type_B"] = 14] = "mV_Type_B";
    SensorType[SensorType["mV_Type_E"] = 15] = "mV_Type_E";
    SensorType[SensorType["mV_Type_J"] = 16] = "mV_Type_J";
    SensorType[SensorType["mV_Type_K"] = 17] = "mV_Type_K";
    SensorType[SensorType["mV_Type_N"] = 18] = "mV_Type_N";
    SensorType[SensorType["mV_Type_R"] = 19] = "mV_Type_R";
    SensorType[SensorType["mV_Type_S"] = 20] = "mV_Type_S";
    SensorType[SensorType["mV_Type_T"] = 21] = "mV_Type_T";
    SensorType[SensorType["mV_Raw_Mul_8"] = 22] = "mV_Raw_Mul_8";
    SensorType[SensorType["mV_Raw_Mul_32"] = 23] = "mV_Raw_Mul_32";
    SensorType[SensorType["Ohm_Ni50_W1617"] = 24] = "Ohm_Ni50_W1617";
    SensorType[SensorType["Ohm_Ni100_W1617"] = 25] = "Ohm_Ni100_W1617";
    SensorType[SensorType["V_0_5"] = 26] = "V_0_5";
    SensorType[SensorType["V_m10_p10"] = 27] = "V_m10_p10";
    SensorType[SensorType["Ohm_Pt_a_391"] = 28] = "Ohm_Pt_a_391";
    SensorType[SensorType["Ohm_Pt_a_385"] = 29] = "Ohm_Pt_a_385";
    SensorType[SensorType["Ohm_Cu_a_428"] = 30] = "Ohm_Cu_a_428";
    SensorType[SensorType["Ohm_Cu_a_426"] = 31] = "Ohm_Cu_a_426";
    SensorType[SensorType["Ohm_Ni_a_617"] = 32] = "Ohm_Ni_a_617";
    SensorType[SensorType["Ohm_Raw"] = 33] = "Ohm_Raw";
})(SensorType || (SensorType = {}));
var OutputMode;
(function (OutputMode) {
    OutputMode[OutputMode["Plus0_Plus5_V"] = 0] = "Plus0_Plus5_V";
    OutputMode[OutputMode["Plus4_Plus20_mA"] = 1] = "Plus4_Plus20_mA";
    OutputMode[OutputMode["Minus10_Plus10_V"] = 2] = "Minus10_Plus10_V";
    OutputMode[OutputMode["Plus0_Plus5_mA"] = 3] = "Plus0_Plus5_mA";
    OutputMode[OutputMode["Plus0_Plus20_mA"] = 4] = "Plus0_Plus20_mA";
    OutputMode[OutputMode["Plus0_Plus24_mA"] = 5] = "Plus0_Plus24_mA";
})(OutputMode || (OutputMode = {}));
;
var UnitsConvertorErrorCode;
(function (UnitsConvertorErrorCode) {
    UnitsConvertorErrorCode[UnitsConvertorErrorCode["ErrorGeneric"] = 1] = "ErrorGeneric";
    UnitsConvertorErrorCode[UnitsConvertorErrorCode["LowLimitOutOfRange"] = 2] = "LowLimitOutOfRange";
    UnitsConvertorErrorCode[UnitsConvertorErrorCode["HighLimitOutOfRange"] = 3] = "HighLimitOutOfRange";
})(UnitsConvertorErrorCode || (UnitsConvertorErrorCode = {}));
function runConfigScript(configScript, confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) {
    //let funcStr = "(function (confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage){log.writeMessage(\"Hello\"); return true; })";
    //
    let funcStr = "(" + configScript + ")";
    let funcVar = eval(funcStr);
    if (funcVar(confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false) {
        return false;
    }
    return true;
}
// Strict mode part
//
"use strict";
let FamilyLMID = 0x1100;
let UartID = 0;
let LMNumberCount = 0;
//let configScriptVersion = 1;		// first logged version
//let configScriptVersion = 2;		// TuningDataSize in LM port has been changed to 716 (1432 / 2)
//let configScriptVersion = 3;		// AIM and AOM signal are now found not by place but by identifier, findSignalByPlace is not used.
//let configScriptVersion = 4;		// AIM filteringTime calculation algorithm has been changed
//let configScriptVersion = 5;		// LM-1 properties SubsysID and Channel have been renamed to SubsystemID and SubsystemChannel
//let configScriptVersion = 6;		// SubsystemChannel renamed to LMNumber
//let configScriptVersion = 7;		// MAC address calculation changed
//let configScriptVersion = 8;		// IP address of LAN controller is written even service is not specified
//let configScriptVersion = 9;		// IP address and port of LAN controller can't be zero - an error is reported
//let configScriptVersion = 10;		// AIM signals parameters algorithm has been changed
//let configScriptVersion = 11;		// If software for LM ethernet controller is not found, default values are used
//let configScriptVersion = 12;		// DiagDataSize changed to TxDiagDataSize
//let configScriptVersion = 13;		// AppDataSize changed to AppLANDataSize
//let configScriptVersion = 14;		// connectionCaption=>connectionID; strID=>equipmentID
//let configScriptVersion = 15;		// added text description fields for every value
//let configScriptVersion = 16;		// added dynamic custom module family
//let configScriptVersion = 17;		// LMNumber limit is 12
//let configScriptVersion = 18;		// i/o modules scripts moved to ConfiurationScript in presets
//let configScriptVersion = 19;		// i/o modules family checking has been removed
//let configScriptVersion = 21;		// DataUID for LM modules is taken from linked port
//let configScriptVersion = 22;		// added builder parameter to interrupt build
//let configScriptVersion = 23;		// added OverrideApp(Diag)DataWordCount properties processing
//let configScriptVersion = 25;		// buildThread has been replaced by builder, added buildNo
//let configScriptVersion = 26;		// added UniqueID computing
//let configScriptVersion = 27;		// First script that supports subsystems filtering
//let configScriptVersion : number = 28;	// Code is written using TypeScript
//let configScriptVersion: number = 29;		// Added module place checking
//let configScriptVersion: number = 30;		// ModuleID for LM is placed in .mct file
//let configScriptVersion: number = 31;		// Add LmDescriptionVersion to Storage Format frame
//let configScriptVersion: number = 32;		// Removed structure ModuleFirmwareCollection
//let configScriptVersion: number = 33;		// Changes in  ModuleFirmware functions, uartID added
//let configScriptVersion: number = 34;		// Changes in LmNumberCount calculation
//let configScriptVersion: number = 35;		// Add Software type checking
//let configScriptVersion: number = 36;		// Changes in App/DiagDataService processing
//let configScriptVersion: number = 37;		// Add setDataFloat function
//let configScriptVersion: number = 38;		// If TuningEnable/AppDataEnable/DiagDataEnable flag is false, IP address is zero
//let configScriptVersion: number = 39;		// Description is added for LmNumberCount and UniqueID
//let configScriptVersion: number = 40;		// Let is used instead of var
//let configScriptVersion: number = 41;		// ScriptDeviceObject is used
let configScriptVersion = 42; // DiagDataSize is written for i/o module frame
//
function main(builder, root, logicModules, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    if (logicModules.length != 0) {
        let subSysID = logicModules[0].propertyString("SubsystemID");
        log.writeMessage("Subsystem " + subSysID + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
    }
    for (let i = 0; i < logicModules.length; i++) {
        if (logicModules[i].moduleFamily == FamilyLMID) {
            LMNumberCount++;
        }
    }
    for (let i = 0; i < logicModules.length; i++) {
        if (logicModules[i].moduleFamily != FamilyLMID) {
            continue;
        }
        let result = module_lm_1(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
        if (result == false) {
            return false;
        }
        if (builder.jsIsInterruptRequested() == true) {
            return true;
        }
    }
    return true;
}
function setData8(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data) {
    if (channel != -1 && equpmentID.length > 0) {
        confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "8;" + caption + ";0x" + data.toString(16));
    }
    if (confFirmware.setData8(frameIndex, offset, data) == false) {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData8");
        return false;
    }
    return true;
}
function setData16(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data) {
    if (channel != -1 && equpmentID.length > 0) {
        confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "16;" + caption + ";0x" + data.toString(16));
    }
    if (confFirmware.setData16(frameIndex, offset, data) == false) {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData16");
        return false;
    }
    return true;
}
function setData32(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data) {
    if (channel != -1 && equpmentID.length > 0) {
        confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";0x" + data.toString(16));
    }
    if (confFirmware.setData32(frameIndex, offset, data) == false) {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData32");
        return false;
    }
    return true;
}
function setDataFloat(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data) {
    if (channel != -1 && equpmentID.length > 0) {
        confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";" + data);
    }
    if (confFirmware.setDataFloat(frameIndex, offset, data) == false) {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setDataFloat");
        return false;
    }
    return true;
}
function storeCrc64(confFirmware, log, channel, equpmentID, frameIndex, start, count, offset) {
    let result = confFirmware.storeCrc64(frameIndex, start, count, offset);
    confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);
    if (result == "") {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
    }
    return result;
}
function storeHash64(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data) {
    let result = confFirmware.storeHash64(frameIndex, offset, data);
    confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);
    if (result == "") {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
    }
    return result;
}
function ipToString(ip) {
    let ip0 = (ip >> 24) & 0xff;
    let ip1 = (ip >> 16) & 0xff;
    let ip2 = (ip >> 8) & 0xff;
    let ip3 = (ip) & 0xff;
    let result = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
    return result;
}
function truncate_to_int(x) {
    if (x > 0) {
        return Math.floor(x);
    }
    else {
        return Math.ceil(x);
    }
}
function valToADC(val, lowLimit, highLimit, lowADC, highADC) {
    if ((highLimit - lowLimit) == 0) {
        return 0; // to exclude division by zero
    }
    let res = (highADC - lowADC) * (val - lowLimit) / (highLimit - lowLimit) + lowADC;
    return Math.round(res);
}
function module_lm_1(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    if (module.moduleFamily == FamilyLMID) {
        let place = module.place;
        if (place != 0) {
            log.errCFG3002("Place", place, 0, 0, module.equipmentId);
            return false;
        }
        // Generate Configuration
        //
        return generate_lm_1_rev3(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
    }
    return false;
}
// Generate configuration for module LM-1
//
//
function generate_lm_1_rev3(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    let checkProperties = ["SubsystemID", "LMNumber", "AppLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID"];
    for (let cp = 0; cp < checkProperties.length; cp++) {
        if (module.propertyValue(checkProperties[cp]) == undefined) {
            log.errCFG3000(checkProperties[cp], module.equipmentId);
            return false;
        }
    }
    const MODULEID_LM1_SF00 = 0x1100;
    const MODULEID_LM1_SF01 = 0x1101;
    const MODULEID_LM1_SR01 = 0x11A0;
    const MODULEID_LM1_SR02 = 0x11A1;
    const MODULEID_LM1_SR03 = 0x11A2;
    const MODULEID_LM1_SR04 = 0x11B0;
    const MODULEID_LM8_SR10 = 0x11D0;
    // Variables
    //
    let subSysID = module.propertyString("SubsystemID");
    let LMNumber = module.propertyInt("LMNumber");
    let moduleId = module.moduleFamily + module.moduleVersion;
    // Constants
    //
    let frameSize = logicModuleDescription.FlashMemory_ConfigFramePayload;
    let frameCount = logicModuleDescription.FlashMemory_ConfigFrameCount;
    if (frameSize < 1016) {
        log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.equipmentId);
        return false;
    }
    if (frameCount < 78 /*2 + 19  frames * 4 channels*/) {
        log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 78, 65535, module.equipmentId);
        return false;
    }
    let uartId = 0x0102;
    let appWordsCount = module.propertyInt("AppLANDataSize");
    let diagWordsCount = logicModuleDescription.Memory_TxDiagDataSize;
    let ssKeyValue = subsystemStorage.ssKey(subSysID);
    if (ssKeyValue == -1) {
        log.errCFG3001(subSysID, module.equipmentId);
        return false;
    }
    let maxLMNumber = 12; // Can be changed!
    let configStartFrames = 2;
    let configFrameCount = 19; // number of frames in each configuration
    let ioModulesMaxCount = 14;
    if (LMNumber < 1 || LMNumber > maxLMNumber) {
        log.errCFG3002("System/LMNumber", LMNumber, 1, maxLMNumber, module.equipmentId);
        return false;
    }
    let descriptionVersion = 1;
    confFirmware.jsSetDescriptionFields(descriptionVersion, "EquipmentID;Frame;Offset;BitNo;Size;Caption;Value");
    confFirmware.writeLog("---\r\n");
    confFirmware.writeLog("Module: LM-1\r\n");
    confFirmware.writeLog("EquipmentID = " + module.equipmentId + "\r\n");
    confFirmware.writeLog("Subsystem ID = " + subSysID + "\r\n");
    confFirmware.writeLog("Key value = " + ssKeyValue + "\r\n");
    confFirmware.writeLog("ModuleID = " + moduleId + "\r\n");
    confFirmware.writeLog("UartID = " + uartId + "\r\n");
    confFirmware.writeLog("Frame size = " + frameSize + "\r\n");
    confFirmware.writeLog("LMNumber = " + LMNumber + "\r\n");
    confFirmware.writeLog("LMDescriptionNumber = " + logicModuleDescription.descriptionNumber() + "\r\n");
    // Configuration storage format
    //
    let frameStorageConfig = 1;
    let ptr = 0;
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Marker", 0xca70) == false) //CFG_Marker
     {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Version", 0x0001) == false) //CFG_Version
     {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
    ptr += 2;
    let ssKey = ssKeyValue << 6; //0000SSKEYY000000b
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "SubsystemKey", ssKey) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
    ptr += 2;
    let buildNo = confFirmware.buildNumber();
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "BuildNo", buildNo) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] BuildNo = " + buildNo + "\r\n");
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMDescriptionNumber", logicModuleDescription.descriptionNumber()) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMDescriptionNumber = " + logicModuleDescription.descriptionNumber() + "\r\n");
    ptr += 2;
    // reserved
    ptr += 4;
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMNumberCount", LMNumberCount) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMNumberCount = " + LMNumberCount + "\r\n");
    ptr += 2;
    let configIndexOffset = ptr + (LMNumber - 1) * (2 /*offset*/ + 4 /*reserved*/);
    let configFrame = configStartFrames + configFrameCount * (LMNumber - 1);
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, configIndexOffset, "ConfigStartFrame", configFrame) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");
    // Service information
    //
    confFirmware.writeLog("Writing service information.\r\n");
    let frameServiceConfig = configFrame;
    ptr = 0;
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "ServiceVersion", 0x0001) == false) //CFG_Ch_Vers
     {
        return false;
    }
    confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "UartID", uartId) == false) //CFG_Ch_Dtype == UARTID?
     {
        return false;
    }
    confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] uartId = " + uartId + "\r\n");
    ptr += 2;
    //Hash (UniqueID) will be counted later, write zero for future replacement
    confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] UniqueID = 0\r\n");
    ptr += 8;
    // I/O Modules configuration
    //
    confFirmware.writeLog("Writing I/O modules configuration.\r\n");
    let frameIOConfig = configFrame + 1;
    if (module.parent().isChassis() === false) {
        log.errCFG3042(module.equipmentId, module.uuid);
    }
    let parent = module.parent().toChassis();
    for (let i = 0; i < parent.childrenCount; i++) {
        if (builder.jsIsInterruptRequested() == true) {
            return true;
        }
        if (parent.child(i).isModule() == false) {
            continue;
        }
        let ioModule = parent.child(i).toModule();
        if (ioModule.moduleFamily == FamilyLMID) {
            continue;
        }
        let ioPlace = ioModule.place;
        if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
            log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioModule.equipmentId);
            return false;
        }
        let ioEquipmentID = ioModule.equipmentId;
        let checkProperties = ["ConfigurationScript"];
        for (let cp = 0; cp < checkProperties.length; cp++) {
            if (ioModule.propertyValue(checkProperties[cp]) == undefined) {
                log.errCFG3000(checkProperties[cp], ioEquipmentID);
                return false;
            }
        }
        let ioModuleFamily = ioModule.moduleFamily;
        let frame = frameIOConfig + ioPlace - 1;
        confFirmware.writeLog("Generating configuration for " + ioModule.caption + ": " + ioEquipmentID + " Place: " + ioModule.place + " Frame: " + frame + "\r\n");
        let configScript = ioModule.propertyString("ConfigurationScript");
        if (configScript.length != 0) {
            if (runConfigScript(configScript, confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false) {
                return false;
            }
        }
        let diagWordsIoCount = ioModule.propertyInt("TxDiagDataSize");
        if (diagWordsIoCount == null) {
            log.errCFG3000("TxDiagDataSize", ioEquipmentID);
            return false;
        }
        if (moduleId == MODULEID_LM1_SR03 ||
            moduleId == MODULEID_LM1_SR04 ||
            moduleId == MODULEID_LM8_SR10) {
            if ((diagWordsIoCount & 1) != 0) {
                diagWordsIoCount++; // Align to word
            }
            // I/o module diag data size
            //
            ptr = 1006;
            if (setData16(confFirmware, log, LMNumber, ioModule.equipmentId, frame, ptr, "DiagDataSize", diagWordsIoCount) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] DiagDataSize = " + diagWordsIoCount + "\r\n");
        }
        diagWordsCount += diagWordsIoCount;
    }
    let lanConfigFrame = frameIOConfig + ioModulesMaxCount;
    // Create LANs configuration
    //
    confFirmware.writeLog("Writing LAN configuration.\r\n");
    let lanFrame = lanConfigFrame;
    // Tuning
    //
    let ethernetcontrollerId = "_ETHERNET01";
    let ethernetControllerObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerId);
    if (ethernetControllerObject == null || ethernetControllerObject.isController() == false) {
        log.errCFG3004(module.equipmentId + ethernetcontrollerId, module.equipmentId);
        return false;
    }
    let ethernetController = ethernetControllerObject.toController();
    let checkTuningProperties = ["TuningServiceID", "TuningEnable", "TuningIP", "TuningPort", "OverrideTuningDataWordCount"];
    for (let cp = 0; cp < checkTuningProperties.length; cp++) {
        if (ethernetController.propertyValue(checkTuningProperties[cp]) == undefined) {
            log.errCFG3000(checkTuningProperties[cp], ethernetController.equipmentId);
            return false;
        }
    }
    confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");
    // Controller
    //
    let tuningWordsCount = 716;
    let tuningIP = 0;
    let tuningPort = 0;
    // Service
    //
    let tuningServiceIP = 0;
    let tuningServicePort = 0;
    let serviceID = ethernetController.propertyString("TuningServiceID");
    if (ethernetController.propertyBool("TuningEnable") == true) {
        tuningIP = ethernetController.propertyIP("TuningIP");
        tuningPort = ethernetController.propertyInt("TuningPort");
        let serviceObject = root.childByEquipmentId(serviceID);
        if (serviceObject == null || serviceObject.isSoftware() == false) {
            log.wrnCFG3008(serviceID, module.equipmentId);
        }
        else {
            // Check software type
            //
            let service = serviceObject.toSoftware();
            if (service.softwareType != SoftwareType.TuningService) {
                log.errCFG3017(ethernetController.equipmentId, "Type", service.equipmentId);
                return false;
            }
            //
            let checkTuningProperties = ["TuningDataIP", "TuningDataPort"];
            for (let cp = 0; cp < checkTuningProperties.length; cp++) {
                if (service.propertyValue(checkTuningProperties[cp]) == undefined) {
                    log.errCFG3000(checkTuningProperties[cp], service.equipmentId);
                    return false;
                }
            }
            tuningServiceIP = service.propertyIP("TuningDataIP");
            tuningServicePort = service.propertyInt("TuningDataPort");
        }
    }
    let controllerTuningWordsCount = tuningWordsCount;
    let tuningDataID = module.propertyValue("TuningLANDataUID");
    let overrideTuningWordsCount = ethernetController.propertyInt("OverrideTuningDataWordCount");
    if (overrideTuningWordsCount != -1) {
        controllerTuningWordsCount = overrideTuningWordsCount;
        tuningDataID = 0;
    }
    if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController, controllerTuningWordsCount, tuningIP, tuningPort, tuningServiceIP, tuningServicePort, tuningDataID, 0, 0, 0, 0, 0, 0) == false) //Subnet2 is not used
     {
        return false;
    }
    lanFrame++;
    // REG / DIAG
    //
    for (let i = 0; i < 2; i++) {
        ethernetcontrollerId = "_ETHERNET0" + (i + 2);
        ethernetControllerObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerId);
        if (ethernetControllerObject == null || ethernetControllerObject.isController() == false) {
            log.errCFG3004(module.equipmentId + ethernetcontrollerId, module.equipmentId);
            return false;
        }
        ethernetController = ethernetControllerObject.toController();
        let checkProperties = ["AppDataServiceID", "AppDataEnable", "AppDataIP", "AppDataPort",
            "DiagDataServiceID", "DiagDataEnable", "DiagDataIP", "DiagDataPort",
            "OverrideAppDataWordCount", "OverrideDiagDataWordCount"];
        for (let cp = 0; cp < checkProperties.length; cp++) {
            if (ethernetController.propertyValue(checkProperties[cp]) == undefined) {
                log.errCFG3000(checkProperties[cp], ethernetController.equipmentId);
                return false;
            }
        }
        confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");
        let servicesName = ["App", "Diag"];
        let ip = [0, 0];
        let port = [0, 0];
        let serviceIP = [0, 0];
        let servicePort = [0, 0];
        for (let s = 0; s < 2; s++) {
            // Service
            let serviceID = ethernetController.propertyString(servicesName[s] + "DataServiceID");
            if (ethernetController.propertyBool(servicesName[s] + "DataEnable") == true) {
                ip[s] = ethernetController.propertyIP(servicesName[s] + "DataIP");
                port[s] = ethernetController.propertyInt(servicesName[s] + "DataPort");
                let serviceObject = root.childByEquipmentId(serviceID);
                if (serviceObject == null || serviceObject.isSoftware() == false) {
                    log.wrnCFG3008(serviceID, module.equipmentId);
                    if (i == 0) // in Ethernet port 1, if service was not found, use default IP addresses
                     {
                        if (s == 0) // this is App
                         {
                            serviceIP[s] = 0xc0a80bfe; //	192.168.11.254
                            servicePort[s] = 13322;
                        }
                        if (s == 1) // this is Diag
                         {
                            serviceIP[s] = 0xc0a815fe; //	192.168.21.254
                            servicePort[s] = 13352;
                        }
                        if (serviceIP[s] != 0 && servicePort[s] != 0) {
                            log.wrnCFG3018(servicesName[s] + "DataService", ipToString(serviceIP[s]), servicePort[s], ethernetController.equipmentId);
                        }
                    }
                }
                else {
                    // Check software type
                    //
                    let service = serviceObject.toSoftware();
                    if ((s == 0 && service.softwareType != SoftwareType.AppDataService) ||
                        (s == 1 && service.softwareType != SoftwareType.DiagDataService)) {
                        log.errCFG3017(ethernetController.equipmentId, "Type", service.equipmentId);
                        return false;
                    }
                    let checkProperties = ["DataReceivingIP", "DataReceivingPort"];
                    for (let cp = 0; cp < checkProperties.length; cp++) {
                        if (service.propertyValue(servicesName[s] + checkProperties[cp]) == undefined) {
                            log.errCFG3000(servicesName[s] + checkProperties[cp], service.equipmentId);
                            return false;
                        }
                    }
                    serviceIP[s] = service.propertyIP(servicesName[s] + "DataReceivingIP");
                    servicePort[s] = service.propertyInt(servicesName[s] + "DataReceivingPort");
                }
            }
        }
        let regDataID = module.propertyValue("AppLANDataUID");
        let diagDataID = module.propertyValue("DiagLANDataUID");
        let controllerAppWordsCount = appWordsCount;
        let overrideRegWordsCount = ethernetController.propertyInt("OverrideAppDataWordCount");
        if (overrideRegWordsCount != -1) {
            controllerAppWordsCount = overrideRegWordsCount;
            regDataID = 0;
        }
        let controllerDiagWordsCount = diagWordsCount;
        let overrideDiagWordsCount = ethernetController.propertyInt("OverrideDiagDataWordCount");
        if (overrideDiagWordsCount != -1) {
            controllerDiagWordsCount = overrideDiagWordsCount;
            diagDataID = 0;
        }
        if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController, controllerAppWordsCount, ip[0], port[0], serviceIP[0], servicePort[0], regDataID, controllerDiagWordsCount, ip[1], port[1], serviceIP[1], servicePort[1], diagDataID) == false) {
            return false;
        }
        lanFrame++;
    }
    // Create TX/RX configuration
    //
    confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");
    let txRxConfigFrame = lanConfigFrame + 3;
    if (generate_lmTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
        return false;
    }
    // create UniqueID
    //
    let startFrame = configStartFrames + configFrameCount * (LMNumber - 1);
    let uniqueID = 0;
    for (let i = 0; i < configFrameCount; i++) {
        let crc = confFirmware.calcCrc32(startFrame + i, 0, frameSize);
        uniqueID ^= crc;
    }
    confFirmware.jsSetUniqueID(LMNumber, uniqueID);
    return true;
}
function generate_txRxIoConfig(confFirmware, equipmentID, LMNumber, frame, offset, log, flags, configFrames, dataFrames, moduleId) {
    // TxRx Block's configuration structure
    //
    let ptr = offset;
    confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] Flags = " + flags +
        "; [" + frame + ":" + (ptr + 2) + "] ConfigFrames = " + configFrames +
        "; [" + frame + ":" + (ptr + 4) + "] DataFrames = " + dataFrames +
        "; [" + frame + ":" + (ptr + 6) + "] ModuleId = " + moduleId + "\r\n");
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "TxRxFlags", flags) == false) // Flags word
     {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Configuration words quantity", configFrames) == false) // Configuration words quantity
     {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Data words quantity", dataFrames) == false) // Data words quantity
     {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "ModuleID", moduleId) == false) // Tx ID
     {
        return false;
    }
    ptr += 2;
    return true;
}
function generate_LANConfiguration(confFirmware, log, frame, module, ethernetController, regWordsCount, regIP, regPort, regServiceIP, regServicePort, regDataID, diagWordsCount, diagIP, diagPort, diagServiceIP, diagServicePort, diagDataID) {
    let ptr = 0;
    let moduleEquipmentID = module.equipmentId;
    let LMNumber = module.propertyInt("LMNumber");
    let controllerEquipmentID = ethernetController.equipmentId;
    //mac
    //
    let hashName = "S" + regIP + diagIP + moduleEquipmentID + regServiceIP + diagServiceIP;
    let hashList = confFirmware.calcHash64(hashName);
    let size = hashList.jsSize();
    if (size != 2) {
        log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
        return false;
    }
    let h0 = hashList.jsAt(0);
    let h1 = hashList.jsAt(1);
    let m1 = 0x4200;
    let m2 = h0 & 0x7fff;
    let m3 = (h0 >> 16) & 0x7fff;
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : MAC address of LM = " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + "\r\n");
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC1", m1) == false) {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC2", m2) == false) {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC3", m3) == false) {
        return false;
    }
    ptr += 2;
    // regIP
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : IP 1 = " + ipToString(regIP) + "\r\n");
    if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "IP 1", regIP) == false) {
        return false;
    }
    ptr += 4;
    // regPort
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 1 = " + regPort + "\r\n");
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1", regPort) == false) {
        return false;
    }
    ptr += 2;
    // diagIP
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : IP 2 = " + ipToString(diagIP) + "\r\n");
    if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "IP 2", diagIP) == false) {
        return false;
    }
    ptr += 4;
    // diagPort
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 2 = " + diagPort + "\r\n");
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2", regPort) == false) {
        return false;
    }
    ptr += 2;
    // regServiceIP
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service IP 1 = " + ipToString(regServiceIP) + "\r\n");
    if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service IP 1", regServiceIP) == false) {
        return false;
    }
    ptr += 4;
    // regServicePort
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service Port 1 = " + regServicePort + "\r\n");
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service Port 1", regServicePort) == false) {
        return false;
    }
    ptr += 2;
    // regWordsCount
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 1 words count = " + regWordsCount + "\r\n");
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1 words count", regWordsCount) == false) {
        return false;
    }
    ptr += 2;
    // diagServiceIP
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service IP 2 = " + ipToString(diagServiceIP) + "\r\n");
    if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service IP 2", diagServiceIP) == false) {
        return false;
    }
    ptr += 4;
    // diagServicePort
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service Port 2 = " + diagServicePort + "\r\n");
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service Port 2", diagServicePort) == false) {
        return false;
    }
    ptr += 2;
    // diagWordsCount
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 2 words count = " + diagWordsCount + "\r\n");
    if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2 words count", diagWordsCount) == false) {
        return false;
    }
    ptr += 2;
    // appDUID
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 1 DUID = " + regDataID + "\r\n");
    if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1 DUID", regDataID) == false) {
        return false;
    }
    ptr += 4;
    // diagDUID
    confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 2 DUID = " + diagDataID + "\r\n");
    if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2 DUID", diagDataID) == false) {
        return false;
    }
    ptr += 4;
    return true;
}
// function returns the amount of transmitting words
//
function generate_lmTxRxOptoConfiguration(confFirmware, log, frame, module, LMNumber, opticModuleStorage, logicModuleDescription) {
    if (module.propertyValue("EquipmentID") == undefined) {
        log.errCFG3000("EquipmentID", "Class_Module");
        return false;
    }
    let portCount = logicModuleDescription.OptoInterface_OptoPortCount;
    let txWordsCount = 0;
    for (let p = 0; p < portCount; p++) {
        let controllerID = module.equipmentId + "_OPTOPORT0";
        controllerID = controllerID + (p + 1);
        let controllerObject = module.childByEquipmentId(controllerID);
        if (controllerObject == null || controllerObject.isController() == false) {
            log.errCFG3004(controllerID, module.equipmentId);
            return false;
        }
        let controller = controllerObject.toController();
        let optoPort = opticModuleStorage.jsGetOptoPort(controller.equipmentId);
        if (optoPort == null) {
            continue;
        }
        if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0) {
            continue;
        }
        confFirmware.writeLog("    OptoPort " + controller.equipmentId + ": connection ID = " + optoPort.equipmentID() +
            " (" + optoPort.connectionID() + ")\r\n");
        let ptr = 0 + p * 2;
        let value = optoPort.txStartAddress();
        if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        ptr = 5 * 2 + p * 2;
        value = optoPort.txDataSizeW();
        if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        txWordsCount += value;
        ptr = 10 * 2 + p * 2;
        value = optoPort.portID();
        if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        ptr = 15 * 2 + p * 2;
        value = optoPort.rxDataSizeW();
        if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        let dataUID = 0;
        if (optoPort.isLinked() == true) {
            let linkedPort = optoPort.linkedPortID();
            let linkedOptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
            if (linkedOptoPort != null) {
                dataUID = linkedOptoPort.txDataID();
            }
        }
        ptr = 20 * 2 + p * 4;
        if (setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
    } // p
    return true;
}
