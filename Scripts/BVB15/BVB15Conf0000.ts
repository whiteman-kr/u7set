// Non-strict mode part
//

enum DeviceObjectType {
	Root = 0,
	System,
	Rack,
	Chassis,
	Module,
	Controller,
	Workstation,
	Software,
	Signal
}


interface Builder {
	jsIsInterruptRequested(): boolean;
	jsBuildNo(): number;
}

interface DeviceObject {
	propertyValue(name: string): any;

	jsDeviceType(): DeviceObjectType;
	jsPropertyInt(name: string): number;
	jsPropertyBool(name: string): boolean;
	jsPropertyString(name: string): string;
	jsPropertyIP(name: string): number;

	jsParent(): DeviceObject;
	jsChild(index: number): DeviceObject;

	childrenCount(): number;

	jsFindChildObjectByMask(equipmentID: string): DeviceObject;
}

interface JsVariantList {
	jsSize(): number;
	jsAt(index: number): number;
}

interface ModuleFirmware {
	jsAddDescription(channel: number, description: string): void;

	setData8(frameIndex: number, offset: number, data: number): boolean;
	setData16(frameIndex: number, offset: number, data: number): boolean;
	setData32(frameIndex: number, offset: number, data: number): boolean;

	storeCrc64(frameIndex: number, start: number, count: number, offset: number): string;
	storeHash64(frameIndex: number, offset: number, dataString: string): string;

	data8(frameIndex: number, offset: number): number;
	data16(frameIndex: number, offset: number): number;
	data32(frameIndex: number, offset: number): number;

	writeLog(message: string): void;

	jsSetDescriptionFields(descriptionVersion: number, description: string): void;

	calcCrc32(frameIndex: number, start: number, count: number): number;
	jsSetUniqueID(LMNumber: number, uniqueID: number): void;
	calcHash64(dataString: string): JsVariantList;

}

interface ModuleFirmwareCollection {

	jsGet(caption: string, subSysID: string, ssKeyValue: number, uartId: number, frameSize: number, frameCount: number, LMDescriptionNumber: number): ModuleFirmware;
}

interface IssueLogger {
	writeMessage(message: string): void;
	writeWarning(message: string): void;
	writeError(message: string): void;

	errCFG3000(propertyName: string, equipmentID: string): void;
	errCFG3001(subSysID: string, module: string): void;
	errCFG3002(name: string, value: number, min: number, max: number, module: string): void;
	errCFG3003(LMNumber: number, module: string): void;
	errCFG3004(controllerID: string, module: string): void;
	wrnCFG3008(softwareID: string, module: string): void;
	errCFG3011(addressProperty: string, address: number, controller: string): void;
	errCFG3012(portProperty: string, port: number, controller: string): void;
	wrnCFG3018(propertyName: string, ip: string, port: number, controller: string): void;

}

interface SignalSet {
}

interface SubsystemStorage {
	ssKey(subSysID: string): number;
}

interface OptoPort {
	connectionID(): string;
	equipmentID(): string;
	linkedPortID(): string;

	isLinked(): boolean;

	portID(): number;

	txStartAddress(): number;
	txDataID(): number;

	txDataSizeW(): number;
	rxDataSizeW(): number;
}

interface OptoModuleStorage {
	jsGetOptoPort(controllerEquipmentID: string): OptoPort;
}

interface LogicModule {
	jsConfigurationStringFile(): string;

	FlashMemory_ConfigFrameSize: number;
	FlashMemory_ConfigFrameCount: number;
	Memory_TxDiagDataSize: number;
	OptoInterface_OptoPortCount: number;
}

