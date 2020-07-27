#include "../lib/WUtils.h"
#include "UalItems.h"
#include "ModuleLogicCompiler.h"
#include "IssueLogger.h"

#include "Loopbacks.h"

namespace Builder
{
	// ---------------------------------------------------------------------------------
	//
	//	Loopback class implementation
	//
	// ---------------------------------------------------------------------------------

	Loopback::Loopback(const UalItem* sourceItem) :
		m_sourceItem(sourceItem)
	{
		TEST_PTR_RETURN(m_sourceItem);

		const UalLoopbackSource* src = m_sourceItem->ualLoopbackSource();

		TEST_PTR_RETURN(src);

		m_loopbackID = src->loopbackId();
	}

	bool Loopback::isConnected(const VFrame30::AfbPin& pin) const
	{
		return m_linkedPins.count(pin.guid()) > 0;
	}

	bool Loopback::isConnected(const QString& signalID) const
	{
		return m_linkedSignals.count(signalID) > 0;
	}

	void Loopback::addLoopbackTarget(const UalItem* targetItem)
	{
		TEST_PTR_RETURN(targetItem);

		assert(targetItem->isLoopbackTarget());

		m_targetItems.push_back(targetItem);
	}

	void Loopback::setUalSignal(UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			assert(false);
			return;
		}

