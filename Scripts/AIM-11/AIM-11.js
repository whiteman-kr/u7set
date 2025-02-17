// Generate configuration for module AIM-11
//
//

function generate_aim_11(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage) {

	const compareEqual = 0;
	const compareLess = 1;
	const compareMore = 2;

	const CHANNEL_1_ENABLE_FLAG = 0x1;
	const CHANNEL_2_ENABLE_FLAG = 0x2;
	const CHANNEL_OHM_ENABLE_FLAG = 0x4;

	// RMode

	const RMode_2Wire = 0;
	const RMode_3Wire = 1;
	const RMode_4Wire = 2;

	const unitsCount = 4;

	const channelsCount = 3;
	const channelNames = ["A", "B", "C"];

	const defaultR = [250, 250, 100];
	const resistorNames = ["RLoad", "RLoad", "R0"];

	const tsConstant = 0.0005;	// 0.5 ms
	const defaultTf = (5 * 0.001) / tsConstant;	// 5 ms

	const defaultFlags = 0;

	const defaultHighBound = [12, 12, 2000];
	const defaultLowBound = [0, 0, 0];

	const defaultK1 = 1.0;
	const defaultK2 = 0.0;

	let unitsConvertor = confFirmware.jsGetUnitsConvertor();
	if (unitsConvertor == null) {
		log.errINT1001("confFirmware.jsGetUnitsConvertor returned null");
		return false;
	}

	// ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
	//

	for (let i = 0; i < unitsCount; i++) {

		const unitPtr = (38 * i) * 2;

		let flags = defaultFlags;

		for (let c = 0; c < channelsCount; c++) {

			let filteringTime = defaultTf;

			let r = defaultR[c];
			let rConnectionMode = RMode_2Wire;

			let k1 = defaultK1;
			let k2 = defaultK2;

			let highValidRange = defaultHighBound[c];
			let lowValidRange = defaultLowBound[c];

			let signalStrId = module.equipmentId + "_CTRLIN_IN0" + (i + 1) + channelNames[c];

			var signal = signalSet.getSignalByEquipmentID(signalStrId);

			if (signal == null) {
				// Generate default values, there is no signal on this place
				//
				log.wrnCFG3007(signalStrId);
			}
			else {
				// TF

				let tf = signal.filteringTime();

				if (tf < 0 * tsConstant || tf > 65535 * tsConstant) {
					log.errCFG3010("FilteringTime", tf, 0 * tsConstant, 65535 * tsConstant, 6, signalStrId);
				}

				tf = tf / tsConstant;

				filteringTime = valToADC(tf, 0, 65535, 0, 0xffff);

				// Electric limits

				let electricUnit = signal.propertyValue("ElectricUnit");
				if (electricUnit == undefined) {
					log.errCFG3000("ElectricUnit", signalStrId);
					return false;
				}

				if (electricUnit == ConfigStruct.ElectricUnit.NoUnit) {

					// Signal should be excluded from build

					let excludeFromBuild = signal.propertyValue("ExcludeFromBuild");
					if (excludeFromBuild == undefined) {
						log.errCFG3000("ExcludeFromBuild", signalStrId);
						return false;
					}

					if (excludeFromBuild == false) {
						log.errCFG3041("ExcludeFromBuild", "false", "true", signalStrId);
						continue;
					}
				}
				else {
					// Get SensorType and check if it is valid for chosen ElectricUnit

					let sensorType = signal.propertyValue("SensorType");
					if (sensorType == undefined) {
						log.errCFG3000("SensorType", signalStrId);
						return false;
					}

					if (electricUnit == ConfigStruct.ElectricUnit.mA) {

						if (sensorType != ConfigStruct.SensorType.V_m10_p10) {
							log.errCFG3041("SensorType", unitsConvertor.sensorTypeName(sensorType), unitsConvertor.sensorTypeName(ConfigStruct.SensorType.V_m10_p10), signalStrId);
							continue;
						}

						// Get RLoad

						r = signal.propertyValue("Rload_Ohm");
						if (r == undefined) {
							log.errCFG3000("Rload_Ohm", signalStrId);
							return false;
						}
					}

					if (electricUnit == ConfigStruct.ElectricUnit.V) {

						if (sensorType != ConfigStruct.SensorType.V_m10_p10) {
							log.errCFG3041("SensorType", unitsConvertor.sensorTypeName(sensorType), unitsConvertor.sensorTypeName(ConfigStruct.SensorType.V_m10_p10), signalStrId);
							continue;
						}
					}

					if (electricUnit == ConfigStruct.ElectricUnit.mV) {

						if (sensorType == ConfigStruct.SensorType.NoSensor || sensorType == ConfigStruct.SensorType.V_m10_p10) {
							log.errCFG3041("SensorType", unitsConvertor.sensorTypeName(sensorType), "mV_Raw_* or mV_Type_*", signalStrId);
							continue;
						}
					}

					if (electricUnit == ConfigStruct.ElectricUnit.Ohm) {
						if (sensorType == ConfigStruct.SensorType.NoSensor) {
							log.errCFG3041("SensorType", unitsConvertor.sensorTypeName(sensorType), "Ohm_*", signalStrId);
							continue;
						}

						// Get R0

						r = signal.propertyValue("R0_Ohm");
						if (r == undefined) {
							log.errCFG3000("R0_Ohm", signalStrId);
							return false;
						}

						// Get RTDConfMode

						rConnectionMode = signal.propertyValue("RTDConfMode");
						if (rConnectionMode == undefined) {
							log.errCFG3000("RTDConfMode", signalStrId);
							return false;
						}
					}

					// Get required signal properties

					let electricHighLimit = signal.propertyValue("ElectricHighLimit");
					if (electricHighLimit == undefined) {
						log.errCFG3000("ElectricHighLimit", signalStrId);
						return false;
					}

					let electricLowLimit = signal.propertyValue("ElectricLowLimit");
					if (electricLowLimit == undefined) {
						log.errCFG3000("ElectricLowLimit", signalStrId);
						return false;
					}

					let decimalPlaces = signal.propertyValue("DecimalPlaces");
					if (decimalPlaces == undefined) {
						log.errCFG3000("DecimalPlaces", signalStrId);
						return false;
					}

					let highEngineeringUnits = signal.highEngineeringUnits();
					let lowEngineeringUnits = signal.lowEngineeringUnits();

					highValidRange = signal.highValidRange();
					lowValidRange = signal.lowValidRange();

					// Check properties of signal

					let signalParamsOk = true;

					if (electricHighLimit < electricLowLimit) {
						// error
						log.errCFG3013("ElectricHighLimit", electricHighLimit, compareLess, "ElectricLowLimit", electricLowLimit, 0, signalStrId);
						signalParamsOk = false;
					}
					if (electricHighLimit == electricLowLimit) {
						// error
						log.errCFG3013("ElectricHighLimit", electricHighLimit, compareEqual, "ElectricLowLimit", electricLowLimit, 0, signalStrId);
						signalParamsOk = false;
					}
					if (signal.highEngineeringUnits() == signal.lowEngineeringUnits()) {
						// error
						log.errCFG3013("HighEngineeringUnits", signal.highEngineeringUnits(), compareEqual, "LowEngineeringUnits", signal.lowEngineeringUnits(), signal.decimalPlaces(), signalStrId);
						signalParamsOk = false;
					}
					if (signal.highValidRange() == signal.lowValidRange()) {
						// error
						log.errCFG3013("HighValidRange", signal.highValidRange(), compareEqual, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
						signalParamsOk = false;
					}
					if (signal.highEngineeringUnits() > signal.lowEngineeringUnits() && signal.highValidRange() < signal.lowValidRange()) {
						// error
						log.errCFG3013("HighValidRange", signal.highValidRange(), compareLess, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
						signalParamsOk = false;
					}
					if (signal.highEngineeringUnits() < signal.lowEngineeringUnits() && signal.highValidRange() > signal.lowValidRange()) {
						// error
						log.errCFG3013("HighValidRange", signal.highValidRange(), compareMore, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
						signalParamsOk = false;
					}

					if (signalParamsOk == false) {
						continue;
					}

					// Convert electric to physical

					let highPhysicalConvertResult = null;
					let lowPhysicalConvertResult = null;

					switch (electricUnit) {
						case ConfigStruct.ElectricUnit.V:
						case ConfigStruct.ElectricUnit.mA:
							{
								highPhysicalConvertResult = unitsConvertor.electricToPhysical_Input(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, r);
								lowPhysicalConvertResult = unitsConvertor.electricToPhysical_Input(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, r);
							}
							break;
						case ConfigStruct.ElectricUnit.mV:
							{
								highPhysicalConvertResult = unitsConvertor.electricToPhysical_ThermoCouple(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType);
								lowPhysicalConvertResult = unitsConvertor.electricToPhysical_ThermoCouple(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType);
							}
							break;
						case ConfigStruct.ElectricUnit.Ohm:
							{
								highPhysicalConvertResult = unitsConvertor.electricToPhysical_ThermoResistor(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, r);
								lowPhysicalConvertResult = unitsConvertor.electricToPhysical_ThermoResistor(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, r);
							}
							break;
						default:
							log.errCFG3041("ElectricUnit", unitsConvertor.electricUnitName(electricUnit), "V, mA, mV, Ohm", signalStrId);
							continue;

					}

					if (highPhysicalConvertResult == null || lowPhysicalConvertResult == null) {
						log.errINT1001("UnitsConvertor call error, module " + module.equipmentId + ", signal " + signalStrId);
						return false;
					}

					if (highPhysicalConvertResult.ok == false) {
						switch (highPhysicalConvertResult.errorCode) {
							case ConfigStruct.UnitsConvertorErrorCode.ErrorGeneric:
								{
									log.errINT1001(highPhysicalConvertResult.errorMessage + ", module " + module.equipmentId + ", signal " + signalStrId);
								}
								break;
							case ConfigStruct.UnitsConvertorErrorCode.LowLimitOutOfRange:
								{
									log.errCFG3010("ElectricLowLimit", electricLowLimit, highPhysicalConvertResult.expectedLowValidRange, highPhysicalConvertResult.expectedHighValidRange, 4, signalStrId);
								}
								break;
							case ConfigStruct.UnitsConvertorErrorCode.HighLimitOutOfRange:
								{
									log.errCFG3010("ElectricHighLimit", electricHighLimit, highPhysicalConvertResult.expectedLowValidRange, highPhysicalConvertResult.expectedHighValidRange, 4, signalStrId);
								}
								break;
							default:
								log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + highPhysicalConvertResult.errorCode + "), signal " + signalStrId);
								return false;
						}
						continue;
					}

					if (lowPhysicalConvertResult.ok == false) {
						switch (lowPhysicalConvertResult.errorCode) {
							case ConfigStruct.UnitsConvertorErrorCode.ErrorGeneric:
								{
									log.errINT1001(lowPhysicalConvertResult.errorMessage + ", module " + module.equipmentId + ", signal " + signalStrId);
								}
								break;
							case ConfigStruct.UnitsConvertorErrorCode.LowLimitOutOfRange:
								{
									log.errCFG3010("ElectricLowLimit", electricLowLimit, lowPhysicalConvertResult.expectedLowValidRange, lowPhysicalConvertResult.expectedHighValidRange, 4, signalStrId);
								}
								break;
							case ConfigStruct.UnitsConvertorErrorCode.HighLimitOutOfRange:
								{
									log.errCFG3010("ElectricHighLimit", electricHighLimit, lowPhysicalConvertResult.expectedLowValidRange, lowPhysicalConvertResult.expectedHighValidRange, 4, signalStrId);
								}
								break;
							default:
								log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + lowPhysicalConvertResult.errorCode + "), signal " + signalStrId);
								return false;
						}
						continue;
					}

					let physicalLowLimit = lowPhysicalConvertResult.toDouble;
					let physicalHighLimit = highPhysicalConvertResult.toDouble;

					//log.writeMessage(signalStrId + " physicalLowLimit = " + physicalLowLimit);
					//log.writeMessage(signalStrId + " physicalHighLimit = " + physicalHighLimit);

					if (physicalHighLimit == physicalLowLimit) {
						// error
						log.errCFG3013("physicalLowLimit", physicalLowLimit, compareEqual, "physicalHighLimit", physicalHighLimit, 0, signalStrId);
						continue;
					}

					// Calculate coefficients

					let y1 = lowEngineeringUnits;
					let y2 = highEngineeringUnits;

					let x1 = physicalLowLimit;
					let x2 = physicalHighLimit;

					if (x1 == x2) // Prevent division by zero
					{
						x1 = 0;
						x2 = 1;
					}

					k1 = (y2 - y1) / (x2 - x1);	// K
					k2 = y1 - k1 * x1;			// B

					// Check valid ranges

					let highSensorPhysicalRange = 0;
					let lowSensorPhysicalRange = 0;

					switch (sensorType) {

						// ---------- mA / V---------------

						case ConfigStruct.SensorType.V_m10_p10:
							{
								switch (electricUnit) {
									case ConfigStruct.ElectricUnit.V:
										highSensorPhysicalRange = 12;
										lowSensorPhysicalRange = -12;
										break;
									case ConfigStruct.ElectricUnit.mA:
										highSensorPhysicalRange = 12;
										lowSensorPhysicalRange = -12;
										break;
									default:
										{
											log.errINT1001("Wrong electric unit type '" + unitsConvertor.electricUnitName(ElectricUnit) + "' in " + signalStrId);
											return false;
										}
								}
							}
							break;

						// ---------- mV ---------------

						case ConfigStruct.SensorType.mV_Raw_m1200_p1200:
							highSensorPhysicalRange = 1200;
							lowSensorPhysicalRange = -1200;
							break;
						case ConfigStruct.SensorType.mV_Type_B:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = 0;
							break;
						case ConfigStruct.SensorType.mV_Type_E:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_J:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_K:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_N:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_R:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_S:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_T:
							highSensorPhysicalRange = 400;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_L:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.mV_Type_M:
							highSensorPhysicalRange = 100;
							lowSensorPhysicalRange = -50;
							break;

						// ---------- Ohm ---------------

						case ConfigStruct.SensorType.Ohm_Raw:
							highSensorPhysicalRange = 10000;
							lowSensorPhysicalRange = 0;
							break;
						case ConfigStruct.SensorType.Ohm_Pt_a_385:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.Ohm_Pt_a_391:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.Ohm_Cu_a_428:
							highSensorPhysicalRange = 200;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.Ohm_Cu_a_426:
							highSensorPhysicalRange = 200;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.Ohm_Pt21:
							highSensorPhysicalRange = 500;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.Ohm_Cu23:
							highSensorPhysicalRange = 180;
							lowSensorPhysicalRange = -50;
							break;
						case ConfigStruct.SensorType.Ohm_Ni_a_617:
							highSensorPhysicalRange = 180;
							lowSensorPhysicalRange = -50;
							break;
						default:
							{
								log.errINT1001("Unknown sensor type '" + unitsConvertor.sensorTypeName(sensorType) + "' in " + signalStrId);
								return false;
							}
					}

					//log.writeMessage(signalStrId + " k1 = " + k1);
					//log.writeMessage(signalStrId + " k2 = " + k2);

					//log.writeMessage(signalStrId + " lowSensorPhysicalRange = " + lowSensorPhysicalRange);
					//log.writeMessage(signalStrId + " highSensorPhysicalRange = " + highSensorPhysicalRange);

					let lowEngineeringUnitsMin = lowSensorPhysicalRange * k1 + k2;
					let lowEngineeringUnitsMax = highSensorPhysicalRange * k1 + k2;

					// Round this value to supplied decimal places
					//
					lowEngineeringUnitsMin = parseFloat(lowEngineeringUnitsMin.toFixed(decimalPlaces));
					lowEngineeringUnitsMax = parseFloat(lowEngineeringUnitsMax.toFixed(decimalPlaces));

					//log.writeMessage(signalStrId + " lowEngineeringUnitsMin = " + lowEngineeringUnitsMin);
					//log.writeMessage(signalStrId + " lowEngineeringUnitsMax = " + lowEngineeringUnitsMax);

					//

					if (lowValidRange < lowEngineeringUnitsMin) {
						log.errCFG3010("LowValidRange", lowValidRange, lowEngineeringUnitsMin, lowEngineeringUnitsMax, decimalPlaces, signalStrId);
					}
					if (highValidRange > lowEngineeringUnitsMax) {
						log.errCFG3010("HighValidRange", highValidRange, lowEngineeringUnitsMin, lowEngineeringUnitsMax, decimalPlaces, signalStrId);
					}

					// Configure flags word

					flags |= (1 << c);	// Set Channel Enable bit

					let sensorTypeCode = 0;

					if (electricUnit == ConfigStruct.ElectricUnit.V) {
						sensorTypeCode = 0;
					}

					if (electricUnit == ConfigStruct.ElectricUnit.mA) {
						sensorTypeCode = 1;
					}

					if (electricUnit == ConfigStruct.ElectricUnit.mV) {
						switch (sensorType) {
							case ConfigStruct.SensorType.mV_Raw_m1200_p1200: sensorTypeCode = 2; break;
							case ConfigStruct.SensorType.mV_Type_K: 			sensorTypeCode = 3; break;
							case ConfigStruct.SensorType.mV_Type_L:			sensorTypeCode = 4; break;
							case ConfigStruct.SensorType.mV_Type_R: 			sensorTypeCode = 5; break;
							case ConfigStruct.SensorType.mV_Type_S: 			sensorTypeCode = 6; break;
							case ConfigStruct.SensorType.mV_Type_B: 			sensorTypeCode = 7; break;
							case ConfigStruct.SensorType.mV_Type_J: 			sensorTypeCode = 8; break;
							case ConfigStruct.SensorType.mV_Type_T: 			sensorTypeCode = 9; break;
							case ConfigStruct.SensorType.mV_Type_E: 			sensorTypeCode = 0xa; break;
							case ConfigStruct.SensorType.mV_Type_N: 			sensorTypeCode = 0xb; break;
							case ConfigStruct.SensorType.mV_Type_M: 			sensorTypeCode = 0xc; break;
							default:
								log.errINT1001("Unknown sensor type '" + unitsConvertor.sensorTypeName(sensorType) + "' in " + signalStrId);
								return false;
						}
					}

					if (electricUnit == ConfigStruct.ElectricUnit.Ohm) {
						switch (sensorType) {
							case ConfigStruct.SensorType.Ohm_Raw: 		sensorTypeCode = 0; break;
							case ConfigStruct.SensorType.Ohm_Pt_a_385: 	sensorTypeCode = 1; break;
							case ConfigStruct.SensorType.Ohm_Pt_a_391: 	sensorTypeCode = 2; break;
							case ConfigStruct.SensorType.Ohm_Cu_a_428: 	sensorTypeCode = 3; break;
							case ConfigStruct.SensorType.Ohm_Cu_a_426: 	sensorTypeCode = 4; break;
							case ConfigStruct.SensorType.Ohm_Pt21: 		sensorTypeCode = 5; break;
							case ConfigStruct.SensorType.Ohm_Cu23: 		sensorTypeCode = 6; break;
							case ConfigStruct.SensorType.Ohm_Ni_a_617: 	sensorTypeCode = 7; break;
							default:
								log.errINT1001("Unknown sensor type '" + unitsConvertor.sensorTypeName(sensorType) + "' in " + signalStrId);
								return false;
						}

						flags |= (rConnectionMode << 18);
					}

					flags |= (sensorTypeCode << (3 + 5 * c));

				}	// End of ElectricUnit is set

				let channelPtr = unitPtr + (2 + c * 12) * 2;

				let fPtr = channelPtr;
				let rPtr = channelPtr + 2 * 2;
				let kPtr = channelPtr + 4 * 2;
				let rangePtr = channelPtr + 8 * 2;

				// Write configuration
				confFirmware.writeLog("    unit " + (i + 1) + " channel " + channelNames[c] + " (" + unitsConvertor.electricUnitName(electricUnit) +
					"): [ " + frame + ":" + fPtr + "] Tf = " + filteringTime +
					"; [" + frame + ":" + rPtr + "] " + resistorNames[c] + " = " + r +
					"; [" + frame + ":" + kPtr + "] K1 = " + k1 +
					"; [" + frame + ":" + (kPtr + 2 * 2) + "] K2 = " + k2 +
					"; [" + frame + ":" + rangePtr + "] HighValidRange = " + highValidRange +
					"; [" + frame + ":" + (rangePtr + 2 * 2) + "] LowValidRange = " + lowValidRange + "\r\n");

				
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, fPtr, "Tf", filteringTime) == false)          // Filtering time constant
				{
					return false;
				}

				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, rPtr, resistorNames[c], r) == false)      // R
				{
					return false;
				}

				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, kPtr, "K1", k1) == false)         // K1
				{
					return false;
				}

				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, kPtr + 2 * 2, "K2", k2) == false)         // K2
				{
					return false;
				}

				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, rangePtr, "HighValidRange", highValidRange) == false)         // In High bound
				{
					return false;
				}

				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, rangePtr + 2 * 2, "LowValidRange", lowValidRange) == false)          // In Low Bound
				{
					return false;
				}
			}

		} // channelsCount

		// Check for Alternative channel is exclusively used

		if ((flags & CHANNEL_OHM_ENABLE_FLAG) != 0) {
			if ((flags & CHANNEL_1_ENABLE_FLAG) != 0 || (flags & CHANNEL_2_ENABLE_FLAG) != 0) {
				log.writeError("Alternative (Ohm)channel can be enabled only if primary channels are disabled, Unit " + (i + 1) + ", module " + module.equipmentId);
			}
		}

		// Write Flags

		confFirmware.writeLog("    unit " + (i + 1) + ": [" + frame + ":" + unitPtr + "] Flags = " + flags + "\r\n");

		if (setData32(confFirmware, log, LMNumber, module.equipmentId, frame, unitPtr, "WordOfFlags", flags) == false) {
			return false;
		}

	} // unitsCount

	// final crc

	let ptr = 376;

	let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
	if (stringCrc64 == "") {
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = 0x" + stringCrc64 + "\r\n");
	ptr += 8;

	ptr = 1008;

	// ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
	//
	let dataTransmittingEnableFlag = false;
	let dataReceiveEnableFlag = true;

	let flags = 0;
	if (dataTransmittingEnableFlag == true)
		flags |= 1;
	if (dataReceiveEnableFlag == true)
		flags |= 2;

	let configFramesQuantity = 3;
	let dataFramesQuantity = 0;

	let txId = module.customModuleFamily + module.moduleVersion;

	if (generate_txRxIoConfig(confFirmware, module.equipmentId, LMNumber, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId) == false) {
		return false;
	}
	ptr += 8;

	// assert if we not on the correct place
	//
	if (ptr != 1016) {
		ptr = 1016;
	}

	return true;
}
