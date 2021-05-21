#pragma once

#include "../lib/OutputLog.h"
#include <QUuid>

#define LOG_ERROR(type, code, message)		writeError(issuePTypeToString(type), code, message, __FILE__, __LINE__, SHORT_FUNC_INFO)

// Warning2 - the least important warning
// Warning1 - just warning
// Warning0 - the most important warning
//
#define LOG_WARNING0(type, code, message)	writeWarning0(issuePTypeToString(type), code, message, __FILE__, __LINE__, SHORT_FUNC_INFO)
#define LOG_WARNING1(type, code, message)	writeWarning1(issuePTypeToString(type), code, message, __FILE__, __LINE__, SHORT_FUNC_INFO)
#define LOG_WARNING2(type, code, message)	writeWarning2(issuePTypeToString(type), code, message, __FILE__, __LINE__, SHORT_FUNC_INFO)

#define LOG_INTERNAL_ERROR(logObject)								logObject->errALC5998(__FILE__, __LINE__, Q_FUNC_INFO)
#define LOG_INTERNAL_ERROR_MSG(logObject, errorMsg)					logObject->errALC5996(errorMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#define LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(result, logObject)	if (result == false) { logObject->errALC5998(__FILE__, __LINE__, Q_FUNC_INFO); return false; }

#define LOG_NULLPTR_ERROR(logObject)							logObject->errALC5997(__FILE__, __LINE__, Q_FUNC_INFO)
#define LOG_IF_NULLPTR_RETURN_FALSE(ptr, logObject)				if (ptr == nullptr) { logObject->errALC5997(__FILE__, __LINE__, Q_FUNC_INFO); return false; }


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

	class BuildIssues
	{
	public:
		// Data
		//
		struct Counter
		{
			int errors = 0;
			int warnings = 0;
		};

		// Methods
		//
	public:
		void clear();

		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids);
		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids, const QString& schemaID);

		void addItemsIssues(OutputMessageLevel level, QUuid itemsUuid);
		void addItemsIssues(OutputMessageLevel level, QUuid itemsUuid, const QString& schemaID);

		void addSchemaIssue(OutputMessageLevel level, const QString& schemaID);

		OutputMessageLevel issueForSchemaItem(const QUuid& itemId) const;
		BuildIssues::Counter issueForSchema(const QString& schemaId) const;

		int count() const;

	private:
		mutable QMutex m_mutex{QMutex::RecursionMode::Recursive};

		std::map<QUuid, OutputMessageLevel> m_items;		// Item ussuses
		std::map<QString, Counter> m_schemas;				// Item ussuses
	};

	class IssueLogger : public OutputLog
	{
		Q_OBJECT

	public:
		explicit IssueLogger(BuildIssues* buildIssues = nullptr);
		virtual ~IssueLogger();

		enum IssueCompareMode
		{
			Equal = 0,
			Less = 1,
			More = 2
		};

	public slots:
		// CMN			Common issues							0000-0999
		//
		void errCMN0010(QString fileName);						// File loading/parsing error, file is damaged or has incompatible format, file name %1.
		void errCMN0011(QString directory);						// Can't create directory %1.
		void errCMN0012(QString fileName);						// Can't create file %1.
		void errCMN0013(QString fileName);						// Write error of file %1.
		void errCMN0014(QString fileName);						// File %1 already exists.
		void errCMN0015(QString fileName1, QString fileName2, QString id);		// %1 and %2 files have the same ID = %3.
		void errCMN0016();										// The build was cancelled.
		void errCMN0017(QString fileName);						// Can't open file %1.
		void errCMN0018(QString fileName, QString cfgXmlSubdir);		// Can't link build file %1 into /%2/configuration.xml.
		void errCMN0019(QString fileID, QString subDir);		// Can't find file with ID = %1 in build subdirectory %2.
		void errCMN0020(QString fileName);						// Can't find build file %1.
		void errCMN0021(QString fileName, QString cfgXmlFileName);	// File %1 already linked to %2.
		void wrnCMN0022(QString issue, QString stdWritablePath);	// Build output path %1. Standard writeble location will be used: %2

		// INT			Internal issues							1000-1999
		//
		void errINT1000(QString debugMessage);

		void errINT1001(QString debugMessage);
		void errINT1001(QString debugMessage, QString schema);
		void errINT1001(QString debugMessage, QString schema, QUuid itemsUuids);
		void errINT1001(QString debugMessage, QString schema, const std::vector<QUuid>& itemsUuids);

		// PDB			Project database issues					2000-2999
		//
		void wrnPDB2000();
		void errPDB2001(int parentFileId, QString filter, QString databaseMessage);
		void errPDB2002(int fileId, QString fileName, QString databaseMessage);
		void errPDB2003();												// Load signals from the project database error
		void errPDB2004();												// Load units from the project database error
		void errPDB2005();												// Load UFB schemas from the project database error
		void errPDB2006(QString projectName, QString dbLastError);		// Opening project %1 error (%2).
		void errPDB2007(QString fileName, QString equipmentId, QString propertyName);	// File is not found in the project database


		void errPDB2020();												// Getting project properties error.

		// CFG			FSC configuration						3000-3999
		//
		void errCFG3000(QString propertyName, QString object);						// Property %1 does not exist in object %2.
		void errCFG3001(QString subSysID, QString module);							// Subsystem %1 is not found in subsystem set (Logic Module %2).
		void errCFG3002(QString name, int value, int min, int max, QString module); // Property %1 has wrong value (%2), valid range is %3..%4 (module %5).
		void errCFG3003(int LMNumber, QString module);								// Property System\\LMNumber (%1) is not unique in Logic Module %2.
		void errCFG3004(QString controllerID, QString module);						// Controller %1 is not found in module %2.

		void wrnCFG3005(QString signalID, QString controllerID);					// Signal %1 is not found in controller %2.
		void wrnCFG3006(int place, QString controllerID);							// Signal with place %1 is not found in controller %2.
		void wrnCFG3007(QString signalID);											// Signal %1 is not found in Application Signals.
		void wrnCFG3008(QString softwareID, QString module);						// Software %1 is not found (Logic Module %2).

		void errCFG3009(QString signalID1, QString signalID2, QString module);		// Calculated SpreadTolerance ADC mismatch, signals %1 and %2 in module %3.
		void errCFG3010(QString name, double value, double min, double max, int precision, QString signalID);	// Property %1 has wrong value (%2), valid range is %3..%4 [precision %5](signal %6).

		void errCFG3011(QString addressProperty, uint address, QString controller);	// IP address in property %1 has undefined value (%2) in controller %3.
		void errCFG3012(QString portProperty, uint port, QString controller);		// Port in property %1 has undefined value (%2) in controller %3.

		void errCFG3013(QString name1, double value1, int compareMode, QString name2, double value2, int precision, QString signalID);	//Property %1 (%2) is %3 property %4 (%5) in signal %6.

		void errCFG3014(QString suffix, QString objectID);									// Can't find child object with suffix %1 in object %2
		void wrnCFG3015(QString objectID, QString propertyName, QString softwareID);		// Property %1.%2 is linked to undefined software ID %3.
		void wrnCFG3016(QString objectID, QString propertyName);							// Property %1.%2 is empty.
		void errCFG3017(QString objectID, QString propertyName, QString softwareID);		// Property %1.%2 is linked to not compatible software ID %3.
		void wrnCFG3018(QString propertyName, QString ip, int port, QString controller);	// Default %1 IP address %2:%3 is used in controller %4.
		void errCFG3019(QString objectID, QString propertyName);							// Property %1.%2 write error.
		void errCFG3020(QString objectID, QString propertyName);							// Property %1.%2 is not found.
		void errCFG3021(QString objectID, QString propertyName, QString softwareID);		// Property %1.%2 is linked to undefined software ID %3.
		void errCFG3022(QString objectID, QString propertyName);							// Property %1.%2 is empty.
		void errCFG3023(QString objectID, QString propertyName);							// Property %1.%2 conversion error.
		void errCFG3025(QString suffix, QString objectID);									// Can't find child controller with suffix %1 in object %2
		void errCFG3026(QString objectID, QString propertyName);							// Value of property %1.%2 is not valid IPv4 address.
			
		void errCFG3027(QString objectID, QString propertyName);							// Ethernet port number property %1.%2 should be in range 0..65535.
		void errCFG3028(QString signalID1, QString signalID2, QString module, QString propertyName);
		void errCFG3029(QString softwareID);												// Software %1 is not linked to ConfigurationService.
		void errCFG3030(QString lmID, QString appDataServiceID);							// Etherent adapters 2 and 3 of LM %1 are connected to same AppDataService %2.		
		void wrnCFG3031(QString objectID, QString propertyName);							// Property %1.%2 should be set to the valid writable catalog of workstation.

		void errCFG3040(QString monitorId, QString tuningServiceId);						// Mode SingleLmControl is not supported by Monitor. Set TuningServiceID.SingleLmControl to false. Monitor EquipmentID %1, TuningServiceID %2.
		void errCFG3041(QString name, QString value, QString message, QString signalId);	// Property %1 has wrong value (%2), required value is %3 in signal %4.

		void errCFG3042(QString moduleEquipmentID, QUuid moduleUuid);		// Module %1 should be installed in chassis.

		void errCFG3043(QString dataSourceIP,
						QString dataSourceEquipmentID,
						QString dataReceivingIP,
						QString receivingEquipmentID);		// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).


		void errCFG3044(QString equipmentID, QString profileID);	// Equipment object %1 is not found (Settings profile - %2).
		void errCFG3045(QString equipmentID, QString propertyName, QString profileID);	// Property %1.%2 is not found (Settings profile - %3).


		// ALP			Application Logic Parsing				4000-4999
		//
		void errALP4000(QString schema, const std::vector<QUuid>& itemsUuids);
		void errALP4001(QString schema, QString propertyName);
		void errALP4002(QString schema, QString hardwareStrId);
		void errALP4003(QString schema, QString hardwareStrId);
		void wrnALP4004(QString schema);
		void wrnALP4005(QString schema);	// Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema %1).
		void errALP4006(QString schema, QString schemaItem, QString pin, QUuid itemUuid);
		void errALP4006(QString schema, QString schemaItem, QString pin, const std::vector<QUuid>& itemsUuids);
		void errALP4007(QString schema, QString schemaItem, QString afbElement, QUuid itemUuid);
		void errALP4008(QString schema, QString schemaItem, QString schemaItemAfbVersion, QString latesAfbVersion, QUuid itemUuid);
		void errALP4009(QString schema, QString schemaItem, QString ufbElement, QUuid itemUuid);
		void errALP4010(QString schema, QString schemaItem, int schemaItemUfbVersion, int latesUfbVersion, QUuid itemUuid);
		void errALP4011(QString schema, QString schemaItem, QUuid itemUuid);
		void errALP4012(QString schema, QString schemaItem, QString pinCaption, QUuid itemUuid);
		void errALP4013(QString schema, QString schemaItem, QString inPin, QString outPin, QUuid itemUuid);
		void errALP4014(QString schema, QString schemaItem, QString itemType, QUuid itemUuid);
		void errALP4015(QString schema, QString schemaItem, QUuid itemUuid);

		void errALP4016(QString schema, QString lmDecriptionFile);				// File LmDescriptionFile %1 is not found (Schema %2).
		void errALP4017(QString schema, QString lmDecriptionFile, int opCode);
		void errALP4017(QString schema, QString lmDecriptionFile, int opCode, QUuid itemUuid);
		void errALP4018(QString schema, QString equipmentId, QString schemaLmDecriptionFile1, QString moduleLmDecriptionFile2);
		void errALP4019(QString schema, QString schemaItem, QString ufbElement, QUuid itemUuid, QString UfbLmDecriptionFile, QString schemaLmDecriptionFile);

		void errALP4020(QString logicModule);									// There is no any input element in applictaion logic for Logic Module %1.
		void errALP4021(QString logicModule, QString schema1, QString schema2, QString schemaItem1, QString schemaItem2, QString signalStrID, const std::vector<QUuid>& itemsUuids);
		void errALP4022(QString schema);										// Schema does not have logic layer (Schema %1).
		void errALP4023(QString schema, QString pinCaption, QUuid itemUuid);	// UFB schema has duplicate pins %1 (UFB schema %2).
		void errALP4024(QString fileName, QString details);						// Schema details parsing error, filename %1, details string %2.
		void errALP4025(QString schema);										// Duplicate SchemaIDs %1, all schemas (including Monitor, Tuning, etc) must have unique SchemaIDs.

		void errALP4040(QString schema, QString schemaItem, QString busTypeId, QUuid itemUuid);		// Bus Related
		void errALP4041(QString schema, QString schemaItem, QUuid itemUuid);						// Bus Related

		void errALP4060(QString schema, QString schemaItem, QUuid itemUuid);						// Loopback detected
		void errALP4061(QString schema, QString loopbackId, const std::vector<QUuid>& itemUuids);	// Duplicate source of LoopbackID

		void wrnALP4070(QString schema, const std::vector<QUuid>& itemsUuids);						// Schema %1 has %2 commented functional item(s).

		void errALP4080(QString schema, QString pannelSchemaId);									// Schema %1 has join to unknown schema %2.
		void errALP4081(QString schema);															// SchemaID %1 has recursive reference, property Join(Left/Top/Right/Bottom)SchemaID must be distincive from SchemaID.
		void errALP4082(QString schema, QString pannelSchemaId);									// Join schemas with different unit, schemas %1 and %2 must have the same unit.

		// Multichannel pasing errors
		//
		void errALP4130(QString schema, QString schemaItem, QUuid itemUuid);
		void errALP4131(QString schema, QString schemaItem, QUuid itemUuid);
		void errALP4132(QString schema, const std::vector<QUuid>& itemsUuids);
		void errALP4133(QString schema, QString appSignalId, QUuid itemUuid);
		void errALP4134(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid);
		void errALP4135(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid);
		void errALP4136(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid);
		void errALP4137(QString schema, QString schemaItem, QString appSignalId, QString equipmentId, QUuid itemUuid);

		// Connections + multichannel connections
		//
		void errALP4150(QString schema, QString schemaItem, QString connectionId, QString equipmentsIds, QUuid itemUuid);
		void errALP4152(QString schema, QString schemaItem, QString connectionId, QString equipmentsId, QUuid itemUuid);
		void errALP4153(QString schema, QString schemaItem, QUuid itemUuid);			// Multichannel transmitter must have the same number of ConnectionIDs as schema's channel number (number of schema's EquipmentIDs), Logic Schema %1, SchemaItem %2.
		void errALP4154(QString schema, QString schemaItem, QUuid itemUuid);			// Property ConnectionID is empty (LogicSchema %2, SchemaItem %3).

		void errALP4200(QString schema, QString schemaItem, QString varName, QString currentValue, QUuid itemUuid);
		void errALP4201(QString schema, QString schemaItemUfb, QString varName, QUuid itemUuid);

		// ALC			Application logic compiler				5000-5999
		//
		void errALC5000(QString appSignalID, QUuid itemUuid, QString schemaID);		// Signal identifier %1 is not found (Logic schema %2).
		void wrnALC5001(QString logicModuleID);									// Application logic for module %1 is not found.
		void errALC5002(QString schemaID, QString appSignalID, QUuid itemUuid);					// Value of signal %1 is undefined (Logic schema %2).
		void errALC5003(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Analog output %1.%2 is connected to discrete signal %3.
		void errALC5004(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid, QString schemaID);		// Output %1.%2 is connected to signal %3 with uncompatible data format.
		void errALC5005(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Output %1.%2 is connected to signal %3 with uncompatible data size.
		void errALC5006(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Discrete output %1.%2 is connected to analog signal %3.
		void errALC5007(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Discrete signal %1 is connected to analog input %2.%3.
		void errALC5008(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid, const QString &schemaID);		// Signal %1 is connected to input %2.%3 with uncompatible data format. (Logic schema %4)
		void errALC5009(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Signal %1 is connected to input %2.%3 with uncompatible data size.
		void errALC5010(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid, QString schemaID);		// Analog signal %1 is connected to discrete input %2.%3.
		void errALC5011(QString itemLabel, QString schemaId, QUuid itemUuid);	// Application item %1 has unknown type, SchemaID %2.
		void wrnALC5012(QString appSignalID);									// Application signal %1 is not bound to any device object.
		void errALC5013(QString appSignalID, QString equipmentID);				// Application signal %1 is bound to unknown device object %2.
		void errALC5014(QString appSignalID);									// Discrete signal %1 must have DataSize equal to 1.
		void errALC5015(QString appSignalID);									// Analog signal %1 must have DataSize equal to 32.
		void errALC5016(QString appSignalID);									// Application signal identifier %1 is not unique.
		void errALC5017(QString appSignalID);									// Custom application signal identifier %1 is not unique.
		void errALC5018(QString port1, QString port2, QString );			// Opto ports %1 and %2 are not compatible (connection %3).
		void errALC5019(QString port, QString connection1, QString connection2);	// Opto port %1 of connection %2 is already used in connection %3.
		void errALC5020(QString port, QString connection);							// LM's port %1 can't work in RS232/485 mode (connection %2).
		void errALC5021(QString port, QString connection);							// Undefined opto port %1 in the connection %2.
		void errALC5022(QString connection);									// Opto ports of the same chassis is linked via connection %1.
		void errALC5023(QString connection);									// Opto connection ID %1 is not unique.
		void errALC5024(QString connection, QUuid transmitterUuid, QString schemaID);			// Transmitter is linked to unknown opto connection %1 (Logic schema %2).
		void errALC5025(QString connection, QUuid receiverUuid, QString schemaID);				// Receiver is linked to unknown opto connection %1.
		void errALC5026(QUuid transmitterUuid, const QList<QUuid>& signalIDs);	// Transmitter input can be linked to one signal only.
		void errALC5027(QUuid transmitterUuid, QString schemaID);									// All transmitter inputs must be directly linked to a signals.
		void errALC5028(QUuid constUuid, QString schemaID);						// Uncompatible constant type (Logic schema %1).
		void errALC5030(QString appSignalID, QString lmEquipmentID, QUuid signalUuid);		// The signal %1 is not associated with LM %2.
		void errALC5031(QString appSignalID);												// The signal %1 can be bind to Logic Module or Equipment Signal.
		void errALC5032(int txDataSize, QString optoPortID, QString moduleID, int optoPortAppDataSize);		// TxData size (%1 words) of opto port %2 exceed value of OptoPortAppDataSize property of module %3 (%4 words).
		void errALC5033(QString appSignalId, QString chassisEquipmentID);		// Can't find logic module associated with signal %1 (no LM in chassis %2).
		void errALC5034(QUuid transmitterUuid, QUuid connectedItemUuid);		// Non-signal element is connected to transmitter.
		void errALC5035(int rxDataSize, QString optoPortID, QString moduleID, int optoPortAppDataSize);		// RxData size (%1 words) of opto port %2 exceed value of OptoPortAppDataSize property of module %3 (%4 words).
		void errALC5036(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Analog signal %1 is connected to discrete signal %2.
		void errALC5037(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Discrete signal %1 is connected to analog signal %2.
		void errALC5038(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Signals %1 and %2 have different data format.
		void errALC5039(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Signals %1 and %2 have different data size.
		void errALC5040(QString connectionID, QUuid item);									// Connection with ID %1 is not found.
		// 5041
		void errALC5042(QString appSignalID, QString connectionID, QUuid receiverUuid, QString schemaID);		// Signal %1 is not exists in connection %2.
		void errALC5043(QString fbCaption, QString paramCaption, QUuid itemUuid);			// Value of parameter %1.%2 must be greater or equal to 0.
		void errALC5044(QString fbCaption, int opcode, QUuid itemUuid);						// Parameter's calculation for AFB %1 (opcode %2) is not implemented.
		void errALC5045(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Required parameter %1 of AFB %2 is missing.
		void errALC5046(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter %1 of AFB %2 must have type Unsigned Int.
		void errALC5047(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter %1 of AFB %2 must have type 16-bit Unsigned Int.
		void errALC5048(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter %1 of AFB %2 must have type 32-bit Unsigned Int.
		void errALC5049(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter %1 of AFB %2 must have type 32-bit Signed Int.
		void errALC5050(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter %1 of AFB %2 must have type 32-bit Float.
		void errALC5051(int paramValue, QString paramCaption, QString fbCaption, QUuid itemUuid);	// Value %1 of parameter %2 of AFB %3 is incorrect.
		void errALC5052(QString fbCaption, QString param1, QString param2, QUuid itemUuid, QString schemaID, QString itemLabel);			// Value of parameter %1.%2 must be greate then the value of %1.%3.
		void wrnALC5053(QString fbCaption, QUuid itemUuid);									// Automatic sorting of XY points of FB %1 has been performed.
		void errALC5054(QString fbCaption, QString param1, QString param2, QUuid itemUuid);			// Parameters %1 and %2 of AFB %3 can't be equal.
		void wrnALC5055(QString connectionID);												// Optical connection %1 is configured manually.
		void errALC5056(QString subsystemID, QString lmEquipmentID);						// SubsystemID %1 assigned in LM %2 is not found in subsystem list.
		void errALC5057(QString afbCaption, QString afbSignal, QUuid itemUuid);				// Uncompatible data format of analog AFB signal %1.%2.
		void errALC5058(QString paramCaption, QString afbCaption, QUuid itemUuid);			// Parameter %1 of AFB %2 can't be 0.
		void errALC5059(QString schemaID, QString connectionID, QString lmID, QUuid transmitterUuid);			// Ports of connection %1 are not accessible in LM %2
		void errALC5060(QString schemaID, QUuid constantUuid);													// Float constant is connected to discrete input (Logic schema %1).
		void errALC5061(QString schemaID, QUuid constantUuid);													// Float constant is connected to 16-bit input (Logic schema %1).
		void errALC5062(QString schemaID, QUuid constantUuid);													// Float constant is connected to SignedInt input (Logic schema %1).
		void errALC5063(QString schemaID, QUuid constantUuid);													// Integer constant is connected to Float input (Logic schema %1).
		void errALC5064(int address);								// Read address %1 of application memory is out of range 0..65535.
		void errALC5065(int address);								// Write address %1 of application memory is out of range 0..65535.
		void errALC5066(int addrTo, int addrFrom, int sizeW);		// Command MOVEMEM %1, %2, %3 can't write to bit-addressed memory.
		void errALC5067(int addrTo, int bit, int value);			// Command MOVBC %1, %2, #%3 can't write out of application bit- or word-addressed memory.
		void errALC5068(QString appSignalID);						// TuningHighBound property of tunable signal %1 must be greate than TuningLowBound.
		void errALC5069(QString appSignalID);						// TuningDefaultValue property of tunable signal %1 must be in range from TuningLowBound to TuningHighBound.
		void wrnALC5070(QString appSignalID);						// Signal %1 has Little Endian byte order.
		void errALC5071(QString schemaID, QString appSignalID, QUuid itemUuid);					// Can't assign value to tunable signal %1 (Logic schema %2).
		void wrnALC5072(int coefCount, QString coefCaption, QUuid itemUuid, QString schemaID);	// Possible error. AFB 'Poly' CoefCount = %1, but coefficient %2 is not equal to 0 (Logic schema %3).
		void wrnALC5073();											// Usage of code memory exceed 95%.
		void errALC5074();											// Usage of code memory exceed 100%.
		void wrnALC5075();											// Usage of bit-addressed memory exceed 95%.
		void errALC5076();											// Usage of bit-addressed memory exceed 100%.
		void wrnALC5077();											// Usage of word-addressed memory exceed 95%.
		void errALC5078();											// Usage of word-addressed memory exceed 100%.
		void wrnALC5079();											// Usage of IDR phase time exceed 90%.
		void errALC5080();											// Usage of IDR phase time exceed 100%.
		void wrnALC5081();											// Usage of ALP phase time exceed 90%.
		void errALC5082();											// Usage of ALP phase time exceed 100%.
		void errALC5083(QString receiverPortID, QString connectionID, QString lmID, QUuid receiverUuid);	// Receiver of connection %1 (port %2) is not associated with LM %3
		void errALC5085(QString portEquipmentID, QString connectionID);	// Rx data size of RS232/485 port %1 is undefined (connection %2).
		void errALC5086(QUuid constItemUuid, QString schemaID);				// Discrete constant must have value 0 or 1 (Logic schema %1).
		void errALC5087(QString schemaID, QString appSignalID, QUuid itemUuid);		// Can't assign value to input signal %1 (Logic schema %2).
		void errALC5088(QString fbCaption, QString paramCaption, QUuid itemUuid);	// Value of parameter %1.%2 must be greater then 0.
		void errALC5089(int addrTo, int bitTo, int addrFrom, int bitFrom);			// Command MOVB %1[%2], %3[%4] can't write out of application bit- or word-addressed memory.
		void errALC5090(QString appSignalID);						// Analog signal aperture should be greate then 0.
		void errALC5091(QString appSignalID);						// Input/output application signal %1 should be bound to equipment signal.
		void errALC5092(QString busTypeID, QString appSignalID);	// Bus type ID %1 of signal %2 is undefined.
		void wrnALC5093(QString appSignalID);						// Coarse aperture of signal %1 less then fine aperture.
		void errALC5094(QString inBusSignalID, QString busTypeID);	// Size of in bus analog signal %1 is not multiple 16 bits (bus type %2).
		void errALC5095(QString busTypeID);							// The bus size must be a multiple of 2 bytes (1 word) (bus type %1).
		void errALC5096(QString inBusSignalID, QString busTypeID);	// Offset of in bus analog signal % is not multiple of 2 bytes (1 word) (bus type %2).
		void errALC5097(QString signalID1, QString signalID2, QString busTypeID);		// Bus signals %1 and %2 are overlapped (bus type %3).
		void errALC5098(QString signalID, QString busTypeID);							// Bus signal %1 offset out of range (bus type %2).
		void errALC5099(QString busTypeID);												// Bus size must be multiple of 2 bytes (bus type %1).
		void errALC5100(QString busTypeID, QUuid item, QString schemaID);				// Bus type ID %1 is undefined (Logic schema %2).
		void errALC5102(QUuid composer1Guid, QUuid composer2Guid, QString schemaID);	// Output of bus composer can't be connected to input of another bus composer (Logic schema %1).
		void errALC5103(QString signalID, QUuid signalUuid, QUuid composerUuid, QString schemaID);		// Different bus types of bus composer and signal %1 (Logic schema %2).
		void errALC5104(QUuid composerUuid, QString signalID, QUuid signalUuid, QString schemaID);		// Bus composer is connected to non-bus signal %1 (Logic schema %2).
		void errALC5105(QString signalID, QUuid signalUuid, QString schemaID);			// Undefined UAL address of signal %1 (Logic schema %2).
		void errALC5106(QString pinCaption, QUuid schemaItemUuid, QString schemaID);	// Pin with caption %1 is not found in schema item (Logic schema %2).
		void errALC5107(QUuid afbUuid, QUuid transmitterUuid, QString schemaID);		// Afb's output cannot be directly connected to the transmitter. Intermediate app signal should be used.
		void errALC5108(QUuid afbUuid, QString schemaID);								// Cannot identify AFB bus type (Logic schema %1).
		void errALC5109(QUuid afbUuid, QString schemaID);								// Different bus types on AFB inputs (Logic schema %1).
		void errALC5110(QUuid item1, QUuid item2, QString schemaID);					// Non-bus output is connected to bus input.
		void errALC5111(QUuid afbUuid, QString schemaID);								// Output of type 'Bus' is occured  in non-bus processing AFB (Logic schema %1).
		void errALC5112(QUuid uuid1, QUuid uuid2, QString schemaID);					// Different bus types on UAL elements (Logic schema %1).
		void errALC5113(QUuid item1, QUuid item2, QString schemaID);					// Bus output is connected to non-bus input.
		void errALC5114(QString itemCaption, QString inputCaption, QUuid itemUuid, QString schemaID);	// Bus size exceed max bus size of input %1.%2 (Logic schema %3).
		void errALC5115(QUuid uuid1, QUuid uuid2, QString schemaID);					// Uncompatible bus data format of UAL elements (Logic schema %1).
		void errALC5116(QUuid uuid1, QUuid uuid2, QString schemaID);					// Disallowed connection of UAL elements (Logic schema %1).
		void errALC5117(QUuid uuid1, QString label1, QUuid uuid2, QString label2, QString schemaID);	// Uncompatible signals connection (Logic schema %1).
		void errALC5118(QString appSignalID, QUuid itemUuid, QString schemaID);			// Signal %1 is not connected to any signal source. (Logic schema %2).
		void errALC5119(QUuid constItemUuid, QString schemaID);							// Type of Constant is uncompatible with type of linked schema items (Logic schema %1).
		void errALC5120(QUuid ualItemUuid, QString ualItemLabel, QString pin, QString schemaID);			// UalSignal is not found for pin %1 (Logic schema %2).
		void errALC5121(QString appSignalID, QUuid ualItemUuid, QString schemaID);		// Can't assign value to input/tunable/opto/const signal %1 (Logic schema %2).
		void errALC5122(QUuid ualItemUuid, QString schemaID, QString itemLabel);		// Different busTypes on AFB output (Logic schema %1, item %2).
		void errALC5123(QUuid ualItemUuid, QString schemaID, QString itemLabel);		// Different busTypes on AFB inputs (Logic schema %1, item %2).
		void errALC5124(QString appSignalID, QUuid signalUuid, QUuid ualItemUuid, QString schemaID);	// Discrete signal %1 is connected to non-discrete or non-mixed bus input (Logic schema %2)
		void errALC5125(QString pinCaption, QUuid transmitterUuid, QString schemaID);	// Input %1 of transmitter is connected unnamed signal (Logic schema %2).
		void errALC5126(QUuid ualItemUuid, QString schemaID);							// Signal and bus inputs sizes are not multiples  (Logic schema %1).
		void errALC5127(QUuid ualItemUuid, QString itemLabel, QString schemaID);		// Output bus type cannot be determined (Logic schema %1, item %2)
		void errALC5128(QUuid ualItemUuid, QString itemLabel, QString schemaID);		// All AFB's bus inputs connected to discretes (Logic schema %1, item %2).
		void errALC5129(QUuid ualItemUuid, QString itemLabel, QString schemaID);		// Unknown AFB type (opCode) (Logic schema %1, item %2).
		void errALC5130(int maxInstances, QString afbComponentCaption, QUuid ualItemUuid, QString itemLabel, QString schemaID);		// Max instances (%1) of AFB component %2 is used (Logic schema %3, item %4)
		void errALC5131(QString appSignalID, QString portID);							// Tx signal %1 is specified in raw data description of opto port %2 as discrete, but connected signal isn't discrete.
		void errALC5132(QString unresolvedBusList);										// Can't resolve bus interdependencies: %1
		void errALC5133(QString signalEquipmentID, QUuid ualItemUuid, QString itemLabel, QString schemaID);			// Signal with equipmentID %1 is not found (Logic schema %2, item %3).
		void errALC5134(QUuid ualItemUuid, QString itemLabel, QString schemaID);		// Integer constant value out of range (Logic schema %1, item %2)
		void errALC5135(QUuid ualItemUuid, QString itemLabel, QString schemaID);		// Float constant value out of range (Logic schema %1, item %2)
		void errALC5136(QString appSignalID);											// The input (or output) signal %1 can be bind to Equipment Signal only.
		void errALC5137(QString appSignalID, QString property);							// Signal %1 property %2 out of SignedInt32 range.
		void errALC5138(QString appSignalID, QString property);							// Signal %1 property %2 out of Float range.
		void wrnALC5139(QString fbCaption, QString param1, QString param2, QUuid itemUuid, QString schemaID, QString itemLabel);			// Values of parameters %1.%2 and %1.%3 are equal.
		void errALC5140(QString softwareID);											// Undefined ConfigurationService IP-address for software %1.
		void errALC5141(QString fbCaption, QString paramCaption, QString rangeStr, QUuid itemUuid, QString schemaID);	// Value of parameter %1.%2 must be in range %3 (Logic schema %4).
		void errALC5142(QString loopbackSourceID, QUuid loopbackSourceItemUuid, QString schemaID);	// Duplicate loopback source ID %1 (Logic schema %2).
		void errALC5143(QString loopbackID, QUuid loopbackTargetItemUuid, QString schemaID);		// LoopbackSource is not exists for LoopbackTarget with ID %1 (Logic schema %2).
		void errALC5144(QString s1ID, QUuid s1Guid, QString s2ID, QUuid s2Guid, QString lbId, QString schemaID);	// Non compatible signals %1 and %2 are connected to same Loopback %3 (Logic schema %4)
		void errALC5145(QString signalID, QUuid signalGuid, QString schemaID);			// Input signal %1 is connected to LoopbackTarget (Logic schema %2).
		void errALC5146(QString signalID, QUuid signalGuid, QString schemaID);			// Tunable signal %1 is connected to LoopbackTarget (Logic schema %2).
		void errALC5147(QString signalID, QString lbID1, QString lbID2);				// Signal %1 is connected to different LoopbackTargets %2 and %3 (Logic schema %4)
		void wrnALC5148(QString signalID);												// Internal signal %1 is unused.
		void errALC5149(QString chassisEquipmentID);									// LM- or BVB-family module is not found in chassis %1
		void errALC5150(QString monitorID, QString tuningServiceID);					// Monitor %1 cannot be connected to TuningService with enabled SingleLmControl mode.
		void errALC5151(QString busTypeID);												// Bus type %1 has not initialized.
		void errALC5152(QString inBusSignal, QString busTypeID);						// Bus input signal %1 placement is out of bus size (bus type %2).
		void errALC5153(QString signalID, QString inbusSignalID, QString schemaID);		// Unknown conversion of signal %1 to inbus signal %2 (Logic schema %3)
		void errALC5154(QString signalID);												// Associated logic module is not found. Signal %1 cannot be processed.
		void errALC5155(QString validitySignalEquipmentID, QString inputSignalID);		// Linked validity signal with EquipmentID %1 is not found (input signal %2).
		void errALC5156(QString validitySignalID, QString inputSignalID);				// Linked validity signal %1 shoud have Discrete Input type (input signal %2).
		void errALC5157(QString appSignalID);											// Analog signal %1 aperture should be less then 100.
		void errALC5158(QString fbCaption, QString param1, QString param2, QUuid itemUuid, QString schemaID, QString itemLabel);			// Value of parameter %1.%2 must be greater or equal then the value of %1.%3.
		void errALC5159(QUuid itemUuid, QString schemaID, QString moduleID);			// Receiver has no connection ID (Schema %1, module %2)
		void errALC5160(QUuid itemUuid, QString schemaID, QString moduleID);			// Transmitter has no connection ID (Schema %1, module %2)
		void errALC5161(QUuid itemUuid, QString schemaID, QString moduleID);			// Receiver has more than one connections ID (Schema %1, module %2)
		void errALC5162(QString connectionID);											// In single-port connection %1 Port2EquipmentID property is not empty.
		void errALC5163(QString connectionID);											// Port1EquipmentID property is empty in connection %1.
		void errALC5164(QString connectionID);											// Port2EquipmentID property is empty in connection %1.
		void wrnALC5165(QString lmEquipmentID);											// Tuning is enabled for module %1 but tunable signals is not found.
		void errALC5166(QString lmEquipmentID);											// Tunable signals is found in module %1 but tuning is not enabled.
		void wrnALC5167(QString appSignalID);											// Signal %1 is excluded from build.
		void errALC5168(QString flagSignalID,
						QString flagTypeStr,
						QString signalWithFlagID,
						QString alreadyAssignedFlagSignalID,
						QUuid itemUuid,
						QString schemaID);					// Duplicate assigning of signal %1 to flag %2 of signal %3. Signal %4 already assigned to this flag.
		void wrnALC5169(QString setFlagsItemLabel, QUuid itemUuid, QString schemaID);	// No flags assigned on set_flags item %1 (Schema %2)
		void errALC5170(QString lmEquipmentID, QString appSignalID, QUuid itemUuid, QString schemaID);	// LM's %1 native signal %2 can't be received via opto connection (Logic schema %3)
		void errALC5171(QString appSignalID, QString equipmentSignalID);				// Internal application signal %1 cannot be linked to equipment input/output signal %2.
		void errALC5172(QString inputCaption, QString itemLabel, QUuid itemUuid, QString schemaID);			// Non-discrete busses is not allowed on input '%1'. (Item %2, logic schema %3).
		void errALC5173(QString signalCaption, QString fbCaption, QUuid itemUuid);		// Required signal %1 of AFB %2 is missing.
		void errALC5174(QString fbCaption, QUuid itemUuid);								// Required AFB %1 is missing.
		void errALC5175(QString signalID, QString inFormat, QString outFormat);			// Unknown conversion of signal %1 from %2 to %3 format.
		void errALC5176(QString signalID, QString propertyName);						// Specific property %1 is not exists in signal %2
		void wrnALC5177(QString fbCaption, QString paramCaption, QUuid itemUuid, QString schemaID);		// Using value 0.0 for parameter %1.%2 is not recommend.
		void wrnALC5178(QUuid constSignalItemUuid, QUuid setFalgsItemUuid, QString schemaID);		// Setting of flags to a constant signal (Logic schema %1).
		void errALC5179(QString itemCaption, QString signalCaption, QUuid itemUuid, QString schemaID);	// Format of AFB signal %1 is not compatible with any known application signals format
		void errALC5180(QString loopbackID, QString itemLabel, QUuid itemUuid, QString schemaID);		// Loopback source %1 is not connected to any signal source (Item %2, logic schema %3).
		void errALC5181(QString itemLabel, QUuid itemUuid, QString schemaID);			// Loopback source cannot be connected to opto/tunable/input signal (Item %1, logic schema %2).
		void errALC5182(QString appSignalID, QString errMsg);							// App signal %1 macro expanding error: %2
		void errALC5183(QString appSignalID1, QString appSignalID2);					// Non unique CustomAppSignalID after macro expansion in signals %1 and %2
		void errALC5184(QString signalID, QUuid signalUuid, QString schemaID);			// Undefined RegBuf address of signal %1 (Logic schema %2).
		void errALC5185(QString signalID, QUuid signalUuid, QString schemaID);			// Undefined IoBuf address of signal %1 (Logic schema %2).
		void errALC5186(QString appSignalID, QString portEquipmentID);					// Signal %1 is not found (opto port %2 raw data description).
		void errALC5187(QString port1ID, QString port2ID);								// Tx data memory areas of ports %1 and %2 are overlapped.
		void errALC5188(QString appSignalID, QString portID);							// Duplicate signal ID %1 in opto port %2.
		void errALC5189(QString appSignalID, QString portID, QString lmID);				// Tx signal %1 specified in opto port %2 raw data description is not exists in LM %3.
		void errALC5190(QString appSignalID, QString portID, QString lmID);				// Rx signal %1 specified in opto port %2 raw data description is not exists in LM %3.
		void errALC5191(QString appSignalID, QString lmID, QUuid itemID, QString schemaID);		// Single-port Rx signal %1 is not associated with LM %2 .
		void wrnALC5192(QString appSignalID, QString portID, QString connectionID);		// Tx signal %1 is defined in port %2 raw data description isn't connected to transmitter (Connection %3).
		void wrnALC5193(QString appSignalID, QString portID, QString connectionID);		// Rx signal %1 specified in port %2 raw data description isn't assigned to receiver (Connection %3).
		void wrnALC5194(QString port1ID, QString port2ID);								// Tx data memory areas of ports %1 and %2 with manual settings are overlapped.
		void errALC5195(QString itemLabel, QUuid itemID, QString schemaID);							// Named signal isn't connected to set_flags item output. Flags cannot be set. (Item %1, schema %2).

		// firmware writing errors

		void wrnALC5800(QString subsystemID, int uartId);								// Flash memory usage for Subsystem %1, UART %2 exceeds 95%.
		void errALC5801(QString subsystemID, int lmNumber, int uartId);					// Not enough memory to store binary data for Subsystem %1, LM Number: %2, UART ID: %3.

		// internal errors

		void errALC5996(QString errorMsg, QString fileName, int lineNo, QString functionName);	// Internal error! %1. File: %1 Line: %2 Function: %3
		void errALC5997(QString fileName, int lineNo, QString functionName);			// Null pointer occurred! File: %1 Line: %2 Function: %3
		void errALC5998(QString fileName, int lineNo, QString functionName);			// Internal error! File: %1 Line: %2 Function: %3

		void errALC5999(QString compilationProcedureName);								// %1 has been finished with errors.

		// EQP			Equipment issues						6000-6999
		//
		void errEQP6000(QString equipmemtId, QUuid equpmentUuid);
		void errEQP6001(QString equipmemtId, QUuid equipmentUuid1, QUuid equipmentUuid2);
		void errEQP6002(QUuid equipmentUuid, QString equipmentId1, QString equipmentId2);
		void errEQP6003(QString lm1, QString lm2, QString ipAddress, QUuid lm1Uuid, QUuid lm2Uuid);		//	Ethernet adapters of LMs %1 and %2 has duplicate IP address %3.
		void errEQP6004(QString lm, QString lmDescriptionFile, QUuid lmUuid);							//	file lmDescriptionFile is not found.

		void errEQP6005(QString subsystemId);	//	Same SubystemIds in subsystems
		void errEQP6006(int subsystemKey);		//	Same ssKey in subsystems
		void errEQP6007(QString subsystemId);	//	All modules in subsystem must have same type, version and LmDescriptionFile (properties ModuleFamily, ModuleVersion, LmDescriptionFile)

        void errEQP6008(QString equipmentId, QString childEquipmentId, int childPlace); // Child childEquipmentId is not allowed in parent equipmentId
		void errEQP6009(QString equipmemtId, QUuid equpmentUuid);	// Property Place must be 0 (Equipment object %1).

		void errEQP6010(QString equipmemtId);						// Device Object %1 not found.
		void errEQP6011(QString equipmemtId, QString buildStep);	// Device Object %1 not found on %2.

		void errEQP6020(QString lm, QUuid lmUuid);					//	Property lmDescriptionFile is empty

		void errEQP6030(QString profileName, QString errorMessage);	// Applying SimProfile %1 error: %2


		// Subset of EQP -- Generation Software Configuration
		//
		void errEQP6100(QString softwareObjectStrId, QUuid uuid);
		void errEQP6101(QString appSignalID, int unitID);			// Signal %1 has wrong unitID: %2.
		void errEQP6102(QString appSignalID, int sensorType);		// Signal %1 has wrong type of sensor: %2.
		void errEQP6103(QString appSignalID, int outputMode);		// Signal %1 has wrong type of output range mode: %2.
		void errEQP6104(QString appSignalID, int inOutType);		// Signal %1 has wrong input/output type: %2.
		void errEQP6105(QString appSignalID, int byteOrder);		// Signal %1 has wrong order of byte: %2.

		void errEQP6106(QString schemaId, QString tuningClientEquipmentId);						// Schema %1 specified in Tuning Client %2 does not exist.
		void errEQP6107(QString property, QString softwareEquipmentId);							// Error parsing property %1 specified in software %2.
		void errEQP6108(QString appSignalId, QString filter, QString tuningClientEquipmentId);	// Signal %1 specified in filter %2 in Tuning Client %3 does not exist.
		void errEQP6109(QString equipmentId, QString tuningClientEquipmentId);					// Tuning Source %1 specified in Tuning Client %2 does not exist.

		void errEQP6110(QString appSignalID);																						//  Signal %1 has wrong physical low Limit
		void errEQP6111(QString appSignalID);																						//  Signal %1 has wrong physical high Limit
		void errEQP6112(QString appSignalID, double wrongValue, double correctValue, QString unit, int precesion);					//	Signal %1 - low engineering limit mismatch low electrical limit: %2 %4, set low electrical Limit: %3 %4.
		void errEQP6113(QString appSignalID, double wrongValue, double correctValue, QString unit, int precesion);					//  Signal %1 - high engineering limit mismatch high electrical limit: %2 %4, set high electrical Limit: %3 %4.
		void errEQP6114(QString appSignalID);																						//  Signal %1 has wrong R0 (ThermoResistor)
		void errEQP6115(QString appSignalID);																						//  Signal %1 has wrong RLoad (mA)
		void errEQP6116(QString appSignalID, double wrongValue, double lowLimit, double highLinmit, QString unit, int precesion);	//  Signal %1 has wrong low electric limit: %2 %5. Electric limit: %3 .. %4 %5.
		void errEQP6117(QString appSignalID, double wrongValue, double lowLimit, double highLinmit, QString unit, int precesion);	//  Signal %1 has wrong high electric limit: %2 %5. Electric limit: %3 .. %4 %5.
		void errEQP6118(QString appSignalID, double wrongValue, double lowLimit, double highLinmit, QString unit, int precesion);	//  Signal %1 has wrong low engineering limit: %2 %5. Engineering limit: %3 .. %4 %5.
		void errEQP6119(QString appSignalID, double wrongValue, double lowLimit, double highLinmit, QString unit, int precesion);	//  Signal %1 has wrong high engineering limit: %2 %5. Engineering limit: %3 .. %4 %5.

		void errEQP6120(QString sourceAppSignalID, QString destinationAppSignalID);													//  Metrology connection with signals: %1 and %2, has wrong type of connection.
		void errEQP6121(QString appSignalID);																						//  Metrology connections contain a non-existent source signal: %1.
		void errEQP6122(QString appSignalID);																						//  Metrology connections contain a non-existent destination signal: %1.



		void errEQP6200(QString monotorId);							// Monitor (%1) cannot be used for tuning in Safety Project. Clear option in %1.TuningEnable or override behavior in menu Project->Project Properties...->Safety Project.
		void errEQP6201(QString tuningServiceId);					// TuningService (%1) cannot be used for multi LM control in Safety Project. Turn On option %1.SingleLmControl or override behavior in menu Project->Project Properties...->Safety Project.

		void errEQP6210(QString behaviorId, QString softwareObjectStrId);	// Client behavior (%1) specified in %2.BehaviorID does not exist.

	public:
		void addItemsIssues(OutputMessageLevel level, int issueCode, const std::vector<QUuid>& itemsUuids);
		void addItemsIssues(OutputMessageLevel level, int issueCode, const std::vector<QUuid>& itemsUuids, const QString& schemaID);

		void addItemsIssues(OutputMessageLevel level, int issueCode, QUuid itemsUuid);
		void addItemsIssues(OutputMessageLevel level, int issueCode, QUuid itemsUuid, const QString& schemaID);

		void addSchemaIssue(OutputMessageLevel level, int issueCode, const QString& schemaID);

		void clearItemsIssues();

	private:
		static QString issuePTypeToString(IssueType it);

	private:
		BuildIssues* m_buildIssues = nullptr;
	};

}

