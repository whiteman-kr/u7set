#include "IssueLogger.h"

namespace Builder
{
	//
	//
	//				BuildIssues
	//
	//
	void BuildIssues::clear()
	{
		m_items.clear();
		m_schemas.clear();
	}

	void BuildIssues::swap(BuildIssues* buildIssues)
	{
		if (buildIssues == nullptr)
		{
			assert(buildIssues);
			return;
		}

		std::swap(m_items, buildIssues->m_items);
		std::swap(m_schemas, buildIssues->m_schemas);
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids)
	{
		for (auto id : itemsUuids)
		{
			m_items[id] = level;
		}
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids, const QString& schemaID)
	{
		for (auto id : itemsUuids)
		{
			m_items[id] = level;
			addSchemaIssue(level, schemaID);
		}
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, QUuid itemsUuid)
	{
		m_items[itemsUuid] = level;
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, QUuid itemsUuid, const QString& schemaID)
	{
		m_items[itemsUuid] = level;
		addSchemaIssue(level, schemaID);
	}

	void BuildIssues::addSchemaIssue(OutputMessageLevel level, const QString& schemaID)
	{
		if (level == OutputMessageLevel::Error)
		{
			m_schemas[schemaID].errors ++;
			return;
		}

		if (level == OutputMessageLevel::Warning0 ||
			level == OutputMessageLevel::Warning1 ||
			level == OutputMessageLevel::Warning2)
		{
			m_schemas[schemaID].warnings ++;
			return;
		}
	}

