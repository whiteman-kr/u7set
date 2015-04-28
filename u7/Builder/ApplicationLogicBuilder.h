#ifndef APPLOGICBUILDER_H
#define APPLOGICBUILDER_H

#include "../../VFrame30/VideoItem.h"

// Forware delcarations
//
class QThread;
class OutputLog;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class DeviceModule;
	class McFirmwareOld;
}

namespace VFrame30
{
	class LogicScheme;
	class SchemeLayer;
	class VideoItemFblElement;
	class FblItemRect;
}

namespace Afbl
{
	class AfbElement;
}

namespace Builder
{

	struct Link
	{
		Link() = default;
		Link(const VFrame30::VideoItemPoint& point1, const VFrame30::VideoItemPoint& point2);

		VFrame30::VideoItemPoint pt1;
		VFrame30::VideoItemPoint pt2;
	};

	struct Bush
	{
		QUuid outputPin;						// Output pin for this branch, can be the only
		std::set<QUuid> inputPins;				// Input pins for this branch
		std::map<QUuid, Link> links;			// Links for this branch
		std::set<std::shared_ptr<VFrame30::FblItemRect>> fblItems;
	};

	struct BushContainer
	{
		std::vector<Bush> bushes;

		int getBranchByPinPos(VFrame30::VideoItemPoint pt) const;
		int getBranchByPinGuid(const QUuid& guid) const;
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBranch
	//
	// ------------------------------------------------------------------------
//	class ApplicationLogicBranch
//	{
//	public:
//		ApplicationLogicBranch();

//		const std::list<std::shared_ptr<VFrame30::FblItemRect>>& items() const;
//		std::list<std::shared_ptr<VFrame30::FblItemRect>>& items();

//	private:
//		std::list<std::shared_ptr<VFrame30::FblItemRect>> m_items;
//	};

	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicModule
	//
	// ------------------------------------------------------------------------
	class ApplicationLogicModule : QObject
	{
		Q_OBJECT

	public:
		ApplicationLogicModule() = delete;
		ApplicationLogicModule(QString moduleStrId);

		bool addBranch(const BushContainer& bushContainer, OutputLog* log);

	private:
		template<typename Iter>
		std::list<std::shared_ptr<VFrame30::FblItemRect>> getItemsWithInput(
			Iter begin,
			Iter end,
			const QUuid& inputGuid);

		template<typename Iter>
		std::list<std::shared_ptr<VFrame30::FblItemRect>> getItemsWithInput(
			Iter begin,
			Iter end,
			const std::list<QUuid>& inputGuids);


	public:
		QString moduleStrId() const;
		void setModuleStrId(QString value);

		const std::list<std::shared_ptr<VFrame30::FblItemRect>>& items() const;
		std::list<std::shared_ptr<VFrame30::FblItemRect>>& items();

	private:
		QString m_moduleStrId;

		//std::list<std::shared_ptr<ApplicationLogicBranch>> m_branches;

		std::list<std::shared_ptr<VFrame30::FblItemRect>> m_items;
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicData
	//
	// ------------------------------------------------------------------------
	class ApplicationLogicData
	{
	public:
		ApplicationLogicData();

		// Public methods
		//
	public:
		bool addData(const BushContainer& bushContainer,
			std::shared_ptr<VFrame30::LogicScheme> scheme,
			std::shared_ptr<VFrame30::SchemeLayer> layer,
			OutputLog* log);

		// Propertie
		//
	public:
//		const std::shared_ptr<VFrame30::LogicScheme> scheme() const;
//		std::shared_ptr<VFrame30::LogicScheme> scheme();

//		const std::shared_ptr<VFrame30::SchemeLayer> layer() const;
//		std::shared_ptr<VFrame30::SchemeLayer> layer();

//		std::list<std::shared_ptr<VFrame30::FblItemRect>> afbItems() const;

	private:
		//std::shared_ptr<VFrame30::LogicScheme> m_scheme;
		//std::shared_ptr<VFrame30::SchemeLayer> m_layer;
		//std::list<std::shared_ptr<VFrame30::FblItemRect>> m_afbItems;
		std::list<std::shared_ptr<ApplicationLogicModule>> m_modules;
	};


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------
	class ApplicationLogicBuilder : QObject
	{
		Q_OBJECT

	public:
		ApplicationLogicBuilder() = delete;
		ApplicationLogicBuilder(DbController* db, OutputLog* log, int changesetId, bool debug);
		virtual ~ApplicationLogicBuilder();

		bool build();

	protected:
		bool loadApplicationLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicScheme>>* out);

		bool compileApplicationLogicScheme(std::shared_ptr<VFrame30::LogicScheme> logicScheme);

		bool compileApplicationLogicLayer(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
										  std::shared_ptr<VFrame30::SchemeLayer> layer);

		bool findBushes(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
						std::shared_ptr<VFrame30::SchemeLayer> layer,
						BushContainer* bushContainer) const;

		bool setBranchConnectionToPin(std::shared_ptr<VFrame30::LogicScheme> scheme,
									  std::shared_ptr<VFrame30::SchemeLayer> layer,
									  BushContainer* branchContainer) const;

		bool setPinConnections(std::shared_ptr<VFrame30::LogicScheme> scheme,
							   std::shared_ptr<VFrame30::SchemeLayer> layer,
							   BushContainer* branchContainer);

	private:
		DbController* db();
		OutputLog* log() const;
		int changesetId() const;

		bool debug() const;
		bool release() const;

		const ApplicationLogicData& applicationData() const;
		ApplicationLogicData& applicationData();

	private:
		DbController* m_db = nullptr;
		mutable OutputLog* m_log = nullptr;
		int m_changesetId = 0;
		int m_debug = false;

		ApplicationLogicData m_applicationData;
	};


}

#endif // APPLOGICBUILDER_H
