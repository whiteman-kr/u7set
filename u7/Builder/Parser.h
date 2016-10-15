#ifndef APPLOGICBUILDER_H
#define APPLOGICBUILDER_H

#include "../../VFrame30/SchemaItem.h"
#include "../../VFrame30/Afb.h"
#include "../../VFrame30/FblItemRect.h"
#include "../../VFrame30/UfbSchema.h"


// Forware delcarations
//
class QThread;
class DbController;

class SignalSet;

namespace Hardware
{
	class DeviceObject;
	class DeviceModule;
	class McFirmwareOld;
	class EquipmentSet;
}

namespace VFrame30
{
	class LogicSchema;
	class SchemaLayer;
	class SchemaItemAfb;
}

namespace Builder
{
	class IssueLogger;

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
		std::shared_ptr<VFrame30::LogicSchema> m_schema;
		Afb::AfbElement m_afbElement;							// Specific instance with initialized Params

		// Methods
		//
		AppLogicItem() = default;
		AppLogicItem(const AppLogicItem&) = default;
		AppLogicItem(std::shared_ptr<VFrame30::FblItemRect> fblItem,
				  std::shared_ptr<VFrame30::LogicSchema> schema);


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
	class AppLogicModule : QObject
	{
		Q_OBJECT

	public:
		AppLogicModule() = delete;
		AppLogicModule(QString moduleEquipmentId);

		bool addBranch(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
					   const BushContainer& bushes,
					   IssueLogger* log);

		bool orderItems(IssueLogger* log);

	private:
		bool setItemsOrder(IssueLogger* log,
						   std::map<QUuid, AppLogicItem>& remainItems,
						   std::list<AppLogicItem>& orderedItems,
						   const std::map<QUuid, AppLogicItem>& constItems);

		// Set connection between SchemaItemInput/SchemaItemOutput by StrIds
		//
		bool setInputOutputsElementsConnection(IssueLogger* log);

		template<typename Iter>
		std::vector<AppLogicItem> getItemsWithInput(
			const Iter& begin,
			const Iter& end,
			const QUuid& inputGuid);

		bool multichannelProcessing(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
									BushContainer* busheContainer,
									IssueLogger* log);

	public:
		QString moduleEquipmentId() const;
		void setModuleEquipmentId(QString value);

		const std::list<AppLogicItem>& items() const;
		std::list<AppLogicItem>& items();

	private:
		QString m_moduleEquipmentId;
		std::list<AppLogicItem> m_items;		// Ordered items
		std::map<QUuid, AppLogicItem> m_fblItemsAcc;	// Temporary buffer, filled in addBranch, cleared in orderItems
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicData
	//
	// ------------------------------------------------------------------------
	class AppLogicData
	{
	public:
		AppLogicData();

		std::shared_ptr<AppLogicModule> getModuleLogicData(QString moduleStrID);

		// Public methods
		//
	public:
		bool addData(QString equipmentId,
					 const BushContainer& bushContainer,
					 std::shared_ptr<VFrame30::LogicSchema> schema,
					 IssueLogger* log);

		bool orderItems(IssueLogger* log);

		// Properties
		//
	public:
		const std::list<std::shared_ptr<AppLogicModule>>& modules() const;
		std::list<std::shared_ptr<AppLogicModule>>& modules();

	private:
		std::list<std::shared_ptr<AppLogicModule>> m_modules;
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------
	class Parser : QObject
	{
		Q_OBJECT

	public:
		Parser() = delete;
		Parser(DbController* db,
			   IssueLogger* log,
			   AppLogicData* appLogicData,
			   Afb::AfbElementCollection* afbCollection,
			   Hardware::EquipmentSet* equipmentSet,
			   SignalSet* signalSet,
			   int changesetId,
			   bool debug);

		virtual ~Parser();

		bool build();

	protected:
		bool loadAppLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicSchema>>* out);

		bool loadUfbSchemas();

		bool checkEquipmentIds(VFrame30::LogicSchema* logicSchema);

		bool checkAfbItemsVersion(VFrame30::LogicSchema* logicSchema);
		bool checkUfbItemsVersion(VFrame30::LogicSchema* logicSchema);

		bool parseAppLogicSchema(std::shared_ptr<VFrame30::LogicSchema> logicSchema);

		bool parseAppLogicLayer(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
								std::shared_ptr<VFrame30::SchemaLayer> layer);

		bool multichannelProcessing(std::shared_ptr<VFrame30::LogicSchema> schema,
									std::shared_ptr<VFrame30::SchemaLayer> layer,
									QString equipmentId);

		bool filterSingleChannelBranchesInMulischema(std::shared_ptr<VFrame30::LogicSchema> schema, QString equipmnetId, BushContainer* bushContainer);

		bool findBushes(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
						std::shared_ptr<VFrame30::SchemaLayer> layer,
						BushContainer* bushContainer) const;

		bool setBranchConnectionToPin(std::shared_ptr<VFrame30::LogicSchema> schema,
									  std::shared_ptr<VFrame30::SchemaLayer> layer,
									  BushContainer* bushContainer) const;

		bool setPinConnections(std::shared_ptr<VFrame30::LogicSchema> schema,
							   std::shared_ptr<VFrame30::SchemaLayer> layer,
							   BushContainer* bushContainer);

		void debugInfo();

	private:
		DbController* db();
		IssueLogger* log() const;
		int changesetId() const;

		bool debug() const;
		bool release() const;

		const AppLogicData* applicationData() const;
		AppLogicData* applicationData();

	private:
		DbController* m_db = nullptr;
		mutable IssueLogger* m_log = nullptr;
		int m_changesetId = 0;
		int m_debug = false;

		AppLogicData* m_applicationData = nullptr;
		Afb::AfbElementCollection* m_afbCollection = nullptr;
		std::map<QString, std::shared_ptr<VFrame30::UfbSchema>> m_ufbs;
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		SignalSet* m_signalSet = nullptr;
	};

}

#endif // APPLOGICBUILDER_H
