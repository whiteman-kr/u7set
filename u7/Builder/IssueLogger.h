#ifndef ISSUELOGGER_H
#define ISSUELOGGER_H

#include <map>

#include <QMutex>
#include <QUuid>

#include "../include/OutputLog.h"

#define LOG_ERROR(type, code, message)		writeError(issuePTypeToString(type) + QString::number(code).rightJustified(4, '0'), message, __FILE__, __LINE__, Q_FUNC_INFO);
#define LOG_WARNING(type, code, message)	writeWarning(issuePTypeToString(type) + QString::number(code).rightJustified(4, '0'), message, __FILE__, __LINE__, Q_FUNC_INFO);


namespace Builder
{
	enum class IssueType
	{
		NotDefined,
		Common,
		Internal,
		ProjectDatabase,
		FscConfiguration,
		AlParsing,
		AlCompiler,
		Equipment
	};


	class IssueLogger : public OutputLog
	{
	public:
		IssueLogger();
		virtual ~IssueLogger();

	public:

		// CMN			Common issues							0000-0999
		//

		// INT			Internal issues							1000-1999
		//

		// PDB			Project database issues					2000-2999
		//
		void wrnPDB2000();

		// CFG			FSC configuration						3000-3999
		//

		// ALP			Application Logic Parsing				4000-4999
		//
		void errALP4000(QString scheme, const std::vector<QUuid>& itemsUuids);

		// ALC			Application logic compiler				5000-5999
		//

		// EQP			Equipment issues						6000-6999
		//

	public:
		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids);
		void swapItemsIssues(std::map<QUuid, OutputMessageLevel>* itemsIssues);
		void clearItemsIssues();

	private:
		static QString issuePTypeToString(IssueType it);

	private:
		QMutex m_mutex;
		std::map<QUuid, OutputMessageLevel> m_itemsIssues;
	};

}
#endif // ISSUELOGGER_H
