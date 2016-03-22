#include "IssueLogger.h"

namespace Builder
{

	IssueLogger::IssueLogger() :
		OutputLog()
	{
	}

	IssueLogger::~IssueLogger()
	{
	}

	// CMN			Common issues							0000-0999
	//

	// INT			Internal issues							1000-1999
	//

	// PDB			Project database issues					2000-2999
	//

	/// IssueCode: PDB2000
	///
	/// IssueType: Warning
	///
	/// Title: The workcopies of the checked out files will be compiled.
	///
	/// Description:
	///		Warning will occur if a project is built with DEBUG option. Unchecked In files can be built thus all
	///		changes on workcopies can be undo later. Debug build does not guarantee keep tracking of changes.\n
	///		Note that during RELEASE compile only checked in files will be taken for build process.
	///
	void IssueLogger::wrnPDB2000()
	{
		LOG_WARNING(IssueType::ProjectDatabase,
				  2000,
				  tr("The workcopies of the checked out files will be compiled."));
	}

	// CFG			FSC configuration						3000-3999
	//

	// ALP			Application Logic Parsing				4000-4999
	//

	/// IssueCode: ALP4000
	///
	/// IssueType: Error
	///
	/// Title: Branch has multiple outputs (Logic Scheme '%1').
	///
	/// Parameters:
	///		%1 Logic Scheme StrID
	///
	/// Description:
	///		Error may occur if there are more than one output is linked to input.
	///
	void IssueLogger::errALP4000(QString scheme, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4000,
				  tr("Branch has multiple outputs (Logic Scheme '%1').")
				  .arg(scheme));
	}

	/// IssueCode: ALP4001
	///
	/// IssueType: Warning
	///
	/// Title: Property DeviceStrIds for Logic Scheme is not set (LogicScheme '%1').
	///
	/// Parameters:
	///		%1 Logic Scheme StrID
	///
	/// Description:
	///		Property DeviceStrIds for an application logic scheme is empty. To bind a scheme to a Logic Module this field must be set
	///		to the Logic Module StrID. (Note that expanded StrID must be used).
	///
	void IssueLogger::wrnALP4001(QString scheme)
	{
		LOG_WARNING(IssueType::AlParsing,
				  4001,
				  tr("Property DeviceStrIds for Logic Scheme is not set (LogicScheme '%1').")
				  .arg(scheme));
	}

	/// IssueCode: ALP4002
	///
	/// IssueType: Warning
	///
	/// Title: HardwareStrId '%1' is not found in the project equipment (Logic Scheme '%2').
	///
	/// Parameters:
	///		%1 Logic Modules StrID
	///		%2 Logic Scheme StrID
	///
	/// Description:
	///		Logic Scheme has property HardwareStrIds but Logic Module with pointed StrID is not found in the project equipment.
	///
	void IssueLogger::wrnALP4002(QString scheme, QString hardwareStrId)
	{
		LOG_WARNING(IssueType::AlParsing,
					4002,
					tr("HardwareStrId '%1' is not found in the project equipment (Logic Scheme '%2').")
					.arg(hardwareStrId)
					.arg(scheme));
	}

	/// IssueCode: ALP4003
	///
	/// IssueType: Warning
	///
	/// Title: HardwareStrId '%1' must be LM family module type (Logic Scheme '%2').
	///
	/// Parameters:
	///		%1 Logic Modules StrID
	///		%2 Logic Scheme StrID
	///
	/// Description:
	///		Logic Scheme has property HardwareStrIds but the equipment object with pointed StrID is not a module or is not LM family type.
	///
	void IssueLogger::wrnALP4003(QString scheme, QString hardwareStrId)
	{
		LOG_WARNING(IssueType::AlParsing,
					4003,
					tr("HardwareStrId '%1' must be LM family module type (Logic Scheme '%2').")
					.arg(hardwareStrId)
					.arg(scheme));
	}

	/// IssueCode: ALP4004
	///
	/// IssueType: Warning
	///
	/// Title: Scheme is excluded from build (Scheme '%1').
	///
	/// Parameters:
	///		%1 Scheme StrID
	///
	/// Description:
	///			Scheme is excluded from build and will not be parsed. To include scheme in the build process set
	///		scheme property ExcludeFromBuild to false.
	///
	void IssueLogger::wrnALP4004(QString scheme)
	{
		LOG_WARNING(IssueType::AlParsing,
					4004,
					tr("Scheme is excluded from build (Scheme '%1').")
					.arg(scheme));
	}

	/// IssueCode: ALP4005
	///
	/// IssueType: Warning
	///
	/// Title: Logic Scheme is empty, there are no any functional blocks in the compile layer (Logic Scheme '%1').
	///
	/// Parameters:
	///		%1 Logic Scheme StrID
	///
	/// Description:
	///			Logic Scheme is empty, there are no any functional blocks in the compile layer.
	///
	void IssueLogger::wrnALP4005(QString scheme)
	{
		LOG_WARNING(IssueType::AlParsing,
					4005,
					tr("Logic Scheme is empty, there are no any functional blocks in the compile layer (Logic Scheme '%1').")
					.arg(scheme));
	}

	/// IssueCode: ALP4006
	///
	/// IssueType: Error
	///
	/// Title: Scheme item '%1' has not linked pin '%2' (Logic Scheme '%3').
	///
	/// Parameters:
	///		%1 Scheme item description
	///		%2 Pin
	///		%3 Logic Scheme StrID
	///
	/// Description:
	///		Scheme item has not linked pin(s), all pins of the function block must be linked.
	///
	void IssueLogger::errALP4006(QString scheme, QString schemeItem, QString pin, QUuid itemUuid)
	{
		std::vector<QUuid> itemsUuids;
		itemsUuids.push_back(itemUuid);

		return errALP4006(scheme, schemeItem, pin, itemsUuids);
	}

	void IssueLogger::errALP4006(QString scheme, QString schemeItem, QString pin, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4006,
				  tr("Scheme item '%1' has not linked pin '%2' (Logic Scheme '%3').")
				  .arg(schemeItem)
				  .arg(pin)
				  .arg(scheme));
	}

	// ALC			Application logic compiler				5000-5999
	//

	// EQP			Equipment issues						6000-6999
	//



	// --
	//
	void IssueLogger::addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids)
	{
		QMutexLocker l(&m_mutex);

		for (auto id : itemsUuids)
		{
			m_itemsIssues[id] = level;
		}
	}

	void IssueLogger::addItemsIssue(OutputMessageLevel level, QUuid itemsUuid)
	{
		QMutexLocker l(&m_mutex);
		m_itemsIssues[itemsUuid] = level;
	}

	void IssueLogger::swapItemsIssues(std::map<QUuid, OutputMessageLevel>* itemsIssues)
	{
		if (itemsIssues == nullptr)
		{
			assert(itemsIssues);
			return;
		}

		QMutexLocker l(&m_mutex);
		std::swap(m_itemsIssues, *itemsIssues);
	}

	void IssueLogger::clearItemsIssues()
	{
		QMutexLocker l(&m_mutex);
		m_itemsIssues.clear();
	}

	QString IssueLogger::issuePTypeToString(IssueType it)
	{
		switch(it)
		{
			case IssueType::NotDefined:
				return "NDF";
			case IssueType::Common:
				return "CMN";
			case IssueType::Internal:
				return "INT";
			case IssueType::ProjectDatabase:
				return "PDB";
			case IssueType::FscConfiguration:
				return "CFG";
			case IssueType::AlParsing:
				return "ALP";
			case IssueType::AlCompiler:
				return "ALC";
			case IssueType::Equipment:
				return "EQP";
			default:
				assert(false);
				return "NDF";
		}
	}
}
