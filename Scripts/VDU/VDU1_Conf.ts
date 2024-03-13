// Strict mode part
//

"use strict";

let FamilyVDUID: number = 0x7100;

let UartID: number = 0;

let LMNumberCount: number = 0;

let configScriptVersion: number = 1;

//

function main(builder: ConfigStruct.Builder, root: ConfigStruct.ScriptDeviceObject, logicModules: ConfigStruct.ScriptDeviceModule[], confFirmware: ConfigStruct.ModuleFirmware,
	log: ConfigStruct.IssueLogger, signalSet: ConfigStruct.SignalSet, subsystemStorage: ConfigStruct.SubsystemStorage, opticModuleStorage: ConfigStruct.OptoModuleStorage, logicModuleDescription: ConfigStruct.LogicModule): boolean
{

	if (logicModules.length != 0)
	{
		log.writeMessage("Subsystem " + " VDU " + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
	}

	for (let i: number = 0; i < logicModules.length; i++)
	{
		if (logicModules[i].moduleFamily == FamilyVDUID)
		{
			LMNumberCount++;
		}
	}

	for (let i: number = 0; i < logicModules.length; i++)
	{

		if (logicModules[i].moduleFamily != FamilyVDUID)
		{
			continue;
		}

		let result: boolean = module_vdu_1(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
		if (result == false)
		{
			return false;
		}

		if (builder.jsIsInterruptRequested() == true)
		{
			return true;
		}
	}

	return true;
}



function module_vdu_1(builder: ConfigStruct.Builder, root: ConfigStruct.ScriptDeviceObject, module: ConfigStruct.ScriptDeviceModule, confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger,
	signalSet: ConfigStruct.SignalSet, subsystemStorage: ConfigStruct.SubsystemStorage, opticModuleStorage: ConfigStruct.OptoModuleStorage, logicModuleDescription: ConfigStruct.LogicModule): boolean
{

	if (module.moduleFamily == FamilyVDUID)
	{
		// Generate Configuration
		//
		return generate_vdu(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
	}

	return false;
}

// Generate configuration for module VDU
//
//
function generate_vdu(builder: ConfigStruct.Builder, root: ConfigStruct.ScriptDeviceObject, module: ConfigStruct.ScriptDeviceModule, confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger,
	signalSet: ConfigStruct.SignalSet, subsystemStorage: ConfigStruct.SubsystemStorage, opticModuleStorage: ConfigStruct.OptoModuleStorage, logicModuleDescription: ConfigStruct.LogicModule)
{

	let checkProperties: string[] = ["AppLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID"];
	for (let cp: number = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.equipmentId);
			return false;
		}
	}

	// Variables
	//
	let subSysID: string = module.equipmentId;
	let LMNumber: number = module.place;
	let moduleId: number = module.moduleFamily + module.moduleVersion;

	// Constants
	//
	let frameSize: number = logicModuleDescription.FlashMemory_ConfigFramePayload;
	let frameCount: number = logicModuleDescription.FlashMemory_ConfigFrameCount;

	
	if (frameSize < 1016)
	{
		log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.equipmentId);
		return false;
	}

	if (frameCount != 2 )
	{
		log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 2, 2, module.equipmentId);
		return false;
	}

	let appWordsCount: number = module.propertyInt("AppLANDataSize");
	let diagWordsCount: number = logicModuleDescription.Memory_TxDiagDataSize;

	let ssKeyValue: number = subsystemStorage.ssKeyForVdu(subSysID);
	if (ssKeyValue == -1) {
		log.errCFG3001(subSysID, module.equipmentId);
		return false;
	}

	let maxLMNumber: number = 14;               // Can be changed!
	let configFrame: number = 1;

	if (LMNumber < 1 || LMNumber > maxLMNumber)
	{
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

	let ptr: number = 0;

	// Configuration storage format
	//
	const VDU_Cfg_Data_Version:number = 1;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "VDU_Cfg_Data_Version", VDU_Cfg_Data_Version) == false)     //CFG_Marker
	{
		return false;
	}
	confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] VDU_Cfg_Data_Version = " + VDU_Cfg_Data_Version + "\r\n");
	ptr += 2;

	// SS Key
	
	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "ssKeyValue", ssKeyValue) == false)     //ssKey
	{
		return false;
	}
	confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] ssKeyValue = " + ssKeyValue + "\r\n");
	ptr += 2;

	// UniqueID
	
	ptr += 8;

	const OptoQuantity:number = 8;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "OptoQuantity", OptoQuantity) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] OptoQuantity = " + OptoQuantity + "\r\n");
	ptr += 2;

	ptr += 4;	// Reserved

	// Create OPTO configuration
	//
	confFirmware.writeLog("Writing OPTO configuration.\r\n");

	if (ConfigLib.generate_vduTxRxOptoConfiguration(confFirmware, log, configFrame, ptr, module, LMNumber, opticModuleStorage, logicModuleDescription) == false)
	{
		return false;
	}

	// Create LANs configuration
	//
	confFirmware.writeLog("Writing LAN configuration.\r\n");

	const lanConfigPtr:number = 89 * 2;
	
	const lanConfigSize:number = 40 * 2;

	ptr = lanConfigPtr;

	const maxLanControllerCount: number = 3;

	let LanQuantity: number = logicModuleDescription.Lan_ControllerCount;

	if (LanQuantity < 1 || LanQuantity > maxLanControllerCount)
	{
		log.writeError(module.equipmentId + ": wrong LAN controllers count (" + LanQuantity + "), expected 1.." + maxLanControllerCount);
		return false;
	}

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, configFrame, ptr, "lanControllerCount", LanQuantity) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + configFrame + ":" + ptr + "] LanQuantity = " + LanQuantity + "\r\n");
	ptr += 2;

	ptr += 4;	// Reserved

	let appAndDiagChannel: number = 0;

	for (let i: number = 0; i < LanQuantity; i++)
	{
		let lanPlace: number = logicModuleDescription.jsLanControllerPlace(i);

		if (lanPlace < 1 || lanPlace > maxLanControllerCount)
		{
			log.writeError(module.equipmentId + ": wrong LAN controller place in LM description (" + lanPlace + "), expected 1.." + maxLanControllerCount);
			return false;
		}

		let lanType: ConfigStruct.LanControllerType = logicModuleDescription.jsLanControllerType(i);

		let ethernetcontrollerId: string = "_ETHERNET0" + lanPlace;

		confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");

		let emptyLan: ConfigStruct.LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: 0,
			dataID: 0
		};

		let appLan: ConfigStruct.LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: appWordsCount,
			dataID: 0
		};

		let diagLan: ConfigStruct.LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: diagWordsCount,
			dataID: 0
		};

		let tuningLan: ConfigStruct.LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: 0,
			dataID: 0
		};

		if (appAndDiagChannel == 0)
		{
			// Set default values for first App and Diag channel

			appLan.serviceIP = 0xc0a80bfe;	//	192.168.11.254
			appLan.servicePort = 13322;

			diagLan.serviceIP = 0xc0a815fe;	//	192.168.21.254
			diagLan.servicePort = 13352;
		}

		if (lanType == ConfigStruct.LanControllerType.AppAndDiagData)
		{

			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.AppDataService, root, module, ethernetcontrollerId, appLan, log) == false)
			{
				return false;
			}

			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.DiagDataService, root, module, ethernetcontrollerId, diagLan, log) == false)
			{
				return false;
			}

			let lans: ConfigStruct.LanConfig[] = [];
			lans.push(appLan);
			lans.push(diagLan);
			lans.push(tuningLan);
			
			const lanDataPtr:number = ptr + lanConfigSize * i;
			
			if (ConfigLib.generate_LANConfiguration_v2(confFirmware, LMNumber, configFrame, lanDataPtr, module, ethernetcontrollerId, lans, log) == false)
			{
				return false;
			}

			appAndDiagChannel++;
		}
	}

	// UniqueId and CRC are computer in ConfigurationBuilder

	return true;
}



