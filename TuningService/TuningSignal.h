#pragma once

#include <HardwareLib/DataProtocols.h>

#include "../UtilsLib/SimpleMutex.h"
#include "../UtilsLib/Queue.h"

namespace Tuning
{

	class TuningSignal
	{
	public:
		struct State
		{
			Hash signalHash = 0;				// calcHash(appSignalID)

			// dynamic signal state read from LM
			//
			bool valid = false;

			TuningValue currentValue;
			TuningValue readLowBound;
			TuningValue readHighBound;

			bool tuningDefaultFlag = false;

			qint64 successfulReadTime = 0;		// time of last succesfull signal reading (UTC), in normal should be permanently update
			qint64 writeRequestTime = 0;		// time of last write request (UTC)
			qint64 successfulWriteTime = 0;		// time of last succesfull signal writing (UTC), usually should be near m_writeRequestTime
			qint64 unsuccessfulWriteTime = 0;	// time of last unsuccesfull signal writing (UTC), usually should be near m_writeRequestTime
			qint64 lmTime = 0;
			quint64 fotipProcessingNumerator = 0;

			//

			quint64 writeCommandID = 0;							// if != 0 - writing in progress
																// if == 0 - no writing in progress (or writing is already finished)
			E::NetworkError writeErrorCode =					// last write error code, NetworkError:  Success, TuningValueOutOfRange, TuningNoReply
									E::NetworkError::Success;

			Hash lastWriteClient = 0;							// last write client's EquipmentID hash

			// this is not a signal state variables howerver included to pass struct TuningSignal::State to state changes queue
			//
			mutable bool setSOR = false;
			mutable bool writingDisabled = false;

			//

			bool writeInProgress() const { return writeCommandID != 0; }
			void saveToProto(Network::TuningSignalState* tss) const;
		};

	public:
		void init(const AppSignal* s, int index, int tuningDataFrameSizeW, QThread* parentThread);

		// static data getters
		//
		QString appSignalID() const { return m_appSignalID; }
		E::SignalType signalType() const { return m_signalType; }

		TuningValueType tuningValueType() const { return m_tuningValueType; }
		QString tuningValueTypeStr() const;

		int offset() const { return m_offset; }
		int bit() const { return m_bit; }
		int frameNo() const { return m_frameNo; }

		TuningValue defaultValue() const { return m_defaultValue; }
		TuningValue lowBound() const { return m_lowBound; }
		TuningValue highBound() const { return m_highBound; }

		Fotip::DataType fotipDataType() const;

		// dynamic state getters / setters

		bool invalidate();

		bool setCurrentState(bool valid, const TuningValue& value,
							 qint64 readTime, qint64 lmTime,
							 quint64 fotipProcessingNumerator,
							 bool setSOR, bool writingDisabled);

		const State& currentStateUnsafe() const;

		TuningValue currentTuningValueUnsafe() const;

		void setReadLowBound(const TuningValue& value, bool setSOR, bool writingDisabled);
		void setReadHighBound(const TuningValue& value, bool setSOR, bool writingDisabled);

		void initWriting(quint64 writeCommandID, const QString& clientID, qint64 time);
		void finalizeWriting(quint64 writeCommandID, E::NetworkError errCode, qint64 time);


	/*	bool valid() const { return m_state.valid(); }


		TuningValue currentValue() const { return m_state.currentValue(); }

		TuningValue readLowBound() const { return m_state.readLowBound(); }

		TuningValue readHighBound() const { return m_state.readHighBound(); }

		bool isTuningDefault() const { return m_state.tuningDefaultFlag(); }

		bool writeInProgress() const;
		E::NetworkError writeErrorCode() const { return m_writeErrorCode; }

		void setWriteClient(const QString& clientEquipmentID) { m_writeClient = calcHash(clientEquipmentID); }
		Hash writeClient() const { return m_writeClient; }

		void setWriteRequestTime(qint64 writeRequestTime) { m_writeRequestTime = writeRequestTime; }
		qint64 writeRequestTime() const { return m_writeRequestTime; }

		void setSuccessfulWriteTime(qint64 writeTime) { m_successfulWriteTime = writeTime; }
		qint64 successfulWriteTime() const { return m_successfulWriteTime; }

		void setUnsuccessfulWriteTime(qint64 writeTime) { m_unsuccessfulWriteTime = writeTime; }
		qint64 unsuccessfulWriteTime() const { return m_unsuccessfulWriteTime; }

		qint64 successfulReadTime() const { return m_successfulReadTime; }

		qint64 lmTime() const { return m_lmTime; }
		quint64 fotipProcessingNumerator() const { return m_fotipProcessingNumerator; }*/

		void saveToProto(Network::TuningSignalState* tss, bool setSOR, bool writingDisabled, QThread* thread) const;

	private:
		QString m_appSignalID;
		Hash m_signalHash = 0;
		E::SignalType m_signalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat m_analogFormat = E::AnalogAppSignalFormat::SignedInt32;

		int m_index = -1;
		int m_offset = -1;
		int m_bit = -1;
		int m_frameNo = -1;

		TuningValueType m_tuningValueType = TuningValueType::Discrete;

		QThread* m_thread = nullptr;

		// signal properties from RPCT Database
		//
		TuningValue m_lowBound;
		TuningValue m_highBound;
		TuningValue m_defaultValue;

		// dynamic TuningSignal state
		//
		mutable SimpleMutex m_stateMutex;
		State m_state;
	};

	using TuningSignalShared = std::shared_ptr<TuningSignal>;
	using TuningSignalConstShared = std::shared_ptr<const TuningSignal>;
	using TuningSignalsChangesQueue = FastThreadSafeQueue<TuningSignal::State>;
}
