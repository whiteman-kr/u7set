#include "TuningSignal.h"
#include "../UtilsLib/WUtils.h"

namespace Tuning
{

	// ----------------------------------------------------------------------------------
	//
	// TuningSignal::State class implementation
	//
	// ----------------------------------------------------------------------------------

	void TuningSignal::State::saveToProto(Network::TuningSignalState* tss) const
	{
		TEST_PTR_RETURN(tss);

		tss->set_signalhash(signalHash);
		tss->set_error(TO_INT(E::NetworkError::Success));
		tss->set_valid(valid);

		currentValue.save(tss->mutable_value());
		readLowBound.save(tss->mutable_readlowbound());
		readHighBound.save(tss->mutable_readhighbound());

		tss->set_writeinprogress(writeInProgress());
		tss->set_writeerrorcode(TO_INT(writeErrorCode));
		tss->set_writeclient(lastWriteClient);

		tss->set_successfulreadtime(successfulReadTime);
		tss->set_writerequesttime(writeRequestTime);
		tss->set_successfulwritetime(successfulWriteTime);
		tss->set_unsuccessfulwritetime(unsuccessfulWriteTime);

		tss->set_setsor(setSOR);
		tss->set_writingdisabled(writingDisabled);

		tss->set_tuningdefault(tuningDefaultFlag);

		tss->set_lmtime(lmTime);
		tss->set_fotipprocessingnumerator(fotipProcessingNumerator);
	}

	// ----------------------------------------------------------------------------------
	//
	// TuningSignal class implementation
	//
	// ----------------------------------------------------------------------------------

	void TuningSignal::init(const AppSignal* s, int index, int tuningDataFrameSizeW, QThread* parentThread)
	{
		TEST_PTR_RETURN(s);
		TEST_PTR_RETURN(parentThread);

		m_thread = parentThread;

		m_appSignalID = s->appSignalID();

		m_signalType = s->signalType();
		m_analogFormat = s->analogSignalFormat();

		m_signalHash = ::calcHash(m_appSignalID);

		m_index = index;

		m_offset = s->tuningAddr().offset();
		m_bit = s->tuningAddr().bit();
		m_frameNo = s->tuningAddr().offset() / tuningDataFrameSizeW;

		m_lowBound = s->tuningLowBound();
		m_highBound = s->tuningHighBound();
		m_defaultValue = s->tuningDefaultValue();

		m_state.signalHash = calcHash(m_appSignalID);

		// update tuning values type

		m_tuningValueType = TuningValue::getTuningValueType(s->signalType(), s->analogSignalFormat());

		m_lowBound.setType(m_tuningValueType);
		m_highBound.setType(m_tuningValueType);
		m_defaultValue.setType(m_tuningValueType);

		m_state.currentValue.setType(m_tuningValueType);
		m_state.readLowBound.setType(m_tuningValueType);
		m_state.readHighBound.setType(m_tuningValueType);
	}

	QString TuningSignal::tuningValueTypeStr() const
	{
		TuningValue tv(m_tuningValueType);

		return tv.typeStr();
	}

	Fotip::DataType TuningSignal::fotipDataType() const
	{
		switch(m_tuningValueType)
		{
		case TuningValueType::Discrete:
			return Fotip::DataType::Discrete;

		case TuningValueType::Float:
			return Fotip::DataType::AnalogFloat;

		case TuningValueType::SignedInt32:
			return Fotip::DataType::AnalogSignedInt;

		default:
			assert(false);
		}

		return Fotip::DataType::Discrete;
	}

	bool TuningSignal::invalidate()
	{
		Q_ASSERT(QThread::currentThread() == m_thread);

		bool prevValid = m_state.valid;

		AUTO_LOCK_BY_CURRENT_THREAD(m_stateMutex);

		m_state.valid = false;

		return prevValid;
	}

