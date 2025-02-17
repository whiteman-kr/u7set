"use strict";

module ConfigStruct
{
	export enum DeviceObjectType
	{
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

	export enum SoftwareType
	{
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


	export enum ElectricUnit
	{
		NoUnit = 0,
		mA = 1,
		mV = 2,
		Ohm = 3,
		V = 4,
	}

	export enum SensorType
	{
		NoSensor = 0,

		Ohm_Pt50_W1391 = 1,
		Ohm_Pt100_W1391 = 2,
		Ohm_Pt50_W1385 = 3,
		Ohm_Pt100_W1385 = 4,

		Ohm_Cu_50_W1428 = 5,
		Ohm_Cu_100_W1428 = 6,
		Ohm_Cu_50_W1426 = 7,
		Ohm_Cu_100_W1426 = 8,

		Ohm_Pt21 = 9,
		Ohm_Cu23 = 10,

		mV_K_TXA = 11,
		mV_L_TXK = 12,
		mV_N_THH = 13,

		//

		mV_Type_B = 14,
		mV_Type_E = 15,
		mV_Type_J = 16,
		mV_Type_K = 17,
		mV_Type_N = 18,
		mV_Type_R = 19,
		mV_Type_S = 20,
		mV_Type_T = 21,

		mV_Raw_Mul_8 = 22,
		mV_Raw_Mul_32 = 23,

		Ohm_Ni50_W1617 = 24,
		Ohm_Ni100_W1617 = 25,

		V_0_5 = 26,
		V_m10_p10 = 27,

		Ohm_Pt_a_391 = 28,
		Ohm_Pt_a_385 = 29,
		Ohm_Cu_a_428 = 30,
		Ohm_Cu_a_426 = 31,
		Ohm_Ni_a_617 = 32,

		Ohm_Raw = 33,
		
		uA_m20_p20 = 34,
		Hz_005_50000 = 35,

		mV_Type_L = 36,
		mV_Type_M = 37,
		mV_Raw_m1200_p1200 = 38,

		Hz_0_60000 = 39,
		Hz_0_50000 = 40
	}

	export enum OutputMode
	{
		Plus0_Plus5_V = 0,
		Plus4_Plus20_mA = 1,
		Minus10_Plus10_V = 2,
		Plus0_Plus5_mA = 3,
		Plus0_Plus20_mA = 4,
		Plus0_Plus24_mA = 5,
	};

	export enum UnitsConvertorErrorCode
	{
		ErrorGeneric = 1,
		LowLimitOutOfRange = 2,
		HighLimitOutOfRange = 3,
	}

	export interface Builder
	{
		jsIsInterruptRequested(): boolean;
	}

	export interface ScriptDeviceObject
	{

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

	export interface ScriptDeviceRack extends ScriptDeviceObject
	{
	}

	export interface ScriptDeviceChassis extends ScriptDeviceObject
	{
	}

	export interface ScriptDeviceModule extends ScriptDeviceObject
	{
		moduleFamily: number;
		moduleVersion: number;
	}

	export interface ScriptDeviceController extends ScriptDeviceObject
	{
	}

	export interface ScriptDeviceAppSignal extends ScriptDeviceObject
	{
	}

	export interface ScriptDeviceWorkstation extends ScriptDeviceObject
	{
	}

	export interface ScriptDeviceSoftware extends ScriptDeviceObject
	{
		softwareType: SoftwareType;
	}

	export interface ModuleFirmware
	{

		setData8(frameIndex: number, offset: number, data: number): boolean;
		setData16(frameIndex: number, offset: number, data: number): boolean;
		setData32(frameIndex: number, offset: number, data: number): boolean;
		setDataFloat(frameIndex: number, offset: number, data: number): boolean;

		data8(frameIndex: number, offset: number): number;
		data16(frameIndex: number, offset: number): number;
		data32(frameIndex: number, offset: number): number;
		dataFloat(frameIndex: number, offset: number): number;

		storeCrc64(frameIndex: number, start: number, count: number, offset: number): string;
		storeHash64(frameIndex: number, offset: number, dataString: string): string;

		calcCrc32(frameIndex: number, start: number, count: number): number;
		calcCrc64(frameIndex: number, start: number, count: number): ArrayBuffer;
		calcHash32(dataString: string): any;
		calcHash64(dataString: string): any;

