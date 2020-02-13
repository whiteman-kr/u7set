#include "../lib/WUtils.h"
#include "UalItems.h"
#include "ModuleLogicCompiler.h"

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
		return m_linkedPins.contains(pin.guid());
	}

	bool Loopback::isConnected(const QString& signalID) const
	{
		return m_linkedSignals.contains(signalID);
	}

	void Loopback::addLoopbackTarget(const UalItem* targetItem)
	{
		TEST_PTR_RETURN(targetItem);

		assert(targetItem->isLoopbackTarget());

		m_targetItems.append(targetItem);
	}

	void Loopback::setUalSignal(UalSignal* ualSignal)
	{
		assert(m_ualSignal == nullptr);

		m_ualSignal = ualSignal;
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

		assert(m_loopbacks.contains(src->loopbackId()) == false);

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

		loopback->setUalSignal(ualSignal);

		m_ualSignalsToLoopbacks.insertMulti(ualSignal, loopback);

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

	QStringList Loopbacks::getLoopbackConnectedSignals(const QString& loopbackID) const
	{
		LoopbackShared loopback = m_loopbacks.value(loopbackID, nullptr);

		if (loopback == nullptr)
		{
			assert(false);
			return QStringList();
		}

		return loopback->linkedSignals().uniqueKeys();
	}

	bool Loopbacks::findSignalsAndPinsLinkedToLoopbacksTargets()
	{
		bool result = true;

		IssueLogger* log = m_compiler.log();

		for(LoopbackShared loopback : m_loopbacks)
		{
			TEST_PTR_CONTINUE(loopback);

			QHash<QString, bool> linkedSignals;
			QHash<const UalItem*, bool> linkedItems;
			QHash<QUuid, const UalItem*> linkedPins;

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

			QList<const UalItem*> linkedItemsKeys = linkedItems.keys();

			for(const UalItem* linkedItem : linkedItemsKeys)
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

			QList<QUuid> loopbackLinkedPins = linkedPins.keys();

			for(const QUuid& linkedPin : loopbackLinkedPins)
			{
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

			for(const QString& signalID : linkedSignals.uniqueKeys())
			{
				assert(m_signalsToLoopbacks.contains(signalID) == false);

				m_signalsToLoopbacks.insert(signalID, loopback);
			}

//			loopback->setLinkedItems(linkedItems);

			loopback->setLinkedPins(linkedPins);

			for(const QUuid& pinGuid : linkedPins.uniqueKeys())
			{
				assert(m_pinsToLoopbacks.contains(pinGuid) == false);

				m_pinsToLoopbacks.insert(pinGuid, loopback);
			}
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
}
