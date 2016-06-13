#ifndef ISSUELOGGER_H
#define ISSUELOGGER_H

#include <map>

#include <QMutex>
#include <QUuid>

#include "../lib/OutputLog.h"

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
		Q_OBJECT

	public:
		IssueLogger();
		virtual ~IssueLogger();

	public:

		// CMN			Common issues							0000-0999
		//
		void errCMN0010(QString fileName);
		void errCMN0011(QString directory);
		void errCMN0012(QString fileName);
		void errCMN0013(QString fileName);

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
		Q_INVOKABLE void errCFG3000(QString propertyName, QString object);              // general errors
		Q_INVOKABLE void errCFG3001(QString subSysID, QString module);
		Q_INVOKABLE void errCFG3002(QString name, int value, int min, int max, QString module);
		Q_INVOKABLE void errCFG3003(int channel, QString module);
		Q_INVOKABLE void errCFG3004(QString controllerID, QString module);

		Q_INVOKABLE void wrnCFG3005(QString signalID, QString controllerID);
		Q_INVOKABLE void wrnCFG3006(int place, QString controllerID);
		Q_INVOKABLE void wrnCFG3007(QString signalID);
		Q_INVOKABLE void wrnCFG3008(QString softwareID, QString module);      // software errors

		Q_INVOKABLE void errCFG3009(QString signalID1, double spredTolerance1, QString signalID2, double spredTolerance2, QString module);
		Q_INVOKABLE void errCFG3010(QString name, double value, double min, double max, int precision, QString signalID);

		Q_INVOKABLE void errCFG3011(QString addressProperty, uint address, QString controller);
		Q_INVOKABLE void errCFG3012(QString portProperty, uint port, QString controller);

		enum IssueCompareMode
		{
			Equal = 0,
			Less = 1,
			More = 2
		};

		Q_INVOKABLE void errCFG3013(QString name1, double value1, int compareMode, QString name2, double value2, int precision, QString signalID);

		void errCFG3014(QString suffix, QString objectID);									// Can't find child object wuith suffix '%1' in object '%2'
		void wrnCFG3015(QString objectID, QString propertyName, QString softwareID);		// Property '%1.%2' is linked to undefined software ID '%3'.
		void wrnCFG3016(QString objectID, QString propertyName);							// Property '%1.%2' is empty.
		void errCFG3017(QString objectID, QString propertyName, QString softwareID);		// Property '%1.%2' is linked to undefined software ID '%3'.
		Q_INVOKABLE void wrnCFG3018(QString propertyName, QString ip, int port, QString controller);
		void errCFG3019(QString objectID, QString propertyName);							// Property '%1.%2' write error.
		void errCFG3020(QString objectID, QString propertyName);							// Property '%1.%2' is not found.

		// ALP			Application Logic Parsing				4000-4999
		//
		void errALP4000(QString schema, const std::vector<QUuid>& itemsUuids);
		void errALP4001(QString schema);
		void errALP4002(QString schema, QString hardwareStrId);
		void errALP4003(QString schema, QString hardwareStrId);
		void wrnALP4004(QString schema);
		void wrnALP4005(QString schema);	// Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema '%1').
		void errALP4006(QString schema, QString schemaItem, QString pin, QUuid itemUuid);
		void errALP4006(QString schema, QString schemaItem, QString pin, const std::vector<QUuid>& itemsUuids);
		void errALP4007(QString schema, QString schemaItem, QString afbElement, QUuid itemUuid);
		void errALP4008(QString schema, QString schemaItem, QString schemaItemAfbVersion, QString latesAfbVersion, QUuid itemUuid);
		void errALP4020(QString logicModule);
		void errALP4021(QString logicModule, QString schema1, QString schema2, QString schemaItem1, QString schemaItem2, QString signalStrID, const std::vector<QUuid>& itemsUuids);
		void errALP4022(QString schema);

		// ALC			Application logic compiler				5000-5999
		//

		void errALC5000(QString appSignalID, QUuid itemUuid);					// Signal identifier '%1' is not found.
		void wrnALC5001(QString logicModuleID);									// Application logic for module '%1' is not found.
		void errALC5002(QString appSignalID, QUuid itemUuid);					// Value of signal '%1' is undefined.
		void errALC5003(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Analog output '%1.%2' is connected to discrete signal '%3'.
		void errALC5004(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Output '%1.%2' is connected to signal '%3' with uncompatible data format.
		void errALC5005(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Output '%1.%2' is connected to signal '%3' with uncompatible data size.
		void errALC5006(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Discrete output '%1.%2' is connected to analog signal '%3'.
		void errALC5007(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Discrete signal '%1' is connected to analog input '%2.%3'.
		void errALC5008(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Signal '%1' is connected to input '%2.%3' with uncompatible data format.
		void errALC5009(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Signal '%1' is connected to input '%2.%3' with uncompatible data size.
		void errALC5010(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Analog signal '%1' is connected to discrete input '%2.%3'.
		void errALC5011(QUuid itemUuid);										// Application item '%1' has unknown type.
		void wrnALC5012(QString appSignalID);									// Application signal '%1' is not bound to any device object.
		void errALC5013(QString appSignalID, QString equipmentID);				// Application signal '%1' is bound to unknown device object '%2'.
		void errALC5014(QString appSignalID);									// Discrete signal '%1' must have DataSize equal to 1.
		void errALC5015(QString appSignalID);									// Analog signal '%1' must have DataSize equal to 32.
		void errALC5016(QString appSignalID);									// Application signal identifier '%1' is not unique.
		void errALC5017(QUuid transmitterUuid, QUuid connectedItemUuid);		// Non-signal element is connected to transmitter.
		void errALC5018(QString port1, QString port2, QString connection);			// Opto ports '%1' and '%2' are not compatible (connection '%3').
		void errALC5019(QString port, QString connection1, QString connection2);	// Opto port '%1' of connection '%2' is already used in connection '%3'.
		void errALC5020(QString port, QString connection);							// LM's port '%1' can't work in RS232/485 mode (connection '%2').
		void errALC5021(QString port, QString connection);							// Undefined opto port '%1' in the connection '%2'.
		void errALC5022(QString connection);									// Opto ports of the same chassis is linked via connection '%1'.
		void errALC5023(QString connection);									// Opto connection caption '%1' is not unique.
		void errALC5024(QString connection, QUuid transmitterUuid);				// Transmitter is linked to unknown opto connection '%1'.
		void errALC5025(QString connection, QUuid receiverUuid);				// Receiver is linked to unknown opto connection '%1'.
		void errALC5026(QUuid transmitterUuid, const QList<QUuid>& signalIDs);	// Transmitter input can be linked to one signal only.
		void errALC5027(QUuid transmitterUuid);									// All transmitter inputs must be directly linked to a signals.

		// EQP			Equipment issues						6000-6999
		//
		void errEQP6000(QString deviceStrId, QUuid deviceUuid);
		void errEQP6001(QString deviceStrId, QUuid deviceUuid1, QUuid deviceUuid2);
		void errEQP6002(QUuid deviceUuid, QString deviceStrId1, QString deviceStrId2);

		// Subset of EQP -- Generation Software Configuration
		//
		void errEQP6100(QString softwareObjectStrId, QUuid uuid);


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
