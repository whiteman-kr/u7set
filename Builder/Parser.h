#pragma once

#include "../VFrame30/SchemaItemAfb.h"
#include "RunOrder.h"

class DbController;

namespace Hardware
{
	class EquipmentSet;
	class OptoModuleStorage;
}

namespace VFrame30
{
	class UfbSchema;
	class LogicSchema;
	class SchemaItemAfb;
	class BusSet;
}

namespace Builder
{
	class Context;
	class IssueLogger;
	class SignalSet;
	class LmDescriptionSet;

	struct Link
	{
		Link() = default;
		Link(const std::list<VFrame30::SchemaPoint>& points);

		VFrame30::SchemaPoint ptBegin() const;
		VFrame30::SchemaPoint ptEnd() const;

		static QUuid getNextId();

		bool isPinOnLink(VFrame30::SchemaPoint pt) const;

		std::list<VFrame30::SchemaPoint> m_points;
	};

	struct Bush
	{
		QUuid outputPin;						// Output pin for this branch, can be the only
		std::set<QUuid> inputPins;				// Input pins for this branch
		std::map<QUuid, Link> links;			// Links for this branch
		std::map<QUuid, std::shared_ptr<VFrame30::FblItemRect>> fblItems;

		VFrame30::FblItemRect* itemByPinGuid(QUuid pinId) const;
		VFrame30::FblItemRect* itemByGuid(QUuid uuid) const;
		VFrame30::AfbPin pinByGuid(QUuid pinId);

		std::vector<QUuid> getAllUuid() const;					// Used for IssueLogger
		std::vector<QUuid> getLinksUuids() const;				// Used for IssueLogger

		std::vector<VFrame30::AfbPin> getInputPinsForItem(QUuid fblItemUuid) const;

		bool hasCommonFbls(const Bush& bush) const;

		//bool hasInputOrOutput(const QUuid& uuid) const;
		//bool hasJoinedInOuts(Bush& bush) const;

		void debugInfo() const;
	};

	struct BushContainer
	{
		std::vector<Bush> bushes;

		int getBranchByPinPos(VFrame30::SchemaPoint pt) const;
		int getBranchByPinGuid(const QUuid& guid) const;

		void removeEmptyBushes();

		void debugInfo();
	};


	// ------------------------------------------------------------------------
	//
	//		AppLogicItem
	//
	// ------------------------------------------------------------------------

	struct AppLogicItem
	{
		// Data
		//
		std::shared_ptr<VFrame30::FblItemRect> m_fblItem;
		std::shared_ptr<VFrame30::Schema> m_schema;

		QUuid m_groupId;								// ShchemaItemUfb is expanded to the group of items, all these expanded items have the same m_groupId
														// This id is empty if item is not in group

		// Methods
		//
		AppLogicItem() = default;
		AppLogicItem(const AppLogicItem&) = default;
		AppLogicItem(std::shared_ptr<VFrame30::FblItemRect> fblItem,
					 std::shared_ptr<VFrame30::Schema> schema);

		const Afb::AfbElement& afbElement() const;
		Afb::AfbElement& afbElement();

		std::shared_ptr<Afb::AfbComponent> afbComponent();
		std::shared_ptr<Afb::AfbComponent> afbComponent() const;

		// Items can be kept in set, it is just comparing m_fblItem pointres
		//
		bool operator < (const AppLogicItem& li) const;
		bool operator == (const AppLogicItem& li) const;
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicModule
	//
	// ------------------------------------------------------------------------
	class AppLogicModule : public QObject
	{
		Q_OBJECT

	public:
		AppLogicModule() = delete;
		AppLogicModule(QString moduleId, QString lmDescriptionFile);

		bool addBranch(std::shared_ptr<VFrame30::Schema> schema,
					   const BushContainer& bushes,
					   IssueLogger* log);

		bool orderItems(IssueLogger* log, bool* interruptProcess);

