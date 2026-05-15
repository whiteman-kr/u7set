// Generate configuration for module LM
//
//
function generate_lm(builder: ConfigStruct.Builder, 
	root: ConfigStruct.ScriptDeviceObject, 
	module: ConfigStruct.ScriptDeviceModule, 
	confFirmware: ConfigStruct.ModuleFirmware, 
	log: ConfigStruct.IssueLogger,
	signalSet: ConfigStruct.SignalSet, 
	subsystemStorage: ConfigStruct.SubsystemStorage, 
	opticModuleStorage: ConfigStruct.OptoModuleStorage, 
	logicModuleDescription: ConfigStruct.LogicModule) {

	let checkProperties: string[] = ["SubsystemID", "LMNumber", "AppLANDataSize", "DiagLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID"];
	for (let cp: number = 0; cp < checkProperties.length; cp++) {
		if (module.propertyValue(checkProperties[cp]) == undefined) {
			log.errCFG3000(checkProperties[cp], module.equipmentId);
			return false;
		}
	}

	const MODULEID_LM1_SF00: number = 0x1100;
	const MODULEID_LM1_SF01: number = 0x1101;
	const MODULEID_LM1_SF41: number = 0x1102;	// LM1-SF41-6PH

	const MODULEID_LM1_SR01: number = 0x11A0;
	const MODULEID_LM1_SR02: number = 0x11A1;
	const MODULEID_LM1_SR03: number = 0x11A2;
	const MODULEID_LM1_SR04: number = 0x11B0;

	const MODULEID_LM1_SR20: number = 0x11A3;
	const MODULEID_LM1_SR05: number = 0x11B2;
	const MODULEID_LM8_SR10: number = 0x11D0;
	const MODULEID_LM11_SR90: number = 0x1190;

	// Variables
	//
	let subSysID: string = module.propertyString("SubsystemID");
	let LMNumber: number = module.propertyInt("LMNumber");
	let moduleId: number = module.moduleFamily + module.moduleVersion;
	let modulePlace: number = module.place;
	
	// LM1-SF41-6PH-specific properties
	//
	let plantTimeEnable: boolean = false;
	if (moduleId == MODULEID_LM1_SF41)
	{
		if (module.propertyValue("PlantTimeEnable") == undefined)
		{
			log.errCFG3000("PlantTimeEnable", module.equipmentId);
			return false;
		}

		plantTimeEnable = module.propertyBool("PlantTimeEnable");
	}

	// Check place
	//
	if (modulePlace != 0)
	{
		log.errCFG3002("Place", modulePlace, 0, 0, module.equipmentId);
		return false;
	}

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

	const appWordsCount: number = module.propertyInt("AppLANDataSize");
	const diagWordsCount: number = module.propertyInt("DiagLANDataSize");

	let ssKeyValue: number = subsystemStorage.ssKey(subSysID);
	if (ssKeyValue == -1) {
		log.errCFG3001(subSysID, module.equipmentId);
		return false;
	}

	const maxLMNumber: number = logicModuleDescription.FlashMemory_MaxConfigurationCount; 
	const configStartFrames: number = logicModuleDescription.FlashMemory_SingleConfigFirstFrame;
	const configFrameCount: number = logicModuleDescription.FlashMemory_SingleConfigFrameCount;

	let ioModulesMaxCount: number = 14;

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
	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "BuildNo", buildNo) == false) {
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

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMNumberCount", LMNumberCount) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMNumberCount = " + LMNumberCount + "\r\n");
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

	let frameServiceConfig: number = configFrame;
	ptr = 0;
	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "ServiceVersion", 0x0001) == false)   //CFG_Ch_Vers
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

	//CFG Values Quantity is not used for now

	ptr += 2;

	//PlantTimeEnable

	if (ConfigLib.setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "PlantTimeEnable", 
		(plantTimeEnable === true ? 1 : 0)) == false)
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] PlantTimeEnable = " + (plantTimeEnable === true ? 1 : 0) + "\r\n");
	ptr += 2;

	// I/O Modules configuration
	//
	confFirmware.writeLog("Writing I/O modules configuration.\r\n");

	let frameIOConfig: number = configFrame + 1;

	if (module.parent().isChassis() === false) {
		log.errCFG3042(module.equipmentId, module.uuid)
	}

	let parent: ConfigStruct.ScriptDeviceChassis = module.parent().toChassis();

	for (let i: number = 0; i < parent.childrenCount; i++) {
		if (builder.jsIsInterruptRequested() == true) {
			return true;
		}

		if (parent.child(i).isModule() == false) {
			continue;
		}

		let ioModule: ConfigStruct.ScriptDeviceModule = parent.child(i).toModule();

		if (ioModule.moduleFamily == FamilyLMID) {
			continue;
		}

		let ioPlace: number = ioModule.place;
		if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
			log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioModule.equipmentId);
			return false;
		}

		let ioEquipmentID: string = ioModule.equipmentId;

		let checkProperties: string[] = ["ConfigurationScript"];
		for (let cp: number = 0; cp < checkProperties.length; cp++) {
			if (ioModule.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], ioEquipmentID);
				return false;
			}
		}

		let ioModuleFamily: number = ioModule.moduleFamily;

		let frame: number = frameIOConfig + ioPlace - 1;

		confFirmware.writeLog("Generating configuration for " + ioModule.caption + ": " + ioEquipmentID + " Place: " + ioModule.place + " Frame: " + frame + "\r\n");

		let configScript: string = ioModule.propertyString("ConfigurationScript");
		if (configScript.length != 0) {
			if (ConfigLib.runConfigScript(configScript, confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false) {
				return false;
			}
		}

		let diagWordsIoCount: number = ioModule.propertyInt("TxDiagDataSize");
		if (diagWordsIoCount == null) {
			log.errCFG3000("TxDiagDataSize", ioEquipmentID);
			return false;
		}

		if ((diagWordsIoCount & 1) != 0) {
			diagWordsIoCount++;	// Align to words
		}

		if (moduleId != MODULEID_LM1_SF00 &&
			moduleId != MODULEID_LM1_SF01 &&
			moduleId != MODULEID_LM1_SR01 &&
			moduleId != MODULEID_LM1_SR02 &&
			moduleId != MODULEID_LM1_SR03 && 
			moduleId != MODULEID_LM1_SR04) {

			// I/o module diag data size are written in new LMs
			//
			ptr = 1006;
			if (ConfigLib.setData16(confFirmware, log, LMNumber, ioModule.equipmentId, frame, ptr, "DiagDataSize", diagWordsIoCount) == false) {
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "] DiagDataSize = " + diagWordsIoCount + "\r\n");
		}
	}

	let lanConfigStartFrame: number = frameIOConfig + ioModulesMaxCount;
	let lanConfigCurrentFrame: number = lanConfigStartFrame;

	// Create LANs configuration
	//
	confFirmware.writeLog("Writing LAN configuration.\r\n");

	const maxLanControllerCount: number = 4;

	let lanControllerCount: number = logicModuleDescription.Lan_ControllerCount;

	if (lanControllerCount < 1 || lanControllerCount > maxLanControllerCount) {
		log.writeError(module.equipmentId + ": wrong LAN controllers count (" + lanControllerCount + "), expected 1.." + maxLanControllerCount);
		return false;
	}

	let appAndDiagChannel: number = 0;

	for (let i: number = 0; i < lanControllerCount; i++) {

		let lanPlace: number = logicModuleDescription.jsLanControllerPlace(i);

		if (lanPlace < 1 || lanPlace > maxLanControllerCount)
		{
			log.writeError(module.equipmentId + ": wrong LAN controller place in LM description (" + lanPlace + "), expected 1.." + maxLanControllerCount);
			return false;
		}

		let lanType: ConfigStruct.LanControllerType = logicModuleDescription.jsLanControllerType(i);
		let lanConfigVersion: ConfigStruct.LanControllerType = logicModuleDescription.jsLanControllerConfigVersion(i);

		let ethernetcontrollerId: string = "_ETHERNET0" + lanPlace;

		lanConfigCurrentFrame = lanConfigStartFrame + (lanPlace - 1);

		confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");

		let tuningLan: ConfigStruct.LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: 716,
			dataID: 0
		};

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

		if (appAndDiagChannel == 0) {
			// Set default values for first App and Diag channel

			appLan.serviceIP = 0xc0a80bfe;	//	192.168.11.254
			appLan.servicePort = 13322;

			diagLan.serviceIP = 0xc0a815fe;	//	192.168.21.254
			diagLan.servicePort = 13352;
		}
		
		if (lanType == ConfigStruct.LanControllerType.Tuning)
		{
			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.TuningService, root, module, ethernetcontrollerId, tuningLan, log) == false)
			{
				return false;
			}

			switch (lanConfigVersion)
			{
				case 0:
					if (ConfigLib.generate_LANConfiguration_v0(confFirmware, lanConfigCurrentFrame, module, ethernetcontrollerId, tuningLan, emptyLan, log) == false)	//Channel is not used
					{
						return false;
					}
					break;

				case 1:
					let lans: ConfigStruct.LanConfig[] = [];
					lans.push(emptyLan);
					lans.push(emptyLan);
					lans.push(tuningLan);

					if (ConfigLib.generate_LANConfiguration_v1(confFirmware, lanConfigCurrentFrame, module, ethernetcontrollerId, lans, log) == false)
					{
						return false;
					}
					break;

				default:
					log.writeError(ethernetcontrollerId + ": LAN description has unknown configuration version: " + lanConfigVersion);
					return false;
			}
		}

		if (lanType == ConfigStruct.LanControllerType.AppAndDiagData) {

			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.AppDataService, root, module, ethernetcontrollerId, appLan, log) == false) {
				return false;
			}

			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.DiagDataService, root, module, ethernetcontrollerId, diagLan, log) == false) {
				return false;
			}

			switch (lanConfigVersion)
			{
				case 0:
					if (ConfigLib.generate_LANConfiguration_v0(confFirmware, lanConfigCurrentFrame, module, ethernetcontrollerId, appLan, diagLan, log) == false)
					{
						return false;
					}
					break;

				case 1:
					let lans: ConfigStruct.LanConfig[] = [];
					lans.push(appLan);
					lans.push(diagLan);
					lans.push(emptyLan);

					if (ConfigLib.generate_LANConfiguration_v1(confFirmware, lanConfigCurrentFrame, module, ethernetcontrollerId, lans, log) == false)
					{
						return false;
					}
					break;

				default:
					log.writeError(ethernetcontrollerId + ": LAN description has unknown configuration version: " + lanConfigVersion);
					return false;
			}

			appAndDiagChannel++;
		}

		if (lanType == ConfigStruct.LanControllerType.TuningAndAppAndDiagData) {

			// For TuningAndAppAndDiagData, we are always using Version 1 of the configuration frame

			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.TuningService, root, module, ethernetcontrollerId, tuningLan, log) == false) {
				return false;
			}
			
			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.AppDataService, root, module, ethernetcontrollerId, appLan, log) == false) {
				return false;
			}

			if (ConfigLib.fillLanServiceData(confFirmware, ConfigStruct.SoftwareType.DiagDataService, root, module, ethernetcontrollerId, diagLan, log) == false) {
				return false;
			}

			let lans: ConfigStruct.LanConfig[] = [];
			lans.push(appLan);
			lans.push(diagLan);
			lans.push(tuningLan);

			switch (lanConfigVersion)
			{
				case 1:
					if (ConfigLib.generate_LANConfiguration_v1(confFirmware, lanConfigCurrentFrame, module, ethernetcontrollerId, lans, log) == false)
					{
						return false;
					}
					break;

				default:
					log.writeError(ethernetcontrollerId + ": LAN description has unsupported configuration version: " + lanConfigVersion);
					return false;
			}

			appAndDiagChannel++;
		}
	}

	// Create TX/RX configuration
	//
	confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");

	let txRxConfigFrame: number = lanConfigCurrentFrame + 1;	// Take next frame after the last LAN config frame

	if (ConfigLib.generate_lmTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
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

	return true;
}
