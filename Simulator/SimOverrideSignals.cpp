#include "SimOverrideSignals.h"
#include "Simulator.h"
#include "SimRam.h"

namespace Sim
{

	OverrideSignalParam::OverrideSignalParam(const OverrideSignalParam& src)
	{
		*this = src;
	}

	OverrideSignalParam::OverrideSignalParam(const Signal& signalParam)
	{
		updateSignalProperties(signalParam);
		return;
	}

	OverrideSignalParam& OverrideSignalParam::operator=(const OverrideSignalParam& src)
	{
		this->m_enabled = src.m_enabled;
		this->m_index = src.m_index;

		this->m_appSignalId = src.m_appSignalId;
		this->m_customSignalId = src.m_customSignalId;
		this->m_caption = src.m_caption;
		this->m_lmEquipmentId = src.m_lmEquipmentId;

		this->m_signalType = src.m_signalType;
		this->m_dataFormat = src.m_dataFormat;
		this->m_byteOrder = src.m_byteOrder;

		this->m_dataSizeW = src.m_dataSizeW;
		this->m_address = src.m_address;
		this->m_ramAccess = src.m_ramAccess;

		this->m_ramOverrides = src.m_ramOverrides;

		this->m_method = src.m_method;
		this->m_value = src.m_value;
		this->m_script = src.m_script;

		return *this;
	}

	void OverrideSignalParam::updateSignalProperties(const Signal& signalParam, QVariant value /*= QVariant()*/)
	{
		m_appSignalId = signalParam.appSignalID();
		m_customSignalId = signalParam.customAppSignalID();
		m_caption = signalParam.caption();
		m_lmEquipmentId = signalParam.lmEquipmentID();

		m_signalType = signalParam.signalType();
		m_dataFormat = signalParam.analogSignalFormat();
		m_byteOrder = signalParam.byteOrder();

		m_dataSizeW = signalParam.sizeW();
		m_address = signalParam.ualAddr();
		m_ramAccess = signalParam.lmRamAccess();

		// Checks
		//
		if (m_dataSizeW > m_ramOverrides.size())
		{
			assert(m_dataSizeW <= m_ramOverrides.size());
			return;
		}

		// Create mask/data records
		//
		switch (m_signalType)
		{
		case E::SignalType::Discrete:
			if (value.isValid() == false ||
				value.type() != m_value.type())
			{
				setDiscreteValue(0);
			}
			else
			{
				setDiscreteValue(m_value.value<quint16>());
			}
			break;

		case E::SignalType::Analog:
			switch (m_dataFormat)
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				if (value.isValid() == false ||
					value.type() != m_value.type())
				{
					setSignedIntvalue(0);
				}
				else
				{
					setSignedIntvalue(m_value.value<qint32>());
				}
				break;
			case E::AnalogAppSignalFormat::Float32:
				if (value.isValid() == false ||
					value.type() != m_value.type())
				{
					setFloatValue(0);
				}
				else
				{
					setFloatValue(m_value.value<float>());
				}
				break;
			default:
				assert(0);
			}
			break;

		default:
			assert(m_signalType == E::SignalType::Analog ||
				   m_signalType == E::SignalType::Discrete);
		}