function runConfigScript(configScript: string, confFirmware: ModuleFirmware, ioModule: DeviceObject, LMNumber: number, frame: number, log: IssueLogger, signalSet: SignalSet, opticModuleStorage: OptoModuleStorage): boolean {
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

var FamilyBVB15ID: number = 0x5600;

var configScriptVersion: number = 1;

var LMDescriptionNumber: number = 0;

//

function main(builder: Builder, root: DeviceObject, logicModules: DeviceObject[], confCollection: ModuleFirmwareCollection,
	log: IssueLogger, signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule): boolean {

	if (logicModules.length != 0) {
		var subSysID: string = logicModules[0].jsPropertyString("SubsystemID");
		log.writeMessage("Subsystem " + subSysID + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
	}

	for (var i: number = 0; i < logicModules.length; i++) {

		if (logicModules[i].jsPropertyInt("ModuleFamily") != FamilyBVB15ID) {
			continue;
		}

		var result: boolean = module_bvb15(builder, root, logicModules[i], confCollection, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
		if (result == false) {
			return false;
		}

		if (builder.jsIsInterruptRequested() == true) {
			return true;
		}

	}

	for (var i: number = 0; i < logicModules.length; i++) {

		if (logicModules[i].jsPropertyInt("ModuleFamily") != FamilyBVB15ID) {
			continue;
		}

		var result: boolean = module_bvb15_1_statistics(builder, logicModules[i], confCollection, log, subsystemStorage, logicModuleDescription);
		if (result == false) {
			return false;
		}

		if (builder.jsIsInterruptRequested() == true) {
			return true;
		}
	}

	return true;
}

function setData8(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "8;" + caption + ";0x" + data.toString(16));

	if (confFirmware.setData8(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData8");
		return false;
	}
	return true;
}

function setData16(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "16;" + caption + ";0x" + data.toString(16));

	if (confFirmware.setData16(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData16");
		return false;
	}
	return true;
}

function setData32(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";0x" + data.toString(16));

	if (confFirmware.setData32(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData32");
		return false;
	}
	return true;
}

function storeCrc64(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, start: number, count: number, offset: number): string {
	var result: string = confFirmware.storeCrc64(frameIndex, start, count, offset);

	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);

	if (result == "") {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
	}
	return result;
}

function storeHash64(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: string): string {
	var result: string = confFirmware.storeHash64(frameIndex, offset, data);

	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);

	if (result == "") {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
	}
	return result;
}

function ipToString(ip: number): string {
	var ip0: number = (ip >> 24) & 0xff;
	var ip1: number = (ip >> 16) & 0xff;
	var ip2: number = (ip >> 8) & 0xff;
	var ip3: number = (ip) & 0xff;
	var result: string = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
	return result;
}

function truncate_to_int(x: number): number {
	if (x > 0) {
		return Math.floor(x);
	}
	else {
		return Math.ceil(x);
	}
}

function valToADC(val: number, lowLimit: number, highLimit: number, lowADC: number, highADC: number): number {
	if ((highLimit - lowLimit) == 0) {
		return 0;		// to exclude division by zero
	}

	var res: number = (highADC - lowADC) * (val - lowLimit) / (highLimit - lowLimit) + lowADC;

	return Math.round(res);
}

function module_bvb15(builder: Builder, root: DeviceObject, module: DeviceObject, confCollection: ModuleFirmwareCollection, log: IssueLogger,
	signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule): boolean {
	if (module.jsDeviceType() != DeviceObjectType.Module) {
		return false;
	}

	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "BVB-15");
		return false;
	}

	var checkProperties: string[] = ["ModuleFamily", "Place"];
	for (var cp: number = 0; cp < checkProperties.length; cp++) {
		if (module.propertyValue(checkProperties[cp]) == undefined) {
			log.errCFG3000(checkProperties[cp], module.jsPropertyString("EquipmentID"));
			return false;
		}
	}

	if (module.jsPropertyInt("ModuleFamily") == FamilyBVB15ID) {
		var place: number = module.jsPropertyInt("Place");

		if (place != 0) {
			log.errCFG3002("Place", place, 0, 0, module.jsPropertyString("EquipmentID"));
			return false;
		}

		// Generate Configuration
		//
		return generate_bvb15_rev1(builder, module, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
	}

	return false;
}

