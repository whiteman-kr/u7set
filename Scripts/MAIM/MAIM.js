// Generate configuration for module MAIM
//
//
function generate_maim(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	var AIMSignalMaxCount = 14;
	
	var tsConstant = 200 * 0.000001;
	
	var defaultTf = valToADC(0/*5000*/ * 0.000001 / tsConstant, 0, 65535, 0, 0xffff);
	var defaultHighBoundC = 20;
	var defaultLowBoundC = -20;
	var defaultHighBoundN = 2;
	var defaultLowBoundN = -2;
	var defaultHighBoundR = 2000;
	var defaultLowBoundR = 0;
	var defaultK1 = 1.0;
	var defaultK2 = 0.0;
	var defaultWordOfFlags = 0;
	
	var signalBlockSize = 28 * 2; // in bytes
	
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");
    if (inControllerObject == null || inControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLIN",module.equipmentId);
		return false;
    }

    let inController =  inControllerObject.toController();
	
	// ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
	//
	
	for (var i = 0; i < AIMSignalMaxCount; i++)
	{
		// find a signal with Place = i
		//
		var signalStrIdTemplate = inController.equipmentId + "_IN";
		
		var entry = i + 1;
		if (entry < 10)
		{
			signalStrIdTemplate = signalStrIdTemplate + "0";
		}
		signalStrIdTemplate = signalStrIdTemplate + entry;
		
		var ptr = signalBlockSize * i;

		for (var s = 0; s < 4; s++)
		{
			var signalStrId = signalStrIdTemplate;
			
			var	defaultHighBound = 0;
			var defaultLowBound = 0;
			
			if (s == 0)
			{
				signalStrId =  signalStrId + "C";
				defaultHighBound = defaultHighBoundC;
				defaultLowBound = defaultLowBoundC;
			}
			else
			{
				if (s == 1)
				{
					signalStrId =  signalStrId + "N";
					defaultHighBound = defaultHighBoundN;
					defaultLowBound = defaultLowBoundN;
				}
				else
				{
					if (s == 2)
					{
						signalStrId =  signalStrId + "RC";
						defaultHighBound = defaultHighBoundR;
						defaultLowBound = defaultLowBoundR;
					}
					else
					{
						// s == 3
						signalStrId =  signalStrId + "RN";
						defaultHighBound = defaultHighBoundR;
						defaultLowBound = defaultLowBoundR;
					}
				}
			}
			
			var signal = signalSet.getSignalByEquipmentID(signalStrId);
			
			if (signal == null)
			{
				// Generate default values, there is no signal on this place
				//
				log.wrnCFG3007(signalStrId);
				
				
				if (s == 0)
				{
					confFirmware.writeLog("    in" + i + "[default]: [" + frame + ":" + ptr + "] Tf = " + defaultTf + 
										"; [" + frame + ":" + (ptr + 2) + "] K1 = " + defaultK1 +
										"; [" + frame + ":" + (ptr + 6) + "] K2 = " + defaultK2 +
										"; [" + frame + ":" + (ptr + 10) + "] HighValidRange = " + defaultHighBound +
										"; [" + frame + ":" + (ptr + 14) + "] LowValidRange = " + defaultLowBound +
										"; [" + frame + ":" + (ptr + 54) + "] WordOfFlags = " + defaultWordOfFlags + "\r\n");
				}
				else
				{
					if (s == 1)
					{
						confFirmware.writeLog("    in" + i + "[default]: [" + frame + ":" + (ptr + 0) + "] K1 = " + defaultK1 +
										"; [" + frame + ":" + (ptr + 4) + "] K2 = " + defaultK2 +
											"; [" + frame + ":" + (ptr + 8) + "] HighValidRange = " + defaultHighBound +
											"; [" + frame + ":" + (ptr + 12) + "] LowValidRange = " + defaultLowBound + "\r\n");
					}
					else
					{
						confFirmware.writeLog("    in" + i + "[default]: [" + frame + ":" + (ptr + 0) + "] HighValidRange = " + defaultHighBound +
											"; [" + frame + ":" + (ptr + 4) + "] LowValidRange = " + defaultLowBound + "\r\n");
					}
				}
				
				if (s == 0)
				{
					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "Tf", defaultTf) == false)          // InA Filtering time constant
					{
						return false;
					}
					ptr += 2;
				}
				
				if (s == 0 || s == 1)
				{
					if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K1", defaultK1) == false)      // InA DefaultK1
					{
						return false;
					}
					ptr += 4;
					
					if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K2", defaultK2) == false)      // InA DefaultK2
					{
						return false;
					}
					ptr += 4;
				}
				
				
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "HighValidRange", defaultHighBound) == false)         // InA High bound
				{
					return false;
				}
				ptr += 4;
				
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LowValidRange", defaultLowBound) == false)          // InA Low Bound
				{
					return false;
				}
				ptr += 4;
				
				if (s == 0) 
				{	
					let flagsPtr = ptr + 36;

					// Word of flags
					//
					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, flagsPtr, "WordOfFlags", defaultWordOfFlags) == false)          // Word of flags
					{
						return false;
					}
				}

			}
			else
			{
				var compareEqual = 0;
				var compareLess = 1;
				var compareMore = 2;
				
				var unitsConvertor = confFirmware.jsGetUnitsConvertor();
				if (unitsConvertor == null)
				{
					log.errINT1001("confFirmware.jsGetUnitsConvertor returned null");
					return false;
				}
				
				
				var electricHighLimit = signal.propertyValue("ElectricHighLimit");
				if (electricHighLimit == undefined) 
				{
					log.errCFG3000("ElectricHighLimit", signalStrId);
					return false;
				}
				
				var electricLowLimit = signal.propertyValue("ElectricLowLimit");
				if (electricLowLimit == undefined) 
				{
					log.errCFG3000("ElectricLowLimit", signalStrId);
					return false;
				}
				
				var electricUnit = signal.propertyValue("ElectricUnit");
				if (electricUnit == undefined) 
				{
					log.errCFG3000("ElectricUnit", signalStrId);
					return false;
				}
				
				var sensorType = signal.propertyValue("SensorType");
				if (sensorType == undefined) 
				{
					log.errCFG3000("SensorType", signalStrId);
					return false;
				}
				
				var highLimit = signal.highEngineeringUnits();
				var lowLimit = signal.lowEngineeringUnits();
				
				var highValidRange = signal.highValidRange();
				var lowValidRange = signal.lowValidRange();

				var wordOfFlags = 0;
				
				// Check properties of signal A
				
				if (electricHighLimit < electricLowLimit)
				{
					// error
					log.errCFG3013("ElectricHighLimit", electricHighLimit, compareLess, "ElectricLowLimit", electricLowLimit, 0, signalStrId);
				}
				if (electricHighLimit == electricLowLimit)
				{
					// error
					log.errCFG3013("ElectricHighLimit", electricHighLimit, compareEqual, "ElectricLowLimit", electricLowLimit, 0, signalStrId);
				}
				if (signal.highEngineeringUnits() == signal.lowEngineeringUnits())
				{
					// error
					log.errCFG3013("HighEngineeringUnits", signal.highEngineeringUnits(), compareEqual, "LowEngineeringUnits", signal.lowEngineeringUnits(), signal.decimalPlaces(), signalStrId);
				}
				if (signal.highValidRange() == signal.lowValidRange())
				{
					// error
					log.errCFG3013("HighValidRange", signal.highValidRange(), compareEqual, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
				}
				if (signal.highEngineeringUnits() > signal.lowEngineeringUnits() && signal.highValidRange() < signal.lowValidRange())
				{
					// error
					log.errCFG3013("HighValidRange", signal.highValidRange(), compareLess, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
				}
				if (signal.highEngineeringUnits() < signal.lowEngineeringUnits() && signal.highValidRange() > signal.lowValidRange())
				{
					// error
					log.errCFG3013("HighValidRange", signal.highValidRange(), compareMore, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
				}
				
				
				// Convert electric to physical
				
				var highPhysical = unitsConvertor.electricToPhysical_Input(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, 0);
				var lowPhysical = unitsConvertor.electricToPhysical_Input(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, 0);
				
				if (highPhysical.ok == false)
				{
					switch (highPhysical.errorCode)
					{
					case ConfigStruct.UnitsConvertorErrorCode.ErrorGeneric:
						{
							log.errINT1001(highPhysical.errorMessage + ", module " + module.propertyValue("EquipmentID") + ", signal " + signalStrId);
						}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.LowLimitOutOfRange:
						{
							log.errCFG3010("ElectricLowLimit", electricLowLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, 4, signalStrId);
						}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.HighLimitOutOfRange:
						{
							log.errCFG3010("ElectricHighLimit", electricHighLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, 4, signalStrId);
						}
						break;
					default:
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + highPhysical.errorCode + "), signal " + signalStrIdA);
					}
				}
				if (lowPhysical.ok == false)
				{
					switch (lowPhysical.errorCode)
					{
					case ConfigStruct.UnitsConvertorErrorCode.ErrorGeneric:
						{
							log.errINT1001(lowPhysical.errorMessage + ", module " + module.propertyValue("EquipmentID") + ", signal " + signalStrId);
						}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.LowLimitOutOfRange:
						{
							log.errCFG3010("ElectricLowLimit", electricLowLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, 4, signalStrId);
						}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.HighLimitOutOfRange:
						{
							log.errCFG3010("ElectricHighLimit", electricHighLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, 4, signalStrId);
						}
						break;
					default:
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + lowPhysical.errorCode + "), signal " + signalStrId);
					}
				}
				
				if (highPhysical.toDouble == lowPhysical.toDouble)
				{
					// error
					log.errCFG3013("calculated HighPhysical", highPhysical.toDouble, compareEqual, "calculated LowPhysical", lowPhysical.toDouble, 0, signalStrId);
				}
				
				// end of convert electric to physical
				
				var tf = signal.filteringTime();
				
				if (tf < 0 * tsConstant || tf > 65535 * tsConstant)
				{
					log.errCFG3010("FilteringTime", tf, 0 * tsConstant, 65535 * tsConstant, 6, signalStrId);
				}
				
				tf = tf / tsConstant;
				
				var filteringTime = valToADC(tf, 0, 65535, 0, 0xffff);
				
				var y1 = lowLimit;
				var y2 = highLimit;
				
				var x1 = lowPhysical.toDouble;
				var x2 = highPhysical.toDouble;
				
				if (x1 == x2) // Prevent division by zero
				{
					x1 = 0;	
					x2 = 1;
				}
				
				var k1 = (y2 - y1) / (x2 - x1);	// K
				var k2 = y1 - k1 * x1;			// B
				
				var flags = 0;
				
				// Check valid ranges
				
				var decimalPlaces = signal.propertyValue("DecimalPlaces");
				
				var lowValidRangeMin = -20;
				var highValidRangeMax = 20;

				var lowValidRangeMinEngineering = lowValidRangeMin * k1 + k2;
				var	highValidRangeMaxEngineering = highValidRangeMax * k1 + k2;
				
				// Round this value to supplied decimal places
				
				lowValidRangeMinEngineering = parseFloat(lowValidRangeMinEngineering.toFixed(decimalPlaces));
				highValidRangeMaxEngineering = parseFloat(highValidRangeMaxEngineering.toFixed(decimalPlaces));

				if (s == 0)
				{
					var unitEnable = signal.propertyValue("UnitEnable");
					if (unitEnable == undefined) 
					{
						log.errCFG3000("UnitEnable", signalStrId);
						return false;
					}
					
					if (unitEnable == true)
					{
						wordOfFlags |= 1;
					}
				}
				
				//
				
				if (lowValidRange < lowValidRangeMinEngineering)
				{
					log.errCFG3010("LowValidRange", lowValidRange, lowValidRangeMinEngineering, highValidRangeMaxEngineering, decimalPlaces, signalStrId);
				}
				if (highValidRange > highValidRangeMaxEngineering)
				{
					log.errCFG3010("HighValidRange", highValidRange, lowValidRangeMinEngineering, highValidRangeMaxEngineering, decimalPlaces, signalStrId);
				}
				
				//
				
				if (s == 0)
				{
					confFirmware.writeLog("    in" + i + ": [" + frame + ":" + ptr + "] Tf = " + filteringTime + 
								
											"; HighPhysicalRange = " + highPhysical.toDouble +
											"; LowPhysicalRange = " + lowPhysical.toDouble +
											
											"; [" + frame + ":" + (ptr + 2) + "] K1 = " + k1 +
											"; [" + frame + ":" + (ptr + 6) + "] K2 = " + k2 +
											"; [" + frame + ":" + (ptr + 10) + "] HighValidRange = " + highValidRange +
											"; [" + frame + ":" + (ptr + 14) + "] LowValidRange = " + lowValidRange +
											"; [" + frame + ":" + (ptr + 54) + "] WordOfFlags = " + wordOfFlags + "\r\n");
				}
				else
				{
					if (s == 1)
					{
						confFirmware.writeLog("    in" + i + ": HighPhysicalRange = " + highPhysical.toDouble +
											"; LowPhysicalRange = " + lowPhysical.toDouble +
											
											"; [" + frame + ":" + (ptr + 0) + "] K1 = " + k1 +
											"; [" + frame + ":" + (ptr + 4) + "] K2 = " + k2 +
											"; [" + frame + ":" + (ptr + 8) + "] HighValidRange = " + highValidRange +
											"; [" + frame + ":" + (ptr + 12) + "] LowValidRange = " + lowValidRange + "\r\n");
					}
					else
					{
						confFirmware.writeLog("    in" + i + ": [" + frame + ":" + (ptr + 0) + "] HighPhysicalRange = " + highPhysical.toDouble +
											"; LowPhysicalRange = " + lowPhysical.toDouble +
										
											"; [" + frame + ":" + (ptr + 0) + "] HighValidRange = " + highValidRange +
											"; [" + frame + ":" + (ptr + 4) + "] LowValidRange = " + lowValidRange + "\r\n");
					}
				}
				
				if (s == 0)
				{
					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "FilteringTime", filteringTime) == false)          // InA Filtering time constant
					{
						return false;
					}
					ptr += 2;
				}
				
				if (s == 0 || s == 1)
				{
					if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K1", k1) == false)         // K1
					{
						return false;
					}
					ptr += 4;
					
					
					if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K2", k2) == false)         // K2
					{
						return false;
					}
					ptr += 4;
				}
				
				
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "HighValidRange", highValidRange) == false)         // InA High bound
				{
					return false;
				}
				ptr += 4;
				
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LowValidRange", lowValidRange) == false)          // InA Low Bound
				{
					return false;
				}
				ptr += 4;

				if (s == 0) 
				{	
					let flagsPtr = ptr + 36;

					// Word of flags
					//
					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, flagsPtr, "WordOfFlags", wordOfFlags) == false)          // Word of flags
					{
						return false;
					}
				}
			}
		}
	}
	
	ptr = 888;
	
	// final crc
	var stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
	if (stringCrc64 == "")
	{
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = 0x" + stringCrc64 + "\r\n");
	ptr += 8;
	
	ptr = 1008;
	
	// ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
	//
	var dataTransmittingEnableFlag = false;
	var dataReceiveEnableFlag = true;
	
	var flags = 0;
	if (dataTransmittingEnableFlag == true)
		flags |= 1;
	if (dataReceiveEnableFlag == true)
		flags |= 2;
	
	var configFramesQuantity = 7;
	var dataFramesQuantity = 0;
	
	let txId = module.moduleFamily + module.moduleVersion;
	
	if (generate_txRxIoConfig(confFirmware, module.equipmentId, LMNumber, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId) == false)
	{
		return false;
	}
	ptr += 8;
	
	// assert if we not on the correct place
	//
	if (ptr != 1016)
	{
		ptr = 1016;
	}
	
	return true;
}			
