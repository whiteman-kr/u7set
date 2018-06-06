#pragma once

#include <QtCore>
#include <QList>
#include "../lib/Signal.h"
#include "../lib/DataProtocols.h"
#include "../u7/Builder/IssueLogger.h"
#include "../TuningService/TuningDataStorage.h"
#include "TuningIPENSocket.h"

namespace TuningIPEN
{

	struct TuningSignalState
	{
		double currentValue = 0;
		double lowLimit = 0;
		double highLimit = 0;

		bool valid = false;
	};


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

		void copySignalsData(const QVector<Signal *> &signalsList, std::vector<QVariantList>& metadata);

		int usedFramesCount() const { return m_usedFramesCount; }
		int framesDataSize() const { return m_usedFramesCount * m_tuningFrameSizeBytes; }

		quint64 generateUniqueID(const QString& lmEquipmentID);

		const char* framesData() const { return m_framesData; }

		void converToBigEndian();

		void setFrameData(int frameNo, const char* fotipData);

		bool getSignalState(const Signal* signal, TuningSignalState* tss);
		bool setSignalState(const Signal* signal, double value, SocketRequest* sr);
	};


	class TuningData : public Tuning::TuningData
	{
		Q_OBJECT

	private:
		TuningFramesData m_tuningFramesData[TYPES_COUNT];

	public:
		TuningData();
		TuningData(QString lmID);

		virtual ~TuningData();

		virtual bool buildTuningData() override;
		bool initTuningData();

		virtual void getTuningData(QByteArray* tuningData) const override;

		void setFrameData(int frameNo, const char* fotipData);

		bool getSignalState(const QString& appSignalID, TuningSignalState* tss);
		bool setSignalState(const QString& appSignalID, double value, SocketRequest* sr);
	};

}