function module_bvb15_1_statistics(builder: Builder, module: DeviceObject, confCollection: ModuleFirmwareCollection, log: IssueLogger,
	subsystemStorage: SubsystemStorage, logicModuleDescription: LogicModule) {
	if (module.jsDeviceType() != DeviceObjectType.Module) {
		return false;
	}

	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "BVB-15");
		return false;
	}

	var checkProperties: string[] = ["ModuleFamily", "SubsystemID"];
	for (var cp: number = 0; cp < checkProperties.length; cp++) {
		if (module.propertyValue(checkProperties[cp]) == undefined) {
			log.errCFG3000(checkProperties[cp], module.jsPropertyString("EquipmentID"));
			return false;
		}
	}

	if (module.jsPropertyInt("ModuleFamily") == FamilyBVB15ID) {

		var checkProperties: string[] = ["LMNumber"];
		for (var cp: number = 0; cp < checkProperties.length; cp++) {
			if (module.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], module.jsPropertyString("EquipmentID"));
				return false;
			}
		}

		// Generate Configuration
		//
		// Variables
		//
		var subSysID: string = module.jsPropertyString("SubsystemID");
		var LMNumber: number = module.jsPropertyInt("LMNumber");

		var frameSize: number = logicModuleDescription.FlashMemory_ConfigFrameSize;
		var frameCount: number = logicModuleDescription.FlashMemory_ConfigFrameCount;

		var uartId: number = 0x1604;                   // Check it !!!!

		var ssKeyValue: number = subsystemStorage.ssKey(subSysID);
		if (ssKeyValue == -1) {
			log.errCFG3001(subSysID, module.jsPropertyString("EquipmentID"));
			return false;
		}

		var configStartFrames: number = 2;
		var configFrameCount: number = 19;          // number of frames in each configuration

		var confFirmware: ModuleFirmware = confCollection.jsGet("BVB-15", subSysID, ssKeyValue, uartId, frameSize, frameCount, LMDescriptionNumber);

		var frameStorageConfig: number = 1;
		var ptr: number = 14;

		var LMNumberCount: number = confFirmware.data16(frameStorageConfig, ptr);
		confFirmware.writeLog("---\r\n");
		confFirmware.writeLog("BVB-15 for subsystem " + subSysID + ", LMNumber " + LMNumber + ": Frame " + frameStorageConfig + ", offset " + ptr + ": LMNumberCount = " + LMNumberCount + "\r\n");
		return true;
	}

	return false;
}