		//	Make deep copy with new guid for everything, items, pins, assocs, etc...
		//
		std::shared_ptr<AppLogicModule> deepCopy(QUuid groupId, const QString& label) const;

		bool checkItemsRelationsConsistency(IssueLogger* log) const;

		static bool checkItemsRelationsConsistency(QString equipmentId,
														const std::list<AppLogicItem>& items,
														IssueLogger* log);

		bool removeInOutItemKeepAssoc(const QUuid& itemGuid);

		void dump() const;

	private:
		bool setItemsOrder(IssueLogger* log,
						   std::map<QUuid, AppLogicItem>& remainItems,
						   std::list<AppLogicItem>& orderedItems,
						   const std::map<QUuid, std::vector<AppLogicItem>>& itemsWithInputs,
						   bool startLoopFromLastItem,
						   bool* interruptProcess);

		// Set connection between SchemaItemInput/SchemaItemOutput by StrIds
		//
		bool setInputOutputsElementsConnection(IssueLogger* log);

		bool multichannelProcessing(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
									BushContainer* busheContainer,
									IssueLogger* log);

	public:
		QString equipmentId() const;
		QString lmDescriptionFile() const;

		const std::list<AppLogicItem>& items() const;
		std::list<AppLogicItem>& items();

	private:
		QString m_equipmentId;							// EuqipmentId or UFB SchemaID
		QString m_lmDescriptionFile;					// LogicModule description filename
		std::list<AppLogicItem> m_items;				// Ordered items
		std::map<QUuid, AppLogicItem> m_fblItemsAcc;	// Temporary buffer, filled in addBranch, cleared in orderItems

		//
		QHash<QString, bool> m_signaledItems;
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicData
	//
	// ------------------------------------------------------------------------
	class AppLogicData
	{
	public:
		explicit AppLogicData(SignalSet* signalSet);

		// Public methods
		//
	public:
		bool addLogicModuleData(QString equipmentId,
								const BushContainer& bushContainer,
								std::shared_ptr<VFrame30::LogicSchema> schema,
								IssueLogger* log);

		bool addUfbData(const BushContainer& bushContainer,
						std::shared_ptr<VFrame30::UfbSchema> schema,
						IssueLogger* log);

		bool orderLogicModuleItems(IssueLogger* log);
		bool orderUfbItems(IssueLogger* log);

		bool expandUfbs(IssueLogger* log);

		static bool bindTwoPins(VFrame30::AfbPin& outPin, VFrame30::AfbPin& inputPin);

		bool setAfbComponents(const LmDescriptionSet* lmDescriptionSet, IssueLogger* log);

		// Properties
		//
	public:
		const std::list<std::shared_ptr<AppLogicModule>>& modules() const;
		std::list<std::shared_ptr<AppLogicModule>>& modules();
		std::shared_ptr<AppLogicModule> module(QString moduleStrID);

		const std::map<QString, std::shared_ptr<AppLogicModule>>& ufbs() const;
		std::shared_ptr<AppLogicModule> ufb(QString ufbId) const;

	private:
		std::list<std::shared_ptr<AppLogicModule>> m_modules;
		std::map<QString, std::shared_ptr<AppLogicModule>> m_ufbs;

		SignalSet* m_signalSet = nullptr;
	};


	// Container to store schemas parse result, in multithreading
	// After parsing all schemas call setToAppData to set result to modules
	//
	class ReadyParseDataContainer
	{
	public:
		void add(QString equipmentId,
				 std::shared_ptr<BushContainer> bushContainer,
				 std::shared_ptr<VFrame30::LogicSchema> schema);

		void setToAppData(AppLogicData* appData, IssueLogger* log);

	private:
		struct AppData
		{
			QString equipmentId;
			std::shared_ptr<BushContainer> bushContainer;
			std::shared_ptr<VFrame30::LogicSchema> schema;
		};