	//
	//
	//				IssueLogger
	//
	//
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
		LOG_WARNING0(IssueType::Common,
				  15,
				  tr("'%1' and '%2' files have the same ID = '%3'.")
				  .arg(fileName1).arg(fileName2).arg(id));
	}

	/// IssueCode: CMN0016
	///
	/// IssueType: Error
	///
	/// Title: The build was cancelled.
	///
	/// Parameters:
	///
	/// Description:
	///		The build was cancelled by user.
	///
	void IssueLogger::errCMN0016()
	{
		LOG_ERROR(IssueType::Common,
				  16,
				  tr("The build was cancelled."));
	}

	/// IssueCode: CMN0017
	///
	/// IssueType: Error
	///
	/// Title: Can't open file '%1'.
	///
	/// Parameters:
	///		%1 File name
	///
	/// Description:
	///		Program can't open file. Check path accessibility.
	///
	void IssueLogger::errCMN0017(QString fileName)
	{
		LOG_ERROR(IssueType::Common,
				  17,
				  tr("Can't open file '%1'.")
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

	void IssueLogger::errINT1001(QString debugMessage, QString schema)
	{
		addSchemaIssue(OutputMessageLevel::Error, schema);

		LOG_ERROR(IssueType::Internal,
				  1001,
				  tr("Internal exception, schema %1: %2.")
					.arg(schema)
					.arg(debugMessage));
	}

	void IssueLogger::errINT1001(QString debugMessage, QString schema, QUuid itemsUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuid, schema);

		LOG_ERROR(IssueType::Internal,
				  1001,
				  tr("Internal exception, schema %1: %2.")
					.arg(schema)
					.arg(debugMessage));
	}

	void IssueLogger::errINT1001(QString debugMessage, QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids, schema);

		LOG_ERROR(IssueType::Internal,
				  1001,
				  tr("Internal exception, schema %1: %2.")
					.arg(schema)
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
		LOG_WARNING2(IssueType::ProjectDatabase,
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

	/// IssueCode: PDB2005
	///
	/// IssueType: Error
	///
	/// Title: Load UFB schemas from the project database error.
	///
	/// Parameters:
	///
	/// Description:
	///		Load UFB schemas from the project database error. Can occur on database connection lost or schema has incompatible format.
	///
	void IssueLogger::errPDB2005()
	{
		LOG_ERROR(IssueType::ProjectDatabase,
				  2005,
				  tr("Load UFB schemas from the project database error."));
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
	///			Occurs if a property does not exist in an object
	///
	void IssueLogger::errCFG3000(QString propertyName, QString object)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3000,
				  tr("Property '%1' does not exist in an object '%2'.")
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
		LOG_WARNING0(IssueType::FscConfiguration,
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
		LOG_WARNING0(IssueType::FscConfiguration,
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
		LOG_WARNING1(IssueType::FscConfiguration,
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
		LOG_WARNING1(IssueType::FscConfiguration,
				  3008,
				  tr("Software '%1' is not found (Logic Module '%2').")
				  .arg(softwareID)
				  .arg(module));
	}


	/// IssueCode: CFG3009
	///
	/// IssueType: Error
	///
	/// Title: Analog inputs SpreadTolerance mismatch, signals %1 and %2 in module '%3. SpreadTolerance, ADC limits, Engineering Units limits, Valid Range limits must be same for both signals.
	///
	/// Parameters:
	///         %1 Signal 1 StrID
	///         %2 Signal 2 StrID
	///         %3 Module StrID
	///
	/// Description:
	///			SpreadTolerance ADC values should be equal in channel A and channel B in AIM module.
	///
	void IssueLogger::errCFG3009(QString signalID1, QString signalID2, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3009,
				  tr("Analog inputs SpreadTolerance mismatch, signals %1 and %2 in module '%3'. SpreadTolerance, ADC limits, Engineering Units limits, Valid Range limits must be same for both signals.")
				  .arg(signalID1)
				  .arg(signalID2)
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
	/// Title: Can't find child object with suffix '%1' in object '%2'
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
				  tr("Can't find child object with suffix '%1' in object '%2'.")
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
		LOG_WARNING1(IssueType::FscConfiguration,
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
		LOG_WARNING1(IssueType::FscConfiguration,
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
		LOG_WARNING2(IssueType::FscConfiguration,
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
				  tr("Property '%1.%2' is not found.").arg(objectID).arg(propertyName));
	}

    /// IssueCode: CFG3021
    ///
    /// IssueType: Error
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
    void IssueLogger::errCFG3021(QString objectID, QString propertyName, QString softwareID)
    {
        LOG_ERROR(IssueType::FscConfiguration,
                  3021,
                  tr("Property '%1.%2' is linked to undefined software ID '%3'.")
                  .arg(objectID)
                  .arg(propertyName)
                  .arg(softwareID));
    }

    /// IssueCode: CFG3022
    ///
    /// IssueType: Error
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
    void IssueLogger::errCFG3022(QString objectID, QString propertyName)
    {
        LOG_ERROR(IssueType::FscConfiguration,
                  3022,
                  tr("Property '%1.%2' is empty.")
                  .arg(objectID)
                  .arg(propertyName));
    }

	/// IssueCode: CFG3023
	///
	/// IssueType: Error
	///
	/// Title: Property '%1.%2' conversion error.
	///
	/// Parameters:
	///			%1 Object equipmentID
	///         %2 Property name
	///
	/// Description:
	///			Error occurs during property conversion.
	///
	void IssueLogger::errCFG3023(QString objectID, QString propertyName)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3023,
				  tr("Property '%1.%2' conversion error.").arg(objectID).arg(propertyName));
	}

	/// IssueCode: CFG3024
	///
	/// IssueType: Warning
	///
	/// Title: Both data channels of AppDataService '%1' is linked to same ArchivingService '%2'.
	///
	/// Parameters:
	///         %1 AppDataService equipmentID
	///         %2 ArchivingService equipmentID
	///
	/// Description:
	///			Both data channels of specified AppDataService is linked to same ArchivingService. Check settings of AppDataService.
	///
	void IssueLogger::wrnCFG3024(QString appDataServiceID, QString archServiceID)
	{
		LOG_WARNING1(IssueType::FscConfiguration,
				  3024,
				  tr("Both data channels of AppDataService '%1' is linked to same ArchivingService '%2'.")
				  .arg(appDataServiceID)
				  .arg(archServiceID));
	}

	/// IssueCode: CFG3025
	///
	/// IssueType: Error
	///
	/// Title: Can't find child controller with suffix '%1' in object '%2'
	///
	/// Parameters:
	///         %1 Suffix
	///         %2 Object ID
	///
	/// Description:
	///			Occurs if cant't find child controller with certain suffix in parent object.
	///
	void IssueLogger::errCFG3025(QString suffix, QString objectID)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3025,
				  tr("Can't find child controller with suffix '%1' in object '%2'.")
				  .arg(suffix)
				  .arg(objectID));
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
		addItemsIssues(OutputMessageLevel::Error, itemsUuids, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4000,
				  tr("Branch has multiple outputs (Logic Schema '%1').")
				  .arg(schema));
	}

	/// IssueCode: ALP4001
	///
	/// IssueType: Error
	///
	/// Title: Property %1 for Schema is not set (LogicSchema '%2').
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///		Some property for an application logic or user functional block schema is empty.
	///		to the Logic Module EquipmentID.
	///
	void IssueLogger::errALP4001(QString schema, QString propertyName)
	{
		addSchemaIssue(OutputMessageLevel::Error, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4001,
				  tr("Property %1 for Schema is not set (LogicSchema '%2').")
				  .arg(propertyName)
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
		addSchemaIssue(OutputMessageLevel::Error, schema);

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
		addSchemaIssue(OutputMessageLevel::Error, schema);

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
	/// Title: Schema '%1' is excluded from build.
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
		addSchemaIssue(OutputMessageLevel::Warning1, schema);

		LOG_WARNING1(IssueType::AlParsing,
					4004,
					tr("Schema '%1' is excluded from build.")
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
		addSchemaIssue(OutputMessageLevel::Warning2, schema);

		LOG_WARNING2(IssueType::AlParsing,
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
		addItemsIssues(OutputMessageLevel::Error, itemsUuids, schema);

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
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

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
	///		To proccess logic block it is required AFB description which in not found. Open schema to upfate AFBs.
	///
	void IssueLogger::errALP4008(QString schema, QString schemaItem, QString schemaItemAfbVersion, QString latesAfbVersion, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4008,
				  tr("SchemaItem '%1' has outdated AFB description version, item's AFB.version %2, the latest is %3 (LogicSchema '%4').")
				  .arg(schemaItem)
				  .arg(schemaItemAfbVersion)
				  .arg(latesAfbVersion)
				  .arg(schema));
	}

	/// IssueCode: ALP4009
	///
	/// IssueType: Error
	///
	/// Title: UFB schema '%1' is not found for schema item '%2' (Logic Schema '%3').
	///
	/// Parameters:
	///		%1 UFB Schema ID
	///		%2 Schema item description
	///		%3 Logic schema StrID
	///
	/// Description:
	///		To proccess logic block it is required UFB schema which in not found.
	///
	void IssueLogger::errALP4009(QString schema, QString schemaItem, QString ufbElement, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4009,
				  tr("UFB schema '%1' is not found for schema item '%2' (Logic Schema '%3').")
				  .arg(ufbElement)
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4010
	///
	/// IssueType: Error
	///
	/// Title: SchemaItem '%1' has outdated UFB version, item's UFB.version %2, the latest is %3 (LogicSchema '%4').
	///
	/// Parameters:
	///		%1 UFB Schema ID
	///		%2 Schema item description
	///		%3 Logic schema StrID
	///
	/// Description:
	///		SchemaItem '%1' has outdated UFB version, item's UFB.version %2, the latest is %3 (LogicSchema '%4'). Open schema to upfate AFBs.
	///
	void IssueLogger::errALP4010(QString schema, QString schemaItem, int schemaItemUfbVersion, int latesUfbVersion, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4010,
				  tr("SchemaItem '%1' has outdated UFB version, item's UFB.version %2, the latest is %3 (LogicSchema '%4').")
				  .arg(schemaItem)
				  .arg(schemaItemUfbVersion)
				  .arg(latesUfbVersion)
				  .arg(schema));
	}

	/// IssueCode: ALP4011
	///
	/// IssueType: Error
	///
	/// Title: User Functional Block cannot have nested another UFB, SchemaItem %1 (UfbSchema '%2').
	///
	/// Parameters:
	///		%1 UFB SchemaItem
	///		%2 UFB Schema ID
	///
	/// Description:
	///		User Functional Block cannot have nested another User Functional Block.
	///
	void IssueLogger::errALP4011(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4011,
				  tr("User Functional Block cannot have nested another UFB, SchemaItem %1 (UfbSchema '%2').")
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4012
	///
	/// IssueType: Error
	///
	/// Title: Cannot find %1 input/output in UFB %2, SchemaItem %1 (LogicSchema  '%3').
	///
	/// Parameters:
	///		%1 UFB SchemaItem
	///		%2 UFB Schema ID
	///
	/// Description:
	///		User Functional Block cannot have nested another User Functional Block.
	///
	void IssueLogger::errALP4012(QString schema, QString schemaItem, QString pinCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4012,
				  tr("Cannot find %1 input/output in UFB %2, SchemaItem %1 (LogicSchema '%3').")
				  .arg(pinCaption)
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4013
	///
	/// IssueType: Error
	///
	/// Title: Empty loop with UFB detected. UFB contains in to out direct link, on Logic Schema these pins also have direct connection, SchemaItem %1, in %2, out %3 (LogicSchema %4).
	///
	/// Parameters:
	///		%1 UFB SchemaItem
	///		%2 UFB SchemaItem input pin caption
	///		%3 UFB SchemaItem output pin caption
	///		%4 UFB SchemaID
	///
	/// Description:
	///		Empty loop with UFB detected. UFB contains in to out direct link, on Logic Schema these pins also have direct connection.
	///
	void IssueLogger::errALP4013(QString schema, QString schemaItem, QString inPin, QString outPin, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4013,
				  tr("Empty loop with UFB detected. UFB contains in to out direct link, on Logic Schema these pins also have direct connection, SchemaItem %1, in %2, out %3 (LogicSchema %4).")
				  .arg(schemaItem)
				  .arg(inPin)
				  .arg(outPin)
				  .arg(schema));
	}

	/// IssueCode: ALP4014
	///
	/// IssueType: Error
	///
	/// Title: User Functional Block cannot contain %1, SchemaItem %2 (UfbSchema '%3').
	///
	/// Parameters:
	///		%1 SchemaItem type
	///		%2 UFB SchemaItem
	///		%3 UFB SchemaID
	///
	/// Description:
	///		User Functional Block can contain only allowed types of items.
	///
	void IssueLogger::errALP4014(QString schema, QString schemaItem, QString itemType, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4014,
				  tr("User Functional Block cannot contain %1, SchemaItem %2 (UfbSchema '%3').")
				  .arg(itemType)
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4015
	///
	/// IssueType: Error
	///
	/// Title: UFB Input or Output item must have only ONE assigned AppSignalIDs, SchemaItem %1 (UfbSchema '%2').
	///
	/// Parameters:
	///		%1 UFB SchemaItem
	///		%2 UFB SchemaID
	///
	/// Description:
	///		UFB Input or Output item must have ONE assigned AppSignalIDs which are used as pins on SchemaItem UFB
	///
	void IssueLogger::errALP4015(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4015,
				  tr("UFB Schema Input or Output item must have only ONE assigned AppSignalIDs, SchemaItem %1 (UfbSchema '%2').")
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4016
	///
	/// IssueType: Error
	///
	/// Title: File LmDescriptionFile %1 is not found (Schema '%2').
	///
	/// Parameters:
	///		%1 SchemaID
	///
	/// Description:
	///		File LmDescriptionFile %1 is not found (Schema '%2').
	///
	void IssueLogger::errALP4016(QString schema, QString lmDecriptionFile)
	{
		addSchemaIssue(OutputMessageLevel::Error, schema);
		LOG_ERROR(IssueType::AlParsing,
				  4016,
				  tr("File LmDescriptionFile %1 is not found (Schema '%2').")
					.arg(lmDecriptionFile)
					.arg(schema));
	}

	/// IssueCode: ALP4017
	///
	/// IssueType: Error
	///
	/// Title: AfbComponent with OpCode %1 is not found in file %2 (Schema '%3').
	///
	/// Parameters:
	///		%1 OpCode
	///		%2 LmDescription filename
	///		%3 Schema
	///
	/// Description:
	///		AfbComponent with OpCode %1 is not found in file %2 (Schema '%3').
	///
	void IssueLogger::errALP4017(QString schema, QString lmDecriptionFile, int opCode)
	{
		addSchemaIssue(OutputMessageLevel::Error, schema);
		LOG_ERROR(IssueType::AlParsing,
				  4017,
				  tr("AfbComponent with OpCode %1 is not found in file %2 (Schema '%3').")
					.arg(opCode)
					.arg(lmDecriptionFile)
					.arg(schema));
	}

	/// IssueCode: ALP4017
	///
	/// IssueType: Error
	///
	/// Title: AfbComponent with OpCode %1 is not found in file %2 (Schema '%3').
	///
	/// Parameters:
	///		%1 OpCode
	///		%2 LmDescription filename
	///		%3 Schema
	///
	/// Description:
	///		AfbComponent with OpCode %1 is not found in file %2 (Schema '%3').
	///
	void IssueLogger::errALP4017(QString schema, QString lmDecriptionFile, int opCode, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);
		LOG_ERROR(IssueType::AlParsing,
				  4017,
				  tr("AfbComponent with OpCode %1 is not found in file %2 (Schema '%3').")
					.arg(opCode)
					.arg(lmDecriptionFile)
					.arg(schema));
	}

	/// IssueCode: ALP4018
	///
	/// IssueType: Error
	///
	/// Title: LogicSchema (%1) and LogicModule (%2) have different LmDescriptionFile (%2 and %3).
	///
	/// Parameters:
	///		%1 LogicModule EquipmentID
	///		%2 LmDescription filename 1
	///		%3 LmDescription filename 2
	///		%4 Schema
	///
	/// Description:
	///		LogicSchema and assigned LogicModule must have the same value of LmDescriptionFile.
	///
	void IssueLogger::errALP4018(QString schema, QString equipmentId, QString schemaLmDecriptionFile1, QString moduleLmDecriptionFile2)
	{
		addSchemaIssue(OutputMessageLevel::Error, schema);
		LOG_ERROR(IssueType::AlParsing,
				  4018,
				  tr("LogicSchema (%1) and LogicModule (%2) have different LmDescriptionFile (%3 and %4).")
					.arg(schema)
					.arg(equipmentId)
					.arg(schemaLmDecriptionFile1)
					.arg(moduleLmDecriptionFile2));

	}

	/// IssueCode: ALP4019
	///
	/// IssueType: Error
	///
	/// Title: UFB Schema has disctinct LmDescriptionFile from LogicSchema, UFB Item %1, UFB Schema %2, LogicSchema %3, UFBSchema LmDescriptionFile %4, LogicSchema LmDescriptionFile %5.
	///
	/// Parameters:
	///		%1 Schema item description
	///		%2 UFB SchemaID
	///		%3 Logic schema StrID
	///		%4 UFBSchema LmDescriptionFile
	///		%5 LogicSchema LmDescriptionFile
	///
	/// Description:
	///		UFB Schema has disctinct LmDescriptionFile from LogicSchema, UFB Item %1, UFB Schema %2, LogicSchema %3, UFBSchema LmDescriptionFile %4, LogicSchema LmDescriptionFile %5.
	///
	void IssueLogger::errALP4019(QString schema, QString schemaItem, QString ufbElement, QUuid itemUuid, QString UfbLmDecriptionFile, QString schemaLmDecriptionFile)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4019,
				  tr("UFB Schema has disctinct LmDescriptionFile from LogicSchema, UFB Item %1, UFB Schema %2, LogicSchema %3, UFBSchema LmDescriptionFile %4, LogicSchema LmDescriptionFile %5.")
				  .arg(schemaItem)
				  .arg(ufbElement)
				  .arg(schema)
				  .arg(UfbLmDecriptionFile)
				  .arg(schemaLmDecriptionFile));
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
		addSchemaIssue(OutputMessageLevel::Error, schema1);
		addSchemaIssue(OutputMessageLevel::Error, schema2);
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
	/// Title: Schema does not have logic layer (Schema '%1').
	///
	/// Parameters:
	///		%1 SchemaID
	///
	/// Description:
	///		Each logic schema or user functional block schema has several layers (Logic, Frame and Notes), but the logic layer is not found.
	///
	void IssueLogger::errALP4022(QString schema)
	{
		addSchemaIssue(OutputMessageLevel::Error, schema);
		LOG_ERROR(IssueType::AlParsing,
				  4022,
				  tr("Schema does not have logic layer (Schema '%1').").arg(schema));
	}

	/// IssueCode: ALP4023
	///
	/// IssueType: Error
	///
	/// Title: UFB schema has duplicate pins %1 (UFB schema %2).
	///
	/// Parameters:
	///		%1 SchemaID
	///
	/// Description:
	///		UFB schema has duplicate pins, all inputs and outputs must be unique.
	///
	void IssueLogger::errALP4023(QString schema, QString pinCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);
		LOG_ERROR(IssueType::AlParsing,
				  4023,
				  tr("UFB schema has duplicate pins %1 (UFB schema %2).").arg(pinCaption).arg(schema));
	}


	/// IssueCode: ALP4040
	///
	/// IssueType: Error
	///
	/// Title: BusTypeID '%1' is not found for schema item '%2' (Logic Schema '%3').
	///
	/// Parameters:
	///		%1 BusTypeID
	///		%2 Schema item description
	///		%3 Logic schema StrID
	///
	/// Description:
	///		To proccess logic block it is required Bus description which is not found.
	///
	void IssueLogger::errALP4040(QString schema, QString schemaItem, QString busTypeId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4040,
				  tr("BusTypeID '%1' is not found for schema item '%2' (Logic Schema '%3').")
				  .arg(busTypeId)
				  .arg(schemaItem)
				  .arg(schema));
	}


	/// IssueCode: ALP4041
	///
	/// IssueType: Error
	///
	/// Title: SchemaItem '%1' has outdated BusType description (LogicSchema '%2').
	///
	/// Parameters:
	///		%1 Schema item description
	///		%2 Logic schema StrID
	///
	/// Description:
	///		SchemaItem has an outdated BusType description.
	///
	void IssueLogger::errALP4041(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4041,
				  tr("SchemaItem '%1' has outdated BusType description (LogicSchema '%2').")
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4060
	///
	/// IssueType: Error
	///
	/// Title: Loopback detected in LogicSchema %1, SchemaItem '%2'. To resolve issue use Loopback Source/Target items.
	///
	/// Parameters:
	///		%1 Logic schema ID
	///		%2 Schema item description
	///
	/// Description:
	///		On schema loopback made via links or signals, to resolve issue use Loopback Source/Target items.
	///
	void IssueLogger::errALP4060(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4060,
				  tr("Loopback detected in LogicSchema %1, SchemaItem '%2'. To resolve issue use Loopback Source/Target items.")
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4130
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
	void IssueLogger::errALP4130(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4130,
				  tr("Singlechannel Logic Schema '%1' cannot contain multichannel signal block ('%2').")
				  .arg(schema)
				  .arg(schemaItem));

	}


	/// IssueCode: ALP4131
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
	void IssueLogger::errALP4131(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4131,
				  tr("Multichannel signal block must have the same number of AppSignalIDs as schema's channel number (number of schema's EquipmentIDs), Logic Schema %1, item %2.")
				  .arg(schema)
				  .arg(schemaItem));
	}


	/// IssueCode: ALP4132
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
	void IssueLogger::errALP4132(QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4132,
				  tr("Schema contains mixed singlechannel and multichannel SignalItems in the branch (LogicSchema '%1').")
				  .arg(schema));
	}

	/// IssueCode: ALP4133
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
	void IssueLogger::errALP4133(QString schema, QString appSignalId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4133,
				  tr("Single channel branch contains signals (%1) from different channels (LogicSchema '%2').")
				  .arg(appSignalId)
				  .arg(schema));
	}

	/// IssueCode: ALP4134
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is not found (LogicSchema '%2', SchemaItem '%3').
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 LogicSchemaID
	///		%3 SchemaItem description
	///
	/// Description:
	///		Signal '%1' is not found (LogicSchema '%2', SchemaItem '%3').
	///
	void IssueLogger::errALP4134(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4134,
				  tr("Signal '%1' is not found (LogicSchema '%2', SchemaItem '%3').")
				  .arg(appSignalId)
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4135
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' does not have valid LM (LogicSchema '%2', SchemaItem '%3').
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 LogicSchemaID
	///		%3 SchemaItem description
	///
	/// Description:
	///		Signal '%1' does not have valid LM (LogicSchema '%2', SchemaItem '%3').
	///
	void IssueLogger::errALP4135(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4135,
				  tr("Signal '%1' does not have valid LM (LogicSchema '%2', SchemaItem '%3').")
				  .arg(appSignalId)
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4136
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is not bound to any schema's EquipmentIds(LMs), (LogicSchema '%2', SchemaItem '%3').
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 LogicSchemaID
	///		%3 SchemaItem description
	///
	/// Description:
	///		Signal '%1' is not bound to any schema's EquipmentIds(LMs), (LogicSchema '%2', SchemaItem '%3').
	///
	void IssueLogger::errALP4136(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4136,
				  tr("Signal '%1' is not bound to any schema's EquipmentIds(LMs), (LogicSchema '%2', SchemaItem '%3').")
				  .arg(appSignalId)
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4137
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is expected to be bound to EquipmentId(LM) %2 (LogicSchema '%3', SchemaItem '%4').
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 EquipmentID of LOM
	///		%3 LogicSchemaID
	///		%4 SchemaItem description
	///
	/// Description:
	///		Signal '%1' is expected to be bound to EquipmentId(LM) %2 (LogicSchema '%3', SchemaItem '%4').
	///
	void IssueLogger::errALP4137(QString schema, QString schemaItem, QString appSignalId, QString equipmentId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4137,
				  tr("Signal '%1' is expected to be bound to EquipmentId(LM) %2 (LogicSchema '%3', SchemaItem '%4').")
				  .arg(appSignalId)
				  .arg(equipmentId)
				  .arg(schema)
				  .arg(schemaItem));
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
		LOG_WARNING2(IssueType::AlCompiler,
				  5001,
				  tr("Application logic for module '%1' is not found.").arg(logicModuleID));
	}


	/// IssueCode: ALC5002
	///
	/// IssueType: Error
	///
	/// Title: Value of signal '%1' is undefined (Logic schema '%2').
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Signal value can not be calculated
	///
	void IssueLogger::errALC5002(QString schemaID, QString appSignalID, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5002,
				  tr("Value of signal '%1' is undefined (Logic schema '%2').").
				  arg(appSignalID).arg(schemaID));
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
	///		%4 Logic schema ID
	///
	/// Description:
	///		Outpuf of AFB is connected to signal with uncompatible data format.
	///
	void IssueLogger::errALC5008(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid, const QString& schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5008,
				  tr("Signal '%1' is connected to input '%2.%3' with uncompatible data format. (Schema '%4')").
					arg(appSignalID).arg(afbCaption).arg(input).arg(schemaID));
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
	/// Title: Application item '%1' has unknown type, SchemaID '%2'.
	///
	/// Parameters:
	///		%1 Item Uuid
	///		%2 SchemaID
	///
	/// Description:
	///		Application item has unknown type. Contact to the RPCT developers.
	///
	void IssueLogger::errALC5011(QString itemLabel, QString schemaId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schemaId);

		LOG_ERROR(IssueType::AlCompiler,
				  5011,
				  tr("Application item '%1' has unknown type, SchemaID '%2'. Contact to the RPCT developers.")
					.arg(itemLabel)
					.arg(schemaId));
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
		LOG_WARNING1(IssueType::AlCompiler,
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
	/// Title: Opto connection ID '%1' is not unique.
	///
	/// Parameters:
	///		%1 Connection ID
	///
	/// Description:
	///		Opto connection ID is not unique. Change connection identifier.
	///
	void IssueLogger::errALC5023(QString connection)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5023,
				  QString(tr("Opto connection ID '%1' is not unique.")).
				  arg(connection));
	}

	/// IssueCode: ALC5024
	///
	/// IssueType: Error
	///
	/// Title: Transmitter is linked to unknown opto connection '%1' (Logic schema '%2').
	///
	/// Parameters:
	///		%1 Connection ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Transmitter is linked to unknown opto connection. Check transmitter's 'ConnectionID' property.
	///
	void IssueLogger::errALC5024(QString connection, QUuid transmitterUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5024,
				  QString(tr("Transmitter is linked to unknown opto connection '%1' (Logic schema '%2').")).
				  arg(connection).arg(schemaID));
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
	void IssueLogger::errALC5025(QString connection, QUuid receiverUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, receiverUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5025,
				  QString(tr("Receiver is linked to unknown opto connection '%1' (Logic schema '%2').")).
				  arg(connection).arg(schemaID));
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

	/// IssueCode: ALC5036
	///
	/// IssueType: Error
	///
	/// Title: Analog signal '%1' is connected to discrete signal '%3'.
	///
	/// Parameters:
	///		%1 analog signal ID
	///		%2 discrete signal ID
	///		%3 analog signal Uuid
	///		%4 discrete signal Uuid
	///
	/// Description:
	///		The analog signal is connected to discrete signal. Change connections
	///
	void IssueLogger::errALC5036(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5036,
				  QString(tr("Analog signal '%1' is connected to discrete signal '%2'.")).
						  arg(srcSignalID).arg(destSignalID));
	}


	/// IssueCode: ALC5037
	///
	/// IssueType: Error
	///
	/// Title: Discrete signal '%1' is connected to analog signal '%3'.
	///
	/// Parameters:
	///		%1 discrete signal ID
	///		%2 analog signal ID
	///		%3 discrete signal Uuid
	///		%4 analog signal Uuid
	///
	/// Description:
	///		The discrete signal is connected to analog signal. Change connections
	///
	void IssueLogger::errALC5037(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5037,
				  QString(tr("Discrete signal '%1' is connected to analog signal '%2'.")).
						  arg(srcSignalID).arg(destSignalID));
	}

	/// IssueCode: ALC5038
	///
	/// IssueType: Error
	///
	/// Title: Signals '%1' and '%2' have different data format.
	///
	/// Parameters:
	///		%1 first signal ID
	///		%2 second signal ID
	///		%3 first signal Uuid
	///		%4 second signal Uuid
	///
	/// Description:
	///		Signals with different data format are connected.
	///
	void IssueLogger::errALC5038(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5038,
				  QString(tr("Signals '%1' and '%2' have different data format.")).
						  arg(srcSignalID).arg(destSignalID));
	}

	/// IssueCode: ALC5039
	///
	/// IssueType: Error
	///
	/// Title: Signals '%1' and '%2' have different data size.
	///
	/// Parameters:
	///		%1 first signal ID
	///		%2 second signal ID
	///		%3 first signal Uuid
	///		%4 second signal Uuid
	///
	/// Description:
	///		Signals with different data size are connected.
	///
	void IssueLogger::errALC5039(QString srcSignalID, QUuid srcUuid, QString destSignalID, QUuid destUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5039,
				  QString(tr("Signals '%1' and '%2' have different data size.")).
						  arg(srcSignalID).arg(destSignalID));
	}

	/// IssueCode: ALC5040
	///
	/// IssueType: Error
	///
	/// Title: Connection with ID '%1' is not found.
	///
	/// Parameters:
	///		%1 connection ID
	///
	/// Description:
	///		Connection with specified identifier is not found. Check connection ID.
	///
	void IssueLogger::errALC5040(QString connectionID, QUuid item)
	{
		addItemsIssues(OutputMessageLevel::Error, item);

		LOG_ERROR(IssueType::AlCompiler,
				  5040,
				  QString(tr("Connection with ID '%1' is not found.")).arg(connectionID));
	}

	/// IssueCode: ALC5041
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' exists in LM '%2'. No receivers needed.
	///
	/// Parameters:
	///		%1 application signal ID
	///		%2 LM's equipmentID
	///
	/// Description:
	///		The signal already exists in specified LM. No receivers is needed. Use iput or output items.
	///
	void IssueLogger::errALC5041(QString appSignalID, QString lmID, QUuid receiverUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, receiverUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5041,
				  QString(tr("Signal '%1' exists in LM '%2'. No receivers needed.")).
				  arg(appSignalID).arg(lmID));
	}

	/// IssueCode: ALC5042
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is not exists in connection '%2'.
	///
	/// Parameters:
	///		%1 application signal ID
	///		%2 LM's equipmentID
	///
	/// Description:
	///		The signal is not exists in connection. Use transmitter to send signal via connection.
	///
	void IssueLogger::errALC5042(QString appSignalID, QString connectionID, QUuid receiverUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, receiverUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5042,
				  QString(tr("Signal '%1' is not exists in connection '%2'. (Logic schema '%3')")).
						arg(appSignalID).
						arg(connectionID).
						arg(schemaID));
	}

	/// IssueCode: ALC5043
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter '%1.%2' must be greater or equal to 0.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 parameter caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Value of specified parameter must be greater or equal to 0. Check parameter value.
	///
	void IssueLogger::errALC5043(QString fbCaption, QString paramCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5043,
				  QString(tr("Value of parameter '%1.%2' must be greater or equal to 0.")).
				  arg(fbCaption).arg(paramCaption));
	}

	/// IssueCode: ALC5044
	///
	/// IssueType: Error
	///
	/// Title: Parameter's calculation for AFB '%1' (opcode %2) is not implemented.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 functional block opcode
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Parameter's calculation for specified AFB is not implemented. Contact to RPCT developers.
	///
	void IssueLogger::errALC5044(QString fbCaption, int opcode, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5044,
				  QString(tr("Parameter's calculation for AFB '%1' (opcode %2) is not implemented.")).
				  arg(fbCaption).arg(opcode));
	}

	/// IssueCode: ALC5045
	///
	/// IssueType: Error
	///
	/// Title: Required parameter '%1' of AFB '%2' is missing.
	///
	/// Parameters:
	///		%1 functional block parameter caption
	///		%2 functional block caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Required parameter of specified AFB is missing. Contact to RPCT developers.
	///
	void IssueLogger::errALC5045(QString paramCaption, QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5045,
				  QString(tr("Required parameter '%1' of AFB '%2' is missing.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5046
	///
	/// IssueType: Error
	///
	/// Title: Parameter '%1' of AFB '%2' must have type Unsigned Int.
	///
	/// Parameters:
	///		%1 functional block parameter caption
	///		%2 functional block caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Specified parameter must have type Unsigned Int. Contact to RPCT developers.
	///
	void IssueLogger::errALC5046(QString paramCaption, QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5046,
				  QString(tr("Parameter '%1' of AFB '%2' must have type Unsigned Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5047
	///
	/// IssueType: Error
	///
	/// Title: Parameter '%1' of AFB '%2' must have type 16-bit Unsigned Int.
	///
	/// Parameters:
	///		%1 functional block parameter caption
	///		%2 functional block caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Specified parameter must have type 16-bit Unsigned Int. Contact to RPCT developers.
	///
	void IssueLogger::errALC5047(QString paramCaption, QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5047,
				  QString(tr("Parameter '%1' of AFB '%2' must have type 16-bit Unsigned Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5048
	///
	/// IssueType: Error
	///
	/// Title: Parameter '%1' of AFB '%2' must have type 32-bit Unsigned Int.
	///
	/// Parameters:
	///		%1 functional block parameter caption
	///		%2 functional block caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Specified parameter must have type 32-bit Unsigned Int. Contact to RPCT developers.
	///
	void IssueLogger::errALC5048(QString paramCaption, QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5048,
				  QString(tr("Parameter '%1' of AFB '%2' must have type 32-bit Unsigned Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5049
	///
	/// IssueType: Error
	///
	/// Title: Parameter '%1' of AFB '%2' must have type 32-bit Signed Int.
	///
	/// Parameters:
	///		%1 functional block parameter caption
	///		%2 functional block caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Specified parameter must have type 32-bit Signed Int. Contact to RPCT developers.
	///
	void IssueLogger::errALC5049(QString paramCaption, QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5049,
				  QString(tr("Parameter '%1' of AFB '%2' must have type 32-bit Signed Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5050
	///
	/// IssueType: Error
	///
	/// Title: Parameter '%1' of AFB '%2' must have type 32-bit Float.
	///
	/// Parameters:
	///		%1 functional block parameter caption
	///		%2 functional block caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Specified parameter must have type 32-bit Float. Contact to RPCT developers.
	///
	void IssueLogger::errALC5050(QString paramCaption, QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5050,
				  QString(tr("Parameter '%1' of AFB '%2' must have type 32-bit Float.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5051
	///
	/// IssueType: Error
	///
	/// Title: Value %1 of parameter '%2' of AFB '%3' is incorrect.
	///
	/// Parameters:
	///		%1 parameter value
	///		%2 functional block parameter caption
	///		%3 functional block caption
	///		%4 application logic item Uuid
	///
	/// Description:
	///		Specified parameter must have type 32-bit Float. Contact to RPCT developers.
	///
	void IssueLogger::errALC5051(int paramValue, QString paramCaption, QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5051,
				  QString(tr("Value %1 of parameter '%2' of AFB '%3' is incorrect.")).
				  arg(paramValue).arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5052
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 parameter 1 caption
	///		%3 parameter 2 caption
	///		%4 application logic item Uuid
	///
	/// Description:
	///		Value of first specified parameter must be greate then the value of second parameneter. Correct prameter's values.
	///
	void IssueLogger::errALC5052(QString fbCaption, QString param1, QString param2, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5052,
				  QString(tr("Value of parameter '%1.%2' must be greate then the value of '%1.%3'.")).
				  arg(fbCaption).arg(param1).arg(param2));
	}

	/// IssueCode: ALC5053
	///
	/// IssueType: Warning
	///
	/// Title: Automatic sorting of XY points of FB '%1' has been performed.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 application logic item Uuid
	///
	/// Description:
	///		Automatic sorting of XY points of FB '%1' has been performed. Check XY points values.
	///
	void IssueLogger::wrnALC5053(QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_WARNING0(IssueType::AlCompiler,
				  5053,
				  QString(tr("Automatic sorting of XY points of FB '%1' has been performed.")).arg(fbCaption));
	}

	/// IssueCode: ALC5054
	///
	/// IssueType: Error
	///
	/// Title:	   Parameters '%1' and '%2' of AFB '%3' can't be equal.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 parameter 1 caption
	///		%3 parameter 2 caption
	///		%4 application logic item Uuid
	///
	/// Description:
	///		Values of specified parameters should not be equal. Check parameters values.
	///
	void IssueLogger::errALC5054(QString fbCaption, QString param1, QString param2, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5054,
				  QString(tr("Parameters '%1' and '%2' of AFB '%3' can't be equal.")).
				  arg(param1).arg(param2).arg(fbCaption));
	}

	/// IssueCode: ALC5055
	///
	/// IssueType: Warning
	///
	/// Title:	   Optical connection '%1' is configured manually.
	///
	/// Parameters:
	///		%1 Optical connection ID
	///
	/// Description:
	///		Note, that optical connection has manual settings.
	///
	void IssueLogger::wrnALC5055(QString connectionID)
	{
		LOG_WARNING2(IssueType::AlCompiler,
				  5055,
				  QString(tr("Optical connection '%1' is configured manually.")).arg(connectionID));
	}

	/// IssueCode: ALC5056
	///
	/// IssueType: Error
	///
	/// Title:	   SubsystemID '%1' assigned in LM '%2' is not found in subsystem list.
	///
	/// Parameters:
	///		%1 Subsystem ID
	///		%2 LM's equipment ID
	///
	/// Description:
	///		SubsystemID assigned in LM is not found in subsystem list. Check LM's SubsystemID property.
	///
	void IssueLogger::errALC5056(QString subsystemID, QString lmEquipmentID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5056,
				  QString(tr("SubsystemID '%1' assigned in LM '%2' is not found in subsystem list.")).
				  arg(subsystemID).arg(lmEquipmentID));
	}


	/// IssueCode: ALC5057
	///
	/// IssueType: Error
	///
	/// Title:	   Uncompatible data format of analog AFB Signal '%1.%2'.
	///
	/// Parameters:
	///		%1 AFB caption
	///		%2 AFB signal caption
	///		%3 AFB Uuid
	///
	/// Description:
	///		Data format of specified AFB Signal is not Float or Signed Int. Contact to RPCT developers.
	///
	void IssueLogger::errALC5057(QString afbCaption, QString afbSignal, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5057,
				  QString(tr("Uncompatible data format of analog AFB signal '%1.%2'.")).
				  arg(afbCaption).arg(afbSignal));
	}

	/// IssueCode: ALC5058
	///
	/// IssueType: Error
	///
	/// Title:	   Parameter '%1' of AFB '%2' can't be 0.
	///
	/// Parameters:
	///		%1 AFB parameter caption
	///		%2 AFB caption
	///		%3 AFB Uuid
	///
	/// Description:
	///		Specified parameter of AFB can't be 0. Check parameter value.
	///
	void IssueLogger::errALC5058(QString paramCaption, QString afbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5058,
				  QString(tr("Parameter '%1' of AFB '%2' can't be 0.")).
				  arg(paramCaption).arg(afbCaption));
	}

	/// IssueCode: ALC5059
	///
	/// IssueType: Error
	///
	/// Title:	   Ports of connection '%1' are not accessible in LM '%2' (Logic schema '%3').
	///
	/// Parameters:
	///		%1 Opto connection ID
	///		%2 Logic module equipmentID
	///		%3 Logic schema ID
	///
	/// Description:
	///		Ports of specified opto connection are not accessible in Logic Module.
	///
	void IssueLogger::errALC5059(QString schemaID, QString connectionID, QString lmID, QUuid transmitterUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, transmitterUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5059,
				  QString(tr("Ports of connection '%1' are not accessible in LM '%2' (Logic schema '%3').")).
				  arg(connectionID).arg(lmID).arg(schemaID));
	}

	/// IssueCode: ALC5060
	///
	/// IssueType: Error
	///
	/// Title:	   Float constant is connected to discrete input (Logic schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Float constant is connected to discrete input. Change contant type to IntegerType.
	///
	void IssueLogger::errALC5060(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, constantUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5060,
				  QString(tr("Float constant is connected to discrete input (Logic schema '%1').")).arg(schemaID));
	}

	/// IssueCode: ALC5061
	///
	/// IssueType: Error
	///
	/// Title:	   Float constant is connected to 16-bit input (Logic schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Float constant is connected to 16-bit input. Change contant type to IntegerType.
	///
	void IssueLogger::errALC5061(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, constantUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5061,
				  QString(tr("Float constant is connected to 16-bit input (Logic schema '%1').")).arg(schemaID));
	}

	/// IssueCode: ALC5062
	///
	/// IssueType: Error
	///
	/// Title:	   Float constant is connected to SignedInt input (Logic schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Float constant is connected to SignedInt input. Change contant type to IntegerType.
	///
	void IssueLogger::errALC5062(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, constantUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5062,
				  QString(tr("Float constant is connected to SignedInt input (Logic schema '%1').")).arg(schemaID));
	}

	/// IssueCode: ALC5063
	///
	/// IssueType: Error
	///
	/// Title:	   Integer constant is connected to Float input (Logic schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Integer constant is connected to Float input. Change contant type to FloatType.
	///
	void IssueLogger::errALC5063(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, constantUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5063,
				  QString(tr("Integer constant is connected to Float input (Logic schema '%1').")).arg(schemaID));
	}

	/// IssueCode: ALC5064
	///
	/// IssueType: Error
	///
	/// Title:	   Read address %1 of application memory is out of range 0..65535.
	///
	/// Parameters:
	///		%1 Application memory read address
	///
	/// Description:
	///		Specified read address of application memory is out of range 0..65535. Contact to RPCT developers.
	///
	void IssueLogger::errALC5064(int address)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5064,
				  QString(tr("Read address %1 of application memory is out of range 0..65535.")).arg(address));
	}

	/// IssueCode: ALC5065
	///
	/// IssueType: Error
	///
	/// Title:	   Write address %1 of application memory is out of range 0..65535.
	///
	/// Parameters:
	///		%1 Application memory write address
	///
	/// Description:
	///		Specified write address of application memory is out of range 0..65535. Contact to RPCT developers.
	///
	void IssueLogger::errALC5065(int address)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5065,
				  QString(tr("Write address %1 of application memory is out of range 0..65535.")).arg(address));
	}

	/// IssueCode: ALC5066
	///
	/// IssueType: Error
	///
	/// Title:	   Command 'MOVEMEM %1, %2, %3' can't write to bit-addressed memory.
	///
	/// Parameters:
	///		%1 Destination address
	///		%2 Source address
	///		%3 Memory size to move
	///
	/// Description:
	///		Command 'MOVEMEM' can't write to bit-addressed memory. Contact to RPCT developers.
	///
	void IssueLogger::errALC5066(int addrTo, int addrFrom, int sizeW)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5066,
				  QString(tr("Command 'MOVEMEM %1, %2, %3' can't write to bit-addressed memory.")).
					arg(addrTo).arg(addrFrom).arg(sizeW));
	}

	/// IssueCode: ALC5067
	///
	/// IssueType: Error
	///
	/// Title:	   Command 'MOVBC %1[%2], #%3' can't write out of application bit- or word-addressed memory.
	///
	/// Parameters:
	///		%1 Destination address
	///		%2 Destination bit
	///		%3 Const bit value
	///
	/// Description:
	///		Command 'MOVBC' can't write out of application bit- or word-addressed memory. Contact to RPCT developers.
	///
	void IssueLogger::errALC5067(int addrTo, int bit, int value)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5067,
				  QString(tr("Command 'MOVBC %1[%2], #%3' can't write out of application bit- or word-addressed memory.")).
					arg(addrTo).arg(bit).arg(value));
	}


	/// IssueCode: ALC5068
	///
	/// IssueType: Error
	///
	/// Title:	   LowEngeneeringUnits property of tuningable signal '%1' must be greate than HighEngeneeringUnits.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		LowEngeneeringUnits property of tuningable signal must be greate than HighEngeneeringUnits.
	///		Correct signal properties.
	///
	void IssueLogger::errALC5068(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5068,
				  QString(tr("LowEngeneeringUnits property of tuningable signal '%1' must be greate than HighEngeneeringUnits.")).
					arg(appSignalID));
	}


	/// IssueCode: ALC5069
	///
	/// IssueType: Error
	///
	/// Title:	   TuningDefaultValue property of tuningable signal '%1' must be in range from LowEngeneeringUnits to HighEngeneeringUnits.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		TuningDefaultValue property of tuningable signal must be in range from LowEngeneeringUnits to HighEngeneeringUnits.
	///		Correct signal TuningDefaultValue property.
	///
	void IssueLogger::errALC5069(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5069,
				  QString(tr("TuningDefaultValue property of tuningable signal '%1' must be in range from LowEngeneeringUnits to HighEngeneeringUnits.")).
					arg(appSignalID));
	}


	/// IssueCode: ALC5070
	///
	/// IssueType: Warning
	///
	/// Title:	   Signal '%1' has Little Endian byte order.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Specified signal has Little Endian byte order.
	///
	void IssueLogger::wrnALC5070(QString appSignalID)
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5070,
				  QString(tr("Signal '%1' has Little Endian byte order.")).
					arg(appSignalID));
	}


	/// IssueCode: ALC5071
	///
	/// IssueType: Error
	///
	/// Title:	   Can't assign value to tuningable signal '%1' (Logic schema '%2').
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Can't assign value to tuningable signal. Such signals are read-only.
	///
	void IssueLogger::errALC5071(QString schemaID, QString appSignalID, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5071,
				  QString(tr("Can't assign value to tuningable signal '%1' (Logic schema '%2').")).
					arg(appSignalID).arg(schemaID));
	}


	/// IssueCode: ALC5072
	///
	/// IssueType: Warning
	///
	/// Title: Possible error. AFB 'Poly' CoefCount = %1, but coefficient '%2' is not equal to 0 (Logic schema %3).
	///
	/// Parameters:
	/// 	%1 AFB Poly CoefCount parameters value
	///		%2 AFB Poly coeficient caption
	/// 	%3 Logic schema ID
	///
	/// Description:
	///		Value of CoefCount param of AFB Poly is less then number of coeficient, that is not equal to 0.
	///		Check CoefCount value, or set specified coeficient to 0.
	///
	void IssueLogger::wrnALC5072(int coefCount, QString coefCaption, QUuid itemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Warning0, itemUuid, schemaID);

		LOG_WARNING1(IssueType::AlCompiler,
				  5072,
				  QString(tr("Possible error. AFB 'Poly' CoefCount = %1, but coefficient '%2' is not equal to 0 (Logic schema '%3').")).
					 arg(coefCount).arg(coefCaption).arg(schemaID));
	}

	/// IssueCode: ALC5073
	///
	/// IssueType: Warning
	///
	/// Title: Usage of code memory exceed 95%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of code memory exceed 95%.
	///
	void IssueLogger::wrnALC5073()
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5073,
				  QString(tr("Usage of code memory exceed 95%.")));
	}

	/// IssueCode: ALC5074
	///
	/// IssueType: Error
	///
	/// Title: Usage of code memory exceed 100%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of code memory exceed 100%.
	///
	void IssueLogger::errALC5074()
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5074,
				  QString(tr("Usage of code memory exceed 100%.")));
	}

	/// IssueCode: ALC5075
	///
	/// IssueType: Warning
	///
	/// Title: Usage of bit-addressed memory exceed 95%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of bit-addressed memory exceed 95%.
	///
	void IssueLogger::wrnALC5075()
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5075,
				  QString(tr("Usage of bit-addressed memory exceed 95%.")));
	}

	/// IssueCode: ALC5076
	///
	/// IssueType: Error
	///
	/// Title: Usage of bit-addressed memory exceed 100%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of code bit-addressed memory exceed 100%.
	///
	void IssueLogger::errALC5076()
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5076,
				  QString(tr("Usage of bit-addressed memory exceed 100%.")));
	}

	/// IssueCode: ALC5077
	///
	/// IssueType: Warning
	///
	/// Title: Usage of word-addressed memory exceed 95%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of word-addressed memory exceed 95%.
	///
	void IssueLogger::wrnALC5077()
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5077,
				  QString(tr("Usage of word-addressed memory exceed 95%.")));
	}

	/// IssueCode: ALC5078
	///
	/// IssueType: Error
	///
	/// Title: Usage of word-addressed memory exceed 100%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of code word-addressed memory exceed 100%.
	///
	void IssueLogger::errALC5078()
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5078,
				  QString(tr("Usage of word-addressed memory exceed 100%.")));
	}

	/// IssueCode: ALC5079
	///
	/// IssueType: Warning
	///
	/// Title: Usage of IDR phase time exceed 90%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of IDR phase time exceed 90%.
	///
	void IssueLogger::wrnALC5079()
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5079,
				  QString(tr("Usage of IDR phase time exceed 90%.")));
	}

	/// IssueCode: ALC5080
	///
	/// IssueType: Error
	///
	/// Title: Usage of IDR phase time exceed 100%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of IDR phase time exceed 100%.
	///
	void IssueLogger::errALC5080()
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5080,
				  QString(tr("Usage of IDR phase time exceed 100%.")));
	}


	/// IssueCode: ALC5081
	///
	/// IssueType: Warning
	///
	/// Title: Usage of ALP phase time exceed 90%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of ALP phase time exceed 90%.
	///
	void IssueLogger::wrnALC5081()
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5081,
				  QString(tr("Usage of ALP phase time exceed 90%.")));
	}


	/// IssueCode: ALC5082
	///
	/// IssueType: Error
	///
	/// Title: Usage of ALP phase time exceed 100%.
	///
	/// Parameters:
	///
	/// Description:
	///		Usage of ALP phase time exceed 100%.
	///
	void IssueLogger::errALC5082()
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5082,
				  QString(tr("Usage of ALP phase time exceed 100%.")));
	}

	/// IssueCode: ALC5083
	///
	/// IssueType: Error
	///
	/// Title: Receiver of connection '%1' (port '%2') is not associated with LM '%3'
	///
	/// Parameters:
	///		%1 Connection ID
	///		%2 Connection port EquipmentID
	///		%3 Logic module EquipmentID
	///
	/// Description:
	///		Receiver of connection is not associated with specified LM. Check receiver placement.
	///
	void IssueLogger::errALC5083(const QString& receiverPortID, const QString& connectionID, const QString& lmID, QUuid receiverUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, receiverUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5083,
				  QString(tr("Receiver of connection '%1' (port '%2') is not associated with LM '%3'.")).
						arg(connectionID).arg(receiverPortID).arg(lmID));
	}

	/// IssueCode: ALC5084
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is not exists in serial connection '%2'. Use PortRawDataDescription to define receiving signals.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Connection ID
	///
	/// Description:
	///		Signal is not exists in specified serial connection. Use PortRawDataDescription to define receiving signals.
	///
	void IssueLogger::errALC5084(const QString& appSignalID, const QString& connectionID, QUuid receiverUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, receiverUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5084,
				  QString(tr("Signal '%1' is not exists in serial connection '%2'. Use PortRawDataDescription to define receiving signals.")).
						arg(appSignalID).arg(connectionID));
	}

	/// IssueCode: ALC5085
	///
	/// IssueType: Error
	///
	/// Title: Rx data size of RS232/485 port '%1' is undefined (connection '%2').
	///
	/// Parameters:
	///		%1 Serial port EquipmentID
	///		%2 Serial connection ID
	///
	/// Description:
	///		Receving data size of specified RS232/485 port is undefined. Use Manual Settings to determine Rx data size.
	///
	void IssueLogger::errALC5085(const QString& portEquipmentID, const QString& connectionID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5085,
				  QString(tr("Rx data size of RS232/485 port '%1' is undefined (connection '%2').")).
						arg(portEquipmentID).arg(connectionID));
	}

	/// IssueCode: ALC5086
	///
	/// IssueType: Error
	///
	/// Title: Constant connected to discrete signal or FB input must have value 0 or 1 (Logic schema '%1').
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Constant connected to discrete signal or FB input must have value 0 or 1. Check constant value.
	///

	void IssueLogger::errALC5086(QUuid constItemUuid, const QString& schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, constItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5086,
				  QString(tr("Constant connected to discrete signal or FB input must have value 0 or 1 (Logic schema '%1').")).
						arg(schemaID));

	}

	/// IssueCode: ALC5087
	///
	/// IssueType: Error
	///
	/// Title:	   Can't assign value to input signal '%1' (Logic schema '%2').
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Can't assign value to input signal. Such signals are read-only.
	///
	void IssueLogger::errALC5087(QString schemaID, QString appSignalID, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5087,
				  QString(tr("Can't assign value to input signal '%1' (Logic schema '%2').")).
					arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5088
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter '%1.%2' must be greater then 0.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 parameter caption
	///		%3 application logic item Uuid
	///
	/// Description:
	///		Value of specified parameter must be greater or equal to 0. Check parameter value.
	///
	void IssueLogger::errALC5088(QString fbCaption, QString paramCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5088,
				  QString(tr("Value of parameter '%1.%2' must be greater then 0.")).
				  arg(fbCaption).arg(paramCaption));
	}

	/// IssueCode: ALC5089
	///
	/// IssueType: Error
	///
	/// Title:	   Command 'MOVB %1[%2], %3[%4]' can't write out of application bit- or word-addressed memory.
	///
	/// Parameters:
	///		%1 Destination address
	///		%2 Destination bit
	///		%3 Source address
	///		%4 Source bit
	///
	/// Description:
	///		Command 'MOVB' can't write out of application bit- or word-addressed memory. Contact to RPCT developers.
	///
	void IssueLogger::errALC5089(int addrTo, int bitTo, int addrFrom, int bitFrom)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5089,
				  QString(tr("Command 'MOVB %1[%2], %3[%4]' can't write out of application bit- or word-addressed memory.")).
					arg(addrTo).arg(bitTo).arg(addrFrom).arg(bitFrom));
	}

	/// IssueCode: ALC5090
	///
	/// IssueType: Error
	///
	/// Title:	   Analog signal '%1' aperture should be greate then 0.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Analog signal aperture should be greate then 0. Check properties of specified signal.
	///
	void IssueLogger::errALC5090(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5090,
				  QString(tr("Analog signal '%1' aperture should be greate then 0.")).
					arg(appSignalID));
	}

	/// IssueCode: ALC5091
	///
	/// IssueType: Error
	///
	/// Title:	   Input/output application signal '%1' should be bound to equipment signal.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Specified output application signal should be bound to equipment signal. Check signals EquipmentID property.
	///
	void IssueLogger::errALC5091(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5091,
				  QString(tr("Input/output application signal '%1' should be bound to equipment signal.")).
					arg(appSignalID));
	}


	/// IssueCode: ALC5186
	///
	/// IssueType: Error
	///
	/// Title: Signal '%1' is not found (opto port '%2' raw data description).
	///
	/// Parameters:
	///		%1 Application signalID
	///		%2 Opto port equpment ID
	///
	/// Description:
	///		Signal specified in opto port raw data description is not found. Check ID of signal.
	///
	void IssueLogger::errALC5186(const QString& appSignalID, const QString& portEquipmentID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5186,
				  QString(tr("Signal '%1' is not found (opto port '%2' raw data description).")).
						arg(appSignalID).arg(portEquipmentID));
	}

	/// IssueCode: ALC5187
	///
	/// IssueType: Error
	///
	/// Title: Tx data memory areas of opto ports '%1' and '%2' are overlapped.
	///
	/// Parameters:
	///		%1 Opto port 1 ID
	///		%2 Opto port 2 ID
	///
	/// Description:
	///		Tx data memory areas of specified opto ports are overlapped. Check manual settinggs of opto ports.
	///
	void IssueLogger::errALC5187(const QString& port1ID, const QString &port2ID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5187,
				  QString(tr("Tx data memory areas of opto ports '%1' and '%2' are overlapped.")).
						arg(port1ID).arg(port2ID));
	}

	/// IssueCode: ALC5188
	///
	/// IssueType: Error
	///
	/// Title: Duplicate signal ID '%1' in opto port '%2' raw data description.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///
	/// Description:
	///		Duplicate signal ID in specified opto port raw data description. Check description.
	///
	void IssueLogger::errALC5188(const QString& appSignalID, const QString &portID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5188,
				  QString(tr("Signal ID '%1' is duplicate in opto port '%2' raw data description.")).
						arg(appSignalID).arg(portID));
	}

	/// IssueCode: ALC5189
	///
	/// IssueType: Error
	///
	/// Title: Tx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Logic module ID
	///
	/// Description:
	///		Transmitted signal specified in opto port raw data description is not exists in associated LM. Check description or signal ID.
	///
	void IssueLogger::errALC5189(const QString& appSignalID, const QString& portID, const QString& lmID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5189,
				  QString(tr("Tx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.")).
						arg(appSignalID).arg(portID).arg(lmID));
	}

	/// IssueCode: ALC5190
	///
	/// IssueType: Error
	///
	/// Title: Rx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Logic module ID
	///
	/// Description:
	///		Receiving signal specified in opto port raw data description is not exists in associated LM. Check description or signal ID.
	///
	void IssueLogger::errALC5190(const QString& appSignalID, const QString& portID, const QString& lmID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5190,
				  QString(tr("Rx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.")).
						arg(appSignalID).arg(portID).arg(lmID));
	}

	/// IssueCode: ALC5191
	///
	/// IssueType: Error
	///
	/// Title: Serial Rx signal '%1' is not associated with LM '%2' (Logic schema '%3').
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic module ID
	///		%3 Logic schema ID
	///
	/// Description:
	///		Receiving signal specified in opto port raw data description is not exists in associated LM. Check description or signal ID.
	///
	void IssueLogger::errALC5191(const QString& appSignalID, const QString& lmID, QUuid itemID, const QString& schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, itemID, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5191,
				  QString(tr("Serial Rx signal '%1' is not associated with LM '%2' (Logic schema '%3').")).
						arg(appSignalID).arg(lmID).arg(schemaID));
	}

	/// IssueCode: ALC5192
	///
	/// IssueType: Error
	///
	/// Title: Tx signal '%1' specified in port '%2' raw data description isn't connected to transmitter (Connection '%3').
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Connection ID
	///
	/// Description:
	///		Tx signal specified in port raw data description isn't connected to transmitter. Connect signal to transmitter.
	///
	void IssueLogger::errALC5192(const QString& appSignalID, const QString& portID, const QString& connectionID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5192,
				  QString(tr("Tx signal '%1' specified in port '%2' raw data description isn't connected to transmitter (Connection '%3').")).
						arg(appSignalID).arg(portID).arg(connectionID));
	}

	/// IssueCode: ALC5193
	///
	/// IssueType: Error
	///
	/// Title: Rx signal '%1' specified in port '%2' raw data description isn't assigned to receiver (Connection '%3').
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Connection ID
	///
	/// Description:
	///		Rx signal specified in port raw data description isn't assigned to reciever.
	///
	void IssueLogger::errALC5193(const QString& appSignalID, const QString& portID, const QString& connectionID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5193,
				  QString(tr("Rx signal '%1' specified in port '%2' raw data description isn't assigned to receiver (Connection '%3').")).
						arg(appSignalID).arg(portID).arg(connectionID));
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
	void IssueLogger::errEQP6000(QString equipmemtId, QUuid equpmentUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, equpmentUuid);

		LOG_ERROR(IssueType::Equipment,
				  6000,
				  tr("Property Place is less then 0 (Equipment object '%1').")
				  .arg(equipmemtId)
				  );
	}

	/// IssueCode: EQP6001
	///
	/// IssueType: Error
	///
	/// Title: Two or more equipment objects have the same EquipmentID '%1'.
	///
	/// Parameters:
	///		%1 Equipment object ID
	///
	/// Description:
	///		Error may occur if two or more equipment objects have the same EquipmentID.
	///     All equipment objects must have unique EquipmentID.
	///
	void IssueLogger::errEQP6001(QString equipmemtId, QUuid equipmentUuid1, QUuid equipmentUuid2)
	{
		addItemsIssues(OutputMessageLevel::Error, equipmentUuid1);
		addItemsIssues(OutputMessageLevel::Error, equipmentUuid2);

		LOG_ERROR(IssueType::Equipment,
				  6001,
				  tr("Two or more equipment objects have the same EquipmentID '%1'.")
				  .arg(equipmemtId)
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
	///		%2 Equipmnet object ID 1
	///		%2 Equipmnet object ID 2
	///
	/// Description:
	///		Error may occur if two or more equipment objects have the same Uuid.
	/// All equipmnet objects must have unique Uuid. In some cases it can be an internal
	/// software error and it shoud be reported to developers.
	///
	void IssueLogger::errEQP6002(QUuid equipmentUuid, QString equipmentId1, QString equipmentId2)
	{
		addItemsIssues(OutputMessageLevel::Error, equipmentUuid);

		LOG_ERROR(IssueType::Equipment,
				  6002,
				  tr("Two or more equipment objects have the same Uuid '%1' (Object1 '%2', Object2 '%3')")
				  .arg(equipmentUuid.toString())
				  .arg(equipmentId1)
				  .arg(equipmentId2)
				  );
	}

	/// IssueCode: EQP6003
	///
	/// IssueType: Error
	///
	/// Title: Ethernet adapters of LMs '%1' and '%2' has duplicate IP address %3.
	///
	/// Parameters:
	///		%1 First LM equipmentID
	///		%2 Second LM equipmentID
	///		%3 duplicate IP address
	///		%4 First LM Uuid
	///		%5 Second LM Uuid
	///
	/// Description:
	///		Ethernet adapters of specified LMs has duplicate IP address. Change IP addresses of adapters to unique values.
	///
	void IssueLogger::errEQP6003(QString lm1, QString lm2, QString ipAddress, QUuid lm1Uuid, QUuid lm2Uuid)
	{
		addItemsIssues(OutputMessageLevel::Error, lm1Uuid);
		addItemsIssues(OutputMessageLevel::Error, lm2Uuid);

		LOG_ERROR(IssueType::Equipment,
				  6003,
				  tr("Ethernet adapters of LMs '%1' and '%2' has duplicate IP address %3.")
				  .arg(lm1)
				  .arg(lm2)
				  .arg(ipAddress)
				  );
	}

	/// IssueCode: EQP6004
	///
	/// IssueType: Error
	///
	/// Title: File LmDescriptionFile %1 is not found, LogicModule %2.
	///
	/// Parameters:
	///		%1 LmDescriptionFile filename
	///		%2 LogicModule EquipmentID
	///
	/// Description:
	///		LogicModule has property LmDescriptionFile, but this file is missing in $(root)/AFBL.
	///
	void IssueLogger::errEQP6004(QString lm, QString lmDescriptionFile, QUuid lmUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, lmUuid);

		LOG_ERROR(IssueType::Equipment,
				  6004,
				  tr("File LmDescriptionFile %1 is not found, LogicModule %2.")
				  .arg(lmDescriptionFile)
				  .arg(lm));
	}

	/// IssueCode: EQP6005
	///
	/// IssueType: Error
	///
	/// Title: Subsystem list has duplicate SubsystemIDs %1.
	///
	/// Parameters:
	///		%1 Duplicate SubsystemID
	///
	/// Description:
	///		Subsystem list has duplicate SubsystemIDs. All subsystems must have unique SubsystemID.
	///
	void IssueLogger::errEQP6005(QString subsystemId)
	{
		LOG_ERROR(IssueType::Equipment,
				  6005,
				  tr("Subsystem list has duplicate SubsystemIDs %1.")
				  .arg(subsystemId));
	}

	/// IssueCode: EQP6006
	///
	/// IssueType: Error
	///
	/// Title: Subsystem list has duplicate ssKeys %1.
	///
	/// Parameters:
	///		%1 Duplicate SubsystemID
	///
	/// Description:
	///		Subsystem list has duplicate ssKeys. All subsystems must have unique ssKeys.
	///
	void IssueLogger::errEQP6006(int subsystemKey)
	{
		LOG_ERROR(IssueType::Equipment,
				  6006,
				  tr("Subsystem list has duplicate ssKeys %1.")
				  .arg(subsystemKey));
	}

	/// IssueCode: EQP6007
	///
	/// IssueType: Error
	///
	/// Title: Subsystem %1 has distinct LogicModule type, version or LmDescriptionFile (properties ModuleFamily, ModuleVersion, LmDescriptionFile).
	///
	///
	/// Parameters:
	///		%1 Duplicate SubsystemID
	///
	/// Description:
	///		All modules in subsystem must have same type, version and LmDescriptionFile (properties ModuleFamily, ModuleVersion, LmDescriptionFile)
	///
	void IssueLogger::errEQP6007(QString subsystemId)
	{
		LOG_ERROR(IssueType::Equipment,
				  6007,
				  tr("Subsystem %1 has distinct LogicModule type, version or LmDescriptionFile (properties ModuleFamily, ModuleVersion, LmDescriptionFile).")
				  .arg(subsystemId));
	}

    /// IssueCode: EQP6008
    ///
    /// IssueType: Error
    ///
    /// Title: Child '%1' with place '%2' is not allowed in parent '%3'.
    ///
    ///
    /// Parameters:
    ///		%1 Parent Equipment ID
    ///		%2 Child Equipment ID
    ///		%3 Child place
    ///
    /// Description:
    ///		Child restriction is failed in an equipment object
    ///
    void IssueLogger::errEQP6008(QString equipmentId, QString childEquipmentId, int childPlace)
    {
        LOG_ERROR(IssueType::Equipment,
                  6008,
                  tr("Child '%1' with place '%2' is not allowed in parent '%3'.")
                  .arg(childEquipmentId)
                  .arg(childPlace)
                  .arg(equipmentId));
    }

	/// IssueCode: EQP6009
	///
	/// IssueType: Error
	///
	/// Title: Property Place must be 0 (Equipment object '%1').
	///
	/// Parameters:
	///		%1 Equipmnet object StrID
	///
	/// Description:
	///		Property Place for Logic Module must be set to 0.
	///
	void IssueLogger::errEQP6009(QString equipmemtId, QUuid equpmentUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, equpmentUuid);

		LOG_ERROR(IssueType::Equipment,
				  6009,
				  tr("Property Place must be 0 (Equipment object '%1').")
				  .arg(equipmemtId)
				  );
	}



	/// IssueCode: EQP6100
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
				  6100,
				  tr("Unknown software type (Software object StrID '%1').")
				  .arg(softwareObjectStrId)
				  );
	}

	/// IssueCode: EQP6101
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong unitID: %2.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Wrong unitID
	///
	/// Description:
	///		Wrong unitID. It is required to set unitID to the correct value.
	///
	void IssueLogger::errEQP6101(QString appSignalID, int unitID)
	{
		LOG_ERROR(IssueType::Equipment,
				  6101,
				  tr("Signal %1 has wrong unitID: %2.")
				  .arg(appSignalID)
				  .arg(unitID)
				  );
	}


	/// IssueCode: EQP6102
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong type of sensor: %2.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Wrong type of sensor
	///
	/// Description:
	///		Wrong type of sensor. It is required to set type of sensor to the correct value.
	///
	void IssueLogger::errEQP6102(QString appSignalID, int sensorType)
	{
		LOG_ERROR(IssueType::Equipment,
				  6102,
				  tr("Signal %1 has wrong type of sensor: %2.")
				  .arg(appSignalID)
				  .arg(sensorType)
				  );
	}


	/// IssueCode: EQP6103
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong type of output range mode: %2.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Wrong type of output range mode
	///
	/// Description:
	///		Wrong type of output range mode. It is required to set type of output range mode to the correct value.
	///
	void IssueLogger::errEQP6103(QString appSignalID, int outputMode)
	{
		LOG_ERROR(IssueType::Equipment,
				  6103,
				  tr("Signal %1 has wrong type of output range mode: %2.")
				  .arg(appSignalID)
				  .arg(outputMode)
				  );
	}


	/// IssueCode: EQP6104
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong input/output type: %2.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Wrong input/output type of signal
	///
	/// Description:
	///		Wrong input/output type of signal. It is required to set input/output type to the correct value.
	///
	void IssueLogger::errEQP6104(QString appSignalID, int inOutType)
	{
		LOG_ERROR(IssueType::Equipment,
				  6104,
				  tr("Signal %1 has wrong input/output type: %2.")
				  .arg(appSignalID)
				  .arg(inOutType)
				  );
	}


	/// IssueCode: EQP6105
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong order of byte: %2.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Wrong order of byte
	///
	/// Description:
	///		Wrong order of byte. It is required to set order of byte to the correct value.
	///
	void IssueLogger::errEQP6105(QString appSignalID, int byteOrder)
	{
		LOG_ERROR(IssueType::Equipment,
				  6105,
				  tr("Signal %1 has wrong order of byte: %2.")
				  .arg(appSignalID)
				  .arg(byteOrder)
				  );
	}

	/// IssueCode: EQP6106
	///
	/// IssueType: Error
	///
	/// Title: Schema %1 specified in Tuning Client %2 does not exist.
	///
	/// Parameters:
	///		%1 Schema ID
	///		%2 TuningClient Equipment ID
	///
	/// Description:
	///		Schema that is specified in Schemas property of the Tuning Client does not exist.
	///
	void IssueLogger::errEQP6106(QString schemaId, QString tuningClientEquipmentId)
	{
		LOG_ERROR(IssueType::Equipment,
				  6106,
				  tr("Schema %1 specified in Tuning Client %2 does not exist.")
				  .arg(schemaId)
				  .arg(tuningClientEquipmentId)
				  );
	}
	// --
	//
	void IssueLogger::addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids)
	{
		QMutexLocker l(&m_mutex);
		m_buildIssues.addItemsIssues(level, itemsUuids);
	}

	void IssueLogger::addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids, const QString& schemaID)
	{
		QMutexLocker l(&m_mutex);
		m_buildIssues.addItemsIssues(level, itemsUuids, schemaID);
	}

	void IssueLogger::addItemsIssues(OutputMessageLevel level, QUuid itemsUuid)
	{
		QMutexLocker l(&m_mutex);
		m_buildIssues.addItemsIssues(level, itemsUuid);
	}

	void IssueLogger::addItemsIssues(OutputMessageLevel level, QUuid itemsUuid, const QString& schemaID)
	{
		QMutexLocker l(&m_mutex);
		m_buildIssues.addItemsIssues(level, itemsUuid, schemaID);
	}

	void IssueLogger::addSchemaIssue(OutputMessageLevel level, const QString& schemaID)
	{
		QMutexLocker l(&m_mutex);
		m_buildIssues.addSchemaIssue(level, schemaID);
	}


	void IssueLogger::swapItemsIssues(BuildIssues* buildIssues)
	{
		if (buildIssues == nullptr)
		{
			assert(buildIssues);
			return;
		}

		QMutexLocker l(&m_mutex);
		m_buildIssues.swap(buildIssues);
	}

	void IssueLogger::clearItemsIssues()
	{
		QMutexLocker l(&m_mutex);
		m_buildIssues.clear();
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
