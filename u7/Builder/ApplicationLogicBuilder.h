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

	struct BranchLink
	{
		BranchLink() = default;
		BranchLink(const VFrame30::VideoItemPoint& point1, const VFrame30::VideoItemPoint& point2);

		VFrame30::VideoItemPoint pt1;
		VFrame30::VideoItemPoint pt2;
	};

	struct Branch
	{
		QUuid outputPin;						// Output pin for this branch, can be the only
		std::set<QUuid> inputPins;				// Input pins for this branch
		std::map<QUuid, BranchLink> links;		// Links for this branch
	};

	struct BranchContainer
	{
		std::vector<Branch> branches;

		int getBranchByPinPos(VFrame30::VideoItemPoint pt) const;
		int getBranchByPinGuid(const QUuid& guid) const;
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
		bool setData(std::shared_ptr<VFrame30::LogicScheme> scheme,
					 std::shared_ptr<VFrame30::SchemeLayer> layer);

		// Propertie
		//
	public:
		const std::shared_ptr<VFrame30::LogicScheme> scheme() const;
		std::shared_ptr<VFrame30::LogicScheme> scheme();

		const std::shared_ptr<VFrame30::SchemeLayer> layer() const;
		std::shared_ptr<VFrame30::SchemeLayer> layer();

		std::list<std::shared_ptr<VFrame30::FblItemRect>> afbItems() const;

	private:
		std::shared_ptr<VFrame30::LogicScheme> m_scheme;
		std::shared_ptr<VFrame30::SchemeLayer> m_layer;
		std::list<std::shared_ptr<VFrame30::FblItemRect>> m_afbItems;
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

		bool findBranches(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
						  std::shared_ptr<VFrame30::SchemeLayer> layer,
						  BranchContainer* branchContainer) const;

		bool setBranchConnectionToPin(std::shared_ptr<VFrame30::LogicScheme> scheme,
									  std::shared_ptr<VFrame30::SchemeLayer> layer,
									  BranchContainer* branchContainer) const;

		bool setPinConnections(std::shared_ptr<VFrame30::LogicScheme> scheme,
							   std::shared_ptr<VFrame30::SchemeLayer> layer,
							   BranchContainer* branchContainer);

	private:
		DbController* db();
		OutputLog* log() const;
		int changesetId() const;
		bool debug() const;
		bool release() const;

	private:
		DbController* m_db = nullptr;
		mutable OutputLog* m_log = nullptr;
		int m_changesetId = 0;
		int m_debug = false;
	};


}

#endif // APPLOGICBUILDER_H
