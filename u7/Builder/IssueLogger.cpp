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


	/// IssueCode: CMN0014
	///
	/// IssueType: Error
	///
	/// Title: File '%1' already exists.
	///
	/// Parameters:
	///		%1 File name
	///
	/// Description:
	///		Program can't create file, because file alredy exists.
	///
	void IssueLogger::errCMN0014(QString fileName)
	{
		LOG_ERROR(IssueType::Common,
				  14,
				  tr("File '%1' already exists.")
				  .arg(fileName));
	}

	/// IssueCode: CMN0015
	///
	/// IssueType: Warning
	///
	/// Title: '%1' and '%2' files have the same ID = '%3'.
	///
	/// Parameters:
	///		%1 File name 1
	///		%2 File name 2
	///		%3 Files identifier
	///
	/// Description:
	///		Build files have same string identifier. Contact to th RPCT developers.
	///
	void IssueLogger::wrnCMN0015(QString fileName1, QString fileName2, QString id)
	{
		LOG_WARNING(IssueType::Common,
				  15,
				  tr("'%1' and '%2' files have the same ID = '%3'.")
				  .arg(fileName1).arg(fileName2).arg(id));
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

	/// IssueCode: INT1001
	///
	/// IssueType: Error
	///
	/// Title: Internal exception: %1.
	///
	/// Parameters:
	///		%1 Debug information
	///
	/// Description:
	///		Error may occur if interanl exception occured. In most cases it is an internal software error
	/// and it shoud be reported to developers.
	///
	void IssueLogger::errINT1001(QString debugMessage)
	{
		LOG_ERROR(IssueType::Internal,
				  1001,
				  tr("Internal exception: %1.")
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

	/// IssueCode: PDB2003
	///
	/// IssueType: Error
	///
	/// Title: Load signals from the project database error.
	///
	/// Parameters:
	///
	/// Description:
	///		May occur if database function getSignals fails or database connection is lost.
	///
	void IssueLogger::errPDB2003()
	{
		LOG_ERROR(IssueType::ProjectDatabase,
				  2003,
				  tr("Load signals from the project database error."));
	}

	/// IssueCode: PDB2004
	///
	/// IssueType: Error
	///
	/// Title: Load units from the project database error.
	///
	/// Parameters:
	///
	/// Description:
	///		May occur if database function getUnits fails or database connection is lost.
	///
	void IssueLogger::errPDB2004()
	{
		LOG_ERROR(IssueType::ProjectDatabase,
				  2004,
				  tr("Load units from the project database error."));
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
	/// Title: Property System\\LMNumber (%1) is not unique in Logic Module '%2'.
	///
	/// Parameters:
	///         %1 LMNumber
	///			%2 Module ID
	///
	/// Description:
	///			Property System\\LMNumber in logic modules for same subsystem must be unique.
	///
	void IssueLogger::errCFG3003(int LMNumber, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3003,
				  tr("Property System\\LMNumber (%1) is not unique in Logic Module '%2'.")
				  .arg(LMNumber)
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

	/// IssueCode: CFG3019
	///
	/// IssueType: Error
	///
	/// Title: Property '%1.%2' write error.
	///
	/// Parameters:
	///			%1 Object equipmentID
	///         %2 Property name
	///
	/// Description:
	///			Error occurs during property writing.
	///
	void IssueLogger::errCFG3019(QString objectID, QString propertyName)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3019,
				  tr("Property '%1.%2' write error.").arg(objectID).arg(propertyName));
	}

	/// IssueCode: CFG3020
	///
	/// IssueType: Error
	///
	/// Title: Property '%1.%2' is not found.
	///
	/// Parameters:
	///			%1 Object equipmentID
	///         %2 Property name
	///
	/// Description:
	///			Property is not found in object.
	///
	void IssueLogger::errCFG3020(QString objectID, QString propertyName)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3020,
				  tr("Property '%1.%2'is not found.").arg(objectID).arg(propertyName));
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


	/// IssueCode: ALP4030
	///
	/// IssueType: Error
	///
	/// Title: Singlechannel Logic Schema '%1' cannot contain multichannel signal block ('%2').
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///		%2 Schema item description
	///
	/// Description:
	///		Singlechannel Logic Schema '%1' cannot contain multichannel signal blocks ('%2'). Only one signal can be assigned for
	/// input/output/internal signal elements.
	///
	void IssueLogger::errALP4030(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4030,
				  tr("Singlechannel Logic Schema '%1' cannot contain multichannel signal block ('%2').")
				  .arg(schema)
				  .arg(schemaItem));

	}


	/// IssueCode: ALP4031
	///
	/// IssueType: Error
	///
	/// Title: Multichannel signal block must have the same number of AppSignalIDs as schema's channel number (number of schema's EquipmentIDs), Logic Schema %1, item %2.
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///		%2 Schema item description
	///
	/// Description:
	///		Multichannel signal block must have the same number of AppSignalIDs as schema's channel number (number of schema's EquipmentIDs), Logic Schema %1, item %2.
	///
	void IssueLogger::errALP4031(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4031,
				  tr("Multichannel signal block must have the same number of AppSignalIDs as schema's channel number (number of schema's EquipmentIDs), Logic Schema %1, item %2.")
				  .arg(schema)
				  .arg(schemaItem));
	}


	/// IssueCode: ALP4032
	///
	/// IssueType: Error
	///
	/// Title: Schema contains mixed singlechannel and multichannel SignalItems in the branch (LogicSchema '%1').
	///
	/// Parameters:
	///		%1 Logic Schema ID
	///
	/// Description:
	///		Schema contains mixed singlechannel and multichannel SignalItems in the branch (LogicSchema '%1').
	/// All Inputs/Outputs/Interconnection Signal elements must be the same type.
	///
	void IssueLogger::errALP4032(QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4032,
				  tr("Schema contains mixed singlechannel and multichannel SignalItems in the branch (LogicSchema '%1').")
				  .arg(schema));
	}

	/// IssueCode: ALP4033
	///
	/// IssueType: Error
	///
	/// Title: Single channel branch contains signals (%1) from different channels (LogicSchema '%2').
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 Logic Schema ID
	///
	/// Description:
	///		Multichannel schema can contain single channel branch, all signals (inputs/outputs/intermediate) in the branch
	/// must be from the same channel.
	///
	void IssueLogger::errALP4033(QString schema, const QString& appSignalId, const QUuid& itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4033,
				  tr("Single channel branch contains signals (%1) from different channels (LogicSchema '%2').")
				  .arg(appSignalId)
				  .arg(schema));
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
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

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
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

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
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5010,
				  tr("Analog signal '%1' is connected to discrete input '%2.%3'.").arg(appSignalID).arg(afbCaption).arg(input));
	}


	/// IssueCode: ALC5011
	///
	/// IssueType: Error
	///
	/// Title: Application item '%1' has unknown type.
	///
	/// Parameters:
	///		%1 Item Uuid
	///
	/// Description:
	///		Application item has unknown type. Contact to the RPCT developers.
	///
	void IssueLogger::errALC5011(QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5011,
				  tr("Application item '%1' has unknown type. Contact to the RPCT developers.").arg(itemUuid.toString()));
	}

	/// IssueCode: ALC5012
	///
	/// IssueType: Warning
	///
	/// Title: Application signal '%1' is not bound to any device object.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Application signal is not bound to any device object. Fill the 'EquipmentID' property of this signal.
	///
	void IssueLogger::wrnALC5012(QString appSignalID)
	{
		LOG_WARNING(IssueType::AlCompiler,
				  5012,
				  tr("Application signal '%1' is not bound to any device object.").arg(appSignalID));
	}

	/// IssueCode: ALC5013
	///
	/// IssueType: Error
	///
	/// Title: Application signal '%1' is bound to unknown device object '%2'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Application signal EquipmentID
	///
	/// Description:
	///		Application signal is bound to unknown device object. Set the 'EquipmentID' property of this signal to correct device object ID.
	///
	void IssueLogger::errALC5013(QString appSignalID, QString equipmentID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5013,
				  tr("Application signal '%1' is bound to unknown device object '%2'.").
					arg(appSignalID).arg(equipmentID));
	}

	/// IssueCode: ALC5014
	///
	/// IssueType: Error
	///
	/// Title: Discrete signal '%1' must have DataSize equal to 1.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Discrete signal has DataSize not equal to 1. Set the 'DataSize' property of this signal to 1.
	///
	void IssueLogger::errALC5014(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5014,
				  tr("Discrete signal '%1' must have DataSize equal to 1.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5015
	///
	/// IssueType: Error
	///
	/// Title: Analog signal '%1' must have DataSize equal to 32.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Analog signal has DataSize not equal to 32. Set the 'DataSize' property of this signal to 32.
	///
	void IssueLogger::errALC5015(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5015,
				  tr("Analog signal '%1' must have DataSize equal to 32.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5016
	///
	/// IssueType: Error
	///
	/// Title: Application signal identifier '%1' is not unique.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Application signal identifier is not unique. Change identifier.
	///
	void IssueLogger::errALC5016(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5016,
				  tr("Application signal identifier '%1' is not unique.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5017
	///
	/// IssueType: Error
	///
	/// Title: Custom application signal identifier '%1' is not unique.
	///
	/// Parameters:
	///		%1 Custom application signal ID
	///
	/// Description:
	///		Custom application signal identifier is not unique. Change identifier.
	///
	void IssueLogger::errALC5017(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5017,
				  tr("Custom application signal identifier '%1' is not unique.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5018
	///
	/// IssueType: Error
	///
	/// Title: Opto ports '%1' and '%2' are not compatible (connection '%3').
	///
	/// Parameters:
	///		%1 Opto port 1 EquipmentID
	///		%2 Opto port 2 EquipmentID
	///		%3 Connection ID
	///
	/// Description:
	///		Opto ports in the connection are not compatible. Only LM-LM or OCM-OCM ports can be connected.
	///		Set correct ports in the connection.
	///
	void IssueLogger::errALC5018(QString port1, QString port2, QString connection)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5018,
				  QString(tr("Opto ports '%1' and '%2' are not compatible (connection '%3').")).
				  arg(port1).arg(port2).arg(connection));
	}

	/// IssueCode: ALC5019
	///
	/// IssueType: Error
	///
	/// Title: Opto port '%1' of connection '%2' is already used in connection '%3'.
	///
	/// Parameters:
	///		%1 Opto port EquipmentID
	///		%2 Connection 1 ID
	///		%3 Connection 2 ID
	///
	/// Description:
	///		The same opto port is used in several connections. Check connections to resolve this problem.
	///
	void IssueLogger::errALC5019(QString port, QString connection1, QString connection2)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5019,
				  QString(tr("Opto port '%1' of connection '%2' is already used in connection '%3'.")).
				  arg(port).arg(connection1).arg(connection2));
	}

	/// IssueCode: ALC5020
	///
	/// IssueType: Error
	///
	/// Title: LM's opto port '%1' can't work in RS232/485 mode (connection '%2').
	///
	/// Parameters:
	///		%1 Opto port EquipmentID
	///		%2 Connection ID
	///
	/// Description:
	///		LM's opto ports can't work in RS232/485 mode. Use OCM's opto ports for RS232/485 mode.
	///
	void IssueLogger::errALC5020(QString port, QString connection)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5020,
				  QString(tr("LM's opto port '%1' can't work in RS232/485 mode (connection '%2').")).
				  arg(port).arg(connection));
	}

	/// IssueCode: ALC5021
	///
	/// IssueType: Error
	///
	/// Title: Undefined opto port '%1' in the connection '%2'.
	///
	/// Parameters:
	///		%1 Opto port EquipmentID
	///		%2 Connection ID
	///
	/// Description:
	///		Undefined opto port in the connection. Check specified port ID.
	///
	void IssueLogger::errALC5021(QString port, QString connection)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5021,
				  QString(tr("Undefined opto port '%1' in the connection '%2'.")).
				  arg(port).arg(connection));
	}

	/// IssueCode: ALC5022
	///
	/// IssueType: Error
	///
	/// Title: Opto ports of the same chassis is linked via connection '%1'.
	///
	/// Parameters:
	///		%1 Connection ID
	///
	/// Description:
	///		Connection of the opto ports of the same chassis is not allowed. Check the connection settings.
	///
	void IssueLogger::errALC5022(QString connection)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5022,
				  QString(tr("Opto ports of the same chassis is linked via connection '%1'.")).
				  arg(connection));
	}

	/// IssueCode: ALC5023
	///
	/// IssueType: Error
	///
	/// Title: Opto connection caption '%1' is not unique.
	///
	/// Parameters:
	///		%1 Connection ID
	///
	/// Description:
	///		Opto connection caption is not unique. Change caption.
	///
	void IssueLogger::errALC5023(QString connection)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5023,
				  QString(tr("Opto connection caption '%1' is not unique.")).
				  arg(connection));
	}

	/// IssueCode: ALC5024
	///
	/// IssueType: Error
	///
	/// Title: Transmitter is linked to unknown opto connection '%1'.
	///
	/// Parameters:
	///		%1 Connection ID
	///		%2 Transmitter Uuid
	///
	/// Description:
	///		Transmitter is linked to unknown opto connection. Check transmitter's 'ConnectionID' property.
	///
	void IssueLogger::errALC5024(QString connection, QUuid transmitterUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5024,
				  QString(tr("Transmitter is linked to unknown opto connection '%1'.")).
				  arg(connection));
	}

	/// IssueCode: ALC5025
	///
	/// IssueType: Error
	///
	/// Title: Receiver is linked to unknown opto connection '%1'.
	///
	/// Parameters:
	///		%1 Connection ID
	///		%2 Receiver Uuid
	///
	/// Description:
	///		Receiver is linked to unknown opto connection. Check receiver's 'ConnectionID' property.
	///
	void IssueLogger::errALC5025(QString connection, QUuid transmitterUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5025,
				  QString(tr("Receiver is linked to unknown opto connection '%1'.")).
				  arg(connection));
	}

	/// IssueCode: ALC5026
	///
	/// IssueType: Error
	///
	/// Title: Transmitter input can be linked to one signal only.
	///
	/// Parameters:
	///		%1 Transmitter Uuid
	///		%2 Signals Uuid list
	///
	/// Description:
	///		Transmitter input can be linked to one signal only. Check transmitter's inputs links.
	///
	void IssueLogger::errALC5026(QUuid transmitterUuid, const QList<QUuid>& signalIDs)
	{
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid);

		for(QUuid signalID : signalIDs)
		{
			addItemsIssues(OutputMessageLevel::Error, signalID);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5026,
				  QString(tr("Transmitter input can be linked to one signal only.")));
	}

	/// IssueCode: ALC5027
	///
	/// IssueType: Error
	///
	/// Title: All transmitter inputs must be directly linked to a signals.
	///
	/// Parameters:
	///		%1 Transmitter Uuid
	///
	/// Description:
	///		All transmitter inputs must be directly linked to a signals. Check transmitter's inputs links.
	///
	void IssueLogger::errALC5027(QUuid transmitterUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5027,
				  QString(tr("All transmitter inputs must be directly linked to a signals.")));
	}

	/// IssueCode: ALC5028
	///
	/// IssueType: Error
	///
	/// Title: Floating point constant is connected to discrete signal '%1'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Constant Uuid
	///		%3 Signal Uuid
	///
	/// Description:
	///		Floating point constant is connected to discrete signal. Change property 'Type' of the constant to 'IntegerType' value.
	///
	void IssueLogger::errALC5028(QString appSignalID, QUuid constUuid, QUuid signalUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, constUuid);
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5028,
				  QString(tr("Floating point constant is connected to discrete signal '%1'.").arg(appSignalID)));
	}

	/// IssueCode: ALC5029
	///
	/// IssueType: Error
	///
	/// Title: The signal '%1' is repeatedly connected to the transmitter '%2'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Connection ID
	///		%3 Signal Uuid
	///		%4 Transmitter Uuid
	///
	/// Description:
	///		The same signal can be connected only once to the transmitter. Check transmitter input links.
	///
	void IssueLogger::errALC5029(QString appSignalID, QString connection, QUuid signalUuid, QUuid transmitterUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, signalUuid);
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5029,
				  QString(tr("The signal '%1' is repeatedly connected to the transmitter '%2'.").
						  arg(appSignalID).arg(connection)));
	}

	/// IssueCode: ALC5030
	///
	/// IssueType: Error
	///
	/// Title: The signal '%1' is not associated with LM '%2'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic module equipmentID
	///		%3 Signal Uuid
	///
	/// Description:
	///		The signal is not associated with logic module. Set the correct value of signal's EquipmentID property.
	///
	void IssueLogger::errALC5030(QString appSignalID, QString lmEquipmentID, QUuid signalUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5030,
				  QString(tr("The signal '%1' is not associated with LM '%2'.").
						  arg(appSignalID).arg(lmEquipmentID)));
	}

	/// IssueCode: ALC5031
	///
	/// IssueType: Error
	///
	/// Title: The signal '%1' can be bind only to Logic Module or Equipment Signal.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		The signal bind to uncorrect equpment. Set the correct value of signal's EquipmentID property.
	///
	void IssueLogger::errALC5031(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5031,
				  QString(tr("The signal '%1' can be bind only to Logic Module or Equipment Signal.").
						  arg(appSignalID)));
	}


	/// IssueCode: ALC5032
	///
	/// IssueType: Error
	///
	/// Title: TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
	///
	/// Parameters:
	///		%1 opto port txData size, words
	///		%2 opto port equipmentID
	///		%3 opto module equipmentID
	///		%4 value of OptoPortAppDataSize of the opto module
	///
	/// Description:
	///		The signal bind to uncorrect equpment. Set the correct value of signal's EquipmentID property.
	///
	void IssueLogger::errALC5032(int txDataSize, QString optoPortID, QString moduleID, int optoPortAppDataSize)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5032,
				  QString(tr("TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).")).
						  arg(txDataSize).arg(optoPortID).arg(moduleID).arg(optoPortAppDataSize));
	}


	/// IssueCode: ALC5033
	///
	/// IssueType: Error
	///
	/// Title: Can't find logic module associated with signal '%1' (no LM in chassis '%2').
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Chassis equipmet ID
	///
	/// Description:
	///		Can't find logic module associated with signal. Set the correct value of signal's EquipmentID property.
	///
	void IssueLogger::errALC5033(QString appSignalID, QString chassisEquipmentID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5033,
				  QString(tr("Can't find logic module associated with signal '%1' (no LM in chassis '%2').").
						  arg(appSignalID).arg(chassisEquipmentID)));
	}

	/// IssueCode: ALC5034
	///
	/// IssueType: Error
	///
	/// Title: Non-signal element is connected to transmitter.
	///
	/// Parameters:
	///		%1 Transmitter Uuid
	///		%2 Connected item Uuid
	///
	/// Description:
	///		Non-signal element is connected to transmitter. Check connections on logic schema.
	///
	void IssueLogger::errALC5034(QUuid transmitterUuid, QUuid connectedItemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid);
		addItemsIssues(OutputMessageLevel::Error, connectedItemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5034,
				  tr("Non-signal element is connected to transmitter."));
	}


	/// IssueCode: ALC5035
	///
	/// IssueType: Error
	///
	/// Title: RxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
	///
	/// Parameters:
	///		%1 opto port rxData size, words
	///		%2 opto port equipmentID
	///		%3 opto module equipmentID
	///		%4 value of OptoPortAppDataSize of the opto module
	///
	/// Description:
	///		The signal bind to uncorrect equpment. Set the correct value of signal's EquipmentID property.
	///
	void IssueLogger::errALC5035(int rxDataSize, QString optoPortID, QString moduleID, int optoPortAppDataSize)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5035,
				  QString(tr("RxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).")).
						  arg(rxDataSize).arg(optoPortID).arg(moduleID).arg(optoPortAppDataSize));
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
