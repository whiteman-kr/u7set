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
    DeviceObjectType[DeviceObjectType["Signal"] = 8] = "Signal";
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
function runConfigScript(configScript, confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) {
    //var funcStr = "(function (confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage){log.writeMessage(\"Hello\"); return true; })";
    //
    var funcStr = "(" + configScript + ")";
    var funcVar = eval(funcStr);
    if (funcVar(confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false) {
        return false;
    }
    return true;
}
// Strict mode part
//
"use strict";
var FamilyBVB15ID = 0x5600;
//var configScriptVersion: number = 1;
//var configScriptVersion: number = 2;	//Changes in LMNumberCount calculation algorithm
var configScriptVersion = 3; //Added software type checking
var LMDescriptionNumber = 0;
//
function main(builder, root, logicModules, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    if (logicModules.length != 0) {
        var subSysID = logicModules[0].jsPropertyString("SubsystemID");
        log.writeMessage("Subsystem " + subSysID + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
    }
    var LMNumberCount = 0;
    for (var i = 0; i < logicModules.length; i++) {
        if (logicModules[i].jsPropertyInt("ModuleFamily") != FamilyBVB15ID) {
            continue;
        }
        var result = module_bvb15(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
        if (result == false) {
            return false;
        }
        LMNumberCount++;
        if (builder.jsIsInterruptRequested() == true) {
            return true;
        }
    }
    // LMNumberCount
    //
    var frameStorageConfig = 1;
    var ptr = 14;
    if (setData16(confFirmware, log, -1, "", frameStorageConfig, ptr, "LMNumberCount", LMNumberCount) == false) {
        return false;
    }
    confFirmware.writeLog("Subsystem " + subSysID + ", frame " + frameStorageConfig + ", offset " + ptr + ": LMNumberCount = " + LMNumberCount + "\r\n");
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
function storeCrc64(confFirmware, log, channel, equpmentID, frameIndex, start, count, offset) {
    var result = confFirmware.storeCrc64(frameIndex, start, count, offset);
    confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);
    if (result == "") {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
    }
    return result;
}
function storeHash64(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data) {
    var result = confFirmware.storeHash64(frameIndex, offset, data);
    confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);
    if (result == "") {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
    }
    return result;
}
function ipToString(ip) {
    var ip0 = (ip >> 24) & 0xff;
    var ip1 = (ip >> 16) & 0xff;
    var ip2 = (ip >> 8) & 0xff;
    var ip3 = (ip) & 0xff;
    var result = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
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
    var res = (highADC - lowADC) * (val - lowLimit) / (highLimit - lowLimit) + lowADC;
    return Math.round(res);
}
function module_bvb15(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    if (module.jsDeviceType() != DeviceObjectType.Module) {
        return false;
    }
    if (module.propertyValue("EquipmentID") == undefined) {
        log.errCFG3000("EquipmentID", "BVB-15");
        return false;
    }
    var checkProperties = ["ModuleFamily", "Place"];
    for (var cp = 0; cp < checkProperties.length; cp++) {
        if (module.propertyValue(checkProperties[cp]) == undefined) {
            log.errCFG3000(checkProperties[cp], module.jsPropertyString("EquipmentID"));
            return false;
        }
    }
    if (module.jsPropertyInt("ModuleFamily") == FamilyBVB15ID) {
        var place = module.jsPropertyInt("Place");
        if (place != 0) {
            log.errCFG3002("Place", place, 0, 0, module.jsPropertyString("EquipmentID"));
            return false;
        }
        // Generate Configuration
        //
        return generate_bvb15_rev1(builder, module, root, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
    }
    return false;
}
// Generate configuration for module BVB-15
//
//
function generate_bvb15_rev1(builder, module, root, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    if (module.propertyValue("EquipmentID") == undefined) {
        log.errCFG3000("EquipmentID", "BVB-15");
        return false;
    }
    var checkProperties = ["SubsystemID", "LMNumber", "SubsystemChannel", "AppLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID",
        "Bit0_TemperatureSensor1", "Bit1_TemperatureSensor2", "Bit2_TemperatureSensor3", "Bit3_E14", "Bit4_E15", "Bit5_E16", "Bit6_SimulationInputMode"];
    for (var cp = 0; cp < checkProperties.length; cp++) {
        if (module.propertyValue(checkProperties[cp]) == undefined) {
            log.errCFG3000(checkProperties[cp], module.jsPropertyString("EquipmentID"));
            return false;
        }
    }
    var equipmentID = module.jsPropertyString("EquipmentID");
    // Variables
    //
    var subSysID = module.jsPropertyString("SubsystemID");
    var LMNumber = module.jsPropertyInt("LMNumber");
    // Constants
    //
    var frameSize = logicModuleDescription.FlashMemory_ConfigFrameSize;
    var frameCount = logicModuleDescription.FlashMemory_ConfigFrameCount;
    if (frameSize < 1016) {
        log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.jsPropertyString("EquipmentID"));
        return false;
    }
    if (frameCount < 78 /*2 + 19  frames * 4 channels*/) {
        log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 78, 65535, module.jsPropertyString("EquipmentID"));
        return false;
    }
    var uartId = 0x1604;
    var appWordsCount = module.jsPropertyInt("AppLANDataSize");
    var diagWordsCount = logicModuleDescription.Memory_TxDiagDataSize;
    var ssKeyValue = subsystemStorage.ssKey(subSysID);
    if (ssKeyValue == -1) {
        log.errCFG3001(subSysID, equipmentID);
        return false;
    }
    var maxLMNumber = 62; // Can be changed!
    var configStartFrames = 2;
    var configFrameCount = 4; // number of frames in each configuration
    var ioModulesMaxCount = 14;
    if (LMNumber < 1 || LMNumber > maxLMNumber) {
        log.errCFG3002("System/LMNumber", LMNumber, 1, maxLMNumber, module.jsPropertyString("EquipmentID"));
        return false;
    }
    var descriptionVersion = 1;
    confFirmware.jsSetDescriptionFields(descriptionVersion, "EquipmentID;Frame;Offset;BitNo;Size;Caption;Value");
    confFirmware.writeLog("---\r\n");
    confFirmware.writeLog("Module: BVB-15\r\n");
    confFirmware.writeLog("EquipmentID = " + equipmentID + "\r\n");
    confFirmware.writeLog("Subsystem ID = " + subSysID + "\r\n");
    confFirmware.writeLog("Key value = " + ssKeyValue + "\r\n");
    confFirmware.writeLog("UartID = " + uartId + "\r\n");
    confFirmware.writeLog("Frame size = " + frameSize + "\r\n");
    confFirmware.writeLog("LMNumber = " + LMNumber + "\r\n");
    confFirmware.writeLog("LMDescriptionNumber = " + LMDescriptionNumber + "\r\n");
    // Configuration storage format
    //
    var frameStorageConfig = 1;
    var ptr = 0;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "Marker", 0xca70) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "Version", 0x0001) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
    ptr += 2;
    var ssKey = ssKeyValue << 6; //0000SSKEYY000000b
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "SubsystemKey", ssKey) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
    ptr += 2;
    var buildNo = confFirmware.buildNumber();
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "BuildNo", buildNo) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] BuildNo = " + buildNo + "\r\n");
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "LMDescriptionNumber", LMDescriptionNumber) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMDescriptionNumber = " + LMDescriptionNumber + "\r\n");
    ptr += 2;
    // reserved
    ptr += 4;
    // write LMNumberCount, if old value is less than current. If it is the same, output an error.
    //
    var oldLMNumberCount = confFirmware.data16(frameStorageConfig, ptr);
    if (oldLMNumberCount == LMNumber) {
        log.errCFG3003(LMNumber, module.jsPropertyString("EquipmentID"));
        return false;
    }
    if (oldLMNumberCount < LMNumber) {
        if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "LMNumberCount", LMNumber) == false) {
            return false;
        }
    }
    ptr += 2;
    var configIndexOffset = ptr + (LMNumber - 1) * (2 /*offset*/ + 4 /*reserved*/);
    var configFrame = configStartFrames + configFrameCount * (LMNumber - 1);
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, configIndexOffset, "ConfigStartFrame", configFrame) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");
    // Service information
    //
    confFirmware.writeLog("Writing service information.\r\n");
    var frameServiceConfig = configFrame;
    ptr = 0;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "ServiceVersion", 0x0001) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "UartID", uartId) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] uartId = " + uartId + "\r\n");
    ptr += 2;
    var hashString = storeHash64(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "SubSystemID Hash", subSysID);
    if (hashString == "") {
        return false;
    }
    confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] subSysID HASH-64 = 0x" + hashString + "\r\n");
    //Hash (UniqueID) will be counted later
    ptr += 8;
    var chassis = module.jsParent();
    for (var i = 0; i < chassis.childrenCount(); i++) {
        var ioModule = chassis.jsChild(i);
        if (ioModule.jsDeviceType() != DeviceObjectType.Module) {
            continue;
        }
        if (ioModule.jsPropertyInt("ModuleFamily") == FamilyBVB15ID) {
            continue;
        }
        var ioPlace = ioModule.jsPropertyInt("Place");
        if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
            log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioModule.jsPropertyString("EquipmentID"));
            return false;
        }
        if (ioModule.propertyValue("EquipmentID") == undefined) {
            log.errCFG3000("EquipmentID", "I/O_module");
            return false;
        }
        var ioEquipmentID = ioModule.jsPropertyString("EquipmentID");
        var diagWordsIoCount = ioModule.jsPropertyInt("TxDiagDataSize");
        if (diagWordsIoCount == null) {
            log.errCFG3000("TxDiagDataSize", ioEquipmentID);
            return false;
        }
        diagWordsCount += diagWordsIoCount;
    }
    // Create LANs configuration
    //
    var lanConfigFrame = configFrame + 1;
    confFirmware.writeLog("Writing LAN configuration.\r\n");
    var lanFrame = lanConfigFrame;
    var ip = [0, 0];
    var port = [0, 0];
    var serviceIP = [0, 0];
    var servicePort = [0, 0];
    var ethernetcontrollerID = "_ETHERNET01";
    var ethernetController = module.jsFindChildObjectByMask(equipmentID + ethernetcontrollerID);
    if (ethernetController == null) {
        log.errCFG3004(equipmentID + ethernetcontrollerID, equipmentID);
        return false;
    }
    var checkProperties = ["AppDataServiceID", "AppDataEnable", "AppDataIP", "AppDataPort",
        "DiagDataServiceID", "DiagDataEnable", "DiagDataIP", "DiagDataPort",
        "OverrideAppDataWordCount", "OverrideDiagDataWordCount"];
    for (var cp = 0; cp < checkProperties.length; cp++) {
        if (ethernetController.propertyValue(checkProperties[cp]) == undefined) {
            log.errCFG3000(checkProperties[cp], ethernetController.jsPropertyString("EquipmentID"));
            return false;
        }
    }
    confFirmware.writeLog("    Ethernet Controller " + equipmentID + ethernetcontrollerID + "\r\n");
    var servicesName = ["App", "Diag"];
    for (var s = 0; s < 2; s++) {
        // Controller
        ip[s] = ethernetController.jsPropertyIP(servicesName[s] + "DataIP");
        port[s] = ethernetController.jsPropertyInt(servicesName[s] + "DataPort");
        if (ip[s] == 0) {
            log.errCFG3011(servicesName[s] + "DataIP", ip[s], ethernetController.jsPropertyString("EquipmentID"));
            return false;
        }
        if (port[s] == 0) {
            log.errCFG3012(servicesName[s] + "DataPort", port[s], ethernetController.jsPropertyString("EquipmentID"));
            return false;
        }
        // Service
        var serviceID = ethernetController.jsPropertyString(servicesName[s] + "DataServiceID");
        if (ethernetController.jsPropertyBool(servicesName[s] + "DataEnable") == true) {
            var service = root.jsFindChildObjectByMask(serviceID);
            if (service == null) {
                log.wrnCFG3008(serviceID, module.jsPropertyString("EquipmentID"));
                if (s == 0) {
                    serviceIP[s] = 0xc0a80bfe; //	192.168.11.254
                    servicePort[s] = 13322;
                }
                if (s == 1) {
                    serviceIP[s] = 0xc0a815fe; //	192.168.21.254
                    servicePort[s] = 13323;
                }
                if (serviceIP[s] != 0 && servicePort[s] != 0) {
                    log.wrnCFG3018(servicesName[s] + "DataService", ipToString(serviceIP[s]), servicePort[s], ethernetController.jsPropertyString("EquipmentID"));
                }
            }
            else {
                if (service.propertyValue("Type") == undefined) {
                    log.errCFG3000("Type", service.jsPropertyString("EquipmentID"));
                    return false;
                }
                var softwareType = service.jsPropertyInt("Type");
                if ((s == 0 && softwareType != SoftwareType.AppDataService) ||
                    (s == 1 && softwareType != SoftwareType.DiagDataService)) {
                    log.errCFG3017(ethernetController.jsPropertyString("EquipmentID"), "Type", service.jsPropertyString("EquipmentID"));
                    return false;
                }
                //
                var serviceDataChannel = service.jsFindChildObjectByMask(serviceID + "_DATACH01");
                if (serviceDataChannel == null) {
                    log.errCFG3004(serviceID + "_DATACH01", equipmentID);
                    return false;
                }
                var checkProperties = ["DataReceivingIP", "DataReceivingPort"];
                for (var cp = 0; cp < checkProperties.length; cp++) {
                    if (serviceDataChannel.propertyValue(servicesName[s] + checkProperties[cp]) == undefined) {
                        log.errCFG3000(servicesName[s] + checkProperties[cp], serviceDataChannel.jsPropertyString("EquipmentID"));
                        return false;
                    }
                }
                serviceIP[s] = serviceDataChannel.jsPropertyIP(servicesName[s] + "DataReceivingIP");
                servicePort[s] = serviceDataChannel.jsPropertyInt(servicesName[s] + "DataReceivingPort");
            }
        }
    }
    var regDataID = module.propertyValue("AppLANDataUID");
    var diagDataID = module.propertyValue("DiagLANDataUID");
    var controllerAppWordsCount = appWordsCount;
    var overrideRegWordsCount = ethernetController.jsPropertyInt("OverrideAppDataWordCount");
    if (overrideRegWordsCount != -1) {
        controllerAppWordsCount = overrideRegWordsCount;
        regDataID = 0;
    }
    var controllerDiagWordsCount = diagWordsCount;
    var overrideDiagWordsCount = ethernetController.jsPropertyInt("OverrideDiagDataWordCount");
    if (overrideDiagWordsCount != -1) {
        controllerDiagWordsCount = overrideDiagWordsCount;
        diagDataID = 0;
    }
    if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController, controllerAppWordsCount, ip[0], port[0], serviceIP[0], servicePort[0], regDataID, controllerDiagWordsCount, ip[1], port[1], serviceIP[1], servicePort[1], diagDataID) == false) {
        return false;
    }
    lanFrame++;
    //}
    // Create TX/RX configuration
    //
    confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");
    var txRxConfigFrame = lanConfigFrame + 1;
    if (generate_lmTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
        return false;
    }
    // Create NIOS configuration
    //
    confFirmware.writeLog("Writing NIOS configuration.\r\n");
    var niosConfigFrame = txRxConfigFrame + 1;
    if (generate_niosConfiguration(confFirmware, log, niosConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
        return false;
    }
    // create UniqueID
    //
    /*var startFrame: number = configStartFrames + configFrameCount * (LMNumber - 1);

    var uniqueID: number = 0;

    for (var i: number = 0; i < configFrameCount; i++) {
        var crc: number = confFirmware.calcCrc32(startFrame + i, 0, frameSize);

        uniqueID ^= crc;
    }

    confFirmware.jsSetUniqueID(LMNumber, uniqueID);*/
    return true;
}
function generate_txRxIoConfig(confFirmware, equipmentID, LMNumber, frame, offset, log, flags, configFrames, dataFrames, txId) {
    // TxRx Block's configuration structure
    //
    var ptr = offset;
    confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] flags = " + flags +
        "; [" + frame + ":" + (ptr + 2) + "] configFrames = " + configFrames +
        "; [" + frame + ":" + (ptr + 4) + "] dataFrames = " + dataFrames +
        "; [" + frame + ":" + (ptr + 6) + "] txId = " + txId + "\r\n");
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "TxRxFlags", flags) == false) {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Configuration words quantity", configFrames) == false) {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Data words quantity", dataFrames) == false) {
        return false;
    }
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Tx ID", txId) == false) {
        return false;
    }
    ptr += 2;
    return true;
}
function generate_LANConfiguration(confFirmware, log, frame, module, ethernetController, regWordsCount, regIP, regPort, regServiceIP, regServicePort, regDataID, diagWordsCount, diagIP, diagPort, diagServiceIP, diagServicePort, diagDataID) {
    var ptr = 0;
    var moduleEquipmentID = module.jsPropertyString("EquipmentID");
    var LMNumber = module.jsPropertyInt("LMNumber");
    var controllerEquipmentID = ethernetController.jsPropertyString("EquipmentID");
    //mac
    //
    var hashName = "S" + regIP + diagIP + moduleEquipmentID + regServiceIP + diagServiceIP;
    var hashList = confFirmware.calcHash64(hashName);
    var size = hashList.jsSize();
    if (size != 2) {
        log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
        return false;
    }
    var h0 = hashList.jsAt(0);
    var h1 = hashList.jsAt(1);
    var m1 = 0x4200;
    var m2 = h0 & 0x7fff;
    var m3 = (h0 >> 16) & 0x7fff;
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
    var portCount = logicModuleDescription.OptoInterface_OptoPortCount;
    var txWordsCount = 0;
    for (var p = 0; p < portCount; p++) {
        var controllerID = module.jsPropertyString("EquipmentID") + "_OPTOPORT0";
        controllerID = controllerID + (p + 1);
        var controller = module.jsFindChildObjectByMask(controllerID);
        if (controller == null) {
            log.errCFG3004(controllerID, module.jsPropertyString("EquipmentID"));
            return false;
        }
        if (controller.propertyValue("EquipmentID") == undefined) {
            log.errCFG3000("EquipmentID", "Class_Controller");
            return false;
        }
        var controllerEquipmentID = controller.jsPropertyString("EquipmentID");
        var optoPort = opticModuleStorage.jsGetOptoPort(controllerEquipmentID);
        if (optoPort == null) {
            continue;
        }
        if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0) {
            continue;
        }
        confFirmware.writeLog("    OptoPort " + controllerEquipmentID + ": connection ID = " + optoPort.equipmentID() +
            " (" + optoPort.connectionID() + ")\r\n");
        var ptr = 0 + p * 2;
        var value = optoPort.txStartAddress();
        if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        ptr = 5 * 2 + p * 2;
        value = optoPort.txDataSizeW();
        if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        txWordsCount += value;
        ptr = 10 * 2 + p * 2;
        value = optoPort.portID();
        if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        ptr = 15 * 2 + p * 2;
        value = optoPort.rxDataSizeW();
        if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
        var dataUID = 0;
        if (optoPort.isLinked() == true) {
            var linkedPort = optoPort.linkedPortID();
            var linkedOptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
            if (linkedOptoPort != null) {
                dataUID = linkedOptoPort.txDataID();
            }
        }
        ptr = 20 * 2 + p * 4;
        if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
    } // p
    return true;
}
function generate_niosConfiguration(confFirmware, log, frame, module, LMNumber, opticModuleStorage, logicModuleDescription) {
    if (module.propertyValue("EquipmentID") == undefined) {
        log.errCFG3000("EquipmentID", "Class_Module");
        return false;
    }
    if (module.propertyValue("EquipmentID") == undefined) {
        log.errCFG3000("EquipmentID", "I/O_module");
        return false;
    }
    var equipmentID = module.propertyValue("EquipmentID");
    var FamilyBUMD1 = 0x5700;
    var FamilyBUMZ1 = 0x5800;
    var ioModulesMaxCount = 16;
    var chassis = module.jsParent();
    var ptr = 0;
    // Label
    var value = 0xbaed;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Label", value) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: Label = " + value + "\r\n");
    ptr += 2;
    // Version
    value = 0x0007;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Version", value) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: Version = " + value + "\r\n");
    ptr += 2;
    // CaseType
    value = 0x0000;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "CaseType", value) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: CaseType = " + value + "\r\n");
    ptr += 2;
    // SubblockType
    value = 0x0000;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "SubblockType", value) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: SubblockType = " + value + "\r\n");
    ptr += 2;
    // CaseNum
    value = 0x0000;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "CaseNum", value) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: CaseNum = " + value + "\r\n");
    ptr += 2;
    // SubblockNum
    value = module.jsPropertyInt("SubsystemChannel");
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "SubblockNum", value) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: SubblockNum = " + value + "\r\n");
    ptr += 2;
    // Blocks[]
    var blocksPtr = ptr + 2 + 2; // bptr is a pointer to Blocks [] array
    var blockPresent = [];
    for (var i = 0; i < ioModulesMaxCount; i++) {
        blockPresent[i] = false;
    }
    var blocksCount = 0;
    var blocksMask = 0;
    for (var i = 0; i < chassis.childrenCount(); i++) {
        var ioModule = chassis.jsChild(i);
        if (ioModule.jsDeviceType() != DeviceObjectType.Module) {
            continue;
        }
        if (ioModule.propertyValue("EquipmentID") == undefined) {
            log.errCFG3000("EquipmentID", "I/O_module");
            return false;
        }
        var customModuleFamily = ioModule.jsPropertyInt("CustomModuleFamily");
        if (customModuleFamily != FamilyBUMD1 && customModuleFamily != FamilyBUMZ1) {
            continue;
        }
        var ioEquipmentID = ioModule.jsPropertyString("EquipmentID");
        var checkProperties = ["ModuleVersion", "Place", "PresetName", "ConfigurationScript", "TxDiagDataSize", "TxAppDataSize"];
        for (var cp = 0; cp < checkProperties.length; cp++) {
            if (ioModule.propertyValue(checkProperties[cp]) == undefined) {
                log.errCFG3000(checkProperties[cp], ioEquipmentID);
                return false;
            }
        }
        var ioPlace = ioModule.jsPropertyInt("Place");
        if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
            log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioEquipmentID);
            return false;
        }
        var zeroIoPlace = ioPlace - 1;
        blockPresent[zeroIoPlace] = true;
        blocksCount++;
        blocksMask |= (1 << zeroIoPlace);
        var blockPtr = blocksPtr + (zeroIoPlace * 4 * 2);
        // Place
        if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "Module Place", ioPlace) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: Module Place = " + ioPlace + "\r\n");
        blockPtr += 2;
        // Id
        var moduleVersion = ioModule.jsPropertyInt("ModuleVersion");
        value = customModuleFamily | moduleVersion;
        if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "ID", value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: ID = " + value + "\r\n");
        blockPtr += 2;
        // TxDiagDataSize
        var diagWordsIoCount = ioModule.jsPropertyInt("TxDiagDataSize");
        if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "DiagDataSize", diagWordsIoCount) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: DiagDataSize = " + diagWordsIoCount + "\r\n");
        blockPtr += 2;
        // TxAppDataSize
        var appWordsIoCount = ioModule.jsPropertyInt("TxAppDataSize");
        if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "AppDataSize", appWordsIoCount) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: AppDataSize = " + appWordsIoCount + "\r\n");
        blockPtr += 2;
    }
    for (var i = 0; i < ioModulesMaxCount; i++) {
        if (blockPresent[i] == true) {
            continue;
        }
        var blockPtr = blocksPtr + (i * 4 * 2);
        // Place
        value = 0xffff;
        if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "Module Place (reserved)", value) == false) {
            return false;
        }
        confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: Module Place (reserved) = " + value + "\r\n");
        blockPtr += 2;
    }
    // ModulesCount
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "ModulesCount", blocksCount) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: ModulesCount = " + blocksCount + "\r\n");
    ptr += 2;
    // ModulesMask
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "ModulesMask", blocksMask) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: ModulesMask = " + blocksMask + "\r\n");
    ptr = 144;
    // Checks
    var Checks = 0;
    if (module.jsPropertyBool("Bit0_TemperatureSensor1") == true) {
        Checks |= (1 << 0);
    }
    if (module.jsPropertyBool("Bit1_TemperatureSensor2") == true) {
        Checks |= (1 << 1);
    }
    if (module.jsPropertyBool("Bit2_TemperatureSensor3") == true) {
        Checks |= (1 << 2);
    }
    if (module.jsPropertyBool("Bit3_E14") == true) {
        Checks |= (1 << 3);
    }
    if (module.jsPropertyBool("Bit4_E15") == true) {
        Checks |= (1 << 4);
    }
    if (module.jsPropertyBool("Bit5_E16") == true) {
        Checks |= (1 << 5);
    }
    if (module.jsPropertyBool("Bit6_SimulationInputMode") == true) {
        Checks |= (1 << 6);
    }
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Checks", Checks) == false) {
        return false;
    }
    confFirmware.writeLog("    [" + frame + ":" + ptr + "]: Checks = " + Checks + "\r\n");
    ptr += 2;
    return true;
}
