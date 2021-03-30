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
	AppSignal
}

enum SoftwareType {
	Monitor = 9000,
	ConfigurationService = 9001,
	AppDataService = 9002,
	ArchiveService = 9003,
	TuningService = 9004,
	DiagDataService = 9005,
	TuningClient = 9006,
	Metrology = 9007,
	ServiceControlManager = 9008,
}

interface Builder {
	jsIsInterruptRequested(): boolean;
}

interface ScriptDeviceObject {
	
	equipmentId: string;
	caption: string;
	uuid: string;
	deviceType: DeviceObjectType;
	place: number;
	childrenCount: number;

	parent(): ScriptDeviceObject;
	child(index: number): ScriptDeviceObject;
	childByEquipmentId(equipmentId: string): ScriptDeviceObject;	

	toRack(): ScriptDeviceRack;
	toChassis(): ScriptDeviceChassis;
	toModule(): ScriptDeviceModule;
	toController(): ScriptDeviceController;
	toAppSignal(): ScriptDeviceAppSignal;
	toWorkstation(): ScriptDeviceWorkstation;
	toSoftware(): ScriptDeviceSoftware;

	isRack(): boolean;
	isChassis(): boolean;
	isModule(): boolean;
	isController(): boolean;
	isAppSignal(): boolean;
	isWorkstation(): boolean;
	isSoftware(): boolean;	

	propertyValue(name: string): any;
	propertyInt(name: string): number;
	propertyBool(name: string): boolean;
	propertyString(name: string): string;
	propertyIP(name: string): number;	
}

interface ScriptDeviceRack extends ScriptDeviceObject {
}

interface ScriptDeviceChassis extends ScriptDeviceObject {
}

interface ScriptDeviceModule extends ScriptDeviceObject {
	moduleFamily: number;
	moduleVersion: number;
}

interface ScriptDeviceController extends ScriptDeviceObject {
}

interface ScriptDeviceAppSignal extends ScriptDeviceObject {
}

interface ScriptDeviceWorkstation extends ScriptDeviceObject {
}

interface ScriptDeviceSoftware extends ScriptDeviceObject {
	softwareType: SoftwareType;
}

interface JsVariantList {
	jsSize(): number;
	jsAt(index: number): number;
}

interface ModuleFirmware {
	setData8(frameIndex: number, offset: number, data: number): boolean;
	setData16(frameIndex: number, offset: number, data: number): boolean;
	setData32( frameIndex: number, offset: number, data: number): boolean;

	data8(frameIndex: number, offset: number): number;
	data16(frameIndex: number, offset: number): number;
	data32(frameIndex: number, offset: number): number;

	storeCrc64(frameIndex: number, start: number, count: number, offset: number): string;
	storeHash64(frameIndex: number, offset: number, dataString: string): string;

	calcCrc32(frameIndex: number, start: number, count: number): number;
	calcHash64(dataString: string): JsVariantList;

	jsSetDescriptionFields(descriptionVersion: number, description: string): void;
	jsAddDescription(channel: number, description: string): void;
	jsSetUniqueID(LMNumber: number, uniqueID: number): void;

	writeLog(message: string): void;
	buildNumber(): number;
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
	errCFG3017(objectID: string, propertyName: string, softwareID: string): void;
	wrnCFG3018(propertyName: string, ip: string, port: number, controller: string): void;
	errCFG3042(moduleId: string, moduleUuid: string): void;		// Title: Module %1 should be installed in chassis.

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

	FlashMemory_ConfigFramePayload: number;
	FlashMemory_ConfigFrameCount: number;
	FlashMemory_ConfigUartId: number;
	Memory_TxDiagDataSize: number;
	OptoInterface_OptoPortCount: number;
}

