#ifndef SIMOVERRIDESIGNALS_H
#define SIMOVERRIDESIGNALS_H

#include <QObject>
#include <QReadWriteLock>
#include "SimAppSignalManager.h"
#include "SimOutput.h"

namespace Sim
{
	struct OverrideRamRecord
	{
		quint16 mask = 0;
		quint16 data = 0;
	};

	struct OverrideSignalParam
	{
		OverrideSignalParam(const Signal& signalParam);

		void setDiscreteValue(quint16 value);
		void setWordValue(quint16 value);
		void setSignedIntvalue(qint32 value);
		void setFloatValue(float value);
		void setDoubleValue(double value);

		// --
		//
		QString appSignalId;
		QString equipmentId;							// LM where this signal lives

		E::SignalType signalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat dataFormat = E::AnalogAppSignalFormat::SignedInt32;

		int dataSizeW = 0;								// DataSize in words, for discrete is 1 word and set OverrideRamRecord::mask
		Address16 address;
		E::LogicModuleRamAccess ramAccess;				// RAM typoe to verride signal data

		std::array<OverrideRamRecord, 4> ramOverrides;	// Set of RAM offsets, masks and data to override, up to 4 words
	};

	class OverrideSignals : public QObject, protected Output
	{
		Q_OBJECT

	public:
		explicit OverrideSignals(Sim::AppSignalManager* appSignalManager, QObject* parent = nullptr);
		virtual ~OverrideSignals();

	public:
		void addSignals(const QStringList& appSignalIds);

	signals:
		void signalsAdded(QStringList appSignalIds);

	private:
		Sim::AppSignalManager* m_appSignalManager = nullptr;

		QReadWriteLock m_lock;
		std::map<QString, OverrideSignalParam> m_signals;
	};

}

#endif // SIMOVERRIDESIGNALS_H
