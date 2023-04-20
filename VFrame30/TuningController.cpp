#include "TuningController.h"
#include "../AppSignalLib/AppSignalParam.h"
#include "../AppSignalLib/ITuningSignalManager.h"

namespace VFrame30
{
	TuningController::TuningController(ITuningSignalManager* signalManager, ITuningConnection* tuningConnection, QWidget* parent) :
		QObject(parent),
		m_signalManager(signalManager),
		m_tuningConnection(tuningConnection)
	{
		assert(m_signalManager);
		return;
	}

	AppSignalParam TuningController::signalParam(const QString& appSignalId, bool* ok) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->signalParam(appSignalId, ok);
	}

	TuningSignalState TuningController::signalState(const QString& appSignalId, bool* ok) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->state(appSignalId, ok);
	}

	QVariant TuningController::signalParam(const QString& appSignalId) const
	{
		bool ok = true;
		QVariant result = QVariant::fromValue(signalParam(appSignalId, &ok));

		if (ok == false)
		{
			return QVariant();
		}

		return result;
	}

	QVariant TuningController::signalState(const QString& appSignalId) const
	{
		bool ok = true;
		QVariant result = QVariant::fromValue(signalState(appSignalId, &ok));

		if (ok == false)
		{
			return QVariant();
		}

		return result;
	}

	bool TuningController::signalExists(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->signalExists(::calcHash(signalId));
	}

	bool TuningController::isDiscrete(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		bool ok = false;

		AppSignalParam asp = m_signalManager->signalParam(::calcHash(signalId), &ok);

		return ok ?
					(asp.type() == E::SignalType::Discrete) :
					false;
	}

	bool TuningController::isAnalog(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		bool ok = false;

		AppSignalParam asp = m_signalManager->signalParam(::calcHash(signalId), &ok);

		return ok ?
					(asp.type() == E::SignalType::Analog) :
					false;
	}

	int TuningController::precision(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		bool ok = false;

		AppSignalParam asp = m_signalManager->signalParam(::calcHash(signalId), &ok);

		return (ok == true && asp.isAnalog() == true) ? asp.precision() : 0;
	}

	QStringList TuningController::signalIdsByTag(QString tag) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->signalIdsByTag(tag);
	}

	bool TuningController::writeValue(QString appSignalId, QVariant value)
	{
		if (checkTuningAccess() == false)
		{
			return true;	// Access is denied, this is not an error
		}

		appSignalId = appSignalId.trimmed();

		bool ok = false;
		AppSignalParam appSignal = signalParam(appSignalId, &ok);
		if (ok == false)
		{
			return false;
		}

		// Adjust value type to match signal type
		//
		auto valueType = value.metaType().id();

		if (valueType != QMetaType::Bool &&
			valueType != QMetaType::Int &&
			valueType != QMetaType::Double)
		{
			qDebug() << "Error: Unsupported value type arrived from script.";
			Q_ASSERT(false);
			return false;
		}

		TuningValueType tuningType = appSignal.tuningType();

		switch (tuningType)
		{
		case TuningValueType::Discrete:
			{
				if (valueType == QMetaType::Bool)
				{
					break;
				}
				if (valueType == QMetaType::Int)
				{
					value = value.toInt() == 0 ? false : true;
					break;
				}
				if (valueType == QMetaType::Double)
				{
					value = value.toDouble() == 0 ? false : true;
					break;
				}
				Q_ASSERT(false);
			}
			break;
		case TuningValueType::SignedInt32:
			{
				if (valueType == QMetaType::Bool)
				{
					qDebug() << "Warning: Writing Bool value to Int32 signal type.";
					value = value.toBool() == false ? static_cast<int>(0) : static_cast<int>(1);
					Q_ASSERT(value.userType() == QMetaType::Int);
					break;
				}
				if (valueType == QMetaType::Int)
				{
					break;
				}
				if (valueType == QMetaType::Double)
				{
					if (double valueDouble = value.toDouble();
						valueDouble < std::numeric_limits<qint32>::min() || valueDouble > std::numeric_limits<qint32>::max())
					{
						qDebug() << "Error: Double value is out of Int32 signal range.";
						Q_ASSERT(false);
						return false;
					}

					value = value.toInt();
					break;
				}
				Q_ASSERT(false);
			}
			break;
		case TuningValueType::Float:
			{
				if (valueType == QMetaType::Bool)
				{
					qDebug() << "Warning: Writing Bool value to Float32 signal type.";
					value = value.toBool() == false ? static_cast<float>(0) : static_cast<float>(1);
					Q_ASSERT(value.userType() == QMetaType::Float);
					break;
				}
				if (valueType == QMetaType::Int || valueType == QMetaType::Double)
				{
					value = value.toFloat();
					break;
				}
				Q_ASSERT(false);
			}
			break;
		case TuningValueType::SignedInt64:
			{
				Q_ASSERT(false);	// No signals of this type exist
				return false;
			}
		case TuningValueType::Double:
			{
				Q_ASSERT(false);	// No signals of this type exist
				return false;
			}
		}

		TuningValue tuningValue(value);

		// Check range for analog signal
		//
		if (appSignal.tuningType() != TuningValueType::Discrete)
		{
			if (tuningValue < appSignal.tuningLowBound() || tuningValue > appSignal.tuningHighBound())
			{
				return false;
			}
		}

		ok &= m_tuningConnection->writeTuningSignal(appSignalId, tuningValue);

		return ok;
	}

	void TuningController::apply()
	{
		if (checkTuningAccess() == false)
		{
			return;	// Access is denied, this is not an error
		}

		m_tuningConnection->applyTuningSignals();

		return;
	}

	bool TuningController::checkTuningAccess() const
	{
		return true;
	}

}