		if (m_ualSignal == nullptr)
		{
			m_ualSignal = ualSignal;
		}
		else
		{
			assert(m_ualSignal == ualSignal);
		}
	}

	QStringList Loopback::linkedSignalsIDs() const
	{
		QStringList list;

		for(const QString& signalID : m_linkedSignals)
		{
			list.append(signalID);
		}

		return list;
	}

	QString Loopback::anyLinkedSignalID() const
	{
		if (m_linkedSignals.empty() == true)
		{
			return QString();
		}

		return *m_linkedSignals.cbegin();
	}

	bool Loopback::getReportStr(QString* reportStr) const
	{
		TEST_PTR_RETURN_FALSE(reportStr);

		if (m_sourceItem == nullptr ||
			m_ualSignal == nullptr)
		{
			assert(false);
			return false;
		}

		*reportStr = QString("%1;%2;%3;%4").
				arg(m_loopbackID).
				arg(m_sourceItem->label()).
				arg(E::valueToString<E::SignalType>(m_ualSignal->signalType())).
				arg(m_ualSignal->refSignalIDsJoined(";"));

		return true;
	}

	// ---------------------------------------------------------------------------------
	//
	//	Loopbacks class implementation
	//
	// ---------------------------------------------------------------------------------

	Loopbacks::Loopbacks(ModuleLogicCompiler& compiler) :
		m_compiler(compiler)
	{
	}

	LoopbackShared Loopbacks::getLoopback(const QString& sourceID) const
	{
		LoopbackShared loopback = m_loopbacks.value(sourceID, LoopbackShared());

		assert(loopback != nullptr);

		return loopback;
	}

	bool Loopbacks::addLoopbackSource(const UalItem* sourceItem)
	{
		TEST_PTR_RETURN_FALSE(sourceItem);

		const UalLoopbackSource* src = sourceItem->ualLoopbackSource();

		TEST_PTR_RETURN_FALSE(src);

		if (m_loopbacks.contains(src->loopbackId()) == true)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(log());
			return false;
		}

		m_loopbacks.insert(src->loopbackId(),std::make_shared<Loopback>(sourceItem));

		return true;
	}

	bool Loopbacks::isSourceExists(const QString& lbSourceID) const
	{
		return m_loopbacks.contains(lbSourceID);
	}

	bool Loopbacks::addLoopbackTarget(const QString& loopbackID, const UalItem* ualItem)
	{
		TEST_PTR_RETURN_FALSE(ualItem);

		LoopbackShared loopback = m_loopbacks.value(loopbackID, LoopbackShared());

		if (loopback == nullptr)
		{
			Q_ASSERT(loopback != nullptr);
			return false;
		}

		loopback->addLoopbackTarget(ualItem);

		return true;
	}

	bool Loopbacks::setUalSignalForLoopback(const UalItem* loopbackSourceItem, UalSignal* ualSignal)
	{
		TEST_PTR_RETURN_FALSE(ualSignal);

		const UalLoopbackSource* source = loopbackSourceItem->ualLoopbackSource();

		if  (source == nullptr)
		{
			LOG_INTERNAL_ERROR(m_compiler.log());
			return false;
		}

		QString loopbackID = source->loopbackId();

		LoopbackShared loopback = m_loopbacks.value(loopbackID, LoopbackShared());

		if (loopback == nullptr)
		{
			assert(false);				// source already should be exists in m_sources
			return false;
		}

		if (loopback->ualSignal() == nullptr)
		{
			loopback->setUalSignal(ualSignal);
			ualSignal->setLoopback(loopback);
		}
		else
		{
			if (loopback->ualSignal() != ualSignal)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_compiler.log());
			}
		}

		m_ualSignalsToLoopbacks.insert(ualSignal, loopback);

		return true;
	}

	UalSignal* Loopbacks::getLoopbackUalSignal(const QString& loopbackID)
	{
		LoopbackShared loopback = getLoopback(loopbackID);

		if (loopback == nullptr)
		{
			assert(false);
			return nullptr;
		}

		return loopback->ualSignal();
	}

	QStringList Loopbacks::getLoopbackLinkedSignals(const QString& loopbackID) const
	{
		LoopbackShared loopback = m_loopbacks.value(loopbackID, nullptr);

		if (loopback == nullptr)
		{
			assert(false);
			return QStringList();
		}

		return loopback->linkedSignalsIDs();
	}

	bool Loopbacks::findSignalsAndPinsLinkedToLoopbacksTargets()
	{
		bool result = true;

		IssueLogger* log = m_compiler.log();

		for(LoopbackShared loopback : m_loopbacks)
		{
			TEST_PTR_CONTINUE(loopback);

			std::set<QString> linkedSignals;
			std::set<const UalItem*> linkedItems;
			std::map<QUuid, const UalItem*> linkedPins;

			bool res = true;

			for(const UalItem* target : loopback->targets())
			{
				TEST_PTR_CONTINUE(target);

				res &= m_compiler.getSignalsAndPinsLinkedToItem(target,
																&linkedSignals,
																&linkedItems,
																&linkedPins);
			}

			if (res == false)
			{
				result = false;
				continue;
			}

			QString firstSignalID;
			Signal* firstSignal = nullptr;
			const UalItem* firstSignalItem = nullptr;

			for(const UalItem* linkedItem : linkedItems)
			{
				if (linkedItem->isSignal() == false)
				{
					continue;
				}

				QString signalID = linkedItem->strID();
				Signal* s = m_compiler.getSignal(signalID);

				if (s == nullptr)
				{
					// this error should be detected early
					//
					LOG_INTERNAL_ERROR(log);
					continue;
				}

				LoopbackShared presentLoopback = m_signalsToLoopbacks.value(signalID, nullptr);

				if (presentLoopback == nullptr)
				{
					m_signalsToLoopbacks.insert(signalID, loopback);
				}
				else
				{
					if (loopback != presentLoopback)
					{
						// Signal %1 is connected to different Loopbacks %2 and %3.
						//
						log->errALC5147(signalID, presentLoopback->loopbackID(), loopback->loopbackID());
						res = false;
					}
				}

				if (firstSignal == nullptr)
				{
					firstSignalID = signalID;
					firstSignal = s;
					firstSignalItem = linkedItem;
					continue;
				}

				// signals compatibility checking
				//

				if (firstSignal->isCompatibleFormat(*s) == false)
				{
					// Non compatible signals %1 and %2 are connected to same Loopback %3 (Logic schema %4)
					//
					log->errALC5144(firstSignalID, firstSignalItem->guid(), signalID, linkedItem->guid(),
									loopback->loopbackID(), linkedItem->schemaID());
					res = false;
				}
			}

			if (res == false)
			{
				result = false;
				continue;
			}

			for(const std::pair<QUuid, const UalItem*>& pr : linkedPins)
			{
				QUuid linkedPin = pr.first;

				LoopbackShared presentLoopback = m_pinsToLoopbacks.value(linkedPin, nullptr);

				if (presentLoopback == nullptr)
				{
					m_pinsToLoopbacks.insert(linkedPin, loopback);
					continue;
				}

				// Pin is connected to different Loopbacks (different sources == assembly OR)
				// This error should be detected early, during parsing
				//
				LOG_INTERNAL_ERROR(log);
				res = false;
			}

			if (res == false)
			{
				result = false;
				continue;
			}

			loopback->setLinkedSignals(linkedSignals);
		}

		return result;
	}

	LoopbackShared Loopbacks::getLoopbackBySignal(const QString& signalID) const
	{
		return m_signalsToLoopbacks.value(signalID, LoopbackShared());
	}

	LoopbackShared Loopbacks::getLoopbackByPin(const VFrame30::AfbPin& pin) const
	{
		return m_pinsToLoopbacks.value(pin.guid(), LoopbackShared());
	}

	QList<LoopbackShared> Loopbacks::getLoopbacksByUalSignal(const UalSignal* ualSignal) const
	{
		return m_ualSignalsToLoopbacks.values(ualSignal);
	}

	bool Loopbacks::checkLoopbacksUalSignals()
	{
		bool result = true;

		for(LoopbackShared loopback : m_loopbacks)
		{
			TEST_PTR_CONTINUE(loopback);

			const Builder::UalItem* srcItem = loopback->sourceItem();

			TEST_PTR_CONTINUE(srcItem);

			if (loopback->ualSignal() == nullptr)
			{
				// Loopback source %1 is not connected to any signal source (Item %2, logic schema %3).
				//
				m_compiler.log()->errALC5180(loopback->loopbackID(), srcItem->label(), srcItem->guid(), srcItem->schemaID());

				result = false;

				continue;
			}

			if (loopback->ualSignal()->isOptoSignal() == true ||
				loopback->ualSignal()->isTunable() == true ||
				loopback->ualSignal()->isInput() == true)
			{
				// Loopback source cannot be connected to opto/tunable/input signal (Item %1, logic schema %2).
				//
				m_compiler.log()->errALC5181(srcItem->label(), srcItem->guid(), srcItem->schemaID());
			}
		}

		return result;
	}

	bool Loopbacks::writeReport(QStringList* file) const
	{
		TEST_PTR_RETURN_FALSE(file);

		QStringList loopbacksIDs = m_loopbacks.keys();

		loopbacksIDs.sort();

		for(const QString& loopbackID : loopbacksIDs)
		{
			LoopbackShared loopback = m_loopbacks.value(loopbackID, nullptr);

			TEST_PTR_CONTINUE(loopback);

			QString str;

			bool res = loopback->getReportStr(&str);

			if (res == true)
			{
				file->append(str);
			}
			else
			{
				assert(false);
			}
		}

		return true;
	}

	QString Loopbacks::getAutoLoopbackID(const UalItem* ualItem, const VFrame30::AfbPin& outputPin)
	{
		if (ualItem == nullptr)
		{
			assert(false);
			return QString();
		}

		return QString("AUTO_LOOPBACK_%1_%2").arg(ualItem->label()).arg(outputPin.caption().toUpper());
	}

	QString Loopbacks::joinedLoopbackIDs(const QList<LoopbackShared>& loopbacks)
	{
		QString ids;

		for(LoopbackShared lb : loopbacks)
		{
			if (ids.isEmpty() == false)
			{
				ids += ", ";
			}

			ids += lb->loopbackID();
		}

		return ids;
	}

	IssueLogger* Loopbacks::log() const
	{
		return m_compiler.log();
	}

}