		mutable QMutex m_mutex;
		std::list<AppData> m_appData;
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------
	class Parser : public QObject
	{
		Q_OBJECT

	public:
		Parser() = delete;
		Parser(Builder::Context* context);

		bool parse();

		const RunOrder& runOrder() const;

	protected:
		bool loadUfbFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::UfbSchema>>* out);
		bool loadAppLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicSchema>>* out);

		template<typename SchemaType>
		bool loadSchemaFiles(DbController* db, std::vector<std::shared_ptr<SchemaType>>* out, int parentFileId, QString filter);

		template<typename SchemaType>
		bool checkSameLabelsAndGuids(const std::vector<std::shared_ptr<SchemaType> >& schemas) const;

		bool checkSameInputsAndOutputs(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& schemas) const;
		bool checkParamsReferencesFormat(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& schemas) const;

		bool checkEquipmentIds(VFrame30::LogicSchema* logicSchema);

		bool checkLmDescription(VFrame30::LogicSchema* logicSchema);

		bool checkAfbItemsVersion(VFrame30::Schema* schema);
		bool checkBusItemsVersion(VFrame30::Schema* schema, const VFrame30::BusSet& busSet);
		bool checkUfbItemsVersion(VFrame30::LogicSchema* logicSchema,
								  const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbs);
		bool checkForUniqueLoopbackId(VFrame30::Schema* schema);
		bool checkForUniqueLoopbackId(std::shared_ptr<AppLogicModule> module);

		bool checkForResolvedReferences(std::shared_ptr<AppLogicModule> module);

		bool parsUfbSchema(std::shared_ptr<VFrame30::UfbSchema> ufbSchema);
		bool parseUfbLayer(std::shared_ptr<VFrame30::UfbSchema> ufbSchema,
						   std::shared_ptr<VFrame30::SchemaLayer> layer);

		bool parseAppLogicSchema(std::shared_ptr<VFrame30::LogicSchema> logicSchema, ReadyParseDataContainer* readyParseDataContainer, bool* interruptProcess);

		bool parseAppLogicLayer(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
								std::shared_ptr<VFrame30::SchemaLayer> layer, ReadyParseDataContainer* readyParseDataContainer);

		bool multichannelProcessing(std::shared_ptr<VFrame30::LogicSchema> schema,
									std::shared_ptr<VFrame30::SchemaLayer> layer,
									QString equipmentId);

		bool filterSingleChannelBranchesInMultiSchema(std::shared_ptr<VFrame30::LogicSchema> schema, QString equipmnetId, BushContainer* bushContainer);

		bool findBushes(std::shared_ptr<VFrame30::Schema> schema,
						std::shared_ptr<VFrame30::SchemaLayer> layer,
						BushContainer* bushContainer) const;

		bool setBranchConnectionToPin(std::shared_ptr<VFrame30::Schema> schema,
									  std::shared_ptr<VFrame30::SchemaLayer> layer,
									  BushContainer* bushContainer) const;

		bool setPinConnections(std::shared_ptr<VFrame30::Schema> schema,
							   std::shared_ptr<VFrame30::SchemaLayer> layer,
							   BushContainer* bushContainer);

		void setRunOrder();

	private:
		DbController* db();
		IssueLogger* log() const;
		int changesetId() const;

		const AppLogicData* applicationData() const;
		AppLogicData* applicationData();

	private:
		Builder::Context* m_context = nullptr;
		DbController* m_db = nullptr;
		mutable IssueLogger* m_log = nullptr;
		int m_changesetId = 0;

		std::shared_ptr<AppLogicData> m_applicationData;
		LmDescriptionSet* m_lmDescriptions = nullptr;
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		SignalSet* m_signalSet = nullptr;
		VFrame30::BusSet* m_busSet = nullptr;
		Hardware::OptoModuleStorage* m_opticModuleStorage = nullptr;

		RunOrder m_runOrder;
	};

}

