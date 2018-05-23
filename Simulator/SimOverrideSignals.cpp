#include "SimOverrideSignals.h"

namespace Sim
{

	OverrideSignalParam::OverrideSignalParam(const Signal& signalParam)
	{
		appSignalId = signalParam.appSignalID();
		equipmentId = signalParam.equipmentID();		// !!!!!!!!!!1????????????????warning_equipmentId_is_port_id_for_inoouts

		signalType = signalParam.signalType();
		dataFormat = signalParam.analogSignalFormat();

		dataSizeW = signalParam.sizeW();
		address = signalParam.ualAddr();

		// Checks
		//
		if (dataSizeW > ramOverrides.size())
		{
			assert(dataSizeW <= ramOverrides.size());
			return;
		}

		// Create mask/data records
		//
		switch (signalType)
		{
		case E::SignalType::Analog:
			switch (dataFormat)
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				setSignedIntvalue(0);
				break;
			case E::AnalogAppSignalFormat::Float32:
				setFloatValue(0);
				break;
//			case E::AnalogAppSignalFormat::Double:???
//				break;
			default:
				assert(0);
			}
			break;

		case E::SignalType::Discrete:
			setDiscreteValue(0);
			break;

		default:
			assert(signalType == E::SignalType::Analog ||
				   signalType == E::SignalType::Discrete);
		}

		return;
	}

	void OverrideSignalParam::setDiscreteValue(quint16 value)
	{
		assert(dataSizeW == 1);

		ramOverrides[0].mask = qToBigEndian<quint16>(0x0001 << address.bit());
		ramOverrides[0].data = qToBigEndian<quint16>((value & 0x0001) << address.bit());

		return;
	}

	void OverrideSignalParam::setWordValue(quint16 value)
	{
		assert(dataSizeW == 1);

		ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[0].data = qToBigEndian<quint16>(value);

		return;
	}

	void OverrideSignalParam::setSignedIntvalue(qint32 value)
	{
		assert(dataSizeW == 2);

		qint32 converted = qToBigEndian<qint32>(value);
		quint16* ptr = reinterpret_cast<quint16*>(&converted);

		ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[0].data = *ptr;

		ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[1].data = *(ptr + 1);

		return;
	}

	void OverrideSignalParam::setFloatValue(float value)
	{
		assert(dataSizeW == 2);

		union Converter
		{
			quint32 asDword;
			float asFloat;
		};

		Converter c;
		c.asFloat = value;

		quint16* ptr = reinterpret_cast<quint16*>(&c.asDword);

		ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[0].data = *ptr;

		ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[1].data = *(ptr + 1);

		return;
	}

	void OverrideSignalParam::setDoubleValue(double value)
	{
		assert(dataSizeW == 4);

		union Converter
		{
			quint64 asDdword;
			double asDouble;
		};

		Converter c;
		c.asDouble = value;

		quint16* ptr = reinterpret_cast<quint16*>(c.asDdword);

		ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[0].data = *ptr;

		ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[1].data = *(ptr + 1);

		ramOverrides[2].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[2].data = *(ptr + 2);

		ramOverrides[3].mask = qToBigEndian<quint16>(0xFFFF);
		ramOverrides[3].data = *(ptr + 3);

		return;
	}


	OverrideSignals::OverrideSignals(Sim::AppSignalManager* appSignalManager, QObject* parent) :
		QObject(parent),
		Output("OverrideSignals"),
		m_appSignalManager(appSignalManager)
	{
		assert(appSignalManager);
		return;
	}

	OverrideSignals::~OverrideSignals()
	{
	}

	void OverrideSignals::addSignals(const QStringList& appSignalIds)
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return;
		}

		QStringList addedSignals;

		for (const QString& id : appSignalIds)
		{
			bool found = false;
			Signal sp = m_appSignalManager->signalParamExt(id, &found);

			if (found == false)
			{
				writeWaning(QString("Cannot add signal to override list, signal %1 not found.").arg(id));
				continue;
			}

			{
				QWriteLocker locker(&m_lock);

				auto[it, ok] = m_signals.emplace(id, sp);
				if (ok == false)
				{
					writeWaning(QString("Signal %1 aldready added to override list.").arg(id));
					continue;
				}
			}
		}

		if (addedSignals.isEmpty() == false)
		{
			emit signalsAdded(addedSignals);
		}

		return;
	}

}
