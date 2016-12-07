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
		TuningData(	QString lmID,
					int tuningFrameSizeBytes,
					int tuningFramesCount);

		virtual ~TuningData();

		bool buildTuningSignalsLists(HashedVector<QString, Signal*> lmAssociatedSignals, Builder::IssueLogger* log);

		virtual bool buildTuningData();
		virtual void getTuningData(QByteArray* tuningData) const;

		quint64 generateUniqueID(const QString& lmEquipmentID);
		quint64 uniqueID() const { return m_uniqueID; }

		int usedFramesCount() const { return m_usedFramesCount; }

		void getSignals(QVector<Signal *>& signalList) const;

		const QVector<Signal*>& getAnalogFloatSignals() const { return m_tuningSignals[TYPE_ANALOG_FLOAT]; }
		const QVector<Signal*>& getAnalogIntSignals() const { return m_tuningSignals[TYPE_ANALOG_INT]; }
		const QVector<Signal*>& getDiscreteSignals() const { return m_tuningSignals[TYPE_DISCRETE]; }

		const QVector<Signal*>& getSignals(int type) const;

		int getSignalsCount() const;

		const QStringList& metadataFields();
		const std::vector<QVariantList>& metadata() const;

		void writeToXml(XmlWriteHelper& xml);
		bool readFromXml(XmlReadHelper& xml);

		static const int TYPE_ANALOG_FLOAT = 0;
		static const int TYPE_ANALOG_INT = 1;
		static const int TYPE_DISCRETE = 2;

		static const int TYPES_COUNT = 3;

	private:
		void writeBigEndianUint16Bit(quint8* dataPtr, int bitNo, quint16 bitValue);

	protected:
		QString m_lmEquipmentID;

		int m_tuningFrameSizeBytes = 0;
		int m_tuningFramesCount = 0;
		quint64 m_uniqueID = 0;
		int m_usedFramesCount = 0;

		static const int TRIPLE_FRAMES = 3;

		QVector<Signal*> m_tuningSignals[TYPES_COUNT];

		int m_tuningSignalSizes[TYPES_COUNT];

		quint8* m_tuningData = nullptr;
		int m_tuningDataSize = 0;

		QHash<QString, Signal*> m_id2SignalMap;

		static QStringList m_metadataFields;
		std::vector<QVariantList> m_metadata;

		bool m_deleteSignals = false;

		// XML serialization constants
		//
		static const char* TUNING_DATA_ELEMENT;
		static const char* LM_ID;
		static const char* UNIQUE_ID;
		static const char* TUNING_FRAME_SIZE_BYTES;
		static const char* TUNING_FRAMES_COUNT;
		static const char* TUNING_USED_FRAMES_COUNT;
		static const char* TUNING_ALL_SIGNALS_COUNT;
		static const char* TUNING_ANALOG_FLOAT_SIGNALS;
		static const char* TUNING_ANALOG_INT_SIGNALS;
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
