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

		size_t getBranchByPinPos(VFrame30::VideoItemPoint pt) const;
		size_t getBranchByPinGuid(const QUuid& guid) const;
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
		bool compileApplicationLogicScheme(VFrame30::LogicScheme* logicScheme);
		bool compileApplicationLogicLayer(VFrame30::LogicScheme* logicScheme, VFrame30::SchemeLayer* layer);

		bool findBranches(VFrame30::LogicScheme* logicScheme,
						  VFrame30::SchemeLayer* layer,
						  BranchContainer* branchContainer) const;

		bool setBranchConnectionToPin(VFrame30::LogicScheme* scheme, VFrame30::SchemeLayer* layer,
									  BranchContainer* branchContainer) const;

		bool setPinConnections(VFrame30::LogicScheme* scheme, VFrame30::SchemeLayer* layer,
							   BranchContainer* branchContainer);

	protected:
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