function runConfigScript(configScript: string, confFirmware: ModuleFirmware, ioModule: ScriptDeviceObject, LMNumber: number, frame: number, log: IssueLogger, signalSet: SignalSet, opticModuleStorage: OptoModuleStorage): boolean {
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

let FamilyMSO3: number = 0x2200;
let VersionMSO3: number = 0x0053;

//let configScriptVersion: number = 1;
//let configScriptVersion: number = 2;	// Changes in LMNumberCount calculation algorithm
//let configScriptVersion: number = 3;	// Added software type checking
let configScriptVersion: number = 4;	// ScriptDeviceObject is used

let LMDescriptionNumber: number = 0;

//

function main(builder: Builder, root: ScriptDeviceObject, logicModules: ScriptDeviceModule[], confFirmware: ModuleFirmware,
	log: IssueLogger, signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule): boolean {

	if (logicModules.length == 0) {
		return true;
	}
	let subSysID: string = logicModules[0].propertyString("SubsystemID");
	log.writeMessage("Subsystem " + subSysID + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);

	let LMNumberCount:number = 0;

	for (let i: number = 0; i < logicModules.length; i++) {

		if (logicModules[i].moduleFamily != FamilyMSO3 || logicModules[i].moduleVersion != VersionMSO3) {
			continue;
		}

		let result: boolean = module_mso3(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
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
	let frameStorageConfig:number = 1;
	let ptr: number = 14;

	if (setData16(confFirmware, log, -1, "", frameStorageConfig, ptr, "LMNumberCount", LMNumberCount) == false) {
		return false;
	}
	confFirmware.writeLog("Subsystem " + subSysID + ", frame " + frameStorageConfig + ", offset " + ptr + ": LMNumberCount = " + LMNumberCount + "\r\n");

	return true;
}

function setData8(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	if (channel != -1 && equpmentID.length > 0) {
		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "8;" + caption + ";0x" + data.toString(16));
	}

	if (confFirmware.setData8(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData8");
		return false;
	}
	return true;
}

function setData16(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	if (channel != -1 && equpmentID.length > 0) {
		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "16;" + caption + ";0x" + data.toString(16));
	}

	if (confFirmware.setData16(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData16");
		return false;
	}
	return true;
}

function setData32(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	if (channel != -1 && equpmentID.length > 0) {
		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";0x" + data.toString(16));
	}

	if (confFirmware.setData32(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData32");
		return false;
	}
	return true;
}

function storeCrc64(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, start: number, count: number, offset: number): string {
	let result: string = confFirmware.storeCrc64(frameIndex, start, count, offset);

	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);

	if (result == "") {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
	}
	return result;
}

function storeHash64(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: string): string {
	let result: string = confFirmware.storeHash64(frameIndex, offset, data);

	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);

	if (result == "") {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
	}
	return result;
}

function ipToString(ip: number): string {
	let ip0: number = (ip >> 24) & 0xff;
	let ip1: number = (ip >> 16) & 0xff;
	let ip2: number = (ip >> 8) & 0xff;
	let ip3: number = (ip) & 0xff;
	let result: string = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
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

	let res: number = (highADC - lowADC) * (val - lowLimit) / (highLimit - lowLimit) + lowADC;

	return Math.round(res);
}

function module_mso3(builder: Builder, root: ScriptDeviceObject, module: ScriptDeviceModule, confFirmware: ModuleFirmware, log: IssueLogger,
	signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule): boolean {

	if (module.moduleVersion == FamilyMSO3 && module.moduleVersion == VersionMSO3) {
		let place: number = module.place;

		if (place != 0) {
			log.errCFG3002("Place", place, 0, 0, module.equipmentId);
			return false;
		}

		// Generate Configuration
		//
		return generate_mso3_rev1(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
	}

	return false;
}

// Generate configuration for module MSO-3
//
//
function generate_mso3_rev1(builder: Builder, root: ScriptDeviceObject, module: ScriptDeviceModule, confFirmware: ModuleFirmware, log: IssueLogger,
	signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {
	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "MSO-3");
		return false;
	}

	{
		let checkProperties: string[] = ["SubsystemID", "LMNumber", "SubsystemChannel", "AppLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID"];
		for (let cp: number = 0; cp < checkProperties.length; cp++) {
			if (module.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], module.equipmentId);
				return false;
			}
		}
	}

	// Variables
	//
	let subSysID: string = module.propertyString("SubsystemID");
	let LMNumber: number = module.propertyInt("LMNumber");

	// Constants
	//
	let frameSize: number = logicModuleDescription.FlashMemory_ConfigFramePayload;
	let frameCount: number = logicModuleDescription.FlashMemory_ConfigFrameCount;

	if (frameSize < 1016) {
		log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.equipmentId);
		return false;
	}

	if (frameCount < 78 /*2 + 19  frames * 4 channels*/) {
		log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 78, 65535, module.equipmentId);
		return false;
	}

	let uartId: number = logicModuleDescription.FlashMemory_ConfigUartId;

	let appWordsCount: number = module.propertyInt("AppLANDataSize");
	let diagWordsCount: number = logicModuleDescription.Memory_TxDiagDataSize;

	let ssKeyValue: number = subsystemStorage.ssKey(subSysID);
	if (ssKeyValue == -1) {
		log.errCFG3001(subSysID, module.equipmentId);
		return false;
	}

	let maxLMNumber: number = 62;               // Can be changed!
	let configStartFrames: number = 2;
	let configFrameCount: number = 4;          // number of frames in each configuration
	let ioModulesMaxCount: number = 72;

	if (LMNumber < 1 || LMNumber > maxLMNumber) {
		log.errCFG3002("System/LMNumber", LMNumber, 1, maxLMNumber, module.equipmentId);
		return false;
	}

	let descriptionVersion = 1;

	confFirmware.jsSetDescriptionFields(descriptionVersion, "EquipmentID;Frame;Offset;BitNo;Size;Caption;Value");

	confFirmware.writeLog("---\r\n");
	confFirmware.writeLog("Module: MSO-3\r\n");
	confFirmware.writeLog("EquipmentID = " + module.equipmentId + "\r\n");
	confFirmware.writeLog("Subsystem ID = " + subSysID + "\r\n");
	confFirmware.writeLog("Key value = " + ssKeyValue + "\r\n");
	confFirmware.writeLog("UartID = " + uartId + "\r\n");
	confFirmware.writeLog("Frame size = " + frameSize + "\r\n");
	confFirmware.writeLog("LMNumber = " + LMNumber + "\r\n");
	confFirmware.writeLog("LMDescriptionNumber = " + LMDescriptionNumber + "\r\n");

	// Configuration storage format
	//
	let frameStorageConfig: number = 1;
	let ptr: number = 0;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Marker", 0xca70) == false)     //CFG_Marker
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Version", 0x0001) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
	ptr += 2;


	let ssKey: number = ssKeyValue << 6;             //0000SSKEYY000000b
	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "SubsystemKey", ssKey) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
	ptr += 2;

	let buildNo: number = confFirmware.buildNumber();
	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "BuildNo", buildNo) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] BuildNo = " + buildNo + "\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMDescriptionNumber", LMDescriptionNumber) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMDescriptionNumber = " + LMDescriptionNumber + "\r\n");
	ptr += 2;

	// reserved
	ptr += 4;

	// write LMNumberCount, if old value is less than current. If it is the same, output an error.
	//
	let oldLMNumberCount: number = confFirmware.data16(frameStorageConfig, ptr);

	if (oldLMNumberCount == LMNumber) {
		log.errCFG3003(LMNumber, module.equipmentId);
		return false;
	}

	if (oldLMNumberCount < LMNumber) {
		if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMNumberCount", LMNumber) == false) {
			return false;
		}
	}
	ptr += 2;

	let configIndexOffset: number = ptr + (LMNumber - 1) * (2/*offset*/ + 4/*reserved*/);
	let configFrame: number = configStartFrames + configFrameCount * (LMNumber - 1);

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, configIndexOffset, "ConfigStartFrame", configFrame) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");

	// Service information
	//
	confFirmware.writeLog("Writing service information.\r\n");

	let frameServiceConfig: number = configFrame;
	ptr = 0;
	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "ServiceVersion", 0x0001) == false)   //CFG_Ch_Vers
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "UartID", uartId) == false)  //CFG_Ch_Dtype == UARTID?
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] uartId = " + uartId + "\r\n");
	ptr += 2;

	let hashString = storeHash64(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "SubSystemID Hash", subSysID);
	if (hashString == "") {
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] subSysID HASH-64 = 0x" + hashString + "\r\n");
	//Hash (UniqueID) will be counted later
	ptr += 8;

	if (module.parent().isChassis() == false)
	{
		log.errCFG3042(module.equipmentId, module.equipmentId);	
		return false;
	}

	let chassis: ScriptDeviceChassis = module.parent().toChassis();

	for (let i: number = 0; i < chassis.childrenCount; i++) {

		if (chassis.child(i).isModule() == false) {
			continue;
		}

		let ioModule: ScriptDeviceModule = chassis.child(i).toModule();

		if (ioModule.moduleVersion == FamilyMSO3 && ioModule.moduleVersion == VersionMSO3) {
			continue;
		}

		let ioPlace: number = ioModule.place;
		if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
			log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioModule.equipmentId);
			return false;
		}

		let ioEquipmentID: string = ioModule.equipmentId;

		let diagWordsIoCount: number = ioModule.propertyInt("TxDiagDataSize");
		if (diagWordsIoCount == null) {
			log.errCFG3000("TxDiagDataSize", ioEquipmentID);
			return false;
		}

		diagWordsCount += diagWordsIoCount;
	}

	// Create LANs configuration
	//
	let lanConfigFrame: number = configFrame + 1;

	confFirmware.writeLog("Writing LAN configuration.\r\n");

	let lanFrame: number = lanConfigFrame;

	let ip: number[] = [0, 0];
	let port: number[] = [0, 0];

	let serviceIP: number[] = [0, 0];
	let servicePort: number[] = [0, 0];

	let ethernetcontrollerID = "_ETHERNET01";

	let ethernetControllerObject: ScriptDeviceObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerID);
	if (ethernetControllerObject == null || ethernetControllerObject.isController() == false) {
		log.errCFG3004(module.equipmentId + ethernetcontrollerID, module.equipmentId);
		return false;
	}

	let ethernetController = ethernetControllerObject.toController();

	{
		let checkProperties: string[] = ["AppDataServiceID", "AppDataEnable", "AppDataIP", "AppDataPort",
			"DiagDataServiceID", "DiagDataEnable", "DiagDataIP", "DiagDataPort",
			"OverrideAppDataWordCount", "OverrideDiagDataWordCount"];

		for (let cp: number = 0; cp < checkProperties.length; cp++) {
			if (ethernetController.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], ethernetController.equipmentId);
				return false;
			}
		}
	}

	confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerID + "\r\n");

	let servicesName: string[] = ["App", "Diag"];

	for (let s: number = 0; s < 2; s++) {
		// Controller

		ip[s] = ethernetController.propertyIP(servicesName[s] + "DataIP");
		port[s] = ethernetController.propertyInt(servicesName[s] + "DataPort");

		if (ip[s] == 0) {
			log.errCFG3011(servicesName[s] + "DataIP", ip[s], ethernetController.equipmentId);
			return false;
		}
		if (port[s] == 0) {
			log.errCFG3012(servicesName[s] + "DataPort", port[s], ethernetController.equipmentId);
			return false;
		}

		// Service
		let serviceID: string = ethernetController.propertyString(servicesName[s] + "DataServiceID");

		if (ethernetController.propertyBool(servicesName[s] + "DataEnable") == true) {
		
			let serviceObject: ScriptDeviceObject = root.childByEquipmentId(serviceID);
			
			if (serviceObject == null || serviceObject.isSoftware() == false) {
				log.wrnCFG3008(serviceID, module.equipmentId);

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
					log.wrnCFG3018(servicesName[s] + "DataService", ipToString(serviceIP[s]), servicePort[s], ethernetController.equipmentId);
				}

			}
			else {
				let service: ScriptDeviceSoftware = serviceObject.toSoftware();
				
				if ((s == 0 && service.softwareType != SoftwareType.AppDataService) ||
					(s == 1 && service.softwareType != SoftwareType.DiagDataService)){
					log.errCFG3017(ethernetController.equipmentId, "Type", service.equipmentId);
					return false;
				}

				//

				let checkProperties: string[] = ["DataReceivingIP", "DataReceivingPort"];
				for (let cp: number = 0; cp < checkProperties.length; cp++) {
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

	let regDataID: number = module.propertyValue("AppLANDataUID");
	let diagDataID: number = module.propertyValue("DiagLANDataUID");

	let controllerAppWordsCount: number = appWordsCount;

	let overrideRegWordsCount: number = ethernetController.propertyInt("OverrideAppDataWordCount");
	if (overrideRegWordsCount != -1) {
		controllerAppWordsCount = overrideRegWordsCount;
		regDataID = 0;
	}

	let controllerDiagWordsCount: number = diagWordsCount;

	let overrideDiagWordsCount: number = ethernetController.propertyInt("OverrideDiagDataWordCount");
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

	let txRxConfigFrame: number = lanConfigFrame + 1;

	if (generate_lmTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
		return false;
	}

	// Create NIOS configuration
	//

	confFirmware.writeLog("Writing NIOS configuration.\r\n");

	let niosConfigFrame: number = txRxConfigFrame + 1;

	if (generate_niosConfiguration(confFirmware, log, niosConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
		return false;
	}

	// create UniqueID
	//
	/*let startFrame: number = configStartFrames + configFrameCount * (LMNumber - 1);

	let uniqueID: number = 0;

	for (let i: number = 0; i < configFrameCount; i++) {
		let crc: number = confFirmware.calcCrc32(startFrame + i, 0, frameSize);

		uniqueID ^= crc;
	}

	confFirmware.jsSetUniqueID(LMNumber, uniqueID);*/

	return true;
}

function generate_txRxIoConfig(confFirmware: ModuleFirmware, equipmentID: string, LMNumber: number, frame: number, offset: number, log: IssueLogger,
	flags: number, configFrames: number, dataFrames: number, txId: number): boolean {
	// TxRx Block's configuration structure
	//
	let ptr: number = offset;

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

function generate_LANConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: ScriptDeviceModule, ethernetController: ScriptDeviceController,
	regWordsCount: number, regIP: number, regPort: number, regServiceIP: number, regServicePort: number, regDataID: number,
	diagWordsCount: number, diagIP: number, diagPort: number, diagServiceIP: number, diagServicePort: number, diagDataID: number): boolean {
	let ptr: number = 0;

	let moduleEquipmentID: string = module.equipmentId;
	let LMNumber: number = module.propertyInt("LMNumber");
	let controllerEquipmentID: string = ethernetController.propertyString("EquipmentID");

	//mac
	//
	let hashName: string = "S" + regIP + diagIP + moduleEquipmentID + regServiceIP + diagServiceIP;
	let hashList: JsVariantList = confFirmware.calcHash64(hashName);
	let size: number = hashList.jsSize();
	if (size != 2) {
		log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
		return false;
	}

	let h0: number = hashList.jsAt(0);
	let h1: number = hashList.jsAt(1);

	let m1: number = 0x4200;
	let m2: number = h0 & 0x7fff;
	let m3: number = (h0 >> 16) & 0x7fff;

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
function generate_lmTxRxOptoConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: ScriptDeviceModule, LMNumber: number, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {
	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "Class_Module");
		return false;
	}

	let portCount: number = logicModuleDescription.OptoInterface_OptoPortCount;

	let txWordsCount: number = 0;

	for (let p: number = 0; p < portCount; p++) {
		let controllerID: string = module.equipmentId + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);

		let controllerObject: ScriptDeviceObject = module.childByEquipmentId(controllerID);
		if (controllerObject == null || controllerObject.isController() == false) {
			log.errCFG3004(controllerID, module.equipmentId);
			return false;
		}

		let controller: ScriptDeviceController = controllerObject.toController();

		let controllerEquipmentID: string = controller.equipmentId;

		let optoPort: OptoPort = opticModuleStorage.jsGetOptoPort(controllerEquipmentID);
		if (optoPort == null) {
			continue;
		}

		if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0) {
			continue;
		}

		confFirmware.writeLog("    OptoPort " + controllerEquipmentID + ": connection ID = " + optoPort.equipmentID() +
			" (" + optoPort.connectionID() + ")\r\n");

		let ptr: number = 0 + p * 2;

		let value: number = optoPort.txStartAddress();
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

		let dataUID: number = 0;
		if (optoPort.isLinked() == true) {
			let linkedPort: string = optoPort.linkedPortID();
			let linkedOptoPort: OptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
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


function generate_niosConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: ScriptDeviceModule, LMNumber: number, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {
	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "Class_Module");
		return false;
	}

	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "I/O_module");
		return false;
	}

	let equipmentID = module.propertyValue("EquipmentID");

	let ioModulesMaxCount: number = 72;

	if (module.parent().isChassis() == false)
	{
		log.errCFG3042(module.equipmentId, module.equipmentId);	
		return false;
	}

	let chassis: ScriptDeviceChassis = module.parent().toChassis();

	let ptr : number = 0;

	// Label

	let value = 0xbaed;
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

	value = module.propertyInt("SubsystemChannel");
	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "SubblockNum", value) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "]: SubblockNum = " + value + "\r\n");

	ptr += 2;

	// Checks

	let Checks = 0;

	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Checks", Checks) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "]: Checks = " + Checks + "\r\n");

	ptr += 2;

	// Blocks[]

	let blocksPtr : number = ptr + 2 + 2;	// bptr is a pointer to Blocks [] array

	let blockPresent: boolean[] = [];
	for (let i: number = 0; i < ioModulesMaxCount; i++) {
		blockPresent[i] = false;
	}

	let blocksCount: number = 0;
	let blocksMask: number = 0;

	for (let i: number = 0; i < chassis.childrenCount; i++) {
		if (chassis.child(i).isModule() == false) {
			continue;
		}

		let ioModule: ScriptDeviceModule = chassis.child(i).toModule();

		if (ioModule.moduleFamily == FamilyMSO3 && ioModule.moduleVersion == VersionMSO3) {
			continue;
		}

		let ioEquipmentID: string = ioModule.equipmentId;

		let checkProperties: string[] = ["ModuleVersion", "Place", "PresetName", "ConfigurationScript", "TxDiagDataSize", "TxAppDataSize"];
		for (let cp: number = 0; cp < checkProperties.length; cp++) {
			if (ioModule.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], ioEquipmentID);
				return false;
			}
		}

		let ioPlace: number = ioModule.place;
		if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
			log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioEquipmentID);
			return false;
		}

		let zeroIoPlace: number = ioPlace - 1;

		blockPresent[zeroIoPlace] = true;

		blocksCount++

		blocksMask |= (1 << zeroIoPlace);

		let blockPtr: number = blocksPtr + (zeroIoPlace * 4 * 2);

		// Place

		if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "Module Place", ioPlace) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: Module Place = " + ioPlace + "\r\n");

		blockPtr += 2;

		// Id

		value = ioModule.moduleFamily | ioModule.moduleVersion;

		if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "ID", value) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: ID = " + value + "\r\n");

		blockPtr += 2;

		blockPtr += 2;	// Reserved

		blockPtr += 2;	// Reserved
	}

	for (let i: number = 0; i < ioModulesMaxCount; i++) {
		if (blockPresent[i] == true) {
			continue;
		}

		let blockPtr: number = blocksPtr + (i * 4 * 2);

		// Place

		value = 0xffff;
		if (setData16(confFirmware, log, LMNumber, equipmentID, frame, blockPtr, "Module Place (reserved)", value) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + blockPtr + "]: Module Place (reserved) = " + value + "\r\n");

		blockPtr += 2;
	}

	// QBlocks

	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "QBlocks", blocksCount) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "]: QBlocks = " + blocksCount + "\r\n");

	ptr += 2;

	// StructSize

	let structSize = 8;

	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "StructSize", structSize) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "]: StructSize = " + structSize + "\r\n");

	return true;
}
