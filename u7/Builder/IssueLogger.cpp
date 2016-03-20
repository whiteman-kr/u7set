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

	// CFG			FSC configuration						3000-3999
	//

	// ALP			Application Logic Parsing				4000-4999
	//

	/// IssueCode: ALP4000
	///
	/// IssueType: Error
	///
	/// Title: Branch has multiple outputs (LogicScheme '%1').
	///
	/// Parameters:
	///		%1 Logic Scheme StrID
	///
	/// Description:
	///		Error may occur if there are more than one output is linked to input
	///
	void IssueLogger::errALP4000(QString scheme, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4000,
				  tr("Branch has multiple outputs (LogicScheme '%1').")
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