// Generate configuration for module BVB-15
//
//
function generate_bvb15_rev1(builder: Builder, module: DeviceObject, root: DeviceObject, confCollection: ModuleFirmwareCollection, log: IssueLogger,
	signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {
	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "BVB-15");
		return false;
	}

	var checkProperties: string[] = ["SubsystemID", "LMNumber", "SubsystemChannel", "AppLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID",
		"Bit0_TemperatureSensor1", "Bit1_TemperatureSensor2", "Bit2_TemperatureSensor3", "Bit3_E14", "Bit4_E15", "Bit5_E16", "Bit6_SimulationInputMode"];
	for (var cp: number = 0; cp < checkProperties.length; cp++) {
		if (module.propertyValue(checkProperties[cp]) == undefined) {
			log.errCFG3000(checkProperties[cp], module.jsPropertyString("EquipmentID"));
			return false;
		}
	}

	var equipmentID: string = module.jsPropertyString("EquipmentID");

	// Variables
	//
	var subSysID: string = module.jsPropertyString("SubsystemID");
	var LMNumber: number = module.jsPropertyInt("LMNumber");

	// Constants
	//
	var frameSize: number = logicModuleDescription.FlashMemory_ConfigFrameSize;
	var frameCount: number = logicModuleDescription.FlashMemory_ConfigFrameCount;

	if (frameSize < 1016) {
		log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.jsPropertyString("EquipmentID"));
		return false;
	}

	if (frameCount < 78 /*2 + 19  frames * 4 channels*/) {
		log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 78, 65535, module.jsPropertyString("EquipmentID"));
		return false;
	}

	var uartId: number = 0x1604;

	var appWordsCount: number = module.jsPropertyInt("AppLANDataSize");
	var diagWordsCount: number = logicModuleDescription.Memory_TxDiagDataSize;

	var ssKeyValue: number = subsystemStorage.ssKey(subSysID);
	if (ssKeyValue == -1) {
		log.errCFG3001(subSysID, equipmentID);
		return false;
	}

	var maxLMNumber: number = 62;               // Can be changed!
	var configStartFrames: number = 2;
	var configFrameCount: number = 4;          // number of frames in each configuration
	var ioModulesMaxCount: number = 14;

	if (LMNumber < 1 || LMNumber > maxLMNumber) {
		log.errCFG3002("System/LMNumber", LMNumber, 1, maxLMNumber, module.jsPropertyString("EquipmentID"));
		return false;
	}

	var confFirmware: ModuleFirmware = confCollection.jsGet("BVB-15", subSysID, ssKeyValue, uartId, frameSize, frameCount, LMDescriptionNumber);

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
	var frameStorageConfig: number = 1;
	var ptr: number = 0;

	if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "Marker", 0xca70) == false)     //CFG_Marker
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "Version", 0x0001) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
	ptr += 2;


	var ssKey: number = ssKeyValue << 6;             //0000SSKEYY000000b
	if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "SubsystemKey", ssKey) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
	ptr += 2;

	var buildNo: number = builder.jsBuildNo();
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
	var oldLMNumberCount: number = confFirmware.data16(frameStorageConfig, ptr);

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

	var configIndexOffset: number = ptr + (LMNumber - 1) * (2/*offset*/ + 4/*reserved*/);
	var configFrame: number = configStartFrames + configFrameCount * (LMNumber - 1);

	if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, configIndexOffset, "ConfigStartFrame", configFrame) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");

	// Service information
	//
	confFirmware.writeLog("Writing service information.\r\n");

	var frameServiceConfig: number = configFrame;
	ptr = 0;
	if (setData16(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "ServiceVersion", 0x0001) == false)   //CFG_Ch_Vers
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "UartID", uartId) == false)  //CFG_Ch_Dtype == UARTID?
	{
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

	var chassis: DeviceObject = module.jsParent();

	for (var i: number = 0; i < chassis.childrenCount(); i++) {

		var ioModule: DeviceObject = chassis.jsChild(i);

		if (ioModule.jsDeviceType() != DeviceObjectType.Module) {
			continue;
		}

		if (ioModule.jsPropertyInt("ModuleFamily") == FamilyBVB15ID) {
			continue;
		}

		var ioPlace: number = ioModule.jsPropertyInt("Place");
		if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
			log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioModule.jsPropertyString("EquipmentID"));
			return false;
		}

		if (ioModule.propertyValue("EquipmentID") == undefined) {
			log.errCFG3000("EquipmentID", "I/O_module");
			return false;
		}

		var ioEquipmentID: string = ioModule.jsPropertyString("EquipmentID");

		var diagWordsIoCount: number = ioModule.jsPropertyInt("TxDiagDataSize");
		if (diagWordsIoCount == null) {
			log.errCFG3000("TxDiagDataSize", ioEquipmentID);
			return false;
		}

		diagWordsCount += diagWordsIoCount;
	}

	// Create LANs configuration
	//
	var lanConfigFrame: number = configFrame + 1;

	confFirmware.writeLog("Writing LAN configuration.\r\n");

	var lanFrame: number = lanConfigFrame;

	var ip: number[] = [0, 0];
	var port: number[] = [0, 0];

	var serviceIP: number[] = [0, 0];
	var servicePort: number[] = [0, 0];


	var ethernetcontrollerID = "_ETHERNET01";
	var ethernetController = module.jsFindChildObjectByMask(equipmentID + ethernetcontrollerID);
	if (ethernetController == null) {
		log.errCFG3004(equipmentID + ethernetcontrollerID, equipmentID);
		return false;
	}
	var checkProperties: string[] = ["AppDataServiceID", "AppDataEnable", "AppDataIP", "AppDataPort",
		"DiagDataServiceID", "DiagDataEnable", "DiagDataIP", "DiagDataPort",
		"OverrideAppDataWordCount", "OverrideDiagDataWordCount"];
		
	for (var cp: number = 0; cp < checkProperties.length; cp++) {
		if (ethernetController.propertyValue(checkProperties[cp]) == undefined) {
			log.errCFG3000(checkProperties[cp], ethernetController.jsPropertyString("EquipmentID"));
			return false;
		}
	}
	confFirmware.writeLog("    Ethernet Controller " + equipmentID + ethernetcontrollerID + "\r\n");

	var servicesName: string[] = ["App", "Diag"];

	for (var s: number = 0; s < 2; s++) {
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
		var serviceID: string = ethernetController.jsPropertyString(servicesName[s] + "DataServiceID");

		if (ethernetController.jsPropertyBool(servicesName[s] + "DataEnable") == true) {
			var service: DeviceObject = root.jsFindChildObjectByMask(serviceID);
			if (service == null) {
				log.wrnCFG3008(serviceID, module.jsPropertyString("EquipmentID"));

				if (s == 0)	// this is App
				{
					serviceIP[s] = 0xc0a80bfe;	//	192.168.11.254
					servicePort[s] = 13322;
				}

				if (s == 1)	// this is Diag
				{
					serviceIP[s] = 0xc0a815fe;	//	192.168.21.254
					servicePort[s] = 13323;
				}

				if (serviceIP[s] != 0 && servicePort[s] != 0) {
					log.wrnCFG3018(servicesName[s] + "DataService", ipToString(serviceIP[s]), servicePort[s], ethernetController.jsPropertyString("EquipmentID"));
				}

			}
			else {
				var serviceDataChannel: DeviceObject = service.jsFindChildObjectByMask(serviceID + "_DATACH01");
				if (serviceDataChannel == null) {
					log.errCFG3004(serviceID + "_DATACH01", equipmentID);
					return false;
				}

				var checkProperties: string[] = ["DataReceivingIP", "DataReceivingPort"];
				for (var cp: number = 0; cp < checkProperties.length; cp++) {
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

	var regDataID: number = module.propertyValue("AppLANDataUID");
	var diagDataID: number = module.propertyValue("DiagLANDataUID");

	var controllerAppWordsCount: number = appWordsCount;

	var overrideRegWordsCount: number = ethernetController.jsPropertyInt("OverrideAppDataWordCount");
	if (overrideRegWordsCount != -1) {
		controllerAppWordsCount = overrideRegWordsCount;
		regDataID = 0;
	}

	var controllerDiagWordsCount: number = diagWordsCount;

	var overrideDiagWordsCount: number = ethernetController.jsPropertyInt("OverrideDiagDataWordCount");
	if (overrideDiagWordsCount != -1) {
		controllerDiagWordsCount = overrideDiagWordsCount;
		diagDataID = 0;
	}

	if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController,
		controllerAppWordsCount, ip[0], port[0], serviceIP[0], servicePort[0], regDataID,
		controllerDiagWordsCount, ip[1], port[1], serviceIP[1], servicePort[1], diagDataID) == false) {
		return false;
	}
	lanFrame++;
	//}

	// Create TX/RX configuration
	//

	confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");

	var txRxConfigFrame: number = lanConfigFrame + 1;

	if (generate_lmTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
		return false;
	}

	// Create NIOS configuration
	//

	confFirmware.writeLog("Writing NIOS configuration.\r\n");

	var niosConfigFrame: number = txRxConfigFrame + 1;

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

function generate_txRxIoConfig(confFirmware: ModuleFirmware, equipmentID: string, LMNumber: number, frame: number, offset: number, log: IssueLogger,
	flags: number, configFrames: number, dataFrames: number, txId: number): boolean {
	// TxRx Block's configuration structure
	//
	var ptr: number = offset;

	confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] flags = " + flags +
		"; [" + frame + ":" + (ptr + 2) + "] configFrames = " + configFrames +
		"; [" + frame + ":" + (ptr + 4) + "] dataFrames = " + dataFrames +
		"; [" + frame + ":" + (ptr + 6) + "] txId = " + txId + "\r\n");

	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "TxRxFlags", flags) == false)        // Flags word
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Configuration words quantity", configFrames) == false) // Configuration words quantity
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Data words quantity", dataFrames) == false)   // Data words quantity
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Tx ID", txId) == false)         // Tx ID
	{
		return false;
	}
	ptr += 2;

	return true;
}

