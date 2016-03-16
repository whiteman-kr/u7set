#ifndef APPLOGICBUILDER_H
#define APPLOGICBUILDER_H

#include "../../VFrame30/SchemeItem.h"
#include "../../VFrame30/Afb.h"
#include "../../VFrame30/FblItemRect.h"


// Forware delcarations
//
class QThread;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class DeviceModule;
	class McFirmwareOld;
	class EquipmentSet;
}

namespace VFrame30
{
	class LogicScheme;
	class SchemeLayer;
	class SchemeItemAfb;
	class FblItemRect;
}

namespace Afb
{
	class AfbElement;
	class AfbElementCollection;
}

namespace Builder
{
	class IssueLogger;

	struct Link
	{
		Link() = default;
		Link(const std::list<VFrame30::SchemePoint>& points);

		VFrame30::SchemePoint ptBegin() const;
		VFrame30::SchemePoint ptEnd() const;

		bool isPinOnLink(VFrame30::SchemePoint pt) const;

		std::list<VFrame30::SchemePoint> m_points;
	};

	struct Bush
	{
		QUuid outputPin;						// Output pin for this branch, can be the only
		std::set<QUuid> inputPins;				// Input pins for this branch
		std::map<QUuid, Link> links;			// Links for this branch
		std::set<std::shared_ptr<VFrame30::FblItemRect>> fblItems;

		VFrame30::FblItemRect* itemByPinGuid(QUuid pinId);
		VFrame30::AfbPin pinByGuid(QUuid pinId);
	};

	struct BushContainer
	{
		std::vector<Bush> bushes;

		int getBranchByPinPos(VFrame30::SchemePoint pt) const;
		int getBranchByPinGuid(const QUuid& guid) const;

		void removeEmptyBushes();
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
		std::shared_ptr<VFrame30::LogicScheme> m_scheme;
		Afb::AfbElement m_afbElement;							// Specific instance with initialized Params

		// Methods
		//
		AppLogicItem() = default;
		AppLogicItem(const AppLogicItem&) = default;
		AppLogicItem(std::shared_ptr<VFrame30::FblItemRect> fblItem,
				  std::shared_ptr<VFrame30::LogicScheme> scheme,
				  std::shared_ptr<Afb::AfbElement> afbElement);


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
		AppLogicModule(QString moduleStrId);

		bool addBranch(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
					   const BushContainer& bushContainer,
					   Afb::AfbElementCollection* afbCollection,
					   IssueLogger* log);

		bool orderItems(IssueLogger* log);

	private:
		// Set connection between SchemeItemInput/SchemeItemOutput by StrIds
		//
		bool setInputOutputsElementsConnection(IssueLogger* log);

		template<typename Iter>
		std::list<AppLogicItem> getItemsWithInput(
			Iter begin,
			Iter end,
			const QUuid& inputGuid);

	public:
		QString moduleStrId() const;
		void setModuleStrId(QString value);

		const std::list<AppLogicItem>& items() const;
		std::list<AppLogicItem>& items();

	private:
		QString m_moduleStrId;
		std::list<AppLogicItem> m_items;		// Ordered items
		std::set<AppLogicItem> m_fblItemsAcc;	// Temporary buffer, filled in addBranch, cleared in orderItems
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
		bool addData(const BushContainer& bushContainer,
			std::shared_ptr<VFrame30::LogicScheme> scheme,
			std::shared_ptr<VFrame30::SchemeLayer> layer,
			Afb::AfbElementCollection* afbCollection,
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
			   int changesetId,
			   bool debug);

		virtual ~Parser();

		bool build();

	protected:
		bool loadAppLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicScheme>>* out);

		bool parseAppLogicScheme(std::shared_ptr<VFrame30::LogicScheme> logicScheme);

		bool parseAppLogicLayer(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
								  std::shared_ptr<VFrame30::SchemeLayer> layer);

		bool findBushes(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
						std::shared_ptr<VFrame30::SchemeLayer> layer,
						BushContainer* bushContainer) const;

		bool setBranchConnectionToPin(std::shared_ptr<VFrame30::LogicScheme> scheme,
									  std::shared_ptr<VFrame30::SchemeLayer> layer,
									  BushContainer* bushContainer) const;

		bool setPinConnections(std::shared_ptr<VFrame30::LogicScheme> scheme,
							   std::shared_ptr<VFrame30::SchemeLayer> layer,
							   BushContainer* bushContainer);

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
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
	};

}

#endif // APPLOGICBUILDER_H
