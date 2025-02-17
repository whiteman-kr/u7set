// Generate configuration for module VDU
//
//
function generate_vdu(builder: ConfigStruct.Builder, root: ConfigStruct.ScriptDeviceObject, module: ConfigStruct.ScriptDeviceModule, confFirmware: ConfigStruct.ModuleFirmware, log: ConfigStruct.IssueLogger,
	signalSet: ConfigStruct.SignalSet, subsystemStorage: ConfigStruct.SubsystemStorage, opticModuleStorage: ConfigStruct.OptoModuleStorage, logicModuleDescription: ConfigStruct.LogicModule)
{

	let checkProperties: string[] = ["SubsystemID", "LMNumber", "AppLANDataSize", "DiagLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID", "MasterPIN"];
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
	let subSysID: string = module.propertyString("SubsystemID");
	let LMNumber: number = module.propertyInt("LMNumber");
	let moduleId: number = module.moduleFamily + module.moduleVersion;
	let masterPIN: string = module.propertyString("MasterPIN");

	// Constants
	//
	let frameSize: number = logicModuleDescription.FlashMemory_ConfigFramePayload;
	let frameCount: number = logicModuleDescription.FlashMemory_ConfigFrameCount;

	if (frameSize < 1016)
	{
		log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.equipmentId);
		return false;
	}

	if (frameCount < 66 /*2 + 4  frames * 16 channels*/)
	{
		log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 66, 65535, module.equipmentId);
		return false;
	}

	let uartId: number = logicModuleDescription.FlashMemory_ConfigUartId;

	const appWordsCount: number = module.propertyInt("AppLANDataSize");
	const diagWordsCount: number = module.propertyInt("DiagLANDataSize");
	//const diagWordsCount: number = logicModuleDescription.Memory_TxDiagDataSize;

	let ssKeyValue: number = subsystemStorage.ssKey(subSysID);
	if (ssKeyValue == -1)
	{
		log.errCFG3001(subSysID, module.equipmentId);
		return false;
	}

	const maxLMNumber: number = logicModuleDescription.FlashMemory_MaxConfigurationCount; 
	const configStartFrames: number = logicModuleDescription.FlashMemory_SingleConfigFirstFrame;
	const configFrameCount: number = logicModuleDescription.FlashMemory_SingleConfigFrameCount;

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
	confFirmware.writeLog("Subsystem ID = " + subSysID + "\r\n");
	confFirmware.writeLog("Key value = " + ssKeyValue + "\r\n");
	confFirmware.writeLog("ModuleID = " + moduleId.toString(16) + "h\r\n");
	confFirmware.writeLog("UartID = " + uartId + "\r\n");
	confFirmware.writeLog("Frame size = " + frameSize + "\r\n");
	confFirmware.writeLog("LMNumber = " + LMNumber + "\r\n");
	confFirmware.writeLog("LMDescriptionNumber = " + logicModuleDescription.descriptionNumber() + "\r\n");
	
	// Configuration storage format
	//
	let frameStorageConfig: number = 1;
	let ptr: number = 0;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Marker", 0xca70) == false)     //CFG_Marker
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
	ptr += 2;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Version", 0x0001) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
	ptr += 2;


	let ssKey: number = ssKeyValue << 6;             //0000SSKEYY000000b
	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "SubsystemKey", ssKey) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
	ptr += 2;

	let buildNo: number = confFirmware.buildNumber();
	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "BuildNo", buildNo) == false)
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] BuildNo = " + buildNo + "\r\n");
	ptr += 2;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMDescriptionNumber", logicModuleDescription.descriptionNumber()) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMDescriptionNumber = " + logicModuleDescription.descriptionNumber() + "\r\n");
	ptr += 2;

	// reserved
	ptr += 4;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMNumberCount", VDUNumberCount) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMNumberCount = " + VDUNumberCount + "\r\n");
	ptr += 2;

	let configIndexOffset: number = ptr + (LMNumber - 1) * (2/*offset*/ + 4/*reserved*/);
	let configFrame: number = configStartFrames + configFrameCount * (LMNumber - 1);

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, configIndexOffset, "ConfigStartFrame", configFrame) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");

	// Service information
	//
	confFirmware.writeLog("Writing service information.\r\n");

	const frameServiceConfig: number = configFrame;
	ptr = 0;
	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "VDUConfigurationVersion", 0x0001) == false)   //CFG_Ch_Vers
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
	ptr += 2;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "UartID", uartId) == false)  //CFG_Ch_Dtype == UARTID?
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] uartId = " + uartId + "\r\n");
	ptr += 2;

	//Hash (UniqueID) will be counted later, write zero for future replacement
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] UniqueID = 0\r\n");
	ptr += 8;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "ConfigDataQuantity", configFrameCount) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] ConfigDataQuantity = " + configFrameCount + "\r\n");
	ptr += 2;

	//Comparsion ID (UniqueID) will be counted later
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] VduConfigID = 0\r\n");
	ptr += 8;

	const OptoQuantity: number = logicModuleDescription.OptoInterface_OptoPortCount;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "OptoQuantity", OptoQuantity) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] OptoQuantity = " + OptoQuantity + "\r\n");
	ptr += 2;

	const LanQuantity: number = logicModuleDescription.Lan_ControllerCount;

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "LanQuantity", LanQuantity) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] LanQuantity = " + LanQuantity + "\r\n");
	ptr += 2;

	// masterPIN
	
	const masterPinHash: number = confFirmware.calcHash32(masterPIN);

	if (ConfigLib.setData32(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "MasterPIN", masterPinHash) == false)
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] MasterPIN = " + masterPinHash + "\r\n");
	ptr += 4;
	
	
	// Create LANs configuration
	//
	const lanConfigFrame: number = frameServiceConfig + 1;

	confFirmware.writeLog("Writing LAN configuration.\r\n");

	const maxLanControllerCount: number = 2;

	let lanControllerCount: number = logicModuleDescription.Lan_ControllerCount;

	if (lanControllerCount < 1 || lanControllerCount > maxLanControllerCount)
	{
		log.writeError(module.equipmentId + ": wrong LAN controllers count (" + lanControllerCount + "), expected 1.." + maxLanControllerCount);
		return false;
	}

	let appAndDiagChannel: number = 0;

	for (let i: number = 0; i < lanControllerCount; i++)
	{
		let lanPlace: number = logicModuleDescription.jsLanControllerPlace(i);

		if (lanPlace < 1 || lanPlace > maxLanControllerCount)
		{
			log.writeError(module.equipmentId + ": wrong LAN controller place in LM description (" + lanPlace + "), expected 1.." + maxLanControllerCount);
			return false;
		}

		let lanType: ConfigStruct.LanControllerType = logicModuleDescription.jsLanControllerType(i);

		let ethernetcontrollerId: string = "_ETHERNET0" + lanPlace;

		let lanFrame: number = lanConfigFrame +  (lanPlace - 1);

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

			if (ConfigLib.generate_LANConfiguration_v1(confFirmware,  lanFrame,  module, ethernetcontrollerId, lans, log) == false)
			{
				return false;
			}

			appAndDiagChannel++;
		}
	}

	
	// Create OPTO configuration
	//
	let txRxConfigFrame: number = lanConfigFrame + maxLanControllerCount;

	confFirmware.writeLog("Writing OPTO configuration.\r\n");

	if (ConfigLib.generate_vduTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame,  module, LMNumber, opticModuleStorage, logicModuleDescription) == false)
	{
		return false;
	}

	// UniqueID
	//
	let startFrame: number = frameServiceConfig;

	let uniqueIDHi: number = 0;
	let uniqueIDLo: number = 0;

	for (let i: number = 0; i < configFrameCount; i++)
	{
		let dataBuffer: ArrayBuffer = confFirmware.calcCrc64(startFrame + i, 0, frameSize);
		if (dataBuffer.byteLength != 8)
		{
			log.writeError(module.equipmentId + ": calcCrc64 function returned " + dataBuffer.byteLength + " bytes, expected 8");
			return false;
		}

		let uint32View = new Uint32Array(dataBuffer);
		uniqueIDHi ^= uint32View[0];
		uniqueIDLo ^= uint32View[1];
	}

	confFirmware.jsSetUniqueID64(LMNumber, uniqueIDLo, uniqueIDHi);

	// VDU Comparsion id (without service frame)
	//
	startFrame = frameServiceConfig + 1;

	let configIDHi: number = 0;
	let configIDLo: number = 0;

	for (let i: number = 0; i < configFrameCount - 1/*service frame is skipped*/; i++)
	{
		let dataBuffer: ArrayBuffer = confFirmware.calcCrc64(startFrame + i, 0, frameSize);
		if (dataBuffer.byteLength != 8)
		{
			log.writeError(module.equipmentId + ": calcCrc64 function returned " + dataBuffer.byteLength + " bytes, expected 8");
			return false;
		}

		let uint32View = new Uint32Array(dataBuffer);
		configIDHi ^= uint32View[0];
		configIDLo ^= uint32View[1];
	}

	ptr = 14;
	if (ConfigLib.setData64(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "VduConfigID", configIDHi, configIDLo) == false)
	{
		return false;
	}
	const oldString: string = "    [" + frameServiceConfig + ":" + ptr + "] VduConfigID = 0\r\n";
	const newString: string = "    [" + frameServiceConfig + ":" + ptr + "] VduConfigID = 0x" + (configIDHi >>> 0).toString(16) + (configIDLo >>> 0).toString(16) + "\r\n";
	confFirmware.replaceLog(subSysID, oldString, newString);
	ptr += 8;

	return true;
}