		return;
	}


	QString OverrideSignalParam::valueString(int base /*= 10*/,
											 E::AnalogFormat analogFormat /*= E::AnalogFormat::g_9_or_9e*/,
											 int precision /*= -14*/) const
	{
		QString result;

		if (m_signalType != E::SignalType::Discrete &&
			m_signalType != E::SignalType::Analog)
		{
			assert(m_signalType == E::SignalType::Discrete ||
				   m_signalType == E::SignalType::Analog);

			return result;
		}

		if (m_signalType == E::SignalType::Discrete)
		{
			assert(m_value.canConvert<quint16>() == true);

			quint16 val = m_value.value<quint16>();

			result = QString{"%1"}.arg(val);
		}

		if (m_signalType == E::SignalType::Analog)
		{
			switch (m_dataFormat)
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				{
					qint32 val = m_value.value<qint32>();

					if (base == 10)
					{
						result = QString{"%1"}.arg(val, 0, base);
					}

					if (base == 16)
					{
						result = QString{"%1h"}.arg(val, 0, base);
					}

				}
				break;
			case E::AnalogAppSignalFormat::Float32:
				{
					float val = m_value.value<float>();


					if (precision == -1)
					{
						result = QString{"%1"}.arg(val, 0, (char)(analogFormat));
					}
					else
					{
						result = QString{"%1"}.arg(val, 0, (char)(analogFormat), precision);
					}

					QLocale c;
					result.replace('.', c.decimalPoint());
				}
				break;
			default:
				assert(false);
			}
		}

		if (m_method == OverrideSignalMethod::Script)
		{
			if (m_scriptError.isEmpty() == false)
			{
				result = QLatin1String("JS: ") + m_scriptError;
			}
			else
			{
				result.prepend(QLatin1String("JS: "));
			}
		}


		return result;
	}

	void OverrideSignalParam::setValue(const QVariant& value, OverrideSignalMethod method, bool changeCurrentMethod)
	{
		if (changeCurrentMethod == true)
		{
			m_method = method;
		}

		switch (method)
		{
		case OverrideSignalMethod::Value:
			{
				switch (m_signalType)
				{
				case E::SignalType::Discrete:
					{
						if (value.canConvert<quint16>() == false)
						{
							assert(value.canConvert<quint16>());
							break;
						}

						quint16 discrValue = value.value<quint16>();
						setDiscreteValue(discrValue);
					}
					break;

				case E::SignalType::Analog:
					{
						switch (m_dataFormat)
						{
						case E::AnalogAppSignalFormat::SignedInt32:
							{
								if (value.canConvert<qint32>() == false)
								{
									assert(value.canConvert<qint32>());
									break;
								}

								qint32 sintValue = value.value<qint32>();
								setSignedIntvalue(sintValue);
							}
							break;

						case E::AnalogAppSignalFormat::Float32:
							{
								if (value.canConvert<float>() == false)
								{
									assert(value.canConvert<float>());
									break;
								}

								float floatValue = value.value<float>();
								setFloatValue(floatValue);
							}
							break;
						default:
							assert(false);
						}
					}
					break;

				default:
					assert(false);
					break;
				}
			}
			break;

		case OverrideSignalMethod::Script:
			{
				m_scriptError.clear();
				m_scriptValueRequiresReset = true;

				if (value.type() == QMetaType::QString)
				{
					m_script = value.toString();

					switch (m_signalType)
					{
					case E::SignalType::Discrete:
						setDiscreteValue(0);
						break;

					case E::SignalType::Analog:
						switch (m_dataFormat)
						{
						case E::AnalogAppSignalFormat::SignedInt32:
							setSignedIntvalue(0);
							break;
						case E::AnalogAppSignalFormat::Float32:
							setFloatValue(0);
							break;
						default:
							assert(false);
						}
						break;
					default:
						assert(false);
						break;
					}
				}
				else
				{
					assert(value.type() == QMetaType::QString);
					m_value = QString("");
				}
			}
			break;

		default:
			assert(false);
		}

		return;
	}

	void OverrideSignalParam::setDiscreteValue(quint16 value)
	{
		assert(m_dataSizeW == 1);

		m_ramOverrides[0].mask = qToBigEndian<quint16>(0x0001 << m_address.bit());
		m_ramOverrides[0].data = qToBigEndian<quint16>((value & 0x0001) << m_address.bit());

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setWordValue(quint16 value)
	{
		assert(m_dataSizeW == 1);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[0].data = qToBigEndian<quint16>(value);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setSignedIntvalue(qint32 value)
	{
		assert(m_dataSizeW == 2);

		qint32 converted = qToBigEndian<qint32>(value);
		quint16* ptr = reinterpret_cast<quint16*>(&converted);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[0].data = *ptr;

			m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[1].data = *(ptr + 1);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setFloatValue(float value)
	{
		assert(m_dataSizeW == 2);

		union Converter
		{
			quint32 asDword;
			float asFloat;
		};

		Converter c;
		c.asFloat = value;
		c.asDword = qToBigEndian(c.asDword);

		quint16* ptr = reinterpret_cast<quint16*>(&c.asDword);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[0].data = *ptr;

			m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[1].data = *(ptr + 1);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setDoubleValue(double value)
	{
		assert(m_dataSizeW == 4);

		union Converter
		{
			quint64 asDdword;
			double asDouble;
		};

		Converter c;
		c.asDouble = value;

		quint16* ptr = reinterpret_cast<quint16*>(c.asDdword);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[0].data = *ptr;

			m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[1].data = *(ptr + 1);

			m_ramOverrides[2].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[2].data = *(ptr + 2);

			m_ramOverrides[3].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[3].data = *(ptr + 3);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	bool OverrideSignalParam::enabled() const
	{
		return m_enabled;
	}

	void OverrideSignalParam::setEnabled(bool en)
	{
		m_enabled = en;
	}

	int OverrideSignalParam::index() const
	{
		return m_index;
	}

	void OverrideSignalParam::setIndex(int value)
	{
		m_index = value;
	}

	const QString& OverrideSignalParam::appSignalId() const
	{
		return m_appSignalId;
	}

	const QString& OverrideSignalParam::customSignalId() const
	{
		return m_customSignalId;
	}

	const QString& OverrideSignalParam::caption() const
	{
		return m_caption;
	}

	const QString& OverrideSignalParam::lmEquipmentId() const
	{
		return m_lmEquipmentId;
	}

	E::SignalType OverrideSignalParam::signalType() const
	{
		return m_signalType;
	}

	E::AnalogAppSignalFormat OverrideSignalParam::dataFormat() const
	{
		return m_dataFormat;
	}

	E::ByteOrder OverrideSignalParam::byteOrder() const
	{
		return m_byteOrder;
	}

	int OverrideSignalParam::dataSizeW() const
	{
		return m_dataSizeW;
	}

	const Address16& OverrideSignalParam::address() const
	{
		return m_address;
	}

	E::LogicModuleRamAccess OverrideSignalParam::ramAccess() const
	{
		return m_ramAccess;
	}

	const OverrideRamRecord& OverrideSignalParam::ramOverrides(size_t index) const
	{
		assert(index <= m_ramOverrides.size());
		return m_ramOverrides[index];
	}

	OverrideSignalMethod OverrideSignalParam::method() const
	{
		return m_method;
	}

	const QVariant& OverrideSignalParam::value() const
	{
		return m_value;
	}

	const QString& OverrideSignalParam::script() const
	{
		return m_script;
	}

	const QString& OverrideSignalParam::scriptError() const
	{
		return m_scriptError;
	}

	void OverrideSignalParam::setScriptError(const QString& value)
	{
		m_scriptError = value;
	}


	OverrideSignals::OverrideSignals(Simulator* simulator, QObject* parent /*= nullptr*/) :
		QObject(parent),
		Output("OverrideSignals"),
		m_simulator(simulator)
	{
		assert(simulator);
		return;
	}

	OverrideSignals::~OverrideSignals()
	{
	}

	void OverrideSignals::clear()
	{
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter ++;

			m_signals.clear();
		}

		emit signalsChanged({});
		return;
	}

	int OverrideSignals::addSignals(const QStringList& appSignalIds)
	{
		if (m_simulator == nullptr)
		{
			assert(m_simulator);
			return 0;
		}

		QStringList addedSignals;

		for (const QString& id : appSignalIds)
		{
			std::optional<Signal> sp = appSignalManager().signalParamExt(id);

			if (sp.has_value() == false)
			{
				writeWaning(QString("Cannot add signal to override list, signal %1 not found.").arg(id));
				continue;
			}

			{
				QWriteLocker locker(&m_lock);
				m_changesCounter ++;

				auto[it, ok] = m_signals.emplace(id, *sp);
				if (ok == false)
				{
					writeWaning(QString("Signal %1 aldready added to override list.").arg(id));
					continue;
				}
				else
				{
					addedSignals << sp->appSignalID();

					// Set index for new signal
					//
					int maxIndex = m_signals.begin()->second.index();

					for (const auto&[id, s] : m_signals)
					{
						Q_UNUSED(id);
						maxIndex = std::max(maxIndex, s.index());
					}

					it->second.setIndex(maxIndex + 1);
				}
			}
		}

		if (addedSignals.isEmpty() == false)
		{
			emit signalsChanged(addedSignals);
		}

		return addedSignals.size();
	}

	void OverrideSignals::removeSignal(QString appSignalId)
	{
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter ++;

			m_signals.erase(appSignalId);
		}

		emit signalsChanged({});
		return;
	}

	void OverrideSignals::setEnable(QString appSignalId, bool enable)
	{
		bool changed = false;

		{
			QWriteLocker locker(&m_lock);
			m_changesCounter ++;

			if (auto it = m_signals.find(appSignalId);
				it != m_signals.end() && it->second.enabled() != enable)
			{
				it->second.setEnabled(enable);
				changed = true;
			}
		}

		if (changed == true)
		{
			emit stateChanged(QStringList{} << appSignalId);
		}

		return;
	}

	void OverrideSignals::setValue(QString appSignalId, OverrideSignalMethod method, const QVariant& value)
	{
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter ++;

			auto it = m_signals.find(appSignalId);

			if (it == m_signals.end())
			{
				writeError(tr("Can't set new value for %1, signal not found").arg(appSignalId));
				return;
			}

			OverrideSignalParam& osp = it->second;
			osp.setValue(value, method, true);
		}

		emit stateChanged(QStringList{} << appSignalId);
		return;
	}

	void OverrideSignals::updateSignals()
	{
		std::vector<OverrideSignalParam> existingSignals =  overrideSignals();

		std::vector<OverrideSignalParam> newSignals;
		newSignals.reserve(existingSignals.size());

		for (const OverrideSignalParam& osp : existingSignals)
		{
			std::optional<Signal> sp = appSignalManager().signalParamExt(osp.appSignalId());

			if (sp.has_value() == false)
			{
				writeWaning(tr("Signal %1 removed from overriden signals.").arg(osp.appSignalId()));
				continue;
			}

			OverrideSignalParam& updateOsp = newSignals.emplace_back(osp);
			updateOsp.updateSignalProperties(*sp, osp.value());
		}

		// Set updated signals
		//
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter ++;

			m_signals.clear();

			for (const OverrideSignalParam& osp : newSignals)
			{
				m_signals.emplace(osp.appSignalId(), osp);
			}
		}

		emit signalsChanged({});

		return;
	}

	bool OverrideSignals::runOverrideScripts(const QString& lmEquipmentId, qint64 workcycle)
	{
		QStringList appSignalIds;

		{
			QWriteLocker locker(&m_lock);

			for (auto& [appSignalId, osp] : m_signals)
			{
				if (osp.method() != Sim::OverrideSignalMethod::Script ||
					osp.lmEquipmentId() != lmEquipmentId)
				{
					continue;
				}

				bool expected = true;
				if (osp.m_scriptValueRequiresReset.compare_exchange_strong(expected, false) == true ||
				    osp.m_scriptValue == nullptr ||
				    osp.m_scriptEngine == nullptr)
				{
					osp.m_scriptValue = std::make_unique<QJSValue>();
					osp.m_scriptEngine = std::make_unique<QJSEngine>();
					osp.m_scriptEngine->installExtensions(QJSEngine::ConsoleExtension);

					*osp.m_scriptValue = osp.m_scriptEngine->evaluate(osp.script());

					if (osp.m_scriptValue->isError() == true)
					{
						QString errorMessage = tr("Override script evaluate error, signal %1, line %2, message %3")
											   .arg(appSignalId)
						                       .arg(osp.m_scriptValue->property("lineNumber").toInt())
						                       .arg(osp.m_scriptValue->toString());

						writeError(errorMessage);

						qDebug() << "Script evaluate error at line " << osp.m_scriptValue->property("lineNumber").toInt();
						qDebug() << "\tSignal: " << appSignalId;
						qDebug() << "\tClass: " << metaObject()->className();
						qDebug() << "\tStack: " << osp.m_scriptValue->property("stack").toString();
						qDebug() << "\tMessage: " << osp.m_scriptValue->toString();

						continue;
					}

					if (osp.m_scriptValue->isUndefined() == true)
					{
						continue;
					}
				}

				// Arguments: function(lastOverrideValue, workcycle)
				//		lastOverrideValue - The last value returned from this function
				//		workcycle - Workcycle counter
				//
				QJSValueList args;

				args << QJSValue{osp.value().toDouble()};
				args << QJSValue{static_cast<uint>(workcycle)};

				QJSValue result = osp.m_scriptValue->call(args);

				if (result.isError() == true)
				{
					osp.setScriptError(tr("Override script uncaught exception, signal %1, line %2")
										.arg(appSignalId)
										.arg(result.property("lineNumber").toInt()));

					//writeWaning(osp.scriptError());

					qDebug() << "Script running uncaught exception at line " << result.property("lineNumber").toInt();
					qDebug() << "\tAppSignalID: " << appSignalId;
					qDebug() << "\tStack: " << result.property("stack").toString();
					qDebug() << "\tMessage: " << result.toString();

					continue;
				}

				if (result.isNumber() == false)
				{
					osp.setScriptError(tr("Override script returned not floating point value, signal %1.")
									   .arg(appSignalId));

					//writeWaning(osp.scriptError());
					continue;
				}

				// Set new value to signal
				//
				osp.setScriptError({});
				double ov = result.toNumber();

				if (ov != osp.value().toDouble())
				{
					osp.setValue(ov, OverrideSignalMethod::Value, false);

					appSignalIds << appSignalId;
				}
			}

			if (appSignalIds.isEmpty() == false)
			{
				m_changesCounter ++;
			}
		}


		if (appSignalIds.isEmpty() == false)
		{
			emit stateChanged(appSignalIds);
		}

		return true;
	}

	void OverrideSignals::requestToResetOverrideScripts(const QString& lmEquipmentId)
	{
		QWriteLocker locker(&m_lock);

		for (auto& [appSignalId, osp] : m_signals)
		{
			if (osp.lmEquipmentId() == lmEquipmentId)
			{
				osp.m_scriptValueRequiresReset = true;
			}
		}
	}

	Sim::AppSignalManager& OverrideSignals::appSignalManager()
	{
		return m_simulator->appSignalManager();
	}

	const Sim::AppSignalManager& OverrideSignals::appSignalManager() const
	{
		return m_simulator->appSignalManager();
	}

	std::optional<OverrideSignalParam> OverrideSignals::overrideSignal(QString appSignalId) const
	{
		std::optional<OverrideSignalParam> result;

		QReadLocker rl(&m_lock);

		auto it = m_signals.find(appSignalId);
		if (it != m_signals.end())
		{
			result = it->second;
		}

		return result;
	}

	std::vector<OverrideSignalParam> OverrideSignals::overrideSignals() const
	{
		std::vector<OverrideSignalParam> result;

		QReadLocker rl(&m_lock);

		result.reserve(m_signals.size());

		for (auto[appSignalId, ovSignalParam] :  m_signals)
		{
			result.push_back(ovSignalParam);
		}

		return result;
	}

	int OverrideSignals::changesCounter() const
	{
		QReadLocker rl(&m_lock);
		return m_changesCounter;
	}

	std::vector<OverrideRamRecord> OverrideSignals::ramOverrideData(const QString& lmEquipmentId, const RamAreaInfo& ramAreaInfo) const
	{
		std::vector<OverrideRamRecord> result;
		E::LogicModuleRamAccess ramAccess = ramAreaInfo.access();

		// Allocate data by size of RamArea
		//
		if (ramAreaInfo.size() > 0x10000)
		{
			writeError(tr("RamArea (offset %1) in LogicModule %2 seems too big (%3)")
						.arg(ramAreaInfo.offset())
						.arg(lmEquipmentId)
						.arg(ramAreaInfo.size()));
			return result;
		}

		// --
		//
		QReadLocker locker(&m_lock);

		for (const auto&[appSignalId, osp] : m_signals)
		{
			if (osp.enabled() == false ||				// Signal is not enabled to override
				(static_cast<int>(osp.ramAccess()) & static_cast<int>(ramAccess)) == 0 ||			// Signal is not in this RAM Area
				osp.lmEquipmentId() != lmEquipmentId)		// Signal is not in this LM
			{
				continue;
			}

			int dataSizeW = osp.dataSizeW();
			int offsetW = osp.address().offset();

			if (offsetW < static_cast<int>(ramAreaInfo.offset()) ||
				offsetW >= static_cast<int>(ramAreaInfo.offset() + ramAreaInfo.size()))
			{
				// Signal is not in this RamArea
				// dataSizeW is not taken into checks, as we suppose that signal can be in only area
				//
				continue;
			}

			if (result.empty() == true)
			{
				result.resize(ramAreaInfo.size());
			}

			offsetW -= ramAreaInfo.offset();	// Make it 0-based

			if (offsetW < 0 || offsetW + dataSizeW > result.size())
			{
				assert(false);
				return result;
			}

			for (int i = 0; i < dataSizeW; i++)
			{
				result[offsetW].overlapRecord(osp.ramOverrides(i));
				offsetW++;
			}
		}

		return result;
	}

}
