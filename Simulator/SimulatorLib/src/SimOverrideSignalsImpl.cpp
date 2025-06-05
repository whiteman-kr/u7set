#include "SimOverrideSignalsImpl.h"
#include "SimulatorPrivate.h"

#include <ProtoCommonHelper.h>
#include <Simulator.pb.h>

#include <SimulatorLib/SimRam.h>

namespace Sim
{
	//
	// OverrideSignalsImpl
	//
	OverrideSignalsImpl::OverrideSignalsImpl(SimulatorPrivate* simulator, QObject* parent /*= nullptr*/) :
		QObject(parent),
		m_simulator(simulator),
		m_log(simulator->log(), "OverrideSignals")
	{
		assert(simulator);
		return;
	}

	void OverrideSignalsImpl::clear()
	{
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter++;

			m_signals.clear();
		}

		emit signalsChanged({});
		return;
	}

	int OverrideSignalsImpl::addSignals(const QStringList& appSignalIds)
	{
		if (m_simulator == nullptr)
		{
			assert(m_simulator);
			return 0;
		}

		QStringList addedSignals;

		for (const QString& id : appSignalIds)
		{
			std::optional<AppSignal> sp = appSignalManager().signalParamExt(id);

			if (sp.has_value() == false)
			{
				m_log.writeWarning(QString("Cannot add signal to override list, signal %1 not found.").arg(id));
				continue;
			}

			{
				QWriteLocker locker(&m_lock);
				m_changesCounter++;

				auto [it, ok] = m_signals.emplace(id, *sp);
				if (ok == false)
				{
					m_log.writeWarning(QString("Signal %1 already added to override list.").arg(id));
					continue;
				}
				else
				{
					addedSignals << sp->appSignalID();

					// Set index for new signal
					//
					int maxIndex = m_signals.begin()->second.index();

					for (const auto& [sid, s] : m_signals)
					{
						Q_UNUSED(sid);
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

		return static_cast<int>(addedSignals.size());
	}

	bool OverrideSignalsImpl::addSignal(QString appSignalId,
										bool enabled,
										int index,
										OverrideSignalMethod method,
										QVariant value,
										QString script)
	{
		std::optional<AppSignal> sp = appSignalManager().signalParamExt(appSignalId);

		if (sp.has_value() == false)
		{
			m_log.writeWarning(QString("Cannot add signal to override list, signal %1 not found.").arg(appSignalId));
			return false;
		}

		{
			QWriteLocker locker(&m_lock);
			m_changesCounter++;

			auto [it, ok] = m_signals.emplace(appSignalId, *sp);
			if (ok == false)
			{
				m_log.writeWarning(QString("Signal %1 already added to override list.").arg(appSignalId));
				return false;
			}
			else
			{
				OverrideSignalParam& osp = it->second;

				osp.setEnabled(enabled);
				osp.setIndex(index);

				switch (method)
				{
				case OverrideSignalMethod::Value:
					osp.setValue(value, method, true);
					break;
				case OverrideSignalMethod::Script:
					osp.setValue(script, method, true);
					break;
				default:
					assert(false);
				}
			}
		}

		emit signalsChanged(QStringList{} << appSignalId);
		return true;
	}

	void OverrideSignalsImpl::removeSignal(const QString& appSignalId)
	{
		return removeSignals({appSignalId});
	}

	void OverrideSignalsImpl::removeSignals(const QStringList& appSignalIds)
	{
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter++;

			std::erase_if(m_signals,
						  [&appSignalIds](const auto& item)
						  {
							  auto const& [key, value] = item;
							  return appSignalIds.contains(key) == true;
						  });
		}

		emit signalsChanged({});
		return;
	}

	bool OverrideSignalsImpl::containsSignal(const QString& appSignalId) const
	{
		QReadLocker locker(&m_lock);
		return m_signals.find(appSignalId) != m_signals.end();
	}

	void OverrideSignalsImpl::setEnable(QString appSignalId, bool enable)
	{
		bool changed = false;

		{
			QWriteLocker locker(&m_lock);
			m_changesCounter++;

			if (auto it = m_signals.find(appSignalId); it != m_signals.end() && it->second.enabled() != enable)
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

	void OverrideSignalsImpl::setValue(QString appSignalId, OverrideSignalMethod method, const QVariant& value)
	{
		std::vector<OverrideSetValueData> v;
		v.emplace_back(appSignalId, method, value);

		return setValues(v);
	}

	void OverrideSignalsImpl::setValues(const std::vector<OverrideSetValueData>& overrideData)
	{
		QStringList appSignalIds;
		appSignalIds.reserve(overrideData.size());

		{
			QWriteLocker locker(&m_lock);
			m_changesCounter++;

			for (const auto& d : overrideData)
			{
				auto it = m_signals.find(d.appSignalId);
				if (it == m_signals.end())
				{
					m_log.writeError(tr("Can't set new value for %1, signal not found").arg(d.appSignalId));
					continue;
				}

				OverrideSignalParam& osp = it->second;
				osp.setValue(d.value, d.method, true);

				appSignalIds.push_back(d.appSignalId);
			}
		}

		emit stateChanged(appSignalIds);
		return;
	}

	void OverrideSignalsImpl::updateSignals()
	{
		std::vector<OverrideSignalParam> existingSignals = overrideSignals();

		std::vector<OverrideSignalParam> newSignals;
		newSignals.reserve(existingSignals.size());

		for (const OverrideSignalParam& osp : existingSignals)
		{
			std::optional<AppSignal> sp = appSignalManager().signalParamExt(osp.appSignalId());

			if (sp.has_value() == false)
			{
				m_log.writeWarning(tr("Signal %1 removed from overriden signals.").arg(osp.appSignalId()));
				continue;
			}

			OverrideSignalParam& updateOsp = newSignals.emplace_back(osp);
			updateOsp.updateSignalProperties(*sp, osp.value());
		}

		// Set updated signals
		//
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter++;

			m_signals.clear();

			for (const OverrideSignalParam& osp : newSignals)
			{
				m_signals.emplace(osp.appSignalId(), osp);
			}
		}

		emit signalsChanged({});

		return;
	}

	bool OverrideSignalsImpl::runOverrideScripts(const QString& lmEquipmentId, qint64 workcycle)
	{
		QStringList appSignalIds;

		{
			QWriteLocker locker(&m_lock);

			for (auto& [appSignalId, osp] : m_signals)
			{
				if (osp.method() != Sim::OverrideSignalMethod::Script || osp.lmEquipmentId() != lmEquipmentId)
				{
					continue;
				}

				bool expected = true;
				if (osp.m_scriptValueRequiresReset.compare_exchange_strong(expected, false) == true || osp.m_scriptValue == nullptr ||
					osp.m_scriptEngine == nullptr)
				{
					osp.m_scriptValue = std::make_unique<QJSValue>();
					osp.m_scriptEngine = std::make_unique<QJSEngine>();
					osp.m_scriptEngine->installExtensions(QJSEngine::ConsoleExtension);

					// Create global variable "signals"
					//
					QJSValue appSignalManager = osp.m_scriptEngine->newQObject(
						new ScriptAppSignalManager{&m_simulator->appSignalManager(), osp.m_scriptEngine.get()});
					osp.m_scriptEngine->globalObject().setProperty("signals", appSignalManager);

					// Evaluate override script
					//
					*osp.m_scriptValue = osp.m_scriptEngine->evaluate(osp.script());

					if (osp.m_scriptValue->isError() == true)
					{
						QString errorMessage = tr("Override script evaluate error, signal %1, line %2, message %3")
												   .arg(appSignalId)
												   .arg(osp.m_scriptValue->property("lineNumber").toInt())
												   .arg(osp.m_scriptValue->toString());

						m_log.writeError(errorMessage);

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

					// writeWaning(osp.scriptError());

					qDebug() << "Script running uncaught exception at line " << result.property("lineNumber").toInt();
					qDebug() << "\tAppSignalID: " << appSignalId;
					qDebug() << "\tStack: " << result.property("stack").toString();
					qDebug() << "\tMessage: " << result.toString();

					continue;
				}

				if (result.isNumber() == false)
				{
					osp.setScriptError(tr("Override script returned not floating point value, signal %1.").arg(appSignalId));

					// writeWaning(osp.scriptError());
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
				m_changesCounter++;
			}
		}


		if (appSignalIds.isEmpty() == false)
		{
			emit stateChanged(appSignalIds);
		}

		return true;
	}

	void OverrideSignalsImpl::requestToResetOverrideScripts(const QString& lmEquipmentId)
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

	bool OverrideSignalsImpl::saveWorkspace(QString fileName) const
	{
		std::fstream output(std::filesystem::path(fileName.toStdWString()), std::ios::out | std::ios::binary);
		if (output.is_open() == false || output.bad() == true)
		{
			return false;
		}

		std::vector<OverrideSignalParam> osignals = overrideSignals();

		::Proto::SimOverrideSignalWorkspace message;

		for (const OverrideSignalParam& osp : osignals)
		{
			::Proto::SimOverrideSignal* signalMessage = message.add_overridesignals();

			signalMessage->set_enabled(osp.enabled());
			signalMessage->set_index(osp.index());

			signalMessage->set_appsignalid(osp.appSignalId().toStdString());
			signalMessage->set_overridemethod(static_cast<qint32>(osp.method()));

			::Proto::Write(signalMessage->mutable_overridevalue(), osp.value());
			signalMessage->set_overridescript(osp.script().toStdString());
		}

		bool ok = message.SerializeToOstream(&output);
		return ok;
	}

	bool OverrideSignalsImpl::loadWorkspace(QString fileName)
	{
		clear();

		std::fstream input(std::filesystem::path(fileName.toStdWString()), std::ios::in | std::ios::binary);
		if (input.is_open() == false || input.bad() == true)
		{
			return false;
		}

		::Proto::SimOverrideSignalWorkspace message;

		bool result = ParseFromIstream(message, input);
		if (result == false)
		{
			return false;
		}

		for (int i = 0; i < message.overridesignals_size(); i++)
		{
			auto ospm = message.overridesignals(i);

			addSignal(QString::fromStdString(ospm.appsignalid()),
					  ospm.enabled(),
					  ospm.index(),
					  static_cast<OverrideSignalMethod>(ospm.overridemethod()),
					  Proto::Read(ospm.overridevalue()),
					  QString::fromStdString(ospm.overridescript()));
		}

		return true;
	}

	void OverrideSignalsImpl::updateRamOverrideData(const QString& lmEquipmentId, Ram& ram) const
	{
		if (int cs = changesCounter(); ram.overrideSignalsLastCounter(cs) == cs)
		{
			// Data has not been changed since the last update.
			//
			return;
		}

		for (std::vector<RamArea*> memoryAreas = ram.memoryAreas(); RamArea * ramArea : memoryAreas)
		{
			Q_ASSERT(ramArea);

			std::vector<OverrideRamRecord> ovData = ramOverrideData(lmEquipmentId, *ramArea);
			ramArea->setOverrideData(std::move(ovData));
		}

		return;
	}

	Sim::AppSignalManagerImpl& OverrideSignalsImpl::appSignalManager()
	{
		return m_simulator->appSignalManager();
	}

	const Sim::AppSignalManagerImpl& OverrideSignalsImpl::appSignalManager() const
	{
		return m_simulator->appSignalManager();
	}

	std::optional<OverrideSignalParam> OverrideSignalsImpl::overrideSignal(QString appSignalId) const
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

	std::vector<OverrideSignalParam> OverrideSignalsImpl::overrideSignals() const
	{
		std::vector<OverrideSignalParam> result;

		QReadLocker rl(&m_lock);

		result.reserve(m_signals.size());

		for (auto [appSignalId, ovSignalParam] : m_signals)
		{
			result.push_back(ovSignalParam);
		}

		return result;
	}

	QStringList OverrideSignalsImpl::overrideSignalIds() const
	{
		QStringList result;

		QReadLocker rl(&m_lock);

		result.reserve(static_cast<int>(m_signals.size()));

		for (auto [appSignalId, ovSignalParam] : m_signals)
		{
			Q_UNUSED(ovSignalParam);
			result.push_back(appSignalId);
		}

		return result;
	}

	int OverrideSignalsImpl::changesCounter() const
	{
		QReadLocker rl(&m_lock);
		return m_changesCounter;
	}

	std::vector<OverrideRamRecord> OverrideSignalsImpl::ramOverrideData(const QString& lmEquipmentId, const RamAreaInfo& ramAreaInfo) const
	{
		std::vector<OverrideRamRecord> result;
		E::LogicModuleRamAccess ramAccess = ramAreaInfo.access();

		// Allocate data by size of RamArea
		//
		if (ramAreaInfo.size() > 0x10000)
		{
			m_log.writeError(tr("RamArea (offset %1) in LogicModule %2 seems too big (%3)")
								 .arg(ramAreaInfo.offset())
								 .arg(lmEquipmentId)
								 .arg(ramAreaInfo.size()));
			return result;
		}

		// --
		//
		QReadLocker locker(&m_lock);

		for (const auto& [appSignalId, osp] : m_signals)
		{
			if (osp.enabled() == false ||                                                 // Signal is not enabled to override
				(static_cast<int>(osp.ramAccess()) & static_cast<int>(ramAccess)) == 0 || // Signal is not in this RAM Area
				osp.lmEquipmentId() != lmEquipmentId)                                     // Signal is not in this LM
			{
				continue;
			}

			int dataSizeW = osp.dataSizeW();
			int offsetW = osp.address().offset();

			if (offsetW < static_cast<int>(ramAreaInfo.offset()) || offsetW >= static_cast<int>(ramAreaInfo.offset() + ramAreaInfo.size()))
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

			offsetW -= ramAreaInfo.offset(); // Make it 0-based

			if (offsetW < 0 || offsetW + dataSizeW > std::ssize(result))
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

} // namespace Sim