	bool TuningSignal::setCurrentState(bool valid, const TuningValue& value,
									   qint64 readTime, qint64 lmTime,
									   quint64 fotipProcessingNumerator,
									   bool setSOR, bool writingDisabled)
	{
		AUTO_LOCK_BY_CURRENT_THREAD(m_stateMutex);

		bool prevValid = m_state.valid;
		TuningValue prevValue = m_state.currentValue;

		//

		m_state.valid = valid;

		if (valid == true)
		{
			m_state.fotipProcessingNumerator = fotipProcessingNumerator;

			m_state.successfulReadTime = readTime;
			m_state.lmTime = lmTime;

			Q_ASSERT(m_state.currentValue.type() == value.type());
			Q_ASSERT(m_defaultValue.type() == value.type());

			m_state.currentValue = value;
			m_state.tuningDefaultFlag = (m_state.currentValue == m_defaultValue);

			m_state.setSOR = setSOR;
			m_state.writingDisabled = writingDisabled;
		}
		else
		{
			m_state.lmTime = 0;
			m_state.fotipProcessingNumerator = 0;
			m_state.tuningDefaultFlag = false;
		}

		// check signal change
		//
		return m_state.valid != prevValid ||
			   m_state.currentValue != prevValue;
	}

	const TuningSignal::State& TuningSignal::currentStateUnsafe() const
	{
		Q_ASSERT(QThread::currentThread() == m_thread);

		return m_state;
	}

	TuningValue TuningSignal::currentTuningValueUnsafe() const
	{
		Q_ASSERT(QThread::currentThread() == m_thread);

		return m_state.currentValue;
	}

	void TuningSignal::setReadLowBound(const TuningValue& value, bool setSOR, bool writingDisabled)
	{
		Q_ASSERT(m_state.readLowBound.type() == value.type());

		AUTO_LOCK_BY_CURRENT_THREAD(m_stateMutex);

		m_state.readLowBound = value;
		m_state.setSOR = setSOR;
		m_state.writingDisabled = writingDisabled;
	}

	void TuningSignal::setReadHighBound(const TuningValue& value, bool setSOR, bool writingDisabled)
	{
		Q_ASSERT(m_state.readHighBound.type() == value.type());

		AUTO_LOCK_BY_CURRENT_THREAD(m_stateMutex);

		m_state.readHighBound = value;
		m_state.setSOR = setSOR;
		m_state.writingDisabled = writingDisabled;
	}

	void TuningSignal::initWriting(quint64 writeCommandID, const QString& clientID, qint64 time)
	{
		Q_ASSERT(writeCommandID != 0);
		Q_ASSERT(QThread::currentThread() == m_thread);

		AUTO_LOCK_BY_CURRENT_THREAD(m_stateMutex);

		m_state.writeCommandID = writeCommandID;
		m_state.writeErrorCode = E::NetworkError::Success;

		m_state.lastWriteClient = calcHash(clientID);
		m_state.writeRequestTime = time;
	}

	void TuningSignal::finalizeWriting(quint64 writeCommandID, E::NetworkError errCode, qint64 time)
	{
		Q_ASSERT(QThread::currentThread() == m_thread);

		AUTO_LOCK_BY_CURRENT_THREAD(m_stateMutex);

		if (writeCommandID == m_state.writeCommandID)
		{
			m_state.writeErrorCode = errCode;

			if (errCode == E::NetworkError::Success)
			{
				m_state.writeCommandID = 0;

				m_state.successfulWriteTime = time;
				m_state.unsuccessfulWriteTime = 0;
			}
			else
			{
				m_state.successfulWriteTime = 0;
				m_state.unsuccessfulWriteTime = time;
			}
		}
	}

	void TuningSignal::saveToProto(Network::TuningSignalState* tss,
								   bool setSOR, bool writingDisabled,
								   QThread* thread) const
	{
		TEST_PTR_RETURN(tss);
		TEST_PTR_RETURN(thread);

		AUTO_LOCK_BY_CURRENT_THREAD(m_stateMutex);

		m_state.setSOR = setSOR;
		m_state.writingDisabled = writingDisabled;

		m_state.saveToProto(tss);

		tss->set_error(TO_INT(E::NetworkError::Success));
	}

}
