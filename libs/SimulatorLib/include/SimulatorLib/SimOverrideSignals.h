#pragma once

#include <array>
#include <atomic>
#include <memory>

#include <QObject>

#include "../../UtilsLib/Address16.h"

#include "SimOverrideRamRecord.h"

class QJSValue;
class QJSEngine;

namespace Sim
{
	class OverrideSignalsImpl;

	enum class OverrideSignalMethod
	{
		Value,  // Override signal with static value
		Script, // Script is used to define override value
	};

	struct OverrideSetValueData
	{
		QString appSignalId;
		OverrideSignalMethod method;
		QVariant value;
	};

	//
	// class OverrideSignalParam - It is really hard to split this class into public and private parts, also there is not too much
	//							   dependencies here, so leave it as is.
	//
	class OverrideSignalParam
	{
	public:
		OverrideSignalParam(const OverrideSignalParam& src);
		OverrideSignalParam(const AppSignal& signalParam);
		~OverrideSignalParam();

		OverrideSignalParam& operator=(const OverrideSignalParam& src);

	public:
		void updateSignalProperties(const AppSignal& signalParam, QVariant value = QVariant());

		QString valueString(int base = 10, E::AnalogFormat analogFormat = E::AnalogFormat::g_9_or_9e, int precision = -1) const;

		void setValue(const QVariant& value, OverrideSignalMethod method, bool changeCurrentMethod);

		void setDiscreteValue(quint16 value);
		void setWordValue(quint16 value);
		void setSignedIntvalue(qint32 value);
		void setFloatValue(float value);
		void setDoubleValue(double value);

		bool sameType(const OverrideSignalParam& another) const;

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
		QString m_lmEquipmentId;                              // LM where this signal lives

		E::SignalType m_signalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat m_dataFormat = E::AnalogAppSignalFormat::SignedInt32;
		E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

		int m_dataSizeW = 0;                                  // DataSize in words, for discrete is 1 word and set OverrideRamRecord::mask
		Address16 m_address;
		E::LogicModuleRamAccess m_ramAccess;                  // RAM type to override signal data

		std::array<Sim::OverrideRamRecord, 4> m_ramOverrides; // Set of RAM offsets, masks and data to override, up to 4 words

		OverrideSignalMethod m_method = OverrideSignalMethod::Value;
		QVariant m_value;

		static const QString s_defaultScriptValue;
		QString m_script = s_defaultScriptValue;

		// Copy operator is present, pay attention to adding new members
		//

		// Not for copy
		//
		QString m_scriptError;

	public:
		std::atomic<bool> m_scriptValueRequiresReset{
			true};                                 // Indicator that m_scriptValue, m_scriptEngine must be reset (deleted and created again)

		std::unique_ptr<QJSValue> m_scriptValue;   // Must be created and worked with in thread where it is used
		std::unique_ptr<QJSEngine> m_scriptEngine; // Must be created and worked with in thread where it is used
	};


	//
	// OverrideSignals
	//
	class OverrideSignals : public QObject
	{
		Q_OBJECT

	private:
		friend class SimulatorPrivate;
		explicit OverrideSignals(OverrideSignalsImpl& m_impl, QObject* parent = nullptr);

	public:
		void clear();

		int addSignals(const QStringList& appSignalIds);
		bool addSignal(QString appSignalId, bool enabled, int index, OverrideSignalMethod method, QVariant value, QString script);

		void removeSignal(const QString& appSignalId);
		void removeSignals(const QStringList& appSignalIds);

		[[nodiscard]] bool containsSignal(const QString& appSignalId) const;

		void setEnable(QString appSignalId, bool enable);

		void setValue(QString appSignalId, OverrideSignalMethod method, const QVariant& value);
		void setValues(const std::vector<OverrideSetValueData>& overrideData);

		bool saveWorkspace(QString fileName) const;
		bool loadWorkspace(QString fileName);

	signals:
		void signalsChanged(QStringList addedAppSignalIds); // Added or deleted signal
		void stateChanged(QStringList appSignalIds);        // Changed value or enable state

	public:
		std::optional<Sim::OverrideSignalParam> overrideSignal(QString appSignalId) const;
		std::vector<Sim::OverrideSignalParam> overrideSignals() const;
		QStringList overrideSignalIds() const;

	private:
		OverrideSignalsImpl& m_impl;
	};
} // namespace Sim