function generate_LANConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: DeviceObject, ethernetController: DeviceObject,
	regWordsCount: number, regIP: number, regPort: number, regServiceIP: number, regServicePort: number, regDataID: number,
	diagWordsCount: number, diagIP: number, diagPort: number, diagServiceIP: number, diagServicePort: number, diagDataID: number): boolean {
	var ptr: number = 0;

	var moduleEquipmentID: string = module.jsPropertyString("EquipmentID");
	var LMNumber: number = module.jsPropertyInt("LMNumber");
	var controllerEquipmentID: string = ethernetController.jsPropertyString("EquipmentID");

	//mac
	//
	var hashName: string = "S" + regIP + diagIP + moduleEquipmentID + regServiceIP + diagServiceIP;
	var hashList: JsVariantList = confFirmware.calcHash64(hashName);
	var size: number = hashList.jsSize();
	if (size != 2) {
		log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
		return false;
	}

	var h0: number = hashList.jsAt(0);
	var h1: number = hashList.jsAt(1);

	var m1: number = 0x4200;
	var m2: number = h0 & 0x7fff;
	var m3: number = (h0 >> 16) & 0x7fff;

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
function generate_lmTxRxOptoConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: DeviceObject, LMNumber: number, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {
	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "Class_Module");
		return false;
	}

	var portCount: number = logicModuleDescription.OptoInterface_OptoPortCount;

	var txWordsCount: number = 0;

	for (var p: number = 0; p < portCount; p++) {
		var controllerID: string = module.jsPropertyString("EquipmentID") + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);

		var controller: DeviceObject = module.jsFindChildObjectByMask(controllerID);
		if (controller == null) {
			log.errCFG3004(controllerID, module.jsPropertyString("EquipmentID"));
			return false;
		}

		if (controller.propertyValue("EquipmentID") == undefined) {
			log.errCFG3000("EquipmentID", "Class_Controller");
			return false;
		}

		var controllerEquipmentID: string = controller.jsPropertyString("EquipmentID");

		var optoPort: OptoPort = opticModuleStorage.jsGetOptoPort(controllerEquipmentID);
		if (optoPort == null) {
			continue;
		}

		if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0) {
			continue;
		}

		confFirmware.writeLog("    OptoPort " + controllerEquipmentID + ": connection ID = " + optoPort.equipmentID() +
			" (" + optoPort.connectionID() + ")\r\n");

		var ptr: number = 0 + p * 2;

		var value: number = optoPort.txStartAddress();
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

		var dataUID: number = 0;
		if (optoPort.isLinked() == true) {
			var linkedPort: string = optoPort.linkedPortID();
			var linkedOptoPort: OptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
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


function generate_niosConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: DeviceObject, LMNumber: number, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {
	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "Class_Module");
		return false;
	}

	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "I/O_module");
		return false;
	}

	var equipmentID = module.propertyValue("EquipmentID");

	var FamilyBUMD1: number = 0x5700;
	var FamilyBUMZ1: number = 0x5800;

	var ioModulesMaxCount: number = 16;

	var chassis: DeviceObject = module.jsParent();

	var ptr : number = 0;

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

	var blocksPtr : number = ptr + 2 + 2;	// bptr is a pointer to Blocks [] array

	var blockPresent: boolean[] = [];
	for (var i: number = 0; i < ioModulesMaxCount; i++) {
		blockPresent[i] = false;
	}

	var blocksCount: number = 0;
	var blocksMask: number = 0;

	for (var i: number = 0; i < chassis.childrenCount(); i++) {
		var ioModule: DeviceObject = chassis.jsChild(i);

		if (ioModule.jsDeviceType() != DeviceObjectType.Module) {
			continue;
		}

		if (ioModule.propertyValue("EquipmentID") == undefined) {
			log.errCFG3000("EquipmentID", "I/O_module");
			return false;
		}

		var customModuleFamily : number = ioModule.jsPropertyInt("CustomModuleFamily");

		if (customModuleFamily != FamilyBUMD1 && customModuleFamily != FamilyBUMZ1) {
			continue;
		}

		var ioEquipmentID: string = ioModule.jsPropertyString("EquipmentID");

		var checkProperties: string[] = ["ModuleVersion", "Place", "PresetName", "ConfigurationScript", "TxDiagDataSize", "TxAppDataSize"];
		for (var cp: number = 0; cp < checkProperties.length; cp++) {
			if (ioModule.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], ioEquipmentID);
				return false;
			}
		}

		var ioPlace: number = ioModule.jsPropertyInt("Place");
		if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
			log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioEquipmentID);
			return false;
		}

		var zeroIoPlace: number = ioPlace - 1;

		blockPresent[zeroIoPlace] = true;

		blocksCount++

		blocksMask |= (1 << zeroIoPlace);

		var blockPtr: number = blocksPtr + (zeroIoPlace * 4 * 2);

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

		var diagWordsIoCount: number = ioModule.jsPropertyInt("TxDiagDataSize");

		if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "DiagDataSize", diagWordsIoCount) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: DiagDataSize = " + diagWordsIoCount + "\r\n");

		blockPtr += 2;

		// TxAppDataSize

		var appWordsIoCount: number = ioModule.jsPropertyInt("TxAppDataSize");

		if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "AppDataSize", appWordsIoCount) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: AppDataSize = " + appWordsIoCount + "\r\n");

		blockPtr += 2;
	}

	for (var i: number = 0; i < ioModulesMaxCount; i++) {
		if (blockPresent[i] == true) {
			continue;
		}

		var blockPtr: number = blocksPtr + (i * 4 * 2);

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
