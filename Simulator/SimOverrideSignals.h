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

		QString valueString(int base = 10,
							E::AnalogFormat analogFormat = E::AnalogFormat::g_9_or_9e,
							int precision = -1) const;

		void setDiscreteValue(quint16 value);
		void setWordValue(quint16 value);
		void setSignedIntvalue(qint32 value);
		void setFloatValue(float value);
		void setDoubleValue(double value);

		// --
		//
		bool m_enabled = true;
		QString m_appSignalId;
		QString m_customSignalId;
		QString m_caption;
		QString m_equipmentId;							// LM where this signal lives

		E::SignalType m_signalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat m_dataFormat = E::AnalogAppSignalFormat::SignedInt32;

		int m_dataSizeW = 0;								// DataSize in words, for discrete is 1 word and set OverrideRamRecord::mask
		Address16 m_address;
		E::LogicModuleRamAccess m_ramAccess;				// RAM typoe to verride signal data

		std::array<OverrideRamRecord, 4> m_ramOverrides;	// Set of RAM offsets, masks and data to override, up to 4 words

		QVariant m_value;									// value set with set* functions
	};


	class OverrideSignals : public QObject, protected Output
	{
		Q_OBJECT

	public:
		explicit OverrideSignals(Sim::AppSignalManager* appSignalManager, QObject* parent = nullptr);
		virtual ~OverrideSignals();

	public:
		void clear();
		int addSignals(const QStringList& appSignalIds);
		void removeSignal(QString appSignalId);

		void setEnable(QString appSignalId, bool enable);

	signals:
		void signalsChanged(QStringList addedAppSignalIds);	// Added or deleted signal
		void stateChanged(QString appSignalId);				// Chenged value or enable state

	public:
		std::vector<OverrideSignalParam> overrideSignals() const;

	private:
		Sim::AppSignalManager* m_appSignalManager = nullptr;

		mutable QReadWriteLock m_lock;
		std::map<QString, OverrideSignalParam> m_signals;		// Key is AppSignalID
	};

}

#endif // SIMOVERRIDESIGNALS_H
