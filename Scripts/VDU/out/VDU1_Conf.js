"use strict";
var ConfigStruct;
(function (ConfigStruct) {
    let DeviceObjectType;
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
    })(DeviceObjectType = ConfigStruct.DeviceObjectType || (ConfigStruct.DeviceObjectType = {}));
    let SoftwareType;
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
    })(SoftwareType = ConfigStruct.SoftwareType || (ConfigStruct.SoftwareType = {}));
    let ElectricUnit;
    (function (ElectricUnit) {
        ElectricUnit[ElectricUnit["NoUnit"] = 0] = "NoUnit";
        ElectricUnit[ElectricUnit["mA"] = 1] = "mA";
        ElectricUnit[ElectricUnit["mV"] = 2] = "mV";
        ElectricUnit[ElectricUnit["Ohm"] = 3] = "Ohm";
        ElectricUnit[ElectricUnit["V"] = 4] = "V";
    })(ElectricUnit = ConfigStruct.ElectricUnit || (ConfigStruct.ElectricUnit = {}));
    let SensorType;
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
        SensorType[SensorType["mV_Type_L"] = 36] = "mV_Type_L";
        SensorType[SensorType["mV_Type_M"] = 37] = "mV_Type_M";
        SensorType[SensorType["mV_Raw_m1200_p1200"] = 38] = "mV_Raw_m1200_p1200";
    })(SensorType = ConfigStruct.SensorType || (ConfigStruct.SensorType = {}));
    let OutputMode;
    (function (OutputMode) {
        OutputMode[OutputMode["Plus0_Plus5_V"] = 0] = "Plus0_Plus5_V";
        OutputMode[OutputMode["Plus4_Plus20_mA"] = 1] = "Plus4_Plus20_mA";
        OutputMode[OutputMode["Minus10_Plus10_V"] = 2] = "Minus10_Plus10_V";
        OutputMode[OutputMode["Plus0_Plus5_mA"] = 3] = "Plus0_Plus5_mA";
        OutputMode[OutputMode["Plus0_Plus20_mA"] = 4] = "Plus0_Plus20_mA";
        OutputMode[OutputMode["Plus0_Plus24_mA"] = 5] = "Plus0_Plus24_mA";
    })(OutputMode = ConfigStruct.OutputMode || (ConfigStruct.OutputMode = {}));
    ;
    let UnitsConvertorErrorCode;
    (function (UnitsConvertorErrorCode) {
        UnitsConvertorErrorCode[UnitsConvertorErrorCode["ErrorGeneric"] = 1] = "ErrorGeneric";
        UnitsConvertorErrorCode[UnitsConvertorErrorCode["LowLimitOutOfRange"] = 2] = "LowLimitOutOfRange";
        UnitsConvertorErrorCode[UnitsConvertorErrorCode["HighLimitOutOfRange"] = 3] = "HighLimitOutOfRange";
    })(UnitsConvertorErrorCode = ConfigStruct.UnitsConvertorErrorCode || (ConfigStruct.UnitsConvertorErrorCode = {}));
    let LanControllerType;
    (function (LanControllerType) {
        LanControllerType[LanControllerType["Unknown"] = 0] = "Unknown";
        LanControllerType[LanControllerType["Tuning"] = 1] = "Tuning";
        LanControllerType[LanControllerType["AppData"] = 2] = "AppData";
        LanControllerType[LanControllerType["DiagData"] = 4] = "DiagData";
        LanControllerType[LanControllerType["AppAndDiagData"] = 6] = "AppAndDiagData";
        LanControllerType[LanControllerType["TuningAndAppAndDiagData"] = 7] = "TuningAndAppAndDiagData";
    })(LanControllerType = ConfigStruct.LanControllerType || (ConfigStruct.LanControllerType = {}));
})(ConfigStruct || (ConfigStruct = {}));
var ConfigLib;
(function (ConfigLib) {
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
    ConfigLib.runConfigScript = runConfigScript;
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
    ConfigLib.setData8 = setData8;
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
    ConfigLib.setData16 = setData16;
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
    ConfigLib.setData32 = setData32;
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
    ConfigLib.setDataFloat = setDataFloat;
    function storeCrc64(confFirmware, log, channel, equpmentID, frameIndex, start, count, offset) {
        let result = confFirmware.storeCrc64(frameIndex, start, count, offset);
        confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);
        if (result == "") {
            log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
        }
        return result;
    }
    ConfigLib.storeCrc64 = storeCrc64;
    function storeHash64(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data) {
        let result = confFirmware.storeHash64(frameIndex, offset, data);
        confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);
        if (result == "") {
            log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
        }
        return result;
    }
    ConfigLib.storeHash64 = storeHash64;
    function ipToString(ip) {
        let ip0 = (ip >> 24) & 0xff;
        let ip1 = (ip >> 16) & 0xff;
        let ip2 = (ip >> 8) & 0xff;
        let ip3 = (ip) & 0xff;
        let result = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
        return result;
    }
    ConfigLib.ipToString = ipToString;
    function truncate_to_int(x) {
        if (x > 0) {
            return Math.floor(x);
        }
        else {
            return Math.ceil(x);
        }
    }
    ConfigLib.truncate_to_int = truncate_to_int;
    function valToADC(val, lowLimit, highLimit, lowADC, highADC) {
        if ((highLimit - lowLimit) == 0) {
            return 0; // to exclude division by zero
        }
        let res = (highADC - lowADC) * (val - lowLimit) / (highLimit - lowLimit) + lowADC;
        return Math.round(res);
    }
    ConfigLib.valToADC = valToADC;
    function fillLanServiceData(confFirmware, softwareType, root, module, ethernetcontrollerId, lan, log) {
        // Build prefix
        let controllerPrefix;
        let servicePrefix;
        let overridePrefix;
        switch (softwareType) {
            case ConfigStruct.SoftwareType.TuningService:
                controllerPrefix = "Tuning";
                servicePrefix = "TuningData";
                overridePrefix = "Tuning";
                break;
            case ConfigStruct.SoftwareType.AppDataService:
                controllerPrefix = "AppData";
                servicePrefix = "AppDataReceiving";
                overridePrefix = "App";
                break;
            case ConfigStruct.SoftwareType.DiagDataService:
                controllerPrefix = "DiagData";
                servicePrefix = "DiagDataReceiving";
                overridePrefix = "Diag";
                break;
            default:
                log.writeError("fillLanServiceData: wrong software type");
                return false;
        }
        // Get ethernet controller
        let ethernetControllerObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerId);
        if (ethernetControllerObject == null || ethernetControllerObject.isController() == false) {
            log.errCFG3004(module.equipmentId + ethernetcontrollerId, module.equipmentId);
            return false;
        }
        let ethernetController = ethernetControllerObject.toController();
        let checkControllerProperties = [controllerPrefix + "ServiceID", controllerPrefix + "Enable", controllerPrefix + "IP", controllerPrefix + "Port", "Override" + overridePrefix + "DataWordCount"];
        for (let cp = 0; cp < checkControllerProperties.length; cp++) {
            if (ethernetController.propertyValue(checkControllerProperties[cp]) == undefined) {
                log.errCFG3000(checkControllerProperties[cp], ethernetController.equipmentId);
                return false;
            }
        }
        // Get data from services
        let serviceID = ethernetController.propertyString(controllerPrefix + "ServiceID");
        if (ethernetController.propertyBool(controllerPrefix + "Enable") == true) {
            // If Enable == true, take IP from service or default if service is not found
            lan.ip = ethernetController.propertyIP(controllerPrefix + "IP");
            lan.port = ethernetController.propertyInt(controllerPrefix + "Port");
            let serviceObject = root.childByEquipmentId(serviceID); // This can be software or controller
            let serviceSoftware = null; // This will be software
            if (serviceObject != null) {
                if (serviceObject.isController() == true) {
                    let parentObject = serviceObject.parent();
                    if (parentObject != null && parentObject.isSoftware() == true) {
                        serviceSoftware = parentObject.toSoftware();
                    }
                }
                else {
                    if (serviceObject.isSoftware() == true) {
                        serviceSoftware = serviceObject.toSoftware();
                    }
                }
            }
            if (serviceObject == null || serviceSoftware == null) {
                //Service was not found
                if (lan.serviceIP != 0 && lan.servicePort != 0) {
                    log.wrnCFG3018(controllerPrefix + "DataService", ConfigLib.ipToString(lan.serviceIP), lan.servicePort, ethernetController.equipmentId);
                }
                else {
                    log.wrnCFG3008(serviceID, module.equipmentId);
                }
            }
            else {
                // Check software type
                //
                if (serviceSoftware.softwareType != softwareType) {
                    log.errCFG3017(ethernetController.equipmentId, "Type", serviceSoftware.equipmentId);
                    return false;
                }
                // Take address from service
                let checkServiceProperties = [servicePrefix + "IP", servicePrefix + "Port"];
                for (let cp = 0; cp < checkServiceProperties.length; cp++) {
                    if (serviceObject.propertyValue(checkServiceProperties[cp]) == undefined) {
                        log.errCFG3000(checkServiceProperties[cp], serviceObject.equipmentId);
                        return false;
                    }
                }
                lan.serviceIP = serviceObject.propertyIP(servicePrefix + "IP");
                lan.servicePort = serviceObject.propertyInt(servicePrefix + "Port");
            }
            lan.dataID = module.propertyValue(overridePrefix + "LANDataUID");
            if (lan.dataID == undefined) {
                log.errCFG3000(overridePrefix + "LANDataUID", module.equipmentId);
                return false;
            }
            let overrideTuningWordsCount = ethernetController.propertyInt("Override" + overridePrefix + "DataWordCount");
            if (overrideTuningWordsCount != -1) {
                lan.wordsCount = overrideTuningWordsCount;
                lan.dataID = 0;
            }
        }
        else {
            // If Enable == false, set service ID is 0 even
            lan.dataID = 0;
            lan.wordsCount = 0;
            lan.serviceIP = 0;
            lan.servicePort = 0;
        }
        return true;
    }
    ConfigLib.fillLanServiceData = fillLanServiceData;
    function generate_LANConfiguration_v0(confFirmware, frame, module, ethernetControllerId, lan1, lan2, log) {
        let lan = [];
        lan.push(lan1);
        lan.push(lan2);
        let ptr = 0;
        let controllerEquipmentID = module.equipmentId + ethernetControllerId;
        let LMNumber = module.propertyInt("LMNumber");
        let m1 = 0;
        let m2 = 0;
        let m3 = 0;
        if (lan1.ip == 0 && lan2.ip == 0 && lan1.serviceIP == 0 && lan2.serviceIP == 0) {
            // mac is empty
            //
        }
        else {
            //mac
            //
            let hashName = "S";
            for (let i = 0; i < lan.length; i++) {
                hashName += lan[i].ip;
            }
            hashName += controllerEquipmentID;
            for (let i = 0; i < lan.length; i++) {
                hashName += lan[i].serviceIP;
            }
            let hashList = confFirmware.calcHash64(hashName);
            if (hashList.length != 2) {
                log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
                return false;
            }
            let h = (hashList[0] + hashList[1]);
            m1 = 0x4200;
            m2 = h & 0x7fff;
            m3 = (h >> 16) & 0x7fff;
            if (confFirmware.checkMacForUnique(m1, m2, m3) == false) {
                log.errINT1001("MAC address " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + " of " + controllerEquipmentID + " is not unique!");
            }
        }
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
        for (let i = 0; i < lan.length; i++) {
            // ip
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " IP = " + ipToString(lan[i].ip) + "\r\n");
            if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " IP", lan[i].ip) == false) {
                return false;
            }
            ptr += 4;
            // port
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Port = " + lan[i].port + "\r\n");
            if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Port", lan[i].port) == false) {
                return false;
            }
            ptr += 2;
        }
        if (lan.length == 1) {
            //	If only one LAN is used - skip LAN 2 data
            ptr += 4;
            ptr += 2;
        }
        for (let i = 0; i < lan.length; i++) {
            // ServiceIP
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Service IP = " + ipToString(lan[i].serviceIP) + "\r\n");
            if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Service IP", lan[i].serviceIP) == false) {
                return false;
            }
            ptr += 4;
            // ServicePort
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Service Port = " + lan[i].servicePort + "\r\n");
            if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Service Port = ", lan[i].servicePort) == false) {
                return false;
            }
            ptr += 2;
            // WordsCount
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Words Count = " + lan[i].wordsCount + "\r\n");
            if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Words Count = ", lan[i].wordsCount) == false) {
                return false;
            }
            ptr += 2;
        }
        if (lan.length == 1) {
            //	If only one LAN is used - skip LAN 2 data
            ptr += 4;
            ptr += 2;
            ptr += 2;
        }
        for (let i = 0; i < lan.length; i++) {
            // DUID
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " DUID = " + lan[i].dataID + "\r\n");
            if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " DUID", lan[i].dataID) == false) {
                return false;
            }
            ptr += 4;
        }
        if (lan.length == 1) {
            //	If only one LAN is used - skip LAN 2 data
            ptr += 4;
        }
        return true;
    }
    ConfigLib.generate_LANConfiguration_v0 = generate_LANConfiguration_v0;
    function generate_LANConfiguration_v2(confFirmware, lmNumber, frame, startPtr, module, ethernetControllerId, lan, log) {
        let ptr = startPtr;
        let controllerEquipmentID = module.equipmentId + ethernetControllerId;
        // Version
        //
        const version = 1;
        confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN Configuration format version = " + version + "\r\n");
        if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "Version", version) == false) {
            return false;
        }
        ptr += 2;
        // MAC address
        //
        let m1 = 0;
        let m2 = 0;
        let m3 = 0;
        let macIsEmpty = true;
        for (let i = 0; i < lan.length; i++) {
            if (lan[i].ip != 0 || lan[i].serviceIP != 0) {
                macIsEmpty = false;
                break;
            }
        }
        if (macIsEmpty == true) {
            // mac is empty
            //
        }
        else {
            // mac
            //
            let hashName = "S";
            for (let i = 0; i < lan.length; i++) {
                hashName += lan[i].ip;
            }
            hashName += controllerEquipmentID;
            for (let i = 0; i < lan.length; i++) {
                hashName += lan[i].serviceIP;
            }
            let hashList = confFirmware.calcHash64(hashName);
            if (hashList.length != 2) {
                log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
                return false;
            }
            let h = (hashList[0] + hashList[1]);
            m1 = 0x4200;
            m2 = h & 0x7fff;
            m3 = (h >> 16) & 0x7fff;
            if (confFirmware.checkMacForUnique(m1, m2, m3) == false) {
                log.errINT1001("MAC address " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + " of " + controllerEquipmentID + " is not unique!");
            }
        }
        confFirmware.writeLog("    [" + frame + ":" + ptr + "] : MAC address of LM = " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + "\r\n");
        if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "MAC1", m1) == false) {
            return false;
        }
        ptr += 2;
        if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "MAC2", m2) == false) {
            return false;
        }
        ptr += 2;
        if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "MAC3", m3) == false) {
            return false;
        }
        ptr += 2;
        for (let i = 0; i < lan.length; i++) {
            // WordOfFlags 
            let flags = 0;
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Flags = " + flags + "\r\n");
            if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Flags", flags) == false) {
                return false;
            }
            ptr += 2;
            // IP
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " IP = " + ipToString(lan[i].ip) + "\r\n");
            if (setData32(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " IP", lan[i].ip) == false) {
                return false;
            }
            ptr += 4;
            // Port
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Port = " + lan[i].port + "\r\n");
            if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Port", lan[i].port) == false) {
                return false;
            }
            ptr += 2;
            // ServiceIP
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Service IP = " + ipToString(lan[i].serviceIP) + "\r\n");
            if (setData32(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Service IP", lan[i].serviceIP) == false) {
                return false;
            }
            ptr += 4;
            // ServicePort
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Service Port = " + lan[i].servicePort + "\r\n");
            if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Service Port", lan[i].servicePort) == false) {
                return false;
            }
            ptr += 2;
            // WordsCount
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " words count = " + lan[i].wordsCount + "\r\n");
            if (setData16(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " words count", lan[i].wordsCount) == false) {
                return false;
            }
            ptr += 2;
            // DUID
            confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " DUID = " + lan[i].dataID + "\r\n");
            if (setData32(confFirmware, log, lmNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " DUID", lan[i].dataID) == false) {
                return false;
            }
            ptr += 4;
            ptr += 4; // Reserved
        }
        return true;
    }
    ConfigLib.generate_LANConfiguration_v2 = generate_LANConfiguration_v2;
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
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
            ptr = 5 * 2 + p * 2;
            value = optoPort.txDataSizeW();
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
            txWordsCount += value;
            ptr = 10 * 2 + p * 2;
            value = optoPort.portID();
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
            ptr = 15 * 2 + p * 2;
            value = optoPort.rxDataSizeW();
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
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
            if (ConfigLib.setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
        } // p
        return true;
    }
    ConfigLib.generate_lmTxRxOptoConfiguration = generate_lmTxRxOptoConfiguration;
    function generate_vduTxRxOptoConfiguration(confFirmware, log, frame, startPtr, module, LMNumber, opticModuleStorage, logicModuleDescription) {
        if (module.propertyValue("EquipmentID") == undefined) {
            log.errCFG3000("EquipmentID", "Class_Module");
            return false;
        }
        let portCount = logicModuleDescription.OptoInterface_OptoPortCount;
        let txWordsCount = 0;
        let ptr = startPtr;
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
            ptr += 2; // reserved
            let value = optoPort.txStartAddress();
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
            ptr += 2;
            value = optoPort.txDataSizeW();
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
            txWordsCount += value;
            ptr += 2;
            value = optoPort.portID();
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
            ptr += 2;
            value = optoPort.rxDataSizeW();
            if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
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
            if (ConfigLib.setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false) {
                return false;
            }
            confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
            ptr += 4;
            ptr += 6; // Reserved
        } // p
        return true;
    }
    ConfigLib.generate_vduTxRxOptoConfiguration = generate_vduTxRxOptoConfiguration;
    function generate_txRxIoConfig(confFirmware, equipmentID, LMNumber, frame, offset, log, flags, configFrames, dataFrames, moduleId) {
        // TxRx Block's configuration structure
        //
        let ptr = offset;
        confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] Flags = " + flags +
            "; [" + frame + ":" + (ptr + 2) + "] ConfigFrames = " + configFrames +
            "; [" + frame + ":" + (ptr + 4) + "] DataFrames = " + dataFrames +
            "; [" + frame + ":" + (ptr + 6) + "] ModuleId = " + moduleId.toString(16) + "h\r\n");
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
    ConfigLib.generate_txRxIoConfig = generate_txRxIoConfig;
})(ConfigLib || (ConfigLib = {}));
let FamilyVDUID = 0x1C00;
let UartID = 0;
let LMNumberCount = 0;
let configScriptVersion = 1;
//
function main(builder, root, logicModules, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    if (logicModules.length != 0) {
        log.writeMessage("Subsystem " + " VDU " + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
    }
    for (let i = 0; i < logicModules.length; i++) {
        if (logicModules[i].moduleFamily == FamilyVDUID) {
            LMNumberCount++;
        }
    }
    for (let i = 0; i < logicModules.length; i++) {
        if (logicModules[i].moduleFamily != FamilyVDUID) {
            continue;
        }
        let result = module_vdu_1(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
        if (result == false) {
            return false;
        }
        if (builder.jsIsInterruptRequested() == true) {
            return true;
        }
    }
    return true;
}
function module_vdu_1(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    if (module.moduleFamily == FamilyVDUID) {
        // Generate Configuration
        //
        return generate_vdu(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
    }
    return false;
}
// Generate configuration for module VDU
//
//
function generate_vdu(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription) {
    let checkProperties = ["AppLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID"];
    for (let cp = 0; cp < checkProperties.length; cp++) {
        if (module.propertyValue(checkProperties[cp]) == undefined) {
            log.errCFG3000(checkProperties[cp], module.equipmentId);
            return false;
        }
    }
    // Variables
    //
    let subSysID = module.equipmentId;
    let LMNumber = module.place;
    let moduleId = module.moduleFamily + module.moduleVersion;
    // Constants
    //
    let frameSize = logicModuleDescription.FlashMemory_ConfigFramePayload;
    let frameCount = logicModuleDescription.FlashMemory_ConfigFrameCount;
    if (frameSize < 1016) {
        log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.equipmentId);
        return false;
    }
    if (frameCount != 2) {
        log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 2, 2, module.equipmentId);
        return false;
    }
    let appWordsCount = module.propertyInt("AppLANDataSize");
    let diagWordsCount = logicModuleDescription.Memory_TxDiagDataSize;
    let ssKeyValue = subsystemStorage.ssKeyForVdu(subSysID);
    if (ssKeyValue == -1) {
        log.errCFG3001(subSysID, module.equipmentId);
        return false;
    }
    let maxLMNumber = 14; // Can be changed!
    let configFrame = 1;
    if (LMNumber < 1 || LMNumber > maxLMNumber) {
        log.errCFG3002("System/LMNumber", LMNumber, 1, maxLMNumber, module.equipmentId);
        return false;
    }
    let descriptionVersion = 1;
    confFirmware.jsSetDescriptionFields(descriptionVersion, "EquipmentID;Frame;Offset;BitNo;Size;Caption;Value");
    confFirmware.writeLog("---\r\n");
    confFirmware.writeLog("Module: VDU-1\r\n");
    confFirmware.writeLog("EquipmentID = " + module.equipmentId + "\r\n");
    confFirmware.writeLog("ModuleID = " + moduleId.toString(16) + "h\r\n");
    confFirmware.writeLog("LMDescriptionNumber = " + logicModuleDescription.descriptionNumber() + "\r\n");
    let ptr = 0;
    // Configuration storage format
    //
    const VDU_Cfg_Data_Version = 1;
    if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "VDU_Cfg_Data_Version", VDU_Cfg_Data_Version) == false) //CFG_Marker
     {
        return false;
    }
    confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] VDU_Cfg_Data_Version = " + VDU_Cfg_Data_Version + "\r\n");
    ptr += 2;
    // SS Key
    if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "ssKeyValue", ssKeyValue) == false) //ssKey
     {
        return false;
    }
    confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] ssKeyValue = " + ssKeyValue + "\r\n");
    ptr += 2;
    // UniqueID
    ptr += 8;
    const OptoQuantity = 8;
    if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "OptoQuantity", OptoQuantity) == false) //CFG_Version
     {
        return false;
    }
    confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] OptoQuantity = " + OptoQuantity + "\r\n");
    ptr += 2;
    ptr += 4; // Reserved
    // Create OPTO configuration
    //
    confFirmware.writeLog("Writing OPTO configuration.\r\n");
    if (ConfigLib.generate_vduTxRxOptoConfiguration(confFirmware, log, configFrame, ptr, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
        return false;
    }
    // Create LANs configuration
    //
    confFirmware.writeLog("Writing LAN configuration.\r\n");
    const lanConfigPtr = 89 * 2;
    const lanConfigSize = 40 * 2;
    ptr = lanConfigPtr;
    const maxLanControllerCount = 3;
    let LanQuantity = logicModuleDescription.Lan_ControllerCount;
    if (LanQuantity < 1 || LanQuantity > maxLanControllerCount) {
        log.writeError(module.equipmentId + ": wrong LAN controllers count (" + LanQuantity + "), expected 1.." + maxLanControllerCount);
        return false;
    }
    if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "lanControllerCount", LanQuantity) == false) //CFG_Version
     {
        return false;
    }
    confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] LanQuantity = " + LanQuantity + "\r\n");
    ptr += 2;
    ptr += 4; // Reserved
    let appAndDiagChannel = 0;
    for (let i = 0; i < LanQuantity; i++) {
        let lanPlace = logicModuleDescription.jsLanControllerPlace(i);
        if (lanPlace < 1 || lanPlace > maxLanControllerCount) {
            log.writeError(module.equipmentId + ": wrong LAN controller place in LM description (" + lanPlace + "), expected 1.." + maxLanControllerCount);
            return false;
        }
        let lanType = logicModuleDescription.jsLanControllerType(i);
        let ethernetcontrollerId = "_ETHERNET0" + lanPlace;
        confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");
        let emptyLan = {
            flags: 0,
            ip: 0,
            port: 0,
            serviceIP: 0,
            servicePort: 0,
            wordsCount: 0,
            dataID: 0
        };
        let appLan = {
            flags: 0,
            ip: 0,
            port: 0,
            serviceIP: 0,
            servicePort: 0,
            wordsCount: appWordsCount,
            dataID: 0
        };
        let diagLan = {
            flags: 0,
            ip: 0,
            port: 0,
            serviceIP: 0,
            servicePort: 0,
            wordsCount: diagWordsCount,
            dataID: 0
        };
        let tuningLan = {
            flags: 0,
            ip: 0,
            port: 0,
            serviceIP: 0,
            servicePort: 0,
            wordsCount: 0,
            dataID: 0
        };
        if (appAndDiagChannel == 0) {
            // Set default values for first App and Diag channel
            appLan.serviceIP = 0xc0a80bfe; //	192.168.11.254
            appLan.servicePort = 13322;
            diagLan.serviceIP = 0xc0a815fe; //	192.168.21.254
            diagLan.servicePort = 13352;
        }
        if (lanType == ConfigStruct.LanControllerType.AppAndDiagData) {
            if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.AppDataService, root, module, ethernetcontrollerId, appLan, log) == false) {
                return false;
            }
            if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.DiagDataService, root, module, ethernetcontrollerId, diagLan, log) == false) {
                return false;
            }
            let lans = [];
            lans.push(appLan);
            lans.push(diagLan);
            lans.push(tuningLan);
            const lanDataPtr = ptr + lanConfigSize * i;
            if (ConfigLib.generate_LANConfiguration_v2(confFirmware, LMNumber, configFrame, lanDataPtr, module, ethernetcontrollerId, lans, log) == false) {
                return false;
            }
            appAndDiagChannel++;
        }
    }
    // UniqueId and CRC are computer in ConfigurationBuilder
    return true;
}