		jsSetDescriptionFields(descriptionVersion: number, description: string): void;
		jsAddDescription(channel: number, description: string): void;
		jsSetUniqueID(LMNumber: number, uniqueID: number): void;
		jsSetUniqueID64(LMNumber: number, uniqueIDLo: number, uniqueIDHi: number, ): void;

		writeLog(message: string): void;
		replaceLog(subsystemID: string, oldMessage: string, newMessage: string): void;
		buildNumber(): number;

		checkMacForUnique(m1: number, m2: number, m3: number): boolean;
	}

	export interface IssueLogger
	{
		writeMessage(message: string): void;
		writeWarning(message: string): void;
		writeError(message: string): void;

		errINT1001(message: string): void;

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

	export interface SignalSet
	{
	}

	export interface SubsystemStorage
	{
		ssKey(subSysID: string): number;
		ssKeyForVdu(subSysID: string): number;
	}

	export interface OptoPort
	{
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

	export interface OptoModuleStorage
	{
		jsGetOptoPort(controllerEquipmentID: string): OptoPort;
	}

	export interface LogicModule
	{
		descriptionNumber(): number;
		jsConfigurationStringFile(): string;

		jsLanControllerType(index: number): number;
		jsLanControllerPlace(index: number): number;

		FlashMemory_ConfigFramePayload: number;
		FlashMemory_ConfigFrameCount: number;
		FlashMemory_ConfigUartId: number;
		FlashMemory_MaxConfigurationCount: number;
		FlashMemory_SingleConfigFirstFrame: number;
		FlashMemory_SingleConfigFrameCount: number;
		FlashMemory_SingleConfigUniqueIdOffset: number;

		Memory_TxDiagDataSize: number;
		OptoInterface_OptoPortCount: number;
		Lan_ControllerCount: number;
	}

	export interface UnitsConvertor
	{
		physicalToElectric(value: number, electricLowLimit: number, electricHighLimit: number, unitID: number, sensorType: number): number;
		electricToPhysical(value: number, electricLowLimit: number, electricHighLimit: number, unitID: number, sensorType: number): number;
	}

	export interface LanConfig
	{
		flags: number;
		ip: number;
		port: number;
		serviceIP: number;
		servicePort: number;
		wordsCount: number;
		dataID: number;
	}

	export enum LanControllerType
	{
		Unknown = 0,
		Tuning = 1,
		AppData = 2,
		DiagData = 4,
		AppAndDiagData = 6,
		TuningAndAppAndDiagData = 7
	}
}


module ConfigLib
{
	export function runConfigScript(configScript: string,
		confFirmware: ConfigStruct.ModuleFirmware,
		ioModule: ConfigStruct.ScriptDeviceObject,
		LMNumber: number,
		frame: number,
		log: ConfigStruct.IssueLogger,
		signalSet: ConfigStruct.SignalSet,
		opticModuleStorage: ConfigStruct.OptoModuleStorage): boolean
	{
		//let funcStr = "(function (confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage){log.writeMessage(\"Hello\"); return true; })";
		//
		let funcStr = "(" + configScript + ")";
		let funcVar = eval(funcStr);
		if (funcVar(confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false)
		{
			return false;
		}

		return true;
	}

	export function setData8(confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean
	{
		if (channel != -1 && equpmentID.length > 0)
		{
			confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "8;" + caption + ";0x" + (data >>> 0).toString(16));
		}

		if (confFirmware.setData8(frameIndex, offset, data) == false)
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData8");
			return false;
		}
		return true;
	}

	export function setData16(confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean
	{
		if (channel != -1 && equpmentID.length > 0)
		{
			confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "16;" + caption + ";0x" + (data >>> 0).toString(16));
		}

		if (confFirmware.setData16(frameIndex, offset, data) == false)
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData16");
			return false;
		}
		return true;
	}

	export function setData32(confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean
	{
		if (channel != -1 && equpmentID.length > 0)
		{
			confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";0x" + (data >>> 0).toString(16));
		}

		if (confFirmware.setData32(frameIndex, offset, data) == false)
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData32");
			return false;
		}
		return true;
	}

