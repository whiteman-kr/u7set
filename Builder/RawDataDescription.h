#pragma once

#include <QVector>

#include "../lib/Types.h"
#include "IssueLogger.h"

namespace Hardware
{
	class OptoPort;


	class RawDataDescriptionItem
	{
	public:
		enum class Type
		{
			TxRawDataSize,				// TX section items
			TxAllModulesRawData,
			TxModuleRawData,
			TxPortRawData,
			TxConst16,
			TxSignal,

			RxRawDataSize,				// RX section items
			RxSignal,
		};

		Type type = Type::TxRawDataSize;

		bool txRawDataSizeIsAuto = false;						// for type - TxRawDataSize
		int txRawDataSize = 0;									//

		int modulePlace = 0;									// for type - TxModuleRawData

		int const16Value = 0;									// for type - TxConst16

		QString portEquipmentID;								// for type - TxPortRawData

		bool rxRawDataSizeIsAuto = false;						// for type - RxRawDataSize
		int rxRawDataSize = 0;									//

		QString appSignalID;									// for type - TxSignal, RxSignal
		E::SignalType signalType = E::SignalType::Discrete;		//
		E::DataFormat dataFormat = E::DataFormat::UnsignedInt;	//
		QString busTypeID;
		int dataSize = 0;										//
		E::ByteOrder byteOrder = E::ByteOrder::BigEndian;		//
		int offsetW = 0;										//
		int bitNo = 0;											//
	};


	class RawDataDescription : public QVector<RawDataDescriptionItem>
	{
	public:
		RawDataDescription();

		bool txRawDataSizeIsValid() const { return m_txRawDataSizeIsValid; }
		bool txRawDataSizeIsAuto() const { return m_txRawDataSizeIsAuto; }
		int txRawDataSize() const { return m_txRawDataSize; }

		bool rxRawDataSizeIsValid() const { return m_rxRawDataSizeIsValid; }
		bool rxRawDataSizeIsAuto() const { return m_rxRawDataSizeIsAuto; }
		int rxRawDataSize() const { return m_rxRawDataSize; }

		bool parse(const OptoPort& optoPort, Builder::IssueLogger* log);

	private:
		void clearAll();

		bool parseTxSignalRawDescription(const QString& portEquipmentID, const QString& str, RawDataDescriptionItem &item, Builder::IssueLogger* log);
		bool parseRxSignalRawDescription(const QString& portEquipmentID, const QString& str, RawDataDescriptionItem &item, Builder::IssueLogger* log);
		bool parseSignalRawDescription(const QString& portEquipmentID, const QString& str, RawDataDescriptionItem &item, Builder::IssueLogger* log);

	private:
		static const char* TX_RAW_DATA_SIZE;
		static const char* TX_ALL_MODULES_RAW_DATA;
		static const char* TX_MODULE_RAW_DATA;
		static const char* TX_PORT_RAW_DATA;
		static const char* TX_CONST16;
		static const char* TX_SIGNAL;

		static const char* RX_RAW_DATA_SIZE;
		static const char* RX_SIGNAL;

		bool m_txRawDataSizeIsValid = false;
		bool m_txRawDataSizeIsAuto = false;
		int m_txRawDataSize = -1;

		bool m_rxRawDataSizeIsValid = false;
		bool m_rxRawDataSizeIsAuto = false;
		int m_rxRawDataSize = -1;
	};

}
