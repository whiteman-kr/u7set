#pragma once

#include <QObject>
#include <QVector>

#include <CommonLib/ConstStrings.h>


class AppSignal;
class XmlWriteHelper;
class XmlReadHelper;


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
		void clearSignalLists();
		void appendTuningSignal(E::TuningSignalType tunSignalType, AppSignal* appSignal);

		virtual void buildTuningData();
		virtual void getTuningData(QByteArray* tuningData) const;

		void calcTuningDataUID(const QString& lmEquipmentID);

		quint32 rupTuningDataUID() const { return m_rupTuningDataUID; }
		quint64 fotipTuningDataUID() const { return m_fotipTuningDataUID; }

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

		//

		int usedTuningDataSizeW() const;

		int usedFramesCount() const { return m_tuningDataUsedFramesCount; }

		void getSignals(QVector<AppSignal *>* signalList) const;

		const QVector<AppSignal*>& getAnalogFloatSignals() const { return m_tuningSignals[TYPE_ANALOG_FLOAT]; }
		const QVector<AppSignal*>& getAnalogIntSignals() const { return m_tuningSignals[TYPE_ANALOG_INT32]; }
		const QVector<AppSignal*>& getDiscreteSignals() const { return m_tuningSignals[TYPE_DISCRETE]; }

		const QVector<AppSignal*>& getSignals(int type) const;

		void getAcquiredAnalogSignals(QVector<AppSignal*>& analogSignals);
		void getAcquiredDiscreteSignals(QVector<AppSignal*>& discreteSignals);

		int getSignalsCount() const;

		void getMetadataFields(QStringList& getMetadataFields, int* metadataVersion) const;
		const std::vector<QVariantList>& metadata() const;

		void writeToXml(XmlWriteHelper& xml);
		bool readFromXml(XmlReadHelper& xml);

		void getTuningSignalsFramesInfo(std::vector<std::pair<quint32, quint32>>* framesInfo) const;

		static const int TYPE_ANALOG_FLOAT = static_cast<int>(E::TuningSignalType::AnalogFloat);
		static const int TYPE_ANALOG_INT32 = static_cast<int>(E::TuningSignalType::AnalogInt32);
		static const int TYPE_DISCRETE = static_cast<int>(E::TuningSignalType::Discrete);

	private:
		void writeBigEndianUint32Bit(quint8* dataPtr, int bitNo, quint32 bitValue);
		void sortSignalsByAcquiredProperty(QVector<AppSignal *>& tuningSignals);
		void sortByAppSignalID(QVector<AppSignal *>& signalList);
		int signalValueSizeBits(int type);
		int getSignalType(const AppSignal* signal);

	protected:
		QString m_lmEquipmentID;

		const int TYPES_COUNT = 0;

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

		//

		quint32 m_rupTuningDataUID = 0;			// 32-bit UID placed in RUP frame header (like a AppDataUID and DiagDataUID)
		quint64 m_fotipTuningDataUID = 0;		// 64-bit UID placed in FOTIP frame header

		int m_tuningDataUsedFramesCount = 0;

		static const int TRIPLE_FRAMES = 3;

		QVector<QVector<AppSignal*>> m_tuningSignals;

		QVector<int> m_tuningSignalSizesB;

		quint8* m_tuningData = nullptr;
		int m_tuningDataSizeB = 0;

		QHash<QString, AppSignal*> m_id2SignalMap;

		static QStringList m_metadataFields;
		std::vector<QVariantList> m_metadata;

		bool m_deleteSignals = false;

		inline static const std::vector<QString> m_signalTypeXmlElement =
		{
			XmlElement::ANALOG_FLOAT_SIGNALS,
			XmlElement::ANALOG_INT32_SIGNALS,
			XmlElement::DISCRETE_SIGNALS
		};
	};

	using TuningDataShared = std::shared_ptr<TuningData>;
	using TuningDataSharedConst = std::shared_ptr<const TuningData>;

	// -------------------------------------------------------------------------------------
	//
	// TuningDataStorage class declaration
	//
	// -------------------------------------------------------------------------------------

	class TuningDataStorage
	{
	public:
		bool appendTuningData(const QString& lmEquipmentID, TuningDataShared tuningData);
		TuningDataShared getTuningData(const QString& lmEquipmentID);
		QStringList getAllTuningSourceIDs() const;

	private:
		std::map<QString, TuningDataShared> m_tuningDataMap;
	};

}
