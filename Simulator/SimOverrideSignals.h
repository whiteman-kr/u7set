#ifndef SIMOVERRIDESIGNALS_H
#define SIMOVERRIDESIGNALS_H

#include <optional>
#include <QObject>
#include <QReadWriteLock>
#include "../lib/Types.h"
#include "../lib/Signal.h"
#include "SimScopedLog.h"


class QJSValue;
class QJSEngine;


namespace Sim
{
	class RamAreaInfo;
	class Simulator;
	class AppSignalManager;

	enum class OverrideSignalMethod
	{
		Value,			// Override signal with static value
		Script,			// Script is used to define override value
	};

	struct OverrideRamRecord
	{
		quint16 mask = 0;
		quint16 data = 0;

		void overlapRecord(const OverrideRamRecord& r)
		{
			mask |= r.mask;
			data |= r.data;
		}

		void applyOverlapping(quint16* ptrW) const
		{
			assert(ptrW);
			*ptrW &= ~mask;
			*ptrW |= data;
		}

//		quint16 overlappedValue(quint16 value) const
//		{
//			value &= ~mask;
//			value |= data;
//			return value;
//		}
	};

	class OverrideSignalParam
	{
	public:
		OverrideSignalParam(const OverrideSignalParam& src);
		OverrideSignalParam(const Signal& signalParam);

		OverrideSignalParam& operator=(const OverrideSignalParam& src);

	public:
		void updateSignalProperties(const Signal& signalParam, QVariant value = QVariant());

		QString valueString(int base = 10,
							E::AnalogFormat analogFormat = E::AnalogFormat::g_9_or_9e,
							int precision = -1) const;

		void setValue(const QVariant& value, OverrideSignalMethod method, bool changeCurrentMethod);

		void setDiscreteValue(quint16 value);
		void setWordValue(quint16 value);
		void setSignedIntvalue(qint32 value);
		void setFloatValue(float value);
		void setDoubleValue(double value);

		// Properties
		//
	public:
		bool enabled() const;
		void setEnabled(bool en);

		int index() const;
		void setIndex(int value);

		const QString& appSignalId() const;
		const QString& customSignalId() const;
		const QString& caption() const;
		const QString& lmEquipmentId() const;

		E::SignalType signalType() const;
		E::AnalogAppSignalFormat dataFormat() const;
		E::ByteOrder byteOrder() const;

		int dataSizeW() const;
		const Address16& address() const;
		E::LogicModuleRamAccess ramAccess() const;

		const OverrideRamRecord& ramOverrides(size_t index) const;

		OverrideSignalMethod method() const;

		const QVariant& value() const;
		const QString& script() const;

		const QString& scriptError() const;
		void setScriptError(const QString& value);

		// --
		//
	private:
		// Copy operator is present, pay attention to adding new members
		//
		bool m_enabled = true;
		int m_index = 0;

		QString m_appSignalId;
		QString m_customSignalId;
		QString m_caption;
		QString m_lmEquipmentId;								// LM where this signal lives

		E::SignalType m_signalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat m_dataFormat = E::AnalogAppSignalFormat::SignedInt32;
		E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

		int m_dataSizeW = 0;								// DataSize in words, for discrete is 1 word and set OverrideRamRecord::mask
		Address16 m_address;
		E::LogicModuleRamAccess m_ramAccess;				// RAM type to verride signal data

		std::array<OverrideRamRecord, 4> m_ramOverrides;	// Set of RAM offsets, masks and data to override, up to 4 words

		OverrideSignalMethod m_method = OverrideSignalMethod::Value;
		QVariant m_value;

		QString m_script =
R"+++((function(lastOverrideValue, workcycle)
{
	// lastOverrideValue - The last value returned from this function
	// workcycle - Workcycle counter

	var result = 0;

	return result;	// Returns value for signal overriding
}))+++";

		// Copy operator is present, pay attention to adding new members
		//

//	public:
//		// Not for copy
//		//
		QString m_scriptError;

	public:
		std::atomic<bool> m_scriptValueRequiresReset{true};	// Indicator that m_scriptValue, m_scriptEngine must be reset (deleted and created again)

		std::unique_ptr<QJSValue> m_scriptValue;			// Must be created and worked with in thrread where it is used
		std::unique_ptr<QJSEngine> m_scriptEngine;			// Must be created and worked with in thrread where it is used
	};


	class OverrideSignals : public QObject
	{
		Q_OBJECT

	public:
		explicit OverrideSignals(Sim::Simulator* simulator, QObject* parent = nullptr);
		virtual ~OverrideSignals();

	public:
		void clear();

		int addSignals(const QStringList& appSignalIds);
		bool addSignal(QString appSignalId, bool enabled, int index, OverrideSignalMethod method, QVariant value, QString script);

		void removeSignal(QString appSignalId);
		bool isSignalInOverrideList(QString appSignalId) const;

		void setEnable(QString appSignalId, bool enable);
		void setValue(QString appSignalId, OverrideSignalMethod method, const QVariant& value);

		void updateSignals();								// Update signal descriptions, type, offsets, etc...

		bool runOverrideScripts(const QString& lmEquipmentId, qint64 workcycle);	// Runs override scripts and sets value to override signals
		void requestToResetOverrideScripts(const QString& lmEquipmentId);			// If module is reset, then script must be restarted, clear global variables, etc

		bool saveWorkspace(QString fileName) const;
		bool loadWorkspace(QString fileName);

	signals:
		void signalsChanged(QStringList addedAppSignalIds);	// Added or deleted signal
		void stateChanged(QStringList appSignalIds);		// Changed value or enable state

	public:
		AppSignalManager& appSignalManager();
		const AppSignalManager& appSignalManager() const;

		std::optional<OverrideSignalParam> overrideSignal(QString appSignalId) const;
		std::vector<OverrideSignalParam> overrideSignals() const;
		QStringList overrideSignalIds() const;

		int changesCounter() const;

		std::vector<OverrideRamRecord> ramOverrideData(const QString& lmEquipmentId, const RamAreaInfo& ramAreaInfo) const;

	private:
		Sim::Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		mutable QReadWriteLock m_lock;
		std::map<QString, OverrideSignalParam> m_signals;	// Key is AppSignalID
		int m_changesCounter = 0;							// This variable is inceremented every time m_signals has
															// any changes, so if it is changeed then RAM requests update
															//
	};

}

#endif // SIMOVERRIDESIGNALS_H
