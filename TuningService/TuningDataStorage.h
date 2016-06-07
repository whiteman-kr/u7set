#pragma once

#include <QtCore>
#include <QList>
#include "../lib/Signal.h"
#include "../u7/Builder/IssueLogger.h"
#include "../TuningService/TuningSocket.h"


namespace Tuning
{

	struct TuningSignalState;

	class TuningFramesData
	{
	public:
		static const int BITS_8 = 8;
		static const int FRAMES_3 = 3;

	private:
		int m_firstFrameNo = 0;
		int m_tuningFrameSizeBytes = 0;
		int m_tuningFrameSizeBits = 0;

		int m_signalSizeBits = 0;
		int m_signalCount = 0;

		//

		int m_tripleFramesCount = 0;
		int m_usedFramesCount = 0;

		char* m_framesData = nullptr;

		void setFramesDataBit(int offset, int bit, int value);

	public:
		TuningFramesData();
		virtual ~TuningFramesData();

		void init(int firstFrameNo, int tuningFrameSizeBytes, int signalSizeBits, int signalCount);

		int firstFrameNo() const { return m_firstFrameNo; }

		void copySignalsData(QList<Signal*> signalsList);

		int usedFramesCount() const { return m_usedFramesCount; }
		int framesDataSize() const { return m_usedFramesCount * m_tuningFrameSizeBytes; }

		quint64 generateUniqueID(const QString& lmEquipmentID);

		const char* framesData() const { return m_framesData; }

		void converToBigEndian();

		void setFrameData(int frameNo, const char* fotipData);

		bool getSignalState(const Signal* signal, TuningSignalState* tss);
		bool setSignalState(const Signal* signal, double value, Tuning::SocketRequest* sr);
	};


	class TuningData : public QObject
	{
		Q_OBJECT

	private:
		QString m_lmEquipmentID;

		int m_tuningFrameSizeBytes = 0;
		int m_tuningFramesCount = 0;
		quint64 m_uniqueID = 0;
		int m_usedFramesCount = 0;

		static const int TYPE_ANALOG_FLOAT = 0;
		static const int TYPE_ANALOG_INT = 1;
		static const int TYPE_DISCRETE = 2;

		static const int TYPES_COUNT = 3;

		TuningFramesData m_tuningFramesData[TYPES_COUNT];

		QList<Signal*> m_tuningSignals[TYPES_COUNT];

		QHash<QString, Signal*> m_id2SignalMap;

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

	public:
		TuningData();
		TuningData(	QString lmID,
					int tuningFrameSizeBytes,
					int tuningFramesCount);

		~TuningData();

		bool buildTuningSignalsLists(HashedVector<QString, Signal*> lmAssociatedSignals, Builder::IssueLogger* log);
		bool buildTuningData();
		quint64 generateUniqueID(const QString& lmEquipmentID);

		bool initTuningData();

		quint64 uniqueID() const { return m_uniqueID; }
		void getTuningData(QByteArray* tuningData) const;

		int usedFramesCount() const { return m_usedFramesCount; }

		void writeToXml(XmlWriteHelper& xml);
		bool readFromXml(XmlReadHelper& xml);

		void getSignals(QList<Signal *>& signalList);

		QList<Signal*> getAnalogFloatSignals() const { return m_tuningSignals[TYPE_ANALOG_FLOAT]; }
		QList<Signal*> getAnalogIntSignals() const { return m_tuningSignals[TYPE_ANALOG_INT]; }
		QList<Signal*> getDiscreteSignals() const { return m_tuningSignals[TYPE_DISCRETE]; }

		void setFrameData(int frameNo, const char* fotipData);

		bool getSignalState(const QString& appSignalID, TuningSignalState* tss);
		bool setSignalState(const QString& appSignalID, double value, Tuning::SocketRequest* sr);
	};


	class TuningDataStorage : public QHash<QString, TuningData*>
	{
	public:
		~TuningDataStorage();
	};

}
