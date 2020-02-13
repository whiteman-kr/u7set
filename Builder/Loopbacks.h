#pragma once

#include <unordered_set>

namespace VFrame30
{
	class AfbPin;
}

namespace Builder
{
	class UalItem;
	class UalSignal;
	class ModuleLogicCompiler;

	class Loopback
	{
	public:
		Loopback(const Builder::UalItem* sourceItem);

		QString loopbackID() const { return m_loopbackID; }

		bool isConnected(const VFrame30::AfbPin& pin) const;
		bool isConnected(const QString& signalID) const;

		void addLoopbackTarget(const UalItem* targetItem);

		void setUalSignal(UalSignal* ualSignal);
		UalSignal* ualSignal() const { return m_ualSignal; }

		const QVector<const UalItem*> targets() const { return m_targetItems; }

		void setLinkedSignals(const QHash<QString, bool>& linkedSignals) { m_linkedSignals = linkedSignals; }
		const QHash<QString, bool>& linkedSignals() const { return m_linkedSignals; }

		QStringList linkedSignalsIDs() const { return m_linkedSignals.uniqueKeys(); }

//		void setLinkedItems(const QHash<const UalItem*, bool>& linkedItems) { m_linkedItems = linkedItems; }
//		const QHash<const UalItem*, bool>& linkedItems() const { return m_linkedItems; }

		void setLinkedPins(const QHash<QUuid, const UalItem*>& linkedPins) { m_linkedPins = linkedPins; }
		const QHash<QUuid, const UalItem*>& linkedPins() const { return m_linkedPins; }

	private:
		const UalItem* m_sourceItem = nullptr;
		QString m_loopbackID;

		// params will be filled during loopbackPreprocessing
		//
		UalSignal* m_ualSignal = nullptr;

		QVector<const UalItem*> m_targetItems;

		QHash<QString, bool> m_linkedSignals;
//		QHash<const UalItem*, bool> m_linkedItems;
		QHash<QUuid, const UalItem*> m_linkedPins;
	};

	typedef std::shared_ptr<Loopback> LoopbackShared;

	class Loopbacks
	{
	public:
		Loopbacks(ModuleLogicCompiler& compiler);

		LoopbackShared getLoopback(const QString& sourceID) const;

		bool addLoopbackSource(const UalItem *sourceItem);
		bool isSourceExists(const QString& lbSourceID) const;

		bool addLoopbackTarget(const QString& loopbackID, const UalItem* ualItem);

		bool setUalSignalForLoopback(const UalItem* loopbackSourceItem, UalSignal* ualSignal);
		UalSignal* getLoopbackUalSignal(const QString& loopbackID);

		QStringList getLoopbackConnectedSignals(const QString& loopbackID) const;

		QList<const UalSignal*> getLoopbacksUalSignals() const { return m_ualSignalsToLoopbacks.uniqueKeys(); }

		bool findSignalsAndPinsLinkedToLoopbacksTargets();

		LoopbackShared getLoopbackBySignal(const QString& signalID) const;
		LoopbackShared getLoopbackByPin(const VFrame30::AfbPin& pin) const;

		QList<LoopbackShared> getLoopbacksByUalSignal(const UalSignal* ualSignal) const;

		static QString getAutoLoopbackID(const UalItem* ualItem, const VFrame30::AfbPin& outputPin);
		static QString joinedLoopbackIDs(const QList<LoopbackShared>& loopbacks);

	private:

	private:
		ModuleLogicCompiler& m_compiler;

		QHash<QString, LoopbackShared> m_loopbacks;					// LoopbackID -> Loopback

		QHash<const UalSignal*, LoopbackShared> m_ualSignalsToLoopbacks;	// UalSignal -> Multi loopbacks

		QHash<QString, LoopbackShared> m_signalsToLoopbacks;		// SignalID -> Loopback
		QHash<QUuid, LoopbackShared> m_pinsToLoopbacks;				// Pin Guid -> Multi Loopbacks
	};
}
