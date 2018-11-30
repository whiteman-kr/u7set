#include "IssueLogger.h"
#include "OptoModule.h"

#include "RawDataDescription.h"

namespace Hardware
{
	const char* RawDataDescription::TX_RAW_DATA_SIZE = "TX_RAW_DATA_SIZE";
	const char* RawDataDescription::TX_ALL_MODULES_RAW_DATA = "TX_ALL_MODULES_RAW_DATA";
	const char* RawDataDescription::TX_MODULE_RAW_DATA = "TX_MODULE_RAW_DATA";
	const char* RawDataDescription::TX_PORT_RAW_DATA = "TX_PORT_RAW_DATA";
	const char* RawDataDescription::TX_CONST16 = "TX_CONST16";
	const char* RawDataDescription::TX_SIGNAL = "TX_SIGNAL";
	const char* RawDataDescription::RX_RAW_DATA_SIZE = "RX_RAW_DATA_SIZE";
	const char* RawDataDescription::RX_SIGNAL = "RX_SIGNAL";


	RawDataDescription::RawDataDescription()
	{
	}

	bool RawDataDescription::parse(const OptoPort& optoPort, Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		clearAll();

		QString equipmentID = optoPort.equipmentID();
		QString rawDataDescriptionStr = optoPort.rawDataDescriptionStr().trimmed().toUpper();

		if (rawDataDescriptionStr.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		// split string

		QStringList list = rawDataDescriptionStr.split("\n", QString::SkipEmptyParts);

		bool needTxRawDataSize = false;
		bool needRxRawDataSize = false;

		QString msg;

		for(QString str : list)
		{
			RawDataDescriptionItem item;

			bool res = true;

			int commentIndex = str.indexOf("//");

			if (commentIndex != -1)
			{
				str = str.mid(0, commentIndex);
			}

			str = str.trimmed();

			if (str.isEmpty() == true)
			{
				continue;
			}

			QString itemTypeStr = str.section("=", 0, 0).trimmed();

			// ------------------------------ TX section items parsing -------------------------------
			//
			if (itemTypeStr == TX_RAW_DATA_SIZE)
			{
				if (m_txRawDataSizeIsValid == true)
				{
					msg = QString("Duplicate TX_RAW_DATA_SIZE section in opto-port '%1' raw data description.").arg(equipmentID);
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}

				m_txRawDataSizeIsValid = true;

				QString sizeStr = str.section("=", 1, 1).trimmed();

				if (sizeStr != "AUTO" )
				{
					int size = sizeStr.toInt(&res);

					if (res == false)
					{
						msg = QString("Invalid TX_RAW_DATA_SIZE value in opto-port '%1' raw data description.").arg(equipmentID);
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						continue;
					}

					m_txRawDataSize = item.txRawDataSize = size;
					m_txRawDataSizeIsAuto = item.txRawDataSizeIsAuto = false;
				}
				else
				{
					m_txRawDataSize = item.txRawDataSize = -1;
					m_txRawDataSizeIsAuto =item.txRawDataSizeIsAuto = true;
				}

				item.type = RawDataDescriptionItem::Type::TxRawDataSize;

				append(item);

				continue;
			}

			if (itemTypeStr == TX_ALL_MODULES_RAW_DATA)
			{
				needTxRawDataSize = true;

				item.type = RawDataDescriptionItem::Type::TxAllModulesRawData;

				append(item);

				continue;
			}

			if (itemTypeStr == TX_MODULE_RAW_DATA)
			{
				needTxRawDataSize = true;

				QString placeStr = str.section("=", 1, 1).trimmed();

				int place = placeStr.toInt(&res);

				if (res == false)
				{
					msg = QString("Invalid TX_MODULE_RAW_DATA value in opto-port '%1' raw data description.").arg(equipmentID);
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}
				else
				{
					item.type = RawDataDescriptionItem::Type::TxModuleRawData;
					item.modulePlace = place;

					append(item);
				}

				continue;
			}

			if (itemTypeStr == TX_PORT_RAW_DATA)
			{
				needTxRawDataSize = true;

				QString portEquipmentID = str.section("=", 1, 1).trimmed();

				item.type = RawDataDescriptionItem::Type::TxPortRawData;
				item.portEquipmentID = portEquipmentID;

				append(item);
				continue;
			}

			if (itemTypeStr == TX_CONST16)
			{
				needTxRawDataSize = true;

				QString constStr = str.section("=", 1, 1).trimmed();

				int const16 = constStr.toInt(&res);

				if (res == false)
				{
					msg = QString("Invalid TX_CONST16 value in opto-port '%1' raw data description.").arg(equipmentID);
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}
				else
				{
					item.type = RawDataDescriptionItem::Type::TxConst16;
					item.const16Value = const16;

					append(item);
				}

				continue;
			}

			if (itemTypeStr == TX_SIGNAL)
			{
				needTxRawDataSize = true;

				bool parseResult = parseTxSignalRawDescription(equipmentID, str, item, log);

				if (parseResult == true)
				{
					append(item);
				}

				result &= parseResult;

				continue;
			}

			// ------------------------------ RX section items parsing -------------------------------
			//
			if (itemTypeStr == RX_RAW_DATA_SIZE)
			{
				if (m_rxRawDataSizeIsValid == true)
				{
					msg = QString("Duplicate RX_RAW_DATA_SIZE section in opto-port '%1' raw data description.").arg(equipmentID);
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}

				m_rxRawDataSizeIsValid = true;

				QString sizeStr = str.section("=", 1, 1).trimmed();

				if (sizeStr != "AUTO" )
				{
					int size = sizeStr.toInt(&res);

					if (res == false)
					{
						msg = QString("Invalid RX_RAW_DATA_SIZE value in opto-port '%1' raw data description.").arg(equipmentID);
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						continue;
					}

					m_rxRawDataSize = item.rxRawDataSize = size;
					m_rxRawDataSizeIsAuto = item.rxRawDataSizeIsAuto = false;
				}
				else
				{
					m_rxRawDataSize = item.rxRawDataSize = -1;
					m_rxRawDataSizeIsAuto = item.rxRawDataSizeIsAuto = true;
				}

				item.type = RawDataDescriptionItem::Type::RxRawDataSize;

				append(item);

				continue;
			}

			if (itemTypeStr == RX_SIGNAL)
			{
				needRxRawDataSize = true;

				bool parseResult = parseRxSignalRawDescription(equipmentID, str, item, log);

				if (parseResult == true)
				{
					append(item);
				}

				result &= parseResult;

				continue;
			}

			msg = QString("Unknown item %1 in opto-port '%2' raw data description.").arg(itemTypeStr).arg(equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			result = false;

			break;
		}

		if (needTxRawDataSize == true && m_txRawDataSizeIsValid == false)
		{
			msg = QString("TX_RAW_DATA_SIZE value is not found in opto-port '%1' raw data description.").arg(equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			result = false;
		}

		if (needRxRawDataSize == true && m_rxRawDataSizeIsValid == false)
		{
			msg = QString("RX_RAW_DATA_SIZE value is not found in opto-port '%1' raw data description.").arg(equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			result = false;
		}

		return result;
	}

	void RawDataDescription::clearAll()
	{
		m_txRawDataSizeIsValid = false;
		m_txRawDataSizeIsAuto = false;
		m_txRawDataSize = -1;

		m_rxRawDataSizeIsValid = false;
		m_rxRawDataSizeIsAuto = false;
		m_rxRawDataSize = -1;

		clear();
	}

	bool RawDataDescription::parseTxSignalRawDescription(const QString& portEquipmentID, const QString& str, RawDataDescriptionItem &item, Builder::IssueLogger* log)
	{
		item.type = RawDataDescriptionItem::Type::TxSignal;

		return parseSignalRawDescription(portEquipmentID, str, item, log);
	}

	bool RawDataDescription::parseRxSignalRawDescription(const QString& portEquipmentID, const QString& str, RawDataDescriptionItem &item, Builder::IssueLogger* log)
	{
		item.type = RawDataDescriptionItem::Type::RxSignal;

		return parseSignalRawDescription(portEquipmentID, str, item, log);
	}


	bool RawDataDescription::parseSignalRawDescription(const QString& portEquipmentID, const QString& str, RawDataDescriptionItem &item, Builder::IssueLogger* log)
	{
		QString keywordStr;

		// item.type must be already filled !
		//
		switch(item.type)
		{
		case RawDataDescriptionItem::Type::TxSignal:
			keywordStr = TX_SIGNAL;
			break;

		case RawDataDescriptionItem::Type::RxSignal:
			keywordStr = RX_SIGNAL;
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(log);
			return false;
		}

		QString msg;

		bool res = true;

		// Tx- Rx- Siganl description examples:
		//
		// TX_SIGNAL=#EXT_DISCRETE1,D,UINT,1,BE,3,4
		// RX_SIGNAL=#EXT_ANALOG1,A,FLOAT,32,LE,6,0

		QString inSignalDescription = str.section("=", 1).trimmed();

		QStringList descItemsList = inSignalDescription.split(",", QString::SkipEmptyParts);

		if (descItemsList.size() != 7)			// must be 7 parameters!
		{
			msg = QString("Invalid %1 description parameters count, must be 7. (Port '%2')").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		// 1) AppSignalID

		item.appSignalID = descItemsList.at(0).trimmed();

		// 2) Signal type

		QString signalTypeStr = descItemsList.at(1).trimmed();

		if (signalTypeStr != "A" && signalTypeStr != "D" && signalTypeStr != "B")
		{
			msg = QString("Invalid %1 value of parameter SignalType in opto-port '%2' raw data description.").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (signalTypeStr == "A")
		{
			item.signalType = E::SignalType::Analog;
		}

		if (signalTypeStr == "D")
		{
			item.signalType = E::SignalType::Discrete;
		}

		if (signalTypeStr == "B")
		{
			item.signalType = E::SignalType::Bus;
		}

		// 3) DataFormat

		QString dataFormatStr = descItemsList.at(2).trimmed();

		if (item.signalType == E::SignalType::Analog &&
			dataFormatStr != "FLOAT" &&
			dataFormatStr != "SINT")
		{
			msg = QString("Invalid %1 value of parameter DataFormat in opto-port '%2' raw data description.").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (item.signalType == E::SignalType::Discrete &&
			dataFormatStr != "UINT")
		{
			msg = QString("Invalid %1 value of parameter DataFormat in opto-port '%2' raw data description.").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (item.signalType == E::SignalType::Bus)
		{
			item.busTypeID = dataFormatStr;
		}

		if (dataFormatStr == "FLOAT")
		{
			item.dataFormat = E::DataFormat::Float;
		}

		if (dataFormatStr == "SINT")
		{
			item.dataFormat = E::DataFormat::SignedInt;
		}

		if (dataFormatStr == "UNIT")
		{
			item.dataFormat = E::DataFormat::UnsignedInt;
		}

		if (item.signalType == E::SignalType::Discrete && item.dataFormat != E::DataFormat::UnsignedInt)
		{
			msg = QString("Discrete signal '%1' in raw data description of port '%2' must have UINT data format.").
					arg(item.appSignalID).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler, msg);
			return false;
		}

		if (item.signalType == E::SignalType::Analog &&
			item.dataFormat != E::DataFormat::Float &&
			item.dataFormat != E::DataFormat::SignedInt)
		{
			msg = QString("Analog signal '%1' in raw data description of port '%2' must have SINT or FLOAT data format.").
					arg(item.appSignalID).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler, msg);
			return false;
		}

		// 4) DataSize

		item.dataSize = descItemsList.at(3).trimmed().toInt(&res);

		if (res == false)
		{
			msg = QString("Invalid %1 value of parameter DataSize in opto-port '%2' raw data description.").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (item.signalType == E::SignalType::Discrete && item.dataSize != SIZE_1BIT)
		{
			msg = QString("Discrete signal '%1' in raw data description of port '%2' must have 1-bit data size.").
					arg(item.appSignalID).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler, msg);
			return false;
		}

		if (item.signalType == E::SignalType::Analog && item.dataSize != SIZE_32BIT)
		{
			msg = QString("Analog signal '%1' in raw data description of port '%2' must have 32-bit data size.").
					arg(item.appSignalID).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler, msg);
			return false;
		}

		if (item.signalType == E::SignalType::Bus && (item.dataSize % SIZE_16BIT) != 0)
		{
			msg = QString("Bus signal '%1' in raw data description of port '%2' must have data size multiple to 16 bit.").
					arg(item.appSignalID).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler, msg);
			return false;
		}

		// 5) ByteOrder

		QString byteOrderStr = descItemsList.at(4).trimmed();

		if (byteOrderStr != "BE" && byteOrderStr != "LE")
		{
			msg = QString("Invalid %1 value of parameter ByteOrder in opto-port '%2' raw data description.").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (byteOrderStr == "BE")
		{
			item.byteOrder = E::ByteOrder::BigEndian;
		}
		else
		{
			item.byteOrder = E::ByteOrder::LittleEndian;
		}

		// 6) OffsetW

		item.offsetW = descItemsList.at(5).trimmed().toInt(&res);

		if (res == false)
		{
			msg = QString("Invalid %1 value of parameter OffsetW in opto-port '%2' raw data description.").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		// 7) BitNo

		item.bitNo = descItemsList.at(6).trimmed().toInt(&res);

		if (res == false)
		{
			msg = QString("Invalid %1 value of parameter BitNo in opto-port '%2' raw data description.").
					arg(keywordStr).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (item.signalType == E::SignalType::Analog && item.bitNo != 0)
		{
			msg = QString("Analog signal '%1' in raw data description of port '%2' must have bitNo equal to 0.").
					arg(item.appSignalID).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler, msg);
			return false;
		}

		if (item.signalType == E::SignalType::Discrete && (item.bitNo < 0 || item.bitNo >= SIZE_16BIT))
		{
			msg = QString("Discrete signal '%1' in raw data description of port '%2' must have bitNo in range 0..15.").
					arg(item.appSignalID).arg(portEquipmentID);

			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler, msg);
			return false;
		}

		return true;
	}

}
