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

	/// IssueCode: CMN0010
	///
	/// IssueType: Error
	///
	/// Title: File loading/parsing error, file is damaged or has incompatible format, file name '%1'.
	///
	/// Parameters:
	///		%1 File name
	///
	/// Description:
	///		May occur if a file is damaged or has incompatible format.
	///
	void IssueLogger::errCMN0010(QString fileName)
	{
		LOG_ERROR(IssueType::Common,
				  10,
				  tr("File loading/parsing error, file is damaged or has incompatible format, file name '%1'.")
				  .arg(fileName));
	}


	/// IssueCode: CMN0011
	///
	/// IssueType: Error
	///
	/// Title: Can't create directory '%1'.
	///
	/// Parameters:
	///		%1 Directory name
	///
	/// Description:
	///		Program can't create directory. Check path accessibility.
	///
	void IssueLogger::errCMN0011(QString directory)
	{
		LOG_ERROR(IssueType::Common,
				  11,
				  tr("Can't create directory '%1'.")
				  .arg(directory));
	}


	/// IssueCode: CMN0012
	///
	/// IssueType: Error
	///
	/// Title: Can't create file '%1'.
	///
	/// Parameters:
	///		%1 File name
	///
	/// Description:
	///		Program can't create file. Check path accessibility.
	///
	void IssueLogger::errCMN0012(QString fileName)
	{
		LOG_ERROR(IssueType::Common,
				  12,
				  tr("Can't create file '%1'.")
				  .arg(fileName));
	}


	/// IssueCode: CMN0013
	///
	/// IssueType: Error
	///
	/// Title: Write error of file '%1'.
	///
	/// Parameters:
	///		%1 File name
	///
	/// Description:
	///		Program can't write to file. Probably the file is opened by another application.
	///
	void IssueLogger::errCMN0013(QString fileName)
	{
		LOG_ERROR(IssueType::Common,
				  13,
				  tr("Write error of file '%1'.")
				  .arg(fileName));
	}


	// INT			Internal issues							1000-1999
	//

	/// IssueCode: INT1000
	///
	/// IssueType: Error
	///
	/// Title: Input parameter(s) error, debug info: %1.
	///
	/// Parameters:
	///		%1 Debug information
	///
	/// Description:
	///		Error may occur if function gets wrong input parameters.
	/// In most cases it is an internal software error and it shoud be reported to developers.
	///
	void IssueLogger::errINT1000(QString debugMessage)
	{
		LOG_ERROR(IssueType::Internal,
				  1000,
				  tr("Input parameter(s) error, debug info: %1.")
				  .arg(debugMessage));
	}

	// PDB			Project database issues					2000-2999
	//

	/// IssueCode: PDB2000
	///
	/// IssueType: Warning
	///
	/// Title: The workcopies of the checked out files will be compiled.
	///
	/// Parameters:
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

	/// IssueCode: PDB2001
	///
	/// IssueType: Error
	///
	/// Title: Error of getting file list from the database, parent file ID %1, filter '%2', database message '%3'.
	///
	/// Parameters:
	///			%1 Parent file identifier
	///			%2 Filter
	///			%3 Database message
	///
	/// Description:
	///			May occur if database function getFileList fails or database connection is lost.
	///
	void IssueLogger::errPDB2001(int parentFileId, QString filter, QString databaseMessage)
	{
		LOG_ERROR(IssueType::ProjectDatabase,
				  2001,
				  tr("Error of getting file list from the database, parent file ID %1, filter '%2', database message '%3'.")
				  .arg(parentFileId)
				  .arg(filter)
				  .arg(databaseMessage));
	}

	/// IssueCode: PDB2002
	///
	/// IssueType: Error
	///
	/// Title: Getting file instance error, file ID %1, file name '%2', database message '%3'.
	///
	/// Parameters:
	///			%1 Database file identifier
	///			%2 File Name
	///			%3 Data message
	///
	/// Description:
	///			May occur if database function getLatestVersion fails or database connection is lost.
	///
	void IssueLogger::errPDB2002(int fileId, QString fileName, QString databaseMessage)
	{
		LOG_ERROR(IssueType::ProjectDatabase,
				  2002,
				  tr("Getting file instance error, file ID %1, file name '%2', database message '%3'.")
				  .arg(fileId)
				  .arg(fileName)
				  .arg(databaseMessage));
	}

	// CFG			FSC configuration						3000-3999
	//

	/// IssueCode: CFG3000
	///
	/// IssueType: Error
	///
	/// Title: Property '%1' does not exist in object '%2'.
	///
	/// Parameters:
	///         %1 Property name
	///			%2 Object StrID
	///
	/// Description:
	///			Occurs if a property does not exist an object
	///
	void IssueLogger::errCFG3000(QString propertyName, QString object)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3000,
				  tr("Property '%1' does not exist in object '%2'.")
				  .arg(propertyName)
				  .arg(object));
	}

	/// IssueCode: CFG3001
	///
	/// IssueType: Error
	///
	/// Title: Subsystem '%1' is not found in subsystem set (Logic Moudle '%2').
	///
	///
	/// Parameters:
	///         %1 Subsystem StrID
	///			%2 Module StrID
	///
	/// Description:
	///			Occurs if subsystem in Logic Module does not exist in the project.
	///
	void IssueLogger::errCFG3001(QString subSysID, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3001,
				  tr("Subsystem '%1' is not found in subsystem set (Logic Moudle '%2').")
				  .arg(subSysID)
				  .arg(module));
	}

	/// IssueCode: CFG3002
	///
	/// IssueType: Error
	///
	/// Title: Property '%1' has wrong value (%2), valid range is %3..%4 (module '%5').
	///
	/// Parameters:
	///         %1 Property Name
	///         %2 Property Value
	///         %3 Min Value
	///         %4 Max Value
	///			%5 Module StrID
	///
	/// Description:
	///			Occurs if a property value is out of range
	///
	void IssueLogger::errCFG3002(QString name, int value, int min, int max, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3002,
				  tr("Property '%1'' has wrong value (%2), valid range is %3..%4 (module '%5').")
				  .arg(name)
				  .arg(value)
				  .arg(min)
				  .arg(max)
				  .arg(module));
	}

	/// IssueCode: CFG3003
	///
	/// IssueType: Error
	///
	/// Title: Property System\\Channel (%1) is not unique (Logic Module '%2').
	///
	/// Parameters:
	///         %1 Channel
	///			%2 Module StrID
	///
	/// Description:
	///			Property System\\Channel in Logic Module must be unique.
	///
	void IssueLogger::errCFG3003(int channel, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3003,
				  tr("Property System\\Channel (%1) is not unique (Logic Module '%2').")
				  .arg(channel)
				  .arg(module));
	}


	/// IssueCode: CFG3004
	///
	/// IssueType: Error
	///
	/// Title: Controller '%1' is not found in module '%2'.
	///
	/// Parameters:
	///			%1 Controller StrID
	///         %2 Module StrID
	///
	/// Description:
	///			Controller is not found in a module.
	///
	void IssueLogger::errCFG3004(QString controllerID, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3004,
				  tr("Controller '%1' is not found in module '%2'.")
				  .arg(controllerID)
				  .arg(module));
	}


	/// IssueCode: CFG3005
	///
	/// IssueType: Warning
	///
	/// Title: Signal '%1' is not found in controller '%2'.
	///
	/// Parameters:
	///			%1 Signal StrID
	///         %2 Controller StrID
	///
	/// Description:
	///			Signal was not found in a controller, default values for signal parameters will be used.
	///
	void IssueLogger::wrnCFG3005(QString signalID, QString controllerID)
	{
		LOG_WARNING(IssueType::FscConfiguration,
				  3005,
				  tr("Signal '%1' is not found in controller '%2'.")
				  .arg(signalID)
				  .arg(controllerID));
	}

	/// IssueCode: CFG3006
	///
	/// IssueType: Warning
	///
	/// Title: Signal with place %1 is not found in controller '%2'.
	///
	/// Parameters:
	///			%1 Signal place
	///         %2 Controller StrID
	///
	/// Description:
	///			Signal with specified place is not found in a controller.
	///
	void IssueLogger::wrnCFG3006(int place, QString controllerID)
	{
		LOG_WARNING(IssueType::FscConfiguration,
				  3006,
				  tr("Signal with place %1 is not found in controller '%2'.")
				  .arg(place)
				  .arg(controllerID));
	}


	/// IssueCode: CFG3007
	///
	/// IssueType: Warning
	///
	/// Title: Signal '%1' is not found in Application Signals.
	///
	/// Parameters:
	///         %1 Signal StrID
	///
	/// Description:
	///			Signal is not found in Application Signals.
	///
	void IssueLogger::wrnCFG3007(QString signalID)
	{
		LOG_WARNING(IssueType::FscConfiguration,
				  3007,
				  tr("Signal '%1' is not found in Application Signals.")
				  .arg(signalID));
	}

	/// IssueCode: CFG3008
	///
	/// IssueType: Warning
	///
	/// Title: Software '%1' is not found (Logic Module '%2').
	///
	/// Parameters:
	///			%1 Software StrID
	///         %2 Module StrID
	///
	/// Description:
	///			Software is not found in Equipment.
	///
	void IssueLogger::wrnCFG3008(QString softwareID, QString module)
	{
		LOG_WARNING(IssueType::FscConfiguration,
				  3008,
				  tr("Software '%1' is not found (Logic Module '%2').")
				  .arg(softwareID)
				  .arg(module));
	}


	/// IssueCode: CFG3009
	///
	/// IssueType: Error
	///
	/// Title: Different SpredTolerance values (signal %1: %2; signal %3: %4) for module '%5'.
	///
	/// Parameters:
	///         %1 Signal 1 StrID
	///			%2 SpredTolerance 1
	///         %3 Signal 2 StrID
	///			%4 SpredTolerance 2
	///         %5 Module StrID
	///
	/// Description:
	///			SpredTolerance values should be equal in one channel in AIM module.
	///
	void IssueLogger::errCFG3009(QString signalID1, double spredTolerance1, QString signalID2, double spredTolerance2, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3009,
				  tr("Different SpredTolerance values (signal %1: %2; signal %3: %4) for module '%5'.")
				  .arg(signalID1)
				  .arg(spredTolerance1)
				  .arg(signalID2)
				  .arg(spredTolerance2)
				  .arg(module));
	}

	/// IssueCode: CFG3010
	///
	/// IssueType: Error
	///
	/// Title: Property '%1' has wrong value (%2), valid range is %3..%4 [precision %5](signal '%6').
	///
	/// Parameters:
	///         %1 Property Name
	///         %2 Property Value
	///         %3 Min Value
	///         %4 Max Value
	///         %5 Precision
	///			%6 Signal StrID
	///
	/// Description:
	///			Occurs if a property value is out of range
	///
	void IssueLogger::errCFG3010(QString name, double value, double min, double max, int precision, QString signalID)
	{
		QString sValue = QString::number(value, 'f', precision);
		QString sMin = QString::number(min, 'f', precision);
		QString sMax = QString::number(max, 'f', precision);

		LOG_ERROR(IssueType::FscConfiguration,
				  3010,
				  tr("Property '%1'' has wrong value (%2), valid range is %3..%4 (signal '%6').")
				  .arg(name)
				  .arg(sValue)
				  .arg(sMin)
				  .arg(sMax)
				  .arg(signalID));
	}

	/// IssueCode: CFG3011
	///
	/// IssueType: Error
	///
	/// Title: IP address in property '%1' has undefined value (%2) in controller '%3'.
	///
	/// Parameters:
	///         %1 Address Property Name
	///         %2 Address Property Value
	///         %3 Controller ID
	///
	/// Description:
	///			Occurs if IP address in an Ethernet controller has undefined value
	///
	void IssueLogger::errCFG3011(QString addressProperty, uint address, QString controller)
	{
		quint8 a1 = (address >> 24) & 0xff;
		quint8 a2 = (address >> 16) & 0xff;
		quint8 a3 = (address >> 8) & 0xff;
		quint8 a4 = address & 0xff;

		QString str = QString("%1.%2.%3.%4").arg(a1).arg(a2).arg(a3).arg(a4);

		LOG_ERROR(IssueType::FscConfiguration,
				  3011,
				  tr("IP address in property '%1' has undefined value (%2) in controller '%3'.")
				  .arg(addressProperty)
				  .arg(str)
				  .arg(controller));
	}

	/// IssueCode: CFG3012
	///
	/// IssueType: Error
	///
	/// Title: Port in property '%1' has undefined value (%2) in controller '%3'.
	///
	/// Parameters:
	///         %1 Port Property Name
	///         %2 Port Property Value
	///         %3 Controller ID
	///
	/// Description:
	///			Occurs if port in an Ethernet controller has undefined value
	///
	void IssueLogger::errCFG3012(QString portProperty, uint port, QString controller)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3012,
				  tr("Port in property '%1' has undefined value (%2) in controller '%3'.")
				  .arg(portProperty)
				  .arg(port)
				  .arg(controller));
	}

	/// IssueCode: CFG3013
	///
	/// IssueType: Error
	///
	/// Title: Property '%1' (%2) is %3 property '%4' (%5) in signal '%6'.
	///
	/// Parameters:
	///         %1 Property 1 Name
	///         %2 Property 2 Value
	///         %3 Compare Mode (IssueCompareMode)
	///         %4 Property 1 Name
	///         %5 Property 2 Value
	///			%6 Signal StrID
	///
	/// Description:
	///			Occurs if a property value is out of range
	///
	void IssueLogger::errCFG3013(QString name1, double value1, int compareMode, QString name2, double value2, int precision, QString signalID)
	{
		QString sValue1 = QString::number(value1, 'f', precision);
		QString sValue2 = QString::number(value2, 'f', precision);

		QString mode = "equal to";
		switch (compareMode)
		{
			case IssueCompareMode::Less:
				mode = "less than";
				break;
			case IssueCompareMode::More:
				mode = "more than";
				break;
		}

		LOG_ERROR(IssueType::FscConfiguration,
				  3013,
				  tr("Property '%1' (%2) is %3 property '%4' (%5) in signal '%6'")
				  .arg(name1)
				  .arg(sValue1)
				  .arg(mode)
				  .arg(name2)
				  .arg(sValue2)
				  .arg(signalID));
	}

	/// IssueCode: CFG3014
	///
	/// IssueType: Error
	///
	/// Title: Can't find child object wuith suffix '%1' in object '%2'
	///
	/// Parameters:
	///         %1 Suffix
	///         %2 Object ID
	///
	/// Description:
	///			Occurs if cant't find child object with certain suffix in parent object.
	///
	void IssueLogger::errCFG3014(QString suffix, QString objectID)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3014,
				  tr("Can't find child object wuith suffix '%1' in object '%2'.")
				  .arg(suffix)
				  .arg(objectID));
	}

	/// IssueCode: CFG3015
	///
	/// IssueType: Warning
	///
	/// Title: Property '%1.%2' is linked to undefined software ID '%3'.
	///
	/// Parameters:
	///         %1 Object ID
	///         %2 Property name
	///			%3 Software ID
	///
	/// Description:
	///			Occurs if adapter property is linked to undefined software ID.
	///
	void IssueLogger::wrnCFG3015(QString objectID, QString propertyName, QString softwareID)
	{
		LOG_WARNING(IssueType::FscConfiguration,
				  3015,
				  tr("Property '%1.%2' is linked to undefined software ID '%3'.")
				  .arg(objectID)
				  .arg(propertyName)
				  .arg(softwareID));
	}

	/// IssueCode: CFG3016
	///
	/// IssueType: Warning
	///
	/// Title: Property '%1.%2' is empty.
	///
	/// Parameters:
	///         %1 Object ID
	///         %2 Property name
	///
	/// Description:
	///			Occurs if adapter property is empty.
	///
	void IssueLogger::wrnCFG3016(QString objectID, QString propertyName)
	{
		LOG_WARNING(IssueType::FscConfiguration,
				  3016,
				  tr("Property '%1.%2' is empty.")
				  .arg(objectID)
				  .arg(propertyName));
	}

	/// IssueCode: CFG3017
	///
	/// IssueType: Error
	///
	/// Title: Property '%1.%2' is linked to not compatible software '%3'.
	///
	/// Parameters:
	///         %1 Object ID
	///         %2 Property name
	///			%3 Software ID
	///
	/// Description:
	///			Occurs if adapter property is linked to not compatible software ID.
	///
	void IssueLogger::errCFG3017(QString objectID, QString propertyName, QString softwareID)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3017,
				  tr("Property '%1.%2' is linked to not compatible software '%3'.")
				  .arg(objectID)
				  .arg(propertyName)
				  .arg(softwareID));
	}

	/// IssueCode: CFG3018
	///
	/// IssueType: Warning
	///
	/// Title: Default '%1' IP address %2:%3 is used in controller %4.
	///
	/// Parameters:
	///			%1 Software StrID
	///         %2 Module StrID
	///
	/// Description:
	///			Warning occurs when default IP address and port are used in ethernet controller.
	///
	void IssueLogger::wrnCFG3018(QString propertyName, QString ip, int port, QString controller)
	{
		LOG_WARNING(IssueType::FscConfiguration,
				  3018,
				  tr("Default '%1' IP address %2:%3 is used in controller %4.")
				  .arg(propertyName)
				  .arg(ip)
				  .arg(port)
				  .arg(controller));
	}


	// ALP			Application Logic Parsing				4000-4999
	//

	/// IssueCode: ALP4000
	///
	/// IssueType: Error
	///
	/// Title: Branch has multiple outputs (Logic Schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///		Error may occur if there are more than one output is linked to input.
	///
	void IssueLogger::errALP4000(QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4000,
				  tr("Branch has multiple outputs (Logic Schema '%1').")
				  .arg(schema));
	}

	/// IssueCode: ALP4001
	///
	/// IssueType: Error
	///
	/// Title: Property EquipmentIDs for Logic Schema is not set (LogicSchema '%1').
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///		Property EquipmentIDs for an application logic schema is empty. To bind a schema to a Logic Module this field must be set
	///		to the Logic Module EquipmentID.
	///
	void IssueLogger::errALP4001(QString schema)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4001,
				  tr("Property EquipmentIDs for Logic Schema is not set (LogicSchema '%1').")
				  .arg(schema));
	}

	/// IssueCode: ALP4002
	///
	/// IssueType: Error
	///
	/// Title: EquipmentID '%1' is not found in the project equipment (LogicSchema '%2').
	///
	/// Parameters:
	///		%1 Logic modules StrID
	///		%2 Logic schema StrID
	///
	/// Description:
	///		Logic Schema has property EquipmentIDs but Logic Module with pointed StrID is not found in the project equipment.
	///
	void IssueLogger::errALP4002(QString schema, QString equipmentId)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4002,
				  tr("EquipmentID '%1' is not found in the project equipment (LogicSchema '%2').")
				  .arg(equipmentId)
				  .arg(schema));
	}

	/// IssueCode: ALP4003
	///
	/// IssueType: Error
	///
	/// Title: EquipmentID '%1' must be LM family module type (LogicSchema '%2').
	///
	/// Parameters:
	///		%1 Logic modules StrID
	///		%2 Logic schema StrID
	///
	/// Description:
	///		Logic Schema has property EquipmentIDs but the equipment object with pointed ID is not a module or is not LM family type.
	///
	void IssueLogger::errALP4003(QString schema, QString equipmentId)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4003,
				  tr("EquipmentID '%1' must be LM family module type (LogicSchema '%2').")
				  .arg(equipmentId)
				  .arg(schema));
	}

	/// IssueCode: ALP4004
	///
	/// IssueType: Warning
	///
	/// Title: Schema is excluded from build (Schema '%1').
	///
	/// Parameters:
	///		%1 Schema StrID
	///
	/// Description:
	///			Schema is excluded from build and will not be parsed. To include schema in the build process set
	///		schema property ExcludeFromBuild to false.
	///
	void IssueLogger::wrnALP4004(QString schema)
	{
		LOG_WARNING(IssueType::AlParsing,
					4004,
					tr("Schema is excluded from build (Schem '%1').")
					.arg(schema));
	}

	/// IssueCode: ALP4005
	///
	/// IssueType: Warning
	///
	/// Title: Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///			Logic Schema is empty, there are no any functional blocks in the compile layer.
	///
	void IssueLogger::wrnALP4005(QString schema)
	{
		LOG_WARNING(IssueType::AlParsing,
					4005,
					tr("Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema '%1').")
					.arg(schema));
	}

	/// IssueCode: ALP4006
	///
	/// IssueType: Error
	///
	/// Title: Schema item '%1' has unlinked pin(s) '%2' (Logic Schema '%3').
	///
	/// Parameters:
	///		%1 Schema item description
	///		%2 Pin
	///		%3 Logic schema StrID
	///
	/// Description:
	///		Schema item has unlinked pin(s), all pins of the function block must be linked.
	///
	void IssueLogger::errALP4006(QString schema, QString schemaItem, QString pin, QUuid itemUuid)
	{
		std::vector<QUuid> itemsUuids;
		itemsUuids.push_back(itemUuid);

		return errALP4006(schema, schemaItem, pin, itemsUuids);
	}

	void IssueLogger::errALP4006(QString schema, QString schemaItem, QString pin, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4006,
				  tr("Schema item '%1' has unlinked pin(s) '%2' (Logic Schema '%3').")
				  .arg(schemaItem)
				  .arg(pin)
				  .arg(schema));
	}

	/// IssueCode: ALP4007
	///
	/// IssueType: Error
	///
	/// Title: AFB description '%1' is not found for schema item '%2' (Logic Schema '%3').
	///
	/// Parameters:
	///		%1 Application functional block StrID
	///		%2 Schema item description
	///		%3 Logic schema StrID
	///
	/// Description:
	///		To proccess logic block it is required AFB description which in not found.
	///
	void IssueLogger::errALP4007(QString schema, QString schemaItem, QString afbElement, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4007,
				  tr("AFB description '%1' is not found for schema item '%2' (Logic Schema '%3').")
				  .arg(afbElement)
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4008
	///
	/// IssueType: Error
	///
	/// Title: SchemaItem '%1' has outdated AFB description version, item's AFB.version %2, the latest is %3 (LogicSchema '%4').
	///
	/// Parameters:
	///		%1 Application functional block StrID
	///		%2 Schema item description
	///		%3 Logic schema StrID
	///
	/// Description:
	///		To proccess logic block it is required AFB description which in not found.
	///
	void IssueLogger::errALP4008(QString schema, QString schemaItem, QString schemaItemAfbVersion, QString latesAfbVersion, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4008,
				  tr("SchemaItem '%1' has outdated AFB description version, item's AFB.version %2, the latest is %3 (LogicSchema '%4').")
				  .arg(schemaItem)
				  .arg(schemaItemAfbVersion)
				  .arg(latesAfbVersion)
				  .arg(schema));
	}

	/// IssueCode: ALP4020
	///
	/// IssueType: Error
	///
	/// Title: There is no any input element in applictaion logic for Logic Module '%1'.
	///
	/// Parameters:
	///		%1 Logic module StrID
	///
	/// Description:
	///		Imposible to set execution order for logic items in logic module as there is no any input element.
	///
	void IssueLogger::errALP4020(QString logicModule)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4020,
				  tr("There is no any input element in applictaion logic for Logic Module '%1'.")
				  .arg(logicModule));
	}

	/// IssueCode: ALP4021
	///
	/// IssueType: Error
	///
	/// Title: Duplicate output signal %1, item '%2' on schema '%3', item '%4' on schema '%5' (Logic Module '%6').
	///
	/// Parameters:
	///		%1 Logic signal StrID
	///		%1 Logic module StrID
	///
	/// Description:
	///		Error may occur if there are two or more outputs have the same logic signal StrID.
	/// Note, outputs can be on different logic schemas for the same logic module.
	///
	void IssueLogger::errALP4021(QString logicModule, QString schema1, QString schema2, QString schemaItem1, QString schemaItem2, QString signalStrID, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4021,
				  tr("Duplicate output signal %1, item '%2' on schema '%3', item '%4' on schema '%5' (Logic Module '%6').")
				  .arg(signalStrID)
				  .arg(schemaItem1)
				  .arg(schema1)
				  .arg(schemaItem2)
				  .arg(schema2)
				  .arg(logicModule)
				  );
	}

	/// IssueCode: ALP4022
	///
	/// IssueType: Error
	///
	/// Title: Schema does not have Logic layer (Logic Schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///		Each logic schema has several layers (Logic, Frame and Notes), but the Logic layer is not found.
	///
	void IssueLogger::errALP4022(QString schema)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4022,
				  tr("Schema does not have Logic layer (Logic Schema '%1').").arg(schema));
	}


	// ALC			Application logic compiler				5000-5999
	//

	/// IssueCode: ALC5000
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is not found in Application Signals.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Signal idendifier is not found in application signals.
	///
	void IssueLogger::errALC5000(QString appSignalID, QUuid itemUuid)
	{
		if (itemUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, itemUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5000,
				  tr("Signal '%1' is not found in Application Signals.").arg(appSignalID));
	}

	/// IssueCode: ALC5001
	///
	/// IssueType: Warning
	///
	/// Title: Application logic for module '%1' is not found.
	///
	/// Parameters:
	///		%1 Logic Module (LM) equipment ID
	///
	/// Description:
	///		Application logic for specified module is not found.
	///
	void IssueLogger::wrnALC5001(QString logicModuleID)
	{
		LOG_WARNING(IssueType::AlCompiler,
				  5001,
				  tr("Application logic for module '%1' is not found.").arg(logicModuleID));
	}


	/// IssueCode: ALC5002
	///
	/// IssueType: Error
	///
	/// Title: Value of signal '%1' is undefined.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Signal value can not be calculated
	///
	void IssueLogger::errALC5002(QString appSignalID, QUuid itemUuid)
	{
		if (itemUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, itemUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5002,
				  tr("Value of signal '%1' is undefined.").arg(appSignalID));
	}

	/// IssueCode: ALC5003
	///
	/// IssueType: Error
	///
	/// Title: Analog output '%1.%2' is connected to discrete signal '%3'.
	///
	/// Parameters:
	///		%1 AFB caption
	///		%2 AFB output
	///		%3 Application signal ID
	///
	/// Description:
	///		Analog outpuf of AFB is connected to discrete signal
	///
	void IssueLogger::errALC5003(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5003,
				  tr("Analog output '%1.%2' is connected to discrete signal '%3'.").arg(afbCaption).arg(output).arg(appSignalID));
	}

	/// IssueCode: ALC5004
	///
	/// IssueType: Error
	///
	/// Title: Output '%1.%2' is connected to signal '%3' with uncompatible data format.
	///
	/// Parameters:
	///		%1 AFB caption
	///		%2 AFB output
	///		%3 Application signal ID
	///
	/// Description:
	///		Outpuf of AFB is connected to signal with uncompatible data format.
	///
	void IssueLogger::errALC5004(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5004,
				  tr("Output '%1.%2' is connected to signal '%3' with uncompatible data format.").arg(afbCaption).arg(output).arg(appSignalID));
	}

	/// IssueCode: ALC5005
	///
	/// IssueType: Error
	///
	/// Title: Output '%1.%2' is connected to signal '%3' with uncompatible data size.
	///
	/// Parameters:
	///		%1 AFB caption
	///		%2 AFB output
	///		%3 Application signal ID
	///
	/// Description:
	///		Outpuf of AFB is connected to signal with uncompatible data size.
	///
	void IssueLogger::errALC5005(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5005,
				  tr("Output '%1.%2' is connected to signal '%3' with uncompatible data size.").arg(afbCaption).arg(output).arg(appSignalID));
	}

	/// IssueCode: ALC5006
	///
	/// IssueType: Error
	///
	/// Title: Discrete output '%1.%2' is connected to analog signal '%3'.
	///
	/// Parameters:
	///		%1 AFB caption
	///		%2 AFB output
	///		%3 Application signal ID
	///
	/// Description:
	///		Discrete outpuf of AFB is connected to analog signal
	///
	void IssueLogger::errALC5006(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5006,
				  tr("Discrete output '%1.%2' is connected to analog signal '%3'.").arg(afbCaption).arg(output).arg(appSignalID));
	}


	/// IssueCode: ALC5007
	///
	/// IssueType: Error
	///
	/// Title: Discrete signal '%1' is connected to analog input '%2.%3'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 AFB caption
	///		%3 AFB input
	///
	/// Description:
	///		Discrete signal is connected to analog input of AFB.
	///
	void IssueLogger::errALC5007(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5007,
				  tr("Discrete signal '%1' is connected to analog input '%2.%3'.").arg(appSignalID).arg(afbCaption).arg(input));
	}

	/// IssueCode: ALC5008
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is connected to input '%2.%3' with uncompatible data format.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 AFB caption
	///		%3 AFB input
	///
	/// Description:
	///		Outpuf of AFB is connected to signal with uncompatible data format.
	///
	void IssueLogger::errALC5008(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5008,
				  tr("Signal '%1' is connected to input '%2.%3' with uncompatible data format.").arg(appSignalID).arg(afbCaption).arg(input));
	}


	/// IssueCode: ALC5009
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is connected to input '%2.%3' with uncompatible data size.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 AFB caption
	///		%3 AFB input
	///
	/// Description:
	///		Outpuf of AFB is connected to signal with uncompatible data size.
	///
	void IssueLogger::errALC5009(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5009,
				  tr("Signal '%1' is connected to input '%2.%3' with uncompatible data size.").arg(appSignalID).arg(afbCaption).arg(input));
	}

	/// IssueCode: ALC5010
	///
	/// IssueType: Error
	///
	/// Title: Analog signal '%1' is connected to discrete input '%2.%3'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 AFB caption
	///		%3 AFB input
	///
	/// Description:
	///		Discrete signal is connected to analog input of AFB.
	///
	void IssueLogger::errALC5010(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid)
	{
		if (signalUuid.isNull() == false)
		{
			addItemsIssues(OutputMessageLevel::Error, signalUuid);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5010,
				  tr("Analog signal '%1' is connected to discrete input '%2.%3'.").arg(appSignalID).arg(afbCaption).arg(input));
	}



	// EQP			Equipment issues						6000-6999
	//

	/// IssueCode: EQP6000
	///
	/// IssueType: Error
	///
	/// Title: Property Place is less then 0 (Equipment object '%1').
	///
	/// Parameters:
	///		%1 Equipmnet object StrID
	///
	/// Description:
	///		Property Place for Chassis, Rack, Module, Controller, Signal, Workstation or Software cannot be less then 0.
	///	By default in most cases property Place is -1 to make user to set the correct value.
	///
	void IssueLogger::errEQP6000(QString deviceStrId, QUuid deviceUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, deviceUuid);

		LOG_ERROR(IssueType::Equipment,
				  6000,
				  tr("Property Place is less then 0 (Equipment object '%1').")
				  .arg(deviceStrId)
				  );
	}

	/// IssueCode: EQP6001
	///
	/// IssueType: Error
	///
	/// Title: Two or more equipment objects have the same StrID '%1'.
	///
	/// Parameters:
	///		%1 Equipmnet object StrID
	///
	/// Description:
	///		Error may occur if two or more equipment objects have the same StrID.
	/// All equipmnet objects must have unique StrID.
	///
	void IssueLogger::errEQP6001(QString deviceStrId, QUuid deviceUuid1, QUuid deviceUuid2)
	{
		addItemsIssues(OutputMessageLevel::Error, deviceUuid1);
		addItemsIssues(OutputMessageLevel::Error, deviceUuid2);

		LOG_ERROR(IssueType::Equipment,
				  6001,
				  tr("Two or more equipment objects have the same StrID '%1'.")
				  .arg(deviceStrId)
				  );
	}

	/// IssueCode: EQP6002
	///
	/// IssueType: Error
	///
	/// Title: Two or more equipment objects have the same Uuid '%1' (Object1 '%2', Object2 '%3').
	///
	/// Parameters:
	///		%1 Equipmnet objects Uuid
	///		%2 Equipmnet object StrID 1
	///		%2 Equipmnet object StrID 2
	///
	/// Description:
	///		Error may occur if two or more equipment objects have the same Uuid.
	/// All equipmnet objects must have unique Uuid. In some cases it can be an internal
	/// software error and it shoud be reported to developers.
	///
	void IssueLogger::errEQP6002(QUuid deviceUuid, QString deviceStrId1, QString deviceStrId2)
	{
		addItemsIssues(OutputMessageLevel::Error, deviceUuid);

		LOG_ERROR(IssueType::Equipment,
				  6002,
				  tr("Two or more equipment objects have the same Uuid '%1' (Object1 '%2', Object2 '%3')")
				  .arg(deviceUuid.toString())
				  .arg(deviceStrId1)
				  .arg(deviceStrId2)
				  );
	}

	/// IssueCode: EQP6003
	///
	/// IssueType: Error
	///
	/// Title: Unknown software type (Software object StrID '%1').
	///
	/// Parameters:
	///		%1 Equipmnet object StrID
	///
	/// Description:
	///		Unknown software type. It is required to set proprety Type to the correct value.
	///
	void IssueLogger::errEQP6100(QString softwareObjectStrId, QUuid uuid)
	{
		addItemsIssues(OutputMessageLevel::Error, uuid);

		LOG_ERROR(IssueType::Equipment,
				  6003,
				  tr("Unknown software type (Software object StrID '%1').")
				  .arg(softwareObjectStrId)
				  );
	}

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

	void IssueLogger::addItemsIssues(OutputMessageLevel level, QUuid itemsUuid)
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