	export function setData64(confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, dataHi: number, dataLo: number): boolean
	{
		if (channel != -1 && equpmentID.length > 0)
		{
			confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "64;" + caption + ";0x" + (dataHi >>> 0).toString(16) + (dataLo >>> 0).toString(16));
		}

		if (confFirmware.setData32(frameIndex, offset, dataHi) == false)
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData64");
			return false;
		}
		if (confFirmware.setData32(frameIndex, offset + 4, dataLo) == false)
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + (offset + 4) + ", frameIndex or offset are out of range in function setData64");
			return false;
		}
		return true;
	}

	export function setDataFloat(confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean
	{
		if (channel != -1 && equpmentID.length > 0)
		{
			confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";" + data);
		}

		if (confFirmware.setDataFloat(frameIndex, offset, data) == false)
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setDataFloat");
			return false;
		}
		return true;
	}

	export function storeCrc64(confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger, channel: number, equpmentID: string, frameIndex: number, start: number, count: number, offset: number): string
	{
		let result: string = confFirmware.storeCrc64(frameIndex, start, count, offset);

		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);

		if (result == "")
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
		}
		return result;
	}

	export function storeHash64(confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: string): string
	{
		let result: string = confFirmware.storeHash64(frameIndex, offset, data);

		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);

		if (result == "")
		{
			log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
		}
		return result;
	}

	export function ipToString(ip: number): string
	{
		let ip0: number = (ip >> 24) & 0xff;
		let ip1: number = (ip >> 16) & 0xff;
		let ip2: number = (ip >> 8) & 0xff;
		let ip3: number = (ip) & 0xff;
		let result: string = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
		return result;
	}

	export function truncate_to_int(x: number): number
	{
		if (x > 0)
		{
			return Math.floor(x);
		}
		else
		{
			return Math.ceil(x);
		}
	}

	export function valToADC(val: number, lowLimit: number, highLimit: number, lowADC: number, highADC: number): number
	{
		if ((highLimit - lowLimit) == 0)
		{
			return 0;		// to exclude division by zero
		}

		let res: number = (highADC - lowADC) * (val - lowLimit) / (highLimit - lowLimit) + lowADC;

		return Math.round(res);
	}

	export function fillLanServiceData(
		confFirmware: ConfigStruct.ModuleFirmware,
		softwareType: ConfigStruct.SoftwareType,
		root: ConfigStruct.ScriptDeviceObject,
		module: ConfigStruct.ScriptDeviceModule,
		ethernetcontrollerId: string,
		lan: ConfigStruct.LanConfig,
		log: ConfigStruct.IssueLogger): boolean
	{

		// Build prefix

		let controllerPrefix: string;
		let servicePrefix: string;
		let overridePrefix: string;

		switch (softwareType)
		{
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

		let ethernetControllerObject: ConfigStruct.ScriptDeviceObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerId);
		if (ethernetControllerObject == null || ethernetControllerObject.isController() == false)
		{
			log.errCFG3004(module.equipmentId + ethernetcontrollerId, module.equipmentId);
			return false;
		}

		let ethernetController: ConfigStruct.ScriptDeviceController = ethernetControllerObject.toController();

		let checkControllerProperties: string[] = [controllerPrefix + "ServiceID", controllerPrefix + "Enable", controllerPrefix + "IP", controllerPrefix + "Port", "Override" + overridePrefix + "DataWordCount"];
		for (let cp: number = 0; cp < checkControllerProperties.length; cp++)
		{
			if (ethernetController.propertyValue(checkControllerProperties[cp]) == undefined)
			{
				log.errCFG3000(checkControllerProperties[cp], ethernetController.equipmentId);
				return false;
			}
		}

		// Get data from services

		let serviceID: string = ethernetController.propertyString(controllerPrefix + "ServiceID");

		if (ethernetController.propertyBool(controllerPrefix + "Enable") == true)
		{

			// If Enable == true, take IP from service or default if service is not found

			lan.ip = ethernetController.propertyIP(controllerPrefix + "IP");
			lan.port = ethernetController.propertyInt(controllerPrefix + "Port");

			let serviceObject: ConfigStruct.ScriptDeviceObject = root.childByEquipmentId(serviceID);	// This can be software or controller
			let serviceSoftware: ConfigStruct.ScriptDeviceSoftware = null;							// This will be software

			if (serviceObject != null)
			{
				if (serviceObject.isController() == true)
				{
					let parentObject: ConfigStruct.ScriptDeviceObject = serviceObject.parent();

					if (parentObject != null && parentObject.isSoftware() == true)
					{
						serviceSoftware = parentObject.toSoftware();
					}
				}
				else
				{
					if (serviceObject.isSoftware() == true)
					{
						serviceSoftware = serviceObject.toSoftware();
					}
				}
			}

			if (serviceObject == null || serviceSoftware == null)
			{

				//Service was not found

				if (lan.serviceIP != 0 && lan.servicePort != 0)
				{
					log.wrnCFG3018(controllerPrefix + "DataService", ConfigLib.ipToString(lan.serviceIP), lan.servicePort, ethernetController.equipmentId);
				}
				else
				{
					log.wrnCFG3008(serviceID, module.equipmentId);
				}
			}
			else
			{
				// Check software type
				//

				if (serviceSoftware.softwareType != softwareType)
				{
					log.errCFG3017(ethernetController.equipmentId, "Type", serviceSoftware.equipmentId);
					return false;
				}

				// Take address from service

				let checkServiceProperties: string[] = [servicePrefix + "IP", servicePrefix + "Port"];
				for (let cp: number = 0; cp < checkServiceProperties.length; cp++)
				{
					if (serviceObject.propertyValue(checkServiceProperties[cp]) == undefined)
					{
						log.errCFG3000(checkServiceProperties[cp], serviceObject.equipmentId);
						return false;
					}
				}

				lan.serviceIP = serviceObject.propertyIP(servicePrefix + "IP");
				lan.servicePort = serviceObject.propertyInt(servicePrefix + "Port");
			}

			lan.dataID = module.propertyValue(overridePrefix + "LANDataUID");
			if (lan.dataID == undefined)
			{
				log.errCFG3000(overridePrefix + "LANDataUID", module.equipmentId);
				return false;
			}

			let overrideTuningWordsCount: number = ethernetController.propertyInt("Override" + overridePrefix + "DataWordCount");
			if (overrideTuningWordsCount != -1)
			{
				lan.wordsCount = overrideTuningWordsCount;
				lan.dataID = 0;
			}
		}
		else
		{
			// If Enable == false, set service ID is 0 even

			lan.dataID = 0;
			lan.wordsCount = 0;
			lan.serviceIP = 0;
			lan.servicePort = 0;
		}


		return true;
	}

	export function generate_LANConfiguration_v0(confFirmware: ConfigStruct.ModuleFirmware, frame: number, module: ConfigStruct.ScriptDeviceModule, ethernetControllerId: string, lan1: ConfigStruct.LanConfig, lan2: ConfigStruct.LanConfig, log: ConfigStruct.IssueLogger): boolean
	{

		let lan: ConfigStruct.LanConfig[] = [];

		lan.push(lan1);
		lan.push(lan2);

		let ptr: number = 0;

		let controllerEquipmentID: string = module.equipmentId + ethernetControllerId;
		let LMNumber: number = module.propertyInt("LMNumber");

		let m1: number = 0;
		let m2: number = 0;
		let m3: number = 0;

		if (lan1.ip == 0 && lan2.ip == 0 && lan1.serviceIP == 0 && lan2.serviceIP == 0)
		{
			// mac is empty
			//
		}
		else
		{
			//mac
			//
			let hashName: string = "S";
			for (let i: number = 0; i < lan.length; i++)
			{
				hashName += lan[i].ip;
			}
			hashName += controllerEquipmentID;
			for (let i: number = 0; i < lan.length; i++)
			{
				hashName += lan[i].serviceIP;
			}

			let hashList: any = confFirmware.calcHash64(hashName);
			if (hashList.length != 2)
			{
				log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
				return false;
			}

			let h: number = (hashList[0] + hashList[1]);
			m1 = 0x4200;
			m2 = h & 0x7fff;
			m3 = (h >> 16) & 0x7fff;

			if (confFirmware.checkMacForUnique(m1, m2, m3) == false)
			{
				log.errINT1001("MAC address " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + " of " + controllerEquipmentID + " is not unique!");
			}
		}

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : MAC address of LM = " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + "\r\n");
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC1", m1) == false)
		{
			return false;
		}
		ptr += 2;
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC2", m2) == false)
		{
			return false;
		}
		ptr += 2;
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC3", m3) == false)
		{
			return false;
		}
		ptr += 2;

		for (let i: number = 0; i < lan.length; i++)
		{

			// ip

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " IP = " + ipToString(lan[i].ip) + "\r\n");

			if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " IP", lan[i].ip) == false)
			{
				return false;
			}
			ptr += 4;

			// port

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Port = " + lan[i].port + "\r\n");

			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Port", lan[i].port) == false)
			{
				return false;
			}
			ptr += 2;
		}

		if (lan.length == 1)
		{
			//	If only one LAN is used - skip LAN 2 data
			ptr += 4;

			ptr += 2;
		}

		for (let i: number = 0; i < lan.length; i++)
		{

			// ServiceIP

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Service IP = " + ipToString(lan[i].serviceIP) + "\r\n");

			if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Service IP", lan[i].serviceIP) == false)
			{
				return false;
			}
			ptr += 4;

			// ServicePort

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Service Port = " + lan[i].servicePort + "\r\n");

			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Service Port = ", lan[i].servicePort) == false)
			{
				return false;
			}
			ptr += 2;

			// WordsCount

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Words Count = " + lan[i].wordsCount + "\r\n");

			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Words Count = ", lan[i].wordsCount) == false)
			{
				return false;
			}
			ptr += 2;
		}

		if (lan.length == 1)
		{
			//	If only one LAN is used - skip LAN 2 data
			ptr += 4;

			ptr += 2;

			ptr += 2;
		}

		for (let i: number = 0; i < lan.length; i++)
		{

			// DUID

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " DUID = " + lan[i].dataID + " (0x" + (lan[i].dataID >>> 0).toString(16) + ")\r\n");

			if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " DUID", lan[i].dataID) == false)
			{
				return false;
			}
			ptr += 4;
		}

		if (lan.length == 1)
		{
			//	If only one LAN is used - skip LAN 2 data
			ptr += 4;
		}

		return true;
	}

	export function generate_LANConfiguration_v1(confFirmware: ConfigStruct.ModuleFirmware,
		frame: number,
		module: ConfigStruct.ScriptDeviceModule,
		ethernetControllerId: string,
		lan: ConfigStruct.LanConfig[],
		log: ConfigStruct.IssueLogger): boolean
	{

		let ptr: number = 0;

		let controllerEquipmentID: string = module.equipmentId + ethernetControllerId;
		let LMNumber: number = module.propertyInt("LMNumber");

		// Version
		//
		const version: number = 1;

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN Configuration format version = " + version + "\r\n");
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Version", version) == false)
		{
			return false;
		}
		ptr += 2;

		// MAC address
		//

		let m1: number = 0;
		let m2: number = 0;
		let m3: number = 0;

		let macIsEmpty: boolean = true;

		for (let i: number = 0; i < lan.length; i++)
		{
			if (lan[i].ip != 0 || lan[i].serviceIP != 0)
			{
				macIsEmpty = false;
				break;
			}
		}

		if (macIsEmpty == true)
		{
			// mac is empty
			//
		}
		else
		{
			// mac
			//
			let hashName: string = "S";
			for (let i: number = 0; i < lan.length; i++)
			{
				hashName += lan[i].ip;
			}
			hashName += controllerEquipmentID;
			for (let i: number = 0; i < lan.length; i++)
			{
				hashName += lan[i].serviceIP;
			}

			let hashList: any = confFirmware.calcHash64(hashName);
			if (hashList.length != 2)
			{
				log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
				return false;
			}

			let h: number = (hashList[0] + hashList[1]);
			m1 = 0x4200;
			m2 = h & 0x7fff;
			m3 = (h >> 16) & 0x7fff;

			if (confFirmware.checkMacForUnique(m1, m2, m3) == false)
			{
				log.errINT1001("MAC address " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + " of " + controllerEquipmentID + " is not unique!");
			}
		}

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : MAC address of LM = " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + "\r\n");
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC1", m1) == false)
		{
			return false;
		}
		ptr += 2;
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC2", m2) == false)
		{
			return false;
		}
		ptr += 2;
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC3", m3) == false)
		{
			return false;
		}
		ptr += 2;

		for (let i: number = 0; i < lan.length; i++)
		{

			// WordOfFlags 

			let flags: number = 0;

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Flags = " + flags + "\r\n");
			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Flags", flags) == false)
			{
				return false;
			}
			ptr += 2;

			// IP

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " IP = " + ipToString(lan[i].ip) + "\r\n");

			if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " IP", lan[i].ip) == false)
			{
				return false;
			}
			ptr += 4;

			// Port

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Port = " + lan[i].port + "\r\n");

			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Port", lan[i].port) == false)
			{
				return false;
			}
			ptr += 2;

			// ServiceIP

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Service IP = " + ipToString(lan[i].serviceIP) + "\r\n");

			if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Service IP", lan[i].serviceIP) == false)
			{
				return false;
			}
			ptr += 4;

			// ServicePort

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Service Port = " + lan[i].servicePort + "\r\n");

			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Service Port", lan[i].servicePort) == false)
			{
				return false;
			}
			ptr += 2;

			// WordsCount

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " words count = " + lan[i].wordsCount + "\r\n");

			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " words count", lan[i].wordsCount) == false)
			{
				return false;
			}
			ptr += 2;

			// DUID

			
			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " DUID = " + lan[i].dataID + " (0x" + (lan[i].dataID >>> 0).toString(16) + ")\r\n");

			if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " DUID", lan[i].dataID) == false)
			{
				return false;
			}
			ptr += 4;

			ptr += 4;	// Reserved

		}
		return true;
	}

	// function returns the amount of transmitting words
	//
	export function generate_lmTxRxOptoConfiguration(confFirmware: ConfigStruct.ModuleFirmware, 
		log: ConfigStruct.IssueLogger, 
		frame: number, 
		module: ConfigStruct.ScriptDeviceModule, 
		LMNumber: number, 
		opticModuleStorage: ConfigStruct.OptoModuleStorage, 
		logicModuleDescription: ConfigStruct.LogicModule)
	{
		if (module.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "Class_Module");
			return false;
		}

		let portCount: number = logicModuleDescription.OptoInterface_OptoPortCount;

		let txWordsCount: number = 0;

		for (let p: number = 0; p < portCount; p++)
		{
			let controllerID: string = module.equipmentId + "_OPTOPORT0";
			controllerID = controllerID + (p + 1);

			let controllerObject: ConfigStruct.ScriptDeviceObject = module.childByEquipmentId(controllerID);
			if (controllerObject == null || controllerObject.isController() == false)
			{
				log.errCFG3004(controllerID, module.equipmentId);
				return false;
			}

			let controller: ConfigStruct.ScriptDeviceController = controllerObject.toController();

			let optoPort: ConfigStruct.OptoPort = opticModuleStorage.jsGetOptoPort(controller.equipmentId);
			if (optoPort == null)
			{
				continue;
			}

			if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0)
			{
				continue;
			}

			confFirmware.writeLog("    OptoPort " + controller.equipmentId + ": connection ID = " + optoPort.equipmentID() +
				" (" + optoPort.connectionID() + ")\r\n");

			let ptr: number = 0 + p * 2;

			let value: number = optoPort.txStartAddress();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

			ptr = 5 * 2 + p * 2;
			value = optoPort.txDataSizeW();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
			txWordsCount += value;

			ptr = 10 * 2 + p * 2;
			value = optoPort.portID();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

			ptr = 15 * 2 + p * 2;
			value = optoPort.rxDataSizeW();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

			let dataUID: number = 0;
			if (optoPort.isLinked() == true)
			{
				let linkedPort: string = optoPort.linkedPortID();
				let linkedOptoPort: ConfigStruct.OptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
				if (linkedOptoPort != null)
				{
					dataUID = linkedOptoPort.txDataID();
				}
			}

			ptr = 20 * 2 + p * 4;
			if (ConfigLib.setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + " (0x" + (dataUID >>> 0).toString(16) + ")\r\n");
		} // p

		return true;
	}

	export function generate_vduTxRxOptoConfiguration(confFirmware: ConfigStruct.ModuleFirmware,
		log: ConfigStruct.IssueLogger,
		frame: number,
		module: ConfigStruct.ScriptDeviceModule,
		LMNumber: number,
		opticModuleStorage: ConfigStruct.OptoModuleStorage,
		logicModuleDescription: ConfigStruct.LogicModule)
	{
		if (module.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "Class_Module");
			return false;
		}

		let portCount: number = logicModuleDescription.OptoInterface_OptoPortCount;

		let txWordsCount: number = 0;

		let ptr: number = 0;

		for (let p: number = 0; p < portCount; p++)
		{
			let controllerID: string = module.equipmentId + "_OPTOPORT0";
			controllerID = controllerID + (p + 1);

			let controllerObject: ConfigStruct.ScriptDeviceObject = module.childByEquipmentId(controllerID);
			if (controllerObject == null || controllerObject.isController() == false)
			{
				log.errCFG3004(controllerID, module.equipmentId);
				return false;
			}

			let controller: ConfigStruct.ScriptDeviceController = controllerObject.toController();

			let optoPort: ConfigStruct.OptoPort = opticModuleStorage.jsGetOptoPort(controller.equipmentId);
			if (optoPort == null)
			{
				continue;
			}

			if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0)
			{
				continue;
			}

			const optoVersion: number = 1;
			confFirmware.writeLog("    OptoPort " + controller.equipmentId + ": connection ID = " + optoPort.equipmentID() +
				" (" + optoPort.connectionID() + ")\r\n");

			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "Version for Opto " + (p + 1), optoVersion) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: Version for Opto " + (p + 1) + " = " + optoVersion + "\r\n");

			ptr += 2;

			let value: number = optoPort.txStartAddress();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

			ptr += 2;

			value = optoPort.txDataSizeW();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
			txWordsCount += value;

			ptr += 2;

			value = optoPort.portID();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

			ptr += 2;

			value = optoPort.rxDataSizeW();
			if (ConfigLib.setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
			{
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

			ptr += 2;

			{
				let txDataUID: number = 0;
				if (optoPort.isLinked() == true)
				{
					txDataUID = optoPort.txDataID();
				}

				if (ConfigLib.setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "Tx Block (Opto) Data UID " + (p + 1), txDataUID) == false)
				{
					return false;
				}
				confFirmware.writeLog("    [" + frame + ":" + ptr + "]: Tx Block (Opto) Data UID " + (p + 1) + " = " + txDataUID + " (0x" + (txDataUID >>> 0).toString(16) + ")\r\n");

				ptr += 4;
			}

			{
				let rxDataUID: number = 0;
				if (optoPort.isLinked() == true)
				{
					let linkedPort: string = optoPort.linkedPortID();
					let linkedOptoPort: ConfigStruct.OptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
					if (linkedOptoPort != null)
					{
						rxDataUID = linkedOptoPort.txDataID();
					}
				}

				if (ConfigLib.setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "Rx Block (Opto) Data UID " + (p + 1), rxDataUID) == false)
				{
					return false;
				}
				confFirmware.writeLog("    [" + frame + ":" + ptr + "]: Rx Block (Opto) Data UID " + (p + 1) + " = " + rxDataUID + " (0x" + (rxDataUID >>> 0).toString(16) + ")\r\n");

				ptr += 4;
			}

			ptr += 6;	// Reserved

		} // p

		return true;
	}

	export function generate_txRxIoConfig(confFirmware: ConfigStruct.ModuleFirmware, equipmentID: string, LMNumber: number, frame: number, offset: number, log: ConfigStruct.IssueLogger,
		flags: number, configFrames: number, dataFrames: number, moduleId: number): boolean
	{
		// TxRx Block's configuration structure
		//
		let ptr: number = offset;

		confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] Flags = " + flags +
			"; [" + frame + ":" + (ptr + 2) + "] ConfigFrames = " + configFrames +
			"; [" + frame + ":" + (ptr + 4) + "] DataFrames = " + dataFrames +
			"; [" + frame + ":" + (ptr + 6) + "] ModuleId = " + moduleId.toString(16) + "h\r\n");

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
		if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "ModuleID", moduleId) == false)         // Tx ID
		{
			return false;
		}
		ptr += 2;

		return true;
	}

}