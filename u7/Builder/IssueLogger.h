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
		void errCMN0010(QString fileName);

		// INT			Internal issues							1000-1999
		//
		void errINT1000(QString debugMessage);

		// PDB			Project database issues					2000-2999
		//
		void wrnPDB2000();
		void errPDB2001(int parentFileId, QString filter, QString databaseMessage);
		void errPDB2002(int fileId, QString fileName, QString databaseMessage);

		// CFG			FSC configuration						3000-3999
		//

		// ALP			Application Logic Parsing				4000-4999
		//
		void errALP4000(QString scheme, const std::vector<QUuid>& itemsUuids);
		void wrnALP4001(QString scheme);
		void wrnALP4002(QString scheme, QString hardwareStrId);
		void wrnALP4003(QString scheme, QString hardwareStrId);
		void wrnALP4004(QString scheme);
		void wrnALP4005(QString scheme);	// Logic Scheme is empty, there are no any functional blocks in the compile layer (Logic Scheme '%1').
		void errALP4006(QString scheme, QString schemeItem, QString pin, QUuid itemUuid);
		void errALP4006(QString scheme, QString schemeItem, QString pin, const std::vector<QUuid>& itemsUuids);
		void errALP4007(QString scheme, QString schemeItem, QString afbElement, QUuid itemUuid);
		void errALP4008(QString logicModule);
		void errALP4009(QString logicModule, QString scheme1, QString scheme2, QString schemeItem1, QString schemeItem2, QString signalStrID, const std::vector<QUuid>& itemsUuids);

		// ALC			Application logic compiler				5000-5999
		//

		// EQP			Equipment issues						6000-6999
		//

	public:
		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids);
		void addItemsIssues(OutputMessageLevel level, QUuid itemsUuid);

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
