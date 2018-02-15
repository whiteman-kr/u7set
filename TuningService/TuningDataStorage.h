#pragma once

#include <QtCore>
#include <QList>

#include "../lib/Types.h"
#include "../lib/Signal.h"
#include "../lib/DataProtocols.h"

#include "../u7/Builder/IssueLogger.h"

namespace Tuning
{

	// -------------------------------------------------------------------------------------
	//
	// TuningData class declaration
	//
	// -------------------------------------------------------------------------------------

	class TuningData : public QObject
	{
		Q_OBJECT

	public:
		TuningData();
		TuningData(const QString& lmID,
				   int tuningFlashFrameCount,
				   int tuningFlashFramePayloadB,
				   int tuningFlashFrameSizeB,
				   int tuningDataOffsetW,
				   int tuningDataSizeW,
				   int tuningDataFrameCount,
				   int tuningDataFramePayloadW,
				   int tuningDataFrameSizeW);

		TuningData(const QString& lmID);		// for IPEN tuning only

		virtual ~TuningData();

	public:
		bool buildTuningSignalsLists(HashedVector<QString, Signal*> lmAssociatedSignals, Builder::IssueLogger* log);

		virtual bool buildTuningData();
		virtual void getTuningData(QByteArray* tuningData) const;

		quint64 generateUniqueID(const QString& lmEquipmentID);
		quint64 uniqueID() const { return m_uniqueID; }

		int tuningFlashFrameCount() const { return m_tuningFlashFrameCount; }
		int tuningFlashFramePayloadB() const { return m_tuningFlashFramePayloadB; }
		int tuningFlashFrameSizeB() const { return m_tuningFlashFrameSizeB; }

		// tuning data memory parameters from LmDescription
		//
		int tuningDataOffsetW() const { return m_tuningDataOffsetW; }
		int tuningDataSizeW() const { return m_tuningDataSizeW; }
		int tuningDataFrameCount() const { return m_tuningDataFrameCount; }
		int tuningDataFramePayloadW() const { return m_tuningDataFramePayloadW; }
		int tuningDataFrameSizeW() const { return m_tuningDataFrameSizeW; }

		int usedFramesCount() const { return m_tuningDataUsedFramesCount; }

		void getSignals(QVector<Signal *>& signalList) const;

		const QVector<Signal*>& getAnalogFloatSignals() const { return m_tuningSignals[TYPE_ANALOG_FLOAT]; }
		const QVector<Signal*>& getAnalogIntSignals() const { return m_tuningSignals[TYPE_ANALOG_INT32]; }
		const QVector<Signal*>& getDiscreteSignals() const { return m_tuningSignals[TYPE_DISCRETE]; }

		const QVector<Signal*>& getSignals(int type) const;

		void getAcquiredAnalogSignals(QVector<Signal*>& analogSignals);
		void getAcquiredDiscreteSignals(QVector<Signal*>& discreteSignals);

		int getSignalsCount() const;

		void getMetadataFields(QStringList& getMetadataFields, int* metadataVersion) const;
		const std::vector<QVariantList>& metadata() const;

		void writeToXml(XmlWriteHelper& xml);
		bool readFromXml(XmlReadHelper& xml);

		static const int TYPE_ANALOG_FLOAT = 0;
		static const int TYPE_ANALOG_INT32 = 1;
		static const int TYPE_DISCRETE = 2;

		static const int TYPES_COUNT = 3;

	private:
		void writeBigEndianUint32Bit(quint8* dataPtr, int bitNo, quint32 bitValue);
		void sortSignalsByAcquiredProperty(QVector<Signal *>& tuningSignals);

	protected:
		QString m_lmEquipmentID;

		// tuning flash memory parameters from LmDescription
		//
		int m_tuningFlashFrameCount = 0;
		int m_tuningFlashFramePayloadB = 0;
		int m_tuningFlashFrameSizeB = 0;

		// tuning data memory parameters from LmDescription
		//
		int m_tuningDataOffsetW = 0;
		int m_tuningDataSizeW = 0;
		int m_tuningDataFrameCount = 0;
		int m_tuningDataFramePayloadW = 0;
		int m_tuningDataFrameSizeW = 0;

		quint64 m_uniqueID = 0;
		int m_tuningDataUsedFramesCount = 0;

		static const int TRIPLE_FRAMES = 3;

		QVector<Signal*> m_tuningSignals[TYPES_COUNT];

		int m_tuningSignalSizes[TYPES_COUNT];

		quint8* m_tuningData = nullptr;
		int m_tuningDataSizeB = 0;

		QHash<QString, Signal*> m_id2SignalMap;

		static QStringList m_metadataFields;
		std::vector<QVariantList> m_metadata;

		bool m_deleteSignals = false;

		// XML serialization string constants
		//
		static const char* TUNING_DATA_ELEMENT;
		static const char* LM_ID;
		static const char* UNIQUE_ID;

		static const char* TUNING_FLASH;
		static const char* TUNING_FLASH_FRAME_COUNT;
		static const char* TUNING_FLASH_FRAME_PAYLOAD_B;
		static const char* TUNING_FLASH_FRAME_SIZE_B;

		static const char* TUNING_DATA;
		static const char* TUNING_DATA_OFFSET_W;
		static const char* TUNING_DATA_SIZE_W;
		static const char* TUNING_DATA_FRAME_COUNT;
		static const char* TUNING_DATA_FRAME_PAYLOAD_W;
		static const char* TUNING_DATA_FRAME_SIZE_W;
		static const char* TUNING_DATA_USED_FRAMES_COUNT;

		static const char* TUNING_ALL_SIGNALS_COUNT;
		static const char* TUNING_ANALOG_FLOAT_SIGNALS;
		static const char* TUNING_ANALOG_INT32_SIGNALS;
		static const char* TUNING_DISCRETE_SIGNALS;
		static const char* TUNING_SIGNALS_COUNT;

		int signalValueSizeBits(int type);
		int getSignalType(const Signal* signal);
	};


	// -------------------------------------------------------------------------------------
	//
	// TuningDataStorage class declaration
	//
	// -------------------------------------------------------------------------------------

	class TuningDataStorage : public QHash<QString, TuningData*>
	{
	public:
		~TuningDataStorage();
	};

}
