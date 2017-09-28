#ifndef ISSUELOGGER_H
#define ISSUELOGGER_H

#include <map>

#include <QMutex>
#include <QUuid>

#include "../lib/OutputLog.h"

#define LOG_ERROR(type, code, message)		writeError(issuePTypeToString(type) + QString::number(code).rightJustified(4, '0'), message, __FILE__, __LINE__, SHORT_FUNC_INFO);

#define LOG_WARNING0(type, code, message)	writeWarning0(issuePTypeToString(type) + QString::number(code).rightJustified(4, '0'), message, __FILE__, __LINE__, SHORT_FUNC_INFO);
#define LOG_WARNING1(type, code, message)	writeWarning1(issuePTypeToString(type) + QString::number(code).rightJustified(4, '0'), message, __FILE__, __LINE__, SHORT_FUNC_INFO);
#define LOG_WARNING2(type, code, message)	writeWarning2(issuePTypeToString(type) + QString::number(code).rightJustified(4, '0'), message, __FILE__, __LINE__, SHORT_FUNC_INFO);


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

	struct BuildIssues
	{
		// Data
		//
		struct Counter
		{
			int errors = 0;
			int warnings = 0;
		};

		std::map<QUuid, OutputMessageLevel> m_items;		// Item ussuses
		std::map<QString, Counter> m_schemas;				// Item ussuses

		// Methods
		//
		void clear();
		void swap(BuildIssues* buildIssues);

		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids);
		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids, const QString& schemaID);

		void addItemsIssues(OutputMessageLevel level, QUuid itemsUuid);
		void addItemsIssues(OutputMessageLevel level, QUuid itemsUuid, const QString& schemaID);

		void addSchemaIssue(OutputMessageLevel level, const QString& schemaID);
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
		void errCMN0010(QString fileName);						// File loading/parsing error, file is damaged or has incompatible format, file name '%1'.
		void errCMN0011(QString directory);						// Can't create directory '%1'.
		void errCMN0012(QString fileName);						// Can't create file '%1'.
		void errCMN0013(QString fileName);						// Write error of file '%1'.
		void errCMN0014(QString fileName);						// File '%1' already exists.
		void wrnCMN0015(QString fileName1, QString fileName2, QString id);		// '%1' and '%2' files have the same ID = '%3'.
		void errCMN0016();										// The build was cancelled.
		void errCMN0017(QString fileName);						// Can't open file '%1'.


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
		void errPDB2003();										// Load signals from the project database error
		void errPDB2004();										// Load units from the project database error
		void errPDB2005();										// Load UFB schemas from the project database error

		// CFG			FSC configuration						3000-3999
		//
		Q_INVOKABLE void errCFG3000(QString propertyName, QString object);              // general errors
		Q_INVOKABLE void errCFG3001(QString subSysID, QString module);
		Q_INVOKABLE void errCFG3002(QString name, int value, int min, int max, QString module);
		Q_INVOKABLE void errCFG3003(int LMNumber, QString module);
		Q_INVOKABLE void errCFG3004(QString controllerID, QString module);

		Q_INVOKABLE void wrnCFG3005(QString signalID, QString controllerID);
		Q_INVOKABLE void wrnCFG3006(int place, QString controllerID);
		Q_INVOKABLE void wrnCFG3007(QString signalID);
		Q_INVOKABLE void wrnCFG3008(QString softwareID, QString module);      // software errors

		Q_INVOKABLE void errCFG3009(QString signalID1, QString signalID2, QString module);
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

		void errCFG3014(QString suffix, QString objectID);									// Can't find child object with suffix '%1' in object '%2'
		void wrnCFG3015(QString objectID, QString propertyName, QString softwareID);		// Property '%1.%2' is linked to undefined software ID '%3'.
		void wrnCFG3016(QString objectID, QString propertyName);							// Property '%1.%2' is empty.
        void errCFG3017(QString objectID, QString propertyName, QString softwareID);		// Property '%1.%2' is linked to not compatible software ID '%3'.
		Q_INVOKABLE void wrnCFG3018(QString propertyName, QString ip, int port, QString controller);
		void errCFG3019(QString objectID, QString propertyName);							// Property '%1.%2' write error.
		void errCFG3020(QString objectID, QString propertyName);							// Property '%1.%2' is not found.
        void errCFG3021(QString objectID, QString propertyName, QString softwareID);		// Property '%1.%2' is linked to undefined software ID '%3'.
        void errCFG3022(QString objectID, QString propertyName);							// Property '%1.%2' is empty.
		void errCFG3023(QString objectID, QString propertyName);							// Property '%1.%2' conversion error.
		void wrnCFG3024(QString appDataServiceID, QString archServiceID);					// Both data channels of AppDataService '%1' is linked to same ArchivingService '%2'.
		void errCFG3025(QString suffix, QString objectID);									// Can't find child controller with suffix '%1' in object '%2'

		// ALP			Application Logic Parsing				4000-4999
		//
		void errALP4000(QString schema, const std::vector<QUuid>& itemsUuids);
		void errALP4001(QString schema, QString propertyName);
		void errALP4002(QString schema, QString hardwareStrId);
		void errALP4003(QString schema, QString hardwareStrId);
		void wrnALP4004(QString schema);
		void wrnALP4005(QString schema);	// Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema '%1').
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

		void errALP4016(QString schema, QString lmDecriptionFile);
		void errALP4017(QString schema, QString lmDecriptionFile, int opCode);
		void errALP4017(QString schema, QString lmDecriptionFile, int opCode, QUuid itemUuid);
		void errALP4018(QString schema, QString equipmentId, QString schemaLmDecriptionFile1, QString moduleLmDecriptionFile2);
		void errALP4019(QString schema, QString schemaItem, QString ufbElement, QUuid itemUuid, QString UfbLmDecriptionFile, QString schemaLmDecriptionFile);

		void errALP4020(QString logicModule);
		void errALP4021(QString logicModule, QString schema1, QString schema2, QString schemaItem1, QString schemaItem2, QString signalStrID, const std::vector<QUuid>& itemsUuids);
		void errALP4022(QString schema);
		void errALP4023(QString schema, QString pinCaption, QUuid itemUuid);

		void errALP4040(QString schema, QString schemaItem, QString busTypeId, QUuid itemUuid);
		void errALP4041(QString schema, QString schemaItem, QUuid itemUuid);

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

		// ALC			Application logic compiler				5000-5999
		//

		void errALC5000(QString appSignalID, QUuid itemUuid);					// Signal identifier '%1' is not found.
		void wrnALC5001(QString logicModuleID);									// Application logic for module '%1' is not found.
		void errALC5002(QString schemaID, QString appSignalID, QUuid itemUuid);					// Value of signal '%1' is undefined (Logic schema '%2').
		void errALC5003(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Analog output '%1.%2' is connected to discrete signal '%3'.
		void errALC5004(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Output '%1.%2' is connected to signal '%3' with uncompatible data format.
		void errALC5005(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Output '%1.%2' is connected to signal '%3' with uncompatible data size.
		void errALC5006(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid);		// Discrete output '%1.%2' is connected to analog signal '%3'.
		void errALC5007(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Discrete signal '%1' is connected to analog input '%2.%3'.
		void errALC5008(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid, const QString &schemaID);		// Signal '%1' is connected to input '%2.%3' with uncompatible data format. (Schema '%4')
		void errALC5009(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Signal '%1' is connected to input '%2.%3' with uncompatible data size.
		void errALC5010(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid);		// Analog signal '%1' is connected to discrete input '%2.%3'.
		void errALC5011(QString itemLabel, QString schemaId, QUuid itemUuid);	// Application item '%1' has unknown type, SchemaID '%2'.
		void wrnALC5012(QString appSignalID);									// Application signal '%1' is not bound to any device object.
		void errALC5013(QString appSignalID, QString equipmentID);				// Application signal '%1' is bound to unknown device object '%2'.
		void errALC5014(QString appSignalID);									// Discrete signal '%1' must have DataSize equal to 1.
		void errALC5015(QString appSignalID);									// Analog signal '%1' must have DataSize equal to 32.
		void errALC5016(QString appSignalID);									// Application signal identifier '%1' is not unique.
		void errALC5017(QString appSignalID);									// Custom application signal identifier '%1' is not unique.
		void errALC5018(QString port1, QString port2, QString connection);			// Opto ports '%1' and '%2' are not compatible (connection '%3').
		void errALC5019(QString port, QString connection1, QString connection2);	// Opto port '%1' of connection '%2' is already used in connection '%3'.
		void errALC5020(QString port, QString connection);							// LM's port '%1' can't work in RS232/485 mode (connection '%2').
		void errALC5021(QString port, QString connection);							// Undefined opto port '%1' in the connection '%2'.
		void errALC5022(QString connection);									// Opto ports of the same chassis is linked via connection '%1'.
		void errALC5023(QString connection);									// Opto connection ID '%1' is not unique.
		void errALC5024(QString connection, QUuid transmitterUuid, QString schemaID);			// Transmitter is linked to unknown opto connection '%1' (Logic schema '%2').
		void errALC5025(QString connection, QUuid receiverUuid, QString schemaID);				// Receiver is linked to unknown opto connection '%1'.
		void errALC5026(QUuid transmitterUuid, const QList<QUuid>& signalIDs);	// Transmitter input can be linked to one signal only.
		void errALC5027(QUuid transmitterUuid);									// All transmitter inputs must be directly linked to a signals.
		void errALC5028(QString appSignalID, QUuid constUuid, QUuid signalUuid);	// Floating point constant is connected to discrete signal '%1'.
		void errALC5029(QString appSignalID, QString connection, QUuid signalUuid, QUuid transmitterUuid);		// The signal '%1' is repeatedly connected to the transmitter '%2'.
		void errALC5030(QString appSignalID, QString lmEquipmentID, QUuid signalUuid);		// The signal '%1' is not associated with LM '%2'.
		void errALC5031(QString appSignalID);												// The signal '%1' can be bind to Logic Module or Equipment Signal.
		void errALC5032(int txDataSize, QString optoPortID, QString moduleID, int optoPortAppDataSize);		// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
		void errALC5033(QString appSignalId, QString chassisEquipmentID);		// Can't find logic module associated with signal '%1' (no LM in chassis '%2').
		void errALC5034(QUuid transmitterUuid, QUuid connectedItemUuid);		// Non-signal element is connected to transmitter.
		void errALC5035(int rxDataSize, QString optoPortID, QString moduleID, int optoPortAppDataSize);		// RxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
		void errALC5036(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Analog signal '%1' is connected to discrete signal '%2'.
		void errALC5037(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Discrete signal '%1' is connected to analog signal '%2'.
		void errALC5038(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Signals '%1' and '%2' have different data format.
		void errALC5039(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid);		// Signals '%1' and '%2' have different data size.
		void errALC5040(QString connectionID, QUuid item);									// Connection with ID '%1' is not found.
		void errALC5041(QString appSignalID, QString lmID, QUuid receiverUuid);				// Signal '%1' exists in LM '%2'. No receivers needed.
		void errALC5042(QString appSignalID, QString connectionID, QUuid receiverUuid, QString schemaID);		// Signal '%1' is not exists in connection '%2'.
		void errALC5043(QString fbCaption, QString paramCaption, QUuid itemUuid);			// Value of parameter '%1.%2' must be greater or equal to 0.
		void errALC5044(QString fbCaption, int opcode, QUuid itemUuid);						// Parameter's calculation for AFB '%1' (opcode %2) is not implemented.
		void errALC5045(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Required parameter '%1' of AFB '%2' is missing.
		void errALC5046(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter '%1' of AFB '%2' must have type Unsigned Int.
		void errALC5047(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter '%1' of AFB '%2' must have type 16-bit Unsigned Int.
		void errALC5048(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter '%1' of AFB '%2' must have type 32-bit Unsigned Int.
		void errALC5049(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter '%1' of AFB '%2' must have type 32-bit Signed Int.
		void errALC5050(QString paramCaption, QString fbCaption, QUuid itemUuid);			// Parameter '%1' of AFB '%2' must have type 32-bit Float.
		void errALC5051(int paramValue, QString paramCaption, QString fbCaption, QUuid itemUuid);	// Value %1 of parameter '%2' of AFB '%3' is incorrect.
		void errALC5052(QString fbCaption, QString param1, QString param2, QUuid itemUuid);			// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
		void wrnALC5053(QString fbCaption, QUuid itemUuid);									// Automatic sorting of XY points of FB '%1' has been performed.
		void errALC5054(QString fbCaption, QString param1, QString param2, QUuid itemUuid);			// Parameters '%1' and '%2' of AFB '%3' can't be equal.
		void wrnALC5055(QString connectionID);												// Optical connection '%1' is configured manually.
		void errALC5056(QString subsystemID, QString lmEquipmentID);						// SubsystemID '%1' assigned in LM '%2' is not found in subsystem list.
		void errALC5057(QString afbCaption, QString afbSignal, QUuid itemUuid);				// Uncompatible data format of analog AFB signal '%1.%2'.
		void errALC5058(QString paramCaption, QString afbCaption, QUuid itemUuid);			// Parameter '%1' of AFB '%2' can't be 0.
		void errALC5059(QString schemaID, QString connectionID, QString lmID, QUuid transmitterUuid);			// Ports of connection '%1' are not accessible in LM '%2'
		void errALC5060(QString schemaID, QUuid constantUuid);													// Float constant is connected to discrete input (Logic schema '%1').
		void errALC5061(QString schemaID, QUuid constantUuid);													// Float constant is connected to 16-bit input (Logic schema '%1').
		void errALC5062(QString schemaID, QUuid constantUuid);													// Float constant is connected to SignedInt input (Logic schema '%1').
		void errALC5063(QString schemaID, QUuid constantUuid);													// Integer constant is connected to Float input (Logic schema '%1').
		void errALC5064(int address);								// Read address %1 of application memory is out of range 0..65535.
		void errALC5065(int address);								// Write address %1 of application memory is out of range 0..65535.
		void errALC5066(int addrTo, int addrFrom, int sizeW);		// Command 'MOVEMEM %1, %2, %3' can't write to bit-addressed memory.
		void errALC5067(int addrTo, int bit, int value);			// Command 'MOVBC %1, %2, #%3' can't write out of application bit- or word-addressed memory.
		void errALC5068(QString appSignalID);						// LowEngeneeringUnits property of tuningable signal '%1' must be greate than HighEngeneeringUnits.
		void errALC5069(QString appSignalID);						// TuningDefaultValue property of tuningable signal '%1' must be in range from LowEngeneeringUnits to HighEngeneeringUnits.
		void wrnALC5070(QString appSignalID);						// Signal '%1' has Little Endian byte order.
		void errALC5071(QString schemaID, QString appSignalID, QUuid itemUuid);					// Can't assign value to tuningable signal '%1' (Logic schema '%2').
		void wrnALC5072(int coefCount, QString coefCaption, QUuid itemUuid, QString schemaID);	// Possible error. AFB 'Poly' CoefCount = %1, but coefficient '%2' is not equal to 0 (Logic schema %3).
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
		void errALC5083(const QString& receiverPortID, const QString& connectionID, const QString& lmID, QUuid receiverUuid);	// Receiver of connection '%1' (port '%2') is not associated with LM '%3'
		void errALC5084(const QString& appSignalID, const QString& connectionID, QUuid receiverUuid);							// Signal '%1' is not exists in serial connection '%2'. Use PortRawDataDescription to define receiving signals.
		void errALC5085(const QString& portEquipmentID, const QString& connectionID);	// Rx data size of RS232/485 port '%1' is undefined (connection '%2').
		void errALC5086(QUuid constItemUuid, const QString& schemaID);				// Constant connected to discrete signal or FB input must have value 0 or 1.
		void errALC5087(QString schemaID, QString appSignalID, QUuid itemUuid);		// Can't assign value to input signal '%1' (Logic schema '%2').
		void errALC5088(QString fbCaption, QString paramCaption, QUuid itemUuid);	// Value of parameter '%1.%2' must be greater then 0.
		void errALC5089(int addrTo, int bitTo, int addrFrom, int bitFrom);			// Command 'MOVB %1[%2], %3[%4]' can't write out of application bit- or word-addressed memory.
		void errALC5090(QString appSignalID);						// Analog signal aperture should be greate then 0.
		void errALC5091(QString appSignalID);						// Input/output application signal '%1' should be bound to equipment signal.
		void errALC5092(QString busTypeID, QString appSignalID);	// Bus type ID '%1' of signal '%2' is undefined.
		void wrnALC5093(QString appSignalID);						// Coarse aperture of signal '%1' less then fine aperture.
		void errALC5094(QString inBusSignalID, QString busTypeID);	// Size of in bus analog signal '%1' is not multiple 16 bits (bus type '%2').
		void errALC5095(QString busTypeID);							// The bus size must be a multiple of 2 bytes (1 word) (bus type '%1').
		void errALC5096(QString inBusSignalID, QString busTypeID);	// Offset of in bus analog signal '%' is not multiple of 2 bytes (1 word) (bus type '%2').

		void errALC5186(const QString& appSignalID, const QString& portEquipmentID);	// Signal '%1' is not found (opto port '%2' raw data description).
		void errALC5187(const QString& port1ID, const QString & port2ID);				// Tx data memory areas of ports '%1' and '%2' are overlapped.
		void errALC5188(const QString& appSignalID, const QString& portID);				// Duplicate signal ID '%1' in opto port '%2'.
		void errALC5189(const QString& appSignalID, const QString& portID, const QString& lmID);		// Tx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.
		void errALC5190(const QString& appSignalID, const QString& portID, const QString& lmID);		// Rx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.
		void errALC5191(const QString& appSignalID, const QString& lmID, QUuid itemID, const QString& schemaID);		// Serial Rx signal '%1' is not associated with LM '%2' .
		void errALC5192(const QString& appSignalID, const QString& portID, const QString& connectionID);	// Tx signal '%1' is defined in port '%2' raw data description isn't connect to transmitter (Connection '%3').
		void errALC5193(const QString& appSignalID, const QString& portID, const QString& connectionID);	// Rx signal '%1' specified in port '%2' raw data description isn't assigned to receiver (Connection '%3').

		// EQP			Equipment issues						6000-6999
		//
		void errEQP6000(QString equipmemtId, QUuid equpmentUuid);
		void errEQP6001(QString equipmemtId, QUuid equipmentUuid1, QUuid equipmentUuid2);
		void errEQP6002(QUuid equipmentUuid, QString equipmentId1, QString equipmentId2);
		void errEQP6003(QString lm1, QString lm2, QString ipAddress, QUuid lm1Uuid, QUuid lm2Uuid);		//	Ethernet adapters of LMs '%1' and '%2' has duplicate IP address %3.
		void errEQP6004(QString lm, QString lmDescriptionFile, QUuid lmUuid);							//	file lmDescriptionFile is not found.

		void errEQP6005(QString subsystemId);	//	Same SubystemIds in subsystems
		void errEQP6006(int subsystemKey);		//	Same ssKey in subsystems
		void errEQP6007(QString subsystemId);	//	All modules in subsystem must have same type, version and LmDescriptionFile (properties ModuleFamily, ModuleVersion, LmDescriptionFile)

        void errEQP6008(QString equipmentId, QString childEquipmentId, int childPlace); // Child childEquipmentId is not allowed in parent equipmentId
		void errEQP6009(QString equipmemtId, QUuid equpmentUuid);

		// Subset of EQP -- Generation Software Configuration
		//
		void errEQP6100(QString softwareObjectStrId, QUuid uuid);
		void errEQP6101(QString appSignalID, int unitID);			// Signal %1 has wrong unitID: %2.
		void errEQP6102(QString appSignalID, int sensorType);		// Signal %1 has wrong type of sensor: %2.
		void errEQP6103(QString appSignalID, int outputMode);		// Signal %1 has wrong type of output range mode: %2.
		void errEQP6104(QString appSignalID, int inOutType);		// Signal %1 has wrong input/output type: %2.
		void errEQP6105(QString appSignalID, int byteOrder);		// Signal %1 has wrong order of byte: %2.

		void errEQP6106(QString schemaId, QString tuningClientEquipmentId);	//Schema %1 specified in Tuning Client %2 does not exist.


	public:
		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids);
		void addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids, const QString& schemaID);

		void addItemsIssues(OutputMessageLevel level, QUuid itemsUuid);
		void addItemsIssues(OutputMessageLevel level, QUuid itemsUuid, const QString& schemaID);

		void addSchemaIssue(OutputMessageLevel level, const QString& schemaID);

		void swapItemsIssues(BuildIssues* buildIssues);
		void clearItemsIssues();

	private:
		static QString issuePTypeToString(IssueType it);

	private:
		QMutex m_mutex;
		BuildIssues m_buildIssues;
	};

}
#endif // ISSUELOGGER_H
