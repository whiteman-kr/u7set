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
		QMutexLocker ml(&m_mutex);

		m_items.clear();
		m_schemas.clear();

		return;
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids)
	{
		QMutexLocker ml(&m_mutex);

		for (auto id : itemsUuids)
		{
			m_items[id] = level;
		}

		return;
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, const std::vector<QUuid>& itemsUuids, const QString& schemaID)
	{
		QMutexLocker ml(&m_mutex);

		for (auto id : itemsUuids)
		{
			m_items[id] = level;
			addSchemaIssue(level, schemaID);
		}

		return;
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, QUuid itemsUuid)
	{
		QMutexLocker ml(&m_mutex);
		m_items[itemsUuid] = level;
	}

	void BuildIssues::addItemsIssues(OutputMessageLevel level, QUuid itemsUuid, const QString& schemaID)
	{
		QMutexLocker ml(&m_mutex);
		m_items[itemsUuid] = level;
		addSchemaIssue(level, schemaID);
	}

	void BuildIssues::addSchemaIssue(OutputMessageLevel level, const QString& schemaID)
	{
		QMutexLocker ml(&m_mutex);

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

	OutputMessageLevel BuildIssues::issueForSchemaItem(const QUuid& itemId) const
	{
		QMutexLocker ml(&m_mutex);

		if (auto it = m_items.find(itemId);
			it == m_items.end())
		{
			// Either Success or did not take part in build
			//
			return OutputMessageLevel::Success;
		}
		else
		{
			return it->second;
		}
	}

	BuildIssues::Counter BuildIssues::issueForSchema(const QString& schemaId) const
	{
		QMutexLocker ml(&m_mutex);

		if (auto it = m_schemas.find(schemaId);
			it == m_schemas.end())
		{
			// Either Success or did not take part in build
			//
			return Builder::BuildIssues::Counter();
		}
		else
		{
			return it->second;
		}
	}

	int BuildIssues::count() const
	{
		QMutexLocker ml(&m_mutex);
		return static_cast<int>(m_items.size() + m_schemas.size());
	}

	//
	//
	//				IssueLogger
	//
	//
	IssueLogger::IssueLogger(BuildIssues* buildIssues /*= nullptr*/) :
		OutputLog(),
		m_buildIssues(buildIssues)
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
	/// Title: File loading/parsing error, file is damaged or has incompatible format, file name %1.
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
				  QString(tr("File loading/parsing error, file is damaged or has incompatible format, file name %1.")).arg(fileName));
	}


	/// IssueCode: CMN0011
	///
	/// IssueType: Error
	///
	/// Title: Can't create directory %1.
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
				  QString(tr("Can't create directory %1.")).arg(directory));
	}


	/// IssueCode: CMN0012
	///
	/// IssueType: Error
	///
	/// Title: Can't create file %1.
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
				  QString(tr("Can't create file %1.")).arg(fileName));
	}


	/// IssueCode: CMN0013
	///
	/// IssueType: Error
	///
	/// Title: Write error of file %1.
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
				  QString(tr("Write error of file %1.")).arg(fileName));
	}


	/// IssueCode: CMN0014
	///
	/// IssueType: Error
	///
	/// Title: File %1 already exists.
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
				  QString(tr("File %1 already exists.")).arg(fileName));
	}

	/// IssueCode: CMN0015
	///
	/// IssueType: Error
	///
	/// Title: %1 and %2 files have the same ID = %3.
	///
	/// Parameters:
	///		%1 File name 1
	///		%2 File name 2
	///		%3 Files identifier
	///
	/// Description:
	///		Build files have same string identifier. Contact to th RPCT developers.
	///
	void IssueLogger::errCMN0015(QString fileName1, QString fileName2, QString id)
	{
		LOG_ERROR(IssueType::Common,
				  15,
				  QString(tr("%1 and %2 files have the same ID = %3.")).arg(fileName1).arg(fileName2).arg(id));
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
				  QString(tr("The build was cancelled.")));
	}

	/// IssueCode: CMN0017
	///
	/// IssueType: Error
	///
	/// Title: Can't open file %1.
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
				  QString(tr("Can't open file %1.")).arg(fileName));
	}

	/// IssueCode: CMN0018
	///
	/// IssueType: Error
	///
	/// Title: Can't link build file %1 into /%2/configuration.xml.
	///
	/// Parameters:
	///		%1 File name
	///		%2 Configuration.xml file subdirectory
	///
	/// Description:
	///		Program can't link specified build file into configuration.xml.
	///
	void IssueLogger::errCMN0018(QString fileName, QString cfgXmlSubdir)
	{
		LOG_ERROR(IssueType::Common,
				  18,
				  QString(tr("Can't link build file %1 into /%2/configuration.xml.")).arg(fileName).arg(cfgXmlSubdir));
	}

	/// IssueCode: CMN0019
	///
	/// IssueType: Error
	///
	/// Title: Can't find file with ID = %1 in build subdirectory %2.
	///
	/// Parameters:
	///		%1 File ID (string)
	///		%2 Build subdirectory
	///
	/// Description:
	///		Program can't find required file in specified build subdirectory.
	///
	void IssueLogger::errCMN0019(QString fileID, QString subDir)
	{
		LOG_ERROR(IssueType::Common,
				  19,
				  QString(tr("Can't find file with ID = %1 in build subdirectory %2.")).arg(fileID).arg(subDir));
	}

	/// IssueCode: CMN0020
	///
	/// IssueType: Error
	///
	/// Title: Can't find build file %1.
	///
	/// Parameters:
	///		%1 File name
	///
	/// Description:
	///		Program can't find required build file with specified file name.
	///
	void IssueLogger::errCMN0020(QString fileName)
	{
		LOG_ERROR(IssueType::Common,
				  20,
				  QString(tr("Can't find build file %1.")).arg(fileName));
	}

	/// IssueCode: CMN0021
	///
	/// IssueType: Error
	///
	/// Title: File %1 already linked to %2.
	///
	/// Parameters:
	///		%1 File name to link
	///		%2 Configuration.xml file name
	///
	/// Description:
	///		Build file already linked to specified configuration.xml file.
	///		In most cases it is an internal software error and it shoud be reported to developers.
	///
	void IssueLogger::errCMN0021(QString fileName, QString cfgXmlFileName)
	{
		LOG_ERROR(IssueType::Common,
				  21,
				  QString(tr("File %1 already linked to %2.")).arg(fileName).arg(cfgXmlFileName));
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
	///		In most cases it is an internal software error and it shoud be reported to developers.
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
		addSchemaIssue(OutputMessageLevel::Error, 1001, schema);

		LOG_ERROR(IssueType::Internal,
				  1001,
				  tr("Internal exception, schema %1: %2.")
					.arg(schema)
					.arg(debugMessage));
	}

	void IssueLogger::errINT1001(QString debugMessage, QString schema, QUuid itemsUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 1001, itemsUuid, schema);

		LOG_ERROR(IssueType::Internal,
				  1001,
				  tr("Internal exception, schema %1: %2.")
					.arg(schema)
					.arg(debugMessage));
	}

	void IssueLogger::errINT1001(QString debugMessage, QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, 100, itemsUuids, schema);

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
	/// Title: Error of getting file list from the database, parent file ID %1, filter %2, database message %3.
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
				  tr("Error of getting file list from the database, parent file ID %1, filter %2, database message %3.")
				  .arg(parentFileId)
				  .arg(filter)
				  .arg(databaseMessage));
	}

	/// IssueCode: PDB2002
	///
	/// IssueType: Error
	///
	/// Title: Getting file instance error, file ID %1, file name %2, database message %3.
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
				  tr("Getting file instance error, file ID %1, file name %2, database message %3.")
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

	/// IssueCode: PDB2006
	///
	/// IssueType: Error
	///
	/// Title: Opening project %1 error (%2).
	///
	/// Parameters:
	///			%1 RPCT project name
	///			%2 Database last error value
	///
	/// Description:
	///		RPCT project opening error due database error.
	///
	void IssueLogger::errPDB2006(QString projectName, QString dbLastError)
	{
		LOG_ERROR(IssueType::ProjectDatabase,
				  2006,
				  QString(tr("Opening project %1 error (%2).")).arg(projectName).arg(dbLastError));
	}

	/// IssueCode: PDB2020B
	///
	/// IssueType: Error
	///
	/// Title: Getting project properties error.
	///
	/// Parameters:
	///
	/// Description:
	///		RPCT project property getting errror.
	///
	void IssueLogger::errPDB2020()
	{
		LOG_ERROR(IssueType::ProjectDatabase,
				  2020,
				  QString(tr("Getting project properties error.")));
	}

	// CFG			FSC configuration						3000-3999
	//

	/// IssueCode: CFG3000
	///
	/// IssueType: Error
	///
	/// Title: Property %1 does not exist in object %2.
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
				  tr("Property %1 does not exist in an object %2.")
				  .arg(propertyName)
				  .arg(object));
	}

	/// IssueCode: CFG3001
	///
	/// IssueType: Error
	///
	/// Title: Subsystem %1 is not found in subsystem set (Logic Module %2).
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
				  tr("Subsystem %1 is not found in subsystem set (Logic Module %2).")
				  .arg(subSysID)
				  .arg(module));
	}

	/// IssueCode: CFG3002
	///
	/// IssueType: Error
	///
	/// Title: Property %1 has wrong value (%2), valid range is %3..%4 (module %5).
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
				  tr("Property %1 has wrong value (%2), valid range is %3..%4 (module %5).")
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
	/// Title: Property System\\LMNumber (%1) is not unique in Logic Module %2.
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
				  tr("Property System\\LMNumber (%1) is not unique in Logic Module %2.")
				  .arg(LMNumber)
				  .arg(module));
	}


	/// IssueCode: CFG3004
	///
	/// IssueType: Error
	///
	/// Title: Controller %1 is not found in module %2.
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
				  tr("Controller %1 is not found in module %2.")
				  .arg(controllerID)
				  .arg(module));
	}


	/// IssueCode: CFG3005
	///
	/// IssueType: Warning
	///
	/// Title: Signal %1 is not found in controller %2.
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
				  tr("Signal %1 is not found in controller %2.")
				  .arg(signalID)
				  .arg(controllerID));
	}

	/// IssueCode: CFG3006
	///
	/// IssueType: Warning
	///
	/// Title: Signal with place %1 is not found in controller %2.
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
				  tr("Signal with place %1 is not found in controller %2.")
				  .arg(place)
				  .arg(controllerID));
	}


	/// IssueCode: CFG3007
	///
	/// IssueType: Warning
	///
	/// Title: Signal %1 is not found in Application Signals.
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
				  tr("Signal %1 is not found in Application Signals.")
				  .arg(signalID));
	}

	/// IssueCode: CFG3008
	///
	/// IssueType: Warning
	///
	/// Title: Software %1 is not found (Logic Module %2).
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
				  tr("Software %1 is not found (Logic Module %2).")
				  .arg(softwareID)
				  .arg(module));
	}


	/// IssueCode: CFG3009
	///
	/// IssueType: Error
	///
	/// Title: Calculated SpreadTolerance ADC mismatch, signals %1 and %2 in module %3. Check High/LowEngineeringUints ranges, ADC range and SpreadTolerance value of signals.
	///
	/// Parameters:
	///         %1 Signal 1 StrID
	///         %2 Signal 2 StrID
	///         %3 Module StrID
	///
	/// Description:
	///			Calculated SpreadTolerance ADC values should be equal in channel A and channel B in AIM module.
	///
	void IssueLogger::errCFG3009(QString signalID1, QString signalID2, QString module)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3009,
				  tr("Calculated SpreadTolerance ADC mismatch, signals %1 and %2 in module %3.")
				  .arg(signalID1)
				  .arg(signalID2)
				  .arg(module));
	}

	/// IssueCode: CFG3010
	///
	/// IssueType: Error
	///
	/// Title: Property %1 has wrong value (%2), valid range is %3..%4 [precision %5](signal %6).
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
				  tr("Property %1 has wrong value (%2), valid range is %3..%4 (signal %6).")
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
	/// Title: IP address in property %1 has undefined value (%2) in controller %3.
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
				  tr("IP address in property %1 has undefined value (%2) in controller %3.")
				  .arg(addressProperty)
				  .arg(str)
				  .arg(controller));
	}

	/// IssueCode: CFG3012
	///
	/// IssueType: Error
	///
	/// Title: Port in property %1 has undefined value (%2) in controller %3.
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
				  tr("Port in property %1 has undefined value (%2) in controller %3.")
				  .arg(portProperty)
				  .arg(port)
				  .arg(controller));
	}

	/// IssueCode: CFG3013
	///
	/// IssueType: Error
	///
	/// Title: Property %1 (%2) is %3 property %4 (%5) in signal %6.
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
				  tr("Property %1 (%2) is %3 property %4 (%5) in signal %6")
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
	/// Title: Can't find child object with suffix %1 in object %2
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
		QString msg = "Can't find child object with suffix " + suffix + " in object " + objectID + ".";

		LOG_ERROR(IssueType::FscConfiguration,
				  3014,
				  msg);
	}

	/// IssueCode: CFG3015
	///
	/// IssueType: Warning
	///
	/// Title: Property %1.%2 is linked to undefined software ID %3.
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
				  tr("Property %1.%2 is linked to undefined software ID %3.")
				  .arg(objectID)
				  .arg(propertyName)
				  .arg(softwareID));
	}

	/// IssueCode: CFG3016
	///
	/// IssueType: Warning
	///
	/// Title: Property %1.%2 is empty.
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
				  tr("Property %1.%2 is empty.")
				  .arg(objectID)
				  .arg(propertyName));
	}

	/// IssueCode: CFG3017
	///
	/// IssueType: Error
	///
	/// Title: Property %1.%2 is linked to not compatible software %3.
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
				  tr("Property %1.%2 is linked to not compatible software %3.")
				  .arg(objectID)
				  .arg(propertyName)
				  .arg(softwareID));
	}

	/// IssueCode: CFG3018
	///
	/// IssueType: Warning
	///
	/// Title: Default %1 IP address %2:%3 is used in controller %4.
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
				  tr("Default %1 IP address %2:%3 is used in controller %4.")
				  .arg(propertyName)
				  .arg(ip)
				  .arg(port)
				  .arg(controller));
	}

	/// IssueCode: CFG3019
	///
	/// IssueType: Error
	///
	/// Title: Property %1.%2 write error.
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
				  tr("Property %1.%2 write error.").arg(objectID).arg(propertyName));
	}

	/// IssueCode: CFG3020
	///
	/// IssueType: Error
	///
	/// Title: Property %1.%2 is not found.
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
				  tr("Property %1.%2 is not found.").arg(objectID).arg(propertyName));
	}

	/// IssueCode: CFG3021
	///
	/// IssueType: Error
	///
	/// Title: Property %1.%2 is linked to undefined software ID %3.
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
				  tr("Property %1.%2 is linked to undefined software ID %3.")
				  .arg(objectID)
				  .arg(propertyName)
				  .arg(softwareID));
	}

	/// IssueCode: CFG3022
    ///
    /// IssueType: Error
    ///
	/// Title: Property %1.%2 is empty.
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
				  tr("Property %1.%2 is empty.")
                  .arg(objectID)
                  .arg(propertyName));
    }

	/// IssueCode: CFG3023
	///
	/// IssueType: Error
	///
	/// Title: Property %1.%2 conversion error.
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
				  tr("Property %1.%2 conversion error.").arg(objectID).arg(propertyName));
	}

	/// IssueCode: CFG3025
	///
	/// IssueType: Error
	///
	/// Title: Can't find child controller with suffix %1 in object %2
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
				  tr("Can't find child controller with suffix %1 in object %2.")
				  .arg(suffix)
				  .arg(objectID));
	}

	/// IssueCode: CFG3026
	///
	/// IssueType: Error
	///
	/// Title: Value of property %1.%2 is not valid IPv4 address.
	///
	/// Parameters:
	///			%1 Object equipmentID
	///         %2 Property name
	///
	/// Description:
	///			Value of property is not valid IPv4 address. Check property value.
	///
	void IssueLogger::errCFG3026(QString objectID, QString propertyName)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3026,
				  tr("Value of property %1.%2 is not valid IPv4 address.").arg(objectID).arg(propertyName));
	}

	/// IssueCode: CFG3027
	///
	/// IssueType: Error
	///
	/// Title: Ethernet port number property %1.%2 should be in range 0..65535.
	///
	/// Parameters:
	///			%1 Object equipmentID
	///         %2 Property name
	///
	/// Description:
	///			Ethernet port number should in range 1..65535.. Check property value.
	///
	void IssueLogger::errCFG3027(QString objectID, QString propertyName)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3027,
				  tr("Ethernet port number property %1.%2 should be in range 0..65535.").arg(objectID).arg(propertyName));
	}

	/// IssueCode: CFG3028
	///
	/// IssueType: Error
	///
	/// Title: Property %4 must have same value in signals %1 and %2, module %3.
	///
	/// Parameters:
	///         %1 Signal 1 StrID
	///         %2 Signal 2 StrID
	///         %3 Module StrID
	///         %4 Property Name
	///
	/// Description:
	///			Property values should be equal in specified signals of the module.
	///
	void IssueLogger::errCFG3028(QString signalID1, QString signalID2, QString module, QString propertyName)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3028,
				  tr("Property %4 must have same value in signals %1 and %2, module %3.")
				  .arg(signalID1)
				  .arg(signalID2)
				  .arg(module)
				  .arg(propertyName));
	}	

	/// IssueCode: CFG3029
	///
	/// IssueType: Error
	///
	/// Title: Software %1 is not linked to ConfigurationService.
	///
	/// Parameters:
	///			%1 software equipmentID
	///
	/// Description:
	///			Software should be linked to ConfigurationService. Check ConfigurationServiceID1 and ConfigurationServiceID2 properties.
	///
	void IssueLogger::errCFG3029(QString softwareID)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3029,
				  QString(tr("Software %1 is not linked to ConfigurationService.")).arg(softwareID));
	}

	/// IssueCode: CFG3030
	///
	/// IssueType: Error
	///
	/// Title: Etherent adapters 2 and 3 of LM %1 are connected to same AppDataService %2.
	///
	/// Parameters:
	///			%1 LM equipmentID
	///         %2 AppDataService equipmentID
	///
	/// Description:
	///			Two LM's etehrnet adapters can't be connected to same AppDataService. Check LM's ethernet adapters settings.
	///
	void IssueLogger::errCFG3030(QString lmID, QString appDataServiceID)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3030,
				  tr("Etherent adapters 2 and 3 of LM %1 are connected to same AppDataService %2.").
						arg(lmID).arg(appDataServiceID));
	}

	/// IssueCode: CFG3031
	///
	/// IssueType: Warning
	///
	/// Title: Property %1.%2 should be set to the valid writable catalog of workstation.
	///
	/// Parameters:
	///         %1 Object ID
	///         %2 Property name
	///
	/// Description:
	///			Archive location is not assigned.
	///
	void IssueLogger::wrnCFG3031(QString objectID, QString propertyName)
	{
		LOG_WARNING0(IssueType::FscConfiguration,
				  3031,
				  tr("Property %1.%2 should be set to the valid writable catalog of workstation.")
				  .arg(objectID)
				  .arg(propertyName));
	}

	/// IssueCode: CFG3040
	///
	/// IssueType: Error
	///
	/// Title: Mode SingleLmControl is not supported by Monitor. Set TuningServiceID.SingleLmControl to false. Monitor EquipmentID %1, TuningServiceID %2.
	///
	/// Parameters:
	///			%1 Monitor EquipmentID
	///         %2 TuningService EquipmentID
	///
	/// Description:
	///			Mode SingleLmControl is not supported by Monitor. Set TuningServiceID.SingleLmControl to false.
	///
	void IssueLogger::errCFG3040(QString monitorId, QString tuningServiceId)
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3040,
				  tr("Mode SingleLmControl is not supported by Monitor. Set TuningServiceID.SingleLmControl to false. Monitor EquipmentID %1, TuningServiceID %2.")
						.arg(monitorId)
						.arg(tuningServiceId));
	}

	/// IssueCode: CFG3041
	///
	/// IssueType: Error
	///
	/// Title: Property %1 has wrong value (%2), required value is %3 in signal %4.
	///
	/// Parameters:
	///         %1 Property Name
	///         %2 Property Value
	///         %3 Required Value
	///			%4 Module StrID
	///
	/// Description:
	///			Occurs if a property value has wrong value, required value is provided
	///
	void IssueLogger::errCFG3041(QString name, QString value, QString message, QString signalId)	// Property %1 has wrong value (%2), required value is %3 in signal %4.
	{
		LOG_ERROR(IssueType::FscConfiguration,
				  3041,
				  tr("Property %1 has wrong value (%2), required value is %3 in signal %4.")
				  .arg(name)
				  .arg(value)
				  .arg(message)
				  .arg(signalId));
	}

	/// IssueCode: CFG3042
	///
	/// IssueType: Error
	///
	/// Title: Module %1 should be installed in chassis.
	///
	/// Parameters:
	///         %1 Module equipmentID
	///
	/// Description:
	///			Modules should be installed in chassis. Check equipment configuration.
	///
	void IssueLogger::errCFG3042(QString moduleEquipmentID, QUuid moduleUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 3042, moduleUuid);

		LOG_ERROR(IssueType::FscConfiguration,
				  3042,
				  tr("Module %1 should be installed in chassis")
				  .arg(moduleEquipmentID));
	}

	//
	// ALP			Application Logic Parsing				4000-4999
	//

	/// IssueCode: ALP4000
	///
	/// IssueType: Error
	///
	/// Title: Branch has multiple outputs (Logic Schema %1).
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///		Error may occur if there are more than one output is linked to input.
	///
	void IssueLogger::errALP4000(QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, 4000, itemsUuids, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4000,
				  tr("Branch has multiple outputs (Logic Schema %1).")
				  .arg(schema));
	}

	/// IssueCode: ALP4001
	///
	/// IssueType: Error
	///
	/// Title: Property %1 for Schema is not set (LogicSchema %2).
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
		addSchemaIssue(OutputMessageLevel::Error, 4001, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4001,
				  tr("Property %1 for Schema is not set (LogicSchema %2).")
				  .arg(propertyName)
				  .arg(schema));
	}

	/// IssueCode: ALP4002
	///
	/// IssueType: Error
	///
	/// Title: EquipmentID %1 is not found in the project equipment (LogicSchema %2).
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
		addSchemaIssue(OutputMessageLevel::Error, 4002, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4002,
				  tr("EquipmentID %1 is not found in the project equipment (LogicSchema %2).")
				  .arg(equipmentId)
				  .arg(schema));
	}

	/// IssueCode: ALP4003
	///
	/// IssueType: Error
	///
	/// Title: EquipmentID %1 must be LM family module type (LogicSchema %2).
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
		addSchemaIssue(OutputMessageLevel::Error, 4003, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4003,
				  tr("EquipmentID %1 must be LM family module type (LogicSchema %2).")
				  .arg(equipmentId)
				  .arg(schema));
	}

	/// IssueCode: ALP4004
	///
	/// IssueType: Warning
	///
	/// Title: Schema %1 is excluded from build.
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
		addSchemaIssue(OutputMessageLevel::Warning1, 4004, schema);

		LOG_WARNING1(IssueType::AlParsing,
					4004,
					tr("Schema %1 is excluded from build.")
					.arg(schema));
	}

	/// IssueCode: ALP4005
	///
	/// IssueType: Warning
	///
	/// Title: Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema %1).
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///			Logic Schema is empty, there are no any functional blocks in the compile layer.
	///
	void IssueLogger::wrnALP4005(QString schema)
	{
		addSchemaIssue(OutputMessageLevel::Warning2,
					   4005,
					   schema);

		LOG_WARNING2(IssueType::AlParsing,
					4005,
					tr("Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema %1).")
					.arg(schema));
	}

	/// IssueCode: ALP4006
	///
	/// IssueType: Error
	///
	/// Title: Schema item %1 has unlinked pin(s) %2 (Logic Schema %3).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4006,
					   itemsUuids,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4006,
				  tr("Schema item %1 has unlinked pin(s) %2 (Logic Schema %3).")
				  .arg(schemaItem)
				  .arg(pin)
				  .arg(schema));
	}

	/// IssueCode: ALP4007
	///
	/// IssueType: Error
	///
	/// Title: AFB description %1 is not found for schema item %2 (Logic Schema %3).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4007,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4007,
				  tr("AFB description %1 is not found for schema item %2 (Logic Schema %3).")
				  .arg(afbElement)
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4008
	///
	/// IssueType: Error
	///
	/// Title: SchemaItem %1 has outdated AFB description version, item's AFB.version %2, the latest is %3 (LogicSchema %4).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4008,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4008,
				  tr("SchemaItem %1 has outdated AFB description version, item's AFB.version %2, the latest is %3 (LogicSchema %4).")
				  .arg(schemaItem)
				  .arg(schemaItemAfbVersion)
				  .arg(latesAfbVersion)
				  .arg(schema));
	}

	/// IssueCode: ALP4009
	///
	/// IssueType: Error
	///
	/// Title: UFB schema %1 is not found for schema item %2 (Logic Schema %3).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4009,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4009,
				  tr("UFB schema %1 is not found for schema item %2 (Logic Schema %3).")
				  .arg(ufbElement)
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4010
	///
	/// IssueType: Error
	///
	/// Title: SchemaItem %1 has outdated UFB version, item's UFB.version %2, the latest is %3 (LogicSchema %4).
	///
	/// Parameters:
	///		%1 UFB Schema ID
	///		%2 Schema item description
	///		%3 Logic schema StrID
	///
	/// Description:
	///		SchemaItem %1 has outdated UFB version, item's UFB.version %2, the latest is %3 (LogicSchema %4). Open schema to upfate AFBs.
	///
	void IssueLogger::errALP4010(QString schema, QString schemaItem, int schemaItemUfbVersion, int latesUfbVersion, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error,
					   4010,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4010,
				  tr("SchemaItem %1 has outdated UFB version, item's UFB.version %2, the latest is %3 (LogicSchema %4).")
				  .arg(schemaItem)
				  .arg(schemaItemUfbVersion)
				  .arg(latesUfbVersion)
				  .arg(schema));
	}

	/// IssueCode: ALP4011
	///
	/// IssueType: Error
	///
	/// Title: User Functional Block cannot have nested another UFB, SchemaItem %1 (UfbSchema %2).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4011,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4011,
				  tr("User Functional Block cannot have nested another UFB, SchemaItem %1 (UfbSchema %2).")
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4012
	///
	/// IssueType: Error
	///
	/// Title: Cannot find %1 input/output in UFB %2, SchemaItem %1 (LogicSchema  %3).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4012,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4012,
				  tr("Cannot find %1 input/output in UFB %2, SchemaItem %1 (LogicSchema %3).")
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
		addItemsIssues(OutputMessageLevel::Error,
					   4013,
					   itemUuid,
					   schema);

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
	/// Title: User Functional Block cannot contain %1, SchemaItem %2 (UfbSchema %3).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4014,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4014,
				  tr("User Functional Block cannot contain %1, SchemaItem %2 (UfbSchema %3).")
				  .arg(itemType)
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4015
	///
	/// IssueType: Error
	///
	/// Title: UFB Input or Output item must have only ONE assigned AppSignalIDs, SchemaItem %1 (UfbSchema %2).
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
		addItemsIssues(OutputMessageLevel::Error,
					   4015,
					   itemUuid,
					   schema);

		LOG_ERROR(IssueType::AlParsing,
				  4015,
				  tr("UFB Schema Input or Output item must have only ONE assigned AppSignalIDs, SchemaItem %1 (UfbSchema %2).")
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4016
	///
	/// IssueType: Error
	///
	/// Title: File LmDescriptionFile %1 is not found (Schema %2).
	///
	/// Parameters:
	///		%1 SchemaID
	///
	/// Description:
	///		File LmDescriptionFile %1 is not found (Schema %2).
	///
	void IssueLogger::errALP4016(QString schema, QString lmDecriptionFile)
	{
		addSchemaIssue(OutputMessageLevel::Error, 4016, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4016,
				  tr("File LmDescriptionFile %1 is not found (Schema %2).")
					.arg(lmDecriptionFile)
					.arg(schema));
	}

	/// IssueCode: ALP4017
	///
	/// IssueType: Error
	///
	/// Title: AfbComponent with OpCode %1 is not found in file %2 (Schema %3).
	///
	/// Parameters:
	///		%1 OpCode
	///		%2 LmDescription filename
	///		%3 Schema
	///
	/// Description:
	///		AfbComponent with OpCode %1 is not found in file %2 (Schema %3).
	///
	void IssueLogger::errALP4017(QString schema, QString lmDecriptionFile, int opCode)
	{
		addSchemaIssue(OutputMessageLevel::Error, 4017, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4017,
				  tr("AfbComponent with OpCode %1 is not found in file %2 (Schema %3).")
					.arg(opCode)
					.arg(lmDecriptionFile)
					.arg(schema));
	}

	/// IssueCode: ALP4017
	///
	/// IssueType: Error
	///
	/// Title: AfbComponent with OpCode %1 is not found in file %2 (Schema %3).
	///
	/// Parameters:
	///		%1 OpCode
	///		%2 LmDescription filename
	///		%3 Schema
	///
	/// Description:
	///		AfbComponent with OpCode %1 is not found in file %2 (Schema %3).
	///
	void IssueLogger::errALP4017(QString schema, QString lmDecriptionFile, int opCode, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4017, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4017,
				  tr("AfbComponent with OpCode %1 is not found in file %2 (Schema %3).")
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
		addSchemaIssue(OutputMessageLevel::Error, 4018, schema);

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
		addItemsIssues(OutputMessageLevel::Error, 4019, itemUuid, schema);

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
	/// Title: There is no any input element in applictaion logic for Logic Module %1.
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
				  tr("There is no any input element in applictaion logic for Logic Module %1.")
				  .arg(logicModule));
	}

	/// IssueCode: ALP4021
	///
	/// IssueType: Error
	///
	/// Title: Duplicate output signal %1, item %2 on schema %3, item %4 on schema %5 (Logic Module %6).
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
		addSchemaIssue(OutputMessageLevel::Error, 4021, schema1);
		addSchemaIssue(OutputMessageLevel::Error, 4021, schema2);
		addItemsIssues(OutputMessageLevel::Error, 4021, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4021,
				  tr("Duplicate output signal %1, item %2 on schema %3, item %4 on schema %5 (Logic Module %6).")
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
	/// Title: Schema does not have logic layer (Schema %1).
	///
	/// Parameters:
	///		%1 SchemaID
	///
	/// Description:
	///		Each logic schema or user functional block schema has several layers (Logic, Frame and Notes), but the logic layer is not found.
	///
	void IssueLogger::errALP4022(QString schema)
	{
		addSchemaIssue(OutputMessageLevel::Error, 4022, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4022,
				  tr("Schema does not have logic layer (Schema %1).").arg(schema));
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
		addItemsIssues(OutputMessageLevel::Error, 4023, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4023,
				  tr("UFB schema has duplicate pins %1 (UFB schema %2).").arg(pinCaption).arg(schema));
	}

	/// IssueCode: ALP4024
	///
	/// IssueType: Error
	///
	/// Title: Schema details parsing error, filename %1, details string %2.
	///
	/// Parameters:
	///		%1 Schema file name
	///		%2 Detaisl string
	///
	/// Description:
	///		Schema details parsing error.
	///
	void IssueLogger::errALP4024(QString fileName, QString details)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4024,
				  tr("Schema details parsing error, filename %1, details string %2.").arg(fileName).arg(details));
	}

	/// IssueCode: ALP4025
	///
	/// IssueType: Error
	///
	/// Title: Duplicate SchemaIDs %1, all schemas (including Monitor, Tuning, etc) must have unique SchemaIDs.
	///
	/// Parameters:
	///		%1 SchemaID
	///
	/// Description:
	///		$root$/Schema/ Has duplicate SchemaIDs.
	///
	void IssueLogger::errALP4025(QString schema)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4025,
				  tr("Duplicate SchemaIDs %1, all schemas (including Monitor, Tuning, etc) must have unique SchemaIDs.").arg(schema));
	}


	/// IssueCode: ALP4040
	///
	/// IssueType: Error
	///
	/// Title: BusTypeID %1 is not found for schema item %2 (Logic Schema %3).
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
		addItemsIssues(OutputMessageLevel::Error, 4040, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4040,
				  tr("BusTypeID %1 is not found for schema item %2 (Logic Schema %3).")
				  .arg(busTypeId)
				  .arg(schemaItem)
				  .arg(schema));
	}


	/// IssueCode: ALP4041
	///
	/// IssueType: Error
	///
	/// Title: SchemaItem %1 has outdated BusType description (LogicSchema %2).
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
		addItemsIssues(OutputMessageLevel::Error, 4041, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4041,
				  tr("SchemaItem %1 has outdated BusType description (LogicSchema %2).")
				  .arg(schemaItem)
				  .arg(schema));
	}

	/// IssueCode: ALP4060
	///
	/// IssueType: Error
	///
	/// Title: Loopback detected in LogicSchema %1, SchemaItem %2. To resolve issue use Loopback Source/Target items.
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
		addItemsIssues(OutputMessageLevel::Error, 4060, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4060,
				  tr("Loopback detected in LogicSchema %1, SchemaItem %2. To resolve issue use Loopback Source/Target items.")
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4061
	///
	/// IssueType: Error
	///
	/// Title: Two or more LoopbackSource have the same LoopbackID %1, Schema %2.
	///
	/// Parameters:
	///		%1 LoopbackID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Two or more LoopbackSource have the same LoopbackID.
	///
	void IssueLogger::errALP4061(QString schema, QString loopbackId, const std::vector<QUuid>& itemUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, 4061, itemUuids, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4061,
				  tr("Two or more LoopbackSource have the same LoopbackID %1, Schema %2.")
				  .arg(loopbackId)
				  .arg(schema));
	}

	/// IssueCode: ALP4070
	///
	/// IssueType: Warning
	///
	/// Title: Schema %1 has %2 commented functional item(s).
	///
	/// Parameters:
	///		%1 SchemaID
	///		%2 Commennted item(s) count
	///
	/// Description:
	///			Schema has one or more commented functional items.
	///
	void IssueLogger::wrnALP4070(QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Warning2, 4070, itemsUuids, schema);

		LOG_WARNING2(IssueType::AlParsing,
					4070,
					tr("Schema %1 has %2 commented functional item(s).")
						.arg(schema)
						.arg(itemsUuids.size()));
	}

	/// IssueCode: ALP4080
	///
	/// IssueType: Error
	///
	/// Title: SchemaItemFrame has reference (property SchemaID) to unknown schema %1, Schema %2.
	///
	/// Parameters:
	///		%1 Not found schema ID
	///		%2 Schema ID
	///
	/// Description:
	///			Schema has SchemaItemFrame wchich has reference (property SchemaID) to unknown schema.
	///
	void IssueLogger::errALP4080(QString schema, QString frameSchemaId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4080, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4080,
				  tr("SchemaItemFrame has reference (property SchemaID) to unknown schema %1, Schema %2.")
						.arg(frameSchemaId)
						.arg(schema));
	}

	/// IssueCode: ALP4081
	///
	/// IssueType: Error
	///
	/// Title: SchemaItemFrame.SchemaID has recursive reference to schema %1, property must be distincive from schema where it is placed.
	///
	/// Parameters:
	///		%1 Schema ID
	///
	/// Description:
	///			SchemaItemFrame.SchemaID has recursive reference to the same schema where it is placed.
	///
	void IssueLogger::errALP4081(QString schema, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4081, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4081,
				  tr("SchemaItemFrame.SchemaID has recursive reference to schema %1, property must be distincive from schema where it is placed.")
						.arg(schema));
	}

	/// IssueCode: ALP4082
	///
	/// IssueType: Error
	///
	/// Title: SchemaItemFrame has reference to schema %1 which has different units and SchemaItemFrame.AutoScale is false, Schema %2.
	///
	/// Parameters:
	///		%1 Not found schema ID
	///		%2 Schema ID
	///
	/// Description:
	///			SchemaItemFrame has reference to schema which has different units and SchemaItemFrame.AutoScale is false. If AutoScale is false then both schemas must have identical units.
	///
	void IssueLogger::errALP4082(QString schema, QString frameSchemaId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4082, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4082,
				  tr("SchemaItemFrame has reference to schema %1 which has different units and SchemaItemFrame.AutoScale is false, Schema %2.")
						.arg(frameSchemaId)
						.arg(schema));
	}

	/// IssueCode: ALP4130
	///
	/// IssueType: Error
	///
	/// Title: Singlechannel Logic Schema %1 cannot contain multichannel signal block (%2).
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///		%2 Schema item description
	///
	/// Description:
	///		Singlechannel Logic Schema %1 cannot contain multichannel signal blocks (%2). Only one signal can be assigned for
	/// input/output/internal signal elements.
	///
	void IssueLogger::errALP4130(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4130, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4130,
				  tr("Singlechannel Logic Schema %1 cannot contain multichannel signal block (%2).")
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
		addItemsIssues(OutputMessageLevel::Error, 4131, itemUuid, schema);

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
	/// Title: Schema contains mixed singlechannel and multichannel SignalItems in the branch (LogicSchema %1).
	///
	/// Parameters:
	///		%1 Logic Schema ID
	///
	/// Description:
	///		Schema contains mixed singlechannel and multichannel SignalItems in the branch (LogicSchema %1).
	/// All Inputs/Outputs/Interconnection Signal elements must be the same type.
	///
	void IssueLogger::errALP4132(QString schema, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, 4132, itemsUuids, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4132,
				  tr("Schema contains mixed singlechannel and multichannel SignalItems in the branch (LogicSchema %1).")
				  .arg(schema));
	}

	/// IssueCode: ALP4133
	///
	/// IssueType: Error
	///
	/// Title: Branch contains signals (%1) from different channels (LogicSchema %2).
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
		addItemsIssues(OutputMessageLevel::Error, 4133, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4133,
				  tr("Branch contains signals (%1) from different channels (LogicSchema %2).")
				  .arg(appSignalId)
				  .arg(schema));
	}

	/// IssueCode: ALP4134
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is not found (LogicSchema %2, SchemaItem %3).
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 LogicSchemaID
	///		%3 SchemaItem description
	///
	/// Description:
	///		Signal %1 is not found (LogicSchema %2, SchemaItem %3).
	///
	void IssueLogger::errALP4134(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4134, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4134,
				  tr("Signal %1 is not found (LogicSchema %2, SchemaItem %3).")
				  .arg(appSignalId)
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4135
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 does not have valid LM (LogicSchema %2, SchemaItem %3).
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 LogicSchemaID
	///		%3 SchemaItem description
	///
	/// Description:
	///		Signal %1 does not have valid LM (LogicSchema %2, SchemaItem %3).
	///
	void IssueLogger::errALP4135(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4135, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4135,
				  tr("Signal %1 does not have valid LM (LogicSchema %2, SchemaItem %3).")
				  .arg(appSignalId)
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4136
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is not bound to any schema's EquipmentIds(LMs), (LogicSchema %2, SchemaItem %3).
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 LogicSchemaID
	///		%3 SchemaItem description
	///
	/// Description:
	///		Signal %1 is not bound to any schema's EquipmentIds(LMs), (LogicSchema %2, SchemaItem %3).
	///
	void IssueLogger::errALP4136(QString schema, QString schemaItem, QString appSignalId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4136, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4136,
				  tr("Signal %1 is not bound to any schema's EquipmentIds(LMs), (LogicSchema %2, SchemaItem %3).")
				  .arg(appSignalId)
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4137
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is expected to be bound to EquipmentId(LM) %2 (LogicSchema %3, SchemaItem %4).
	///
	/// Parameters:
	///		%1 AppSignalID
	///		%2 EquipmentID of LOM
	///		%3 LogicSchemaID
	///		%4 SchemaItem description
	///
	/// Description:
	///		Signal %1 is expected to be bound to EquipmentId(LM) %2 (LogicSchema %3, SchemaItem %4).
	///
	void IssueLogger::errALP4137(QString schema, QString schemaItem, QString appSignalId, QString equipmentId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4137, itemUuid, schema);

		LOG_ERROR(IssueType::AlParsing,
				  4137,
				  tr("Signal %1 is expected to be bound to EquipmentId(LM) %2 (LogicSchema %3, SchemaItem %4).")
				  .arg(appSignalId)
				  .arg(equipmentId)
				  .arg(schema)
				  .arg(schemaItem));
	}

	/// IssueCode: ALP4150
	///
	/// IssueType: Error
	///
	/// Title:	   Schema item (%1) has connection(s) (%2) which is not accessible in logic schema's (%3) associated LM(s): %4.
	///
	/// Parameters:
	///		%1 SchemaItem description
	///		%2 Connection ID
	///		%3 Logic schema ID
	///		%4 Logic module EquipmentID(s)
	///
	/// Description:
	///		Connection item (Transmitter or Receiver) has connection(s) which is not accessible in logic schema's associated LM(s).
	///
	void IssueLogger::errALP4150(QString schema, QString schemaItem, QString connectionId, QString equipmentsIds, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4150, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4150,
				  QString(tr("Schema item (%1) has connection(s) (%2) which is not accessible in logic schema's (%3) associated LM(s): %4."))
						.arg(schemaItem)
						.arg(connectionId)
						.arg(schema)
						.arg(equipmentsIds));
	}

	/// IssueCode: ALP4152
	///
	/// IssueType: Error
	///
	/// Title:		Connection in singlechannel schema usage must have one ConnectionID for multichannel case it must have the same number of ConnectionIDs as logic schema.
	/// must have the only AppSignalID per channel, LogicSchemaID: %1, Receiver Item: %2, ConnectionID: %3, EquipmentID %4.
	///
	/// Parameters:
	///		%1 Logic Schema ID
	///		%2 Receiver item description
	///		%3 ConnectionIDs
	///		%4 Channel EquipmentID(s)
	///
	/// Description:
	///		Receiver must have the only AppSignalID per channel.
	///
	void IssueLogger::errALP4152(QString schema, QString schemaItem, QString connectionId, QString equipmentsId, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4152, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4152,
				  QString(tr("Receiver must have the only AppSignalID per channel, LogicSchemaID: %1, Receiver Item: %2, ConnectionID: %3, EquipmentID %4."))
						.arg(schema)
						.arg(schemaItem)
						.arg(connectionId)
						.arg(equipmentsId));
	}

	/// IssueCode: ALP4153
	///
	/// IssueType: Error
	///
	///
	/// Title:		Multichannel transmitter/receiver must have the same number of ConnectionIDs as schema's channel number (number of schema's EquipmentIDs), LogicSchema %2, SchemaItem %3.
	///
	/// Parameters:
	///		%1 Logic Schema ID
	///		%2 Transmitter item description
	///
	/// Description:
	///		Multichannel transmitter/receiver must have the same number of ConnectionIDs as schema's channel number (number of schema's EquipmentIDs).
	///
	void IssueLogger::errALP4153(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4153, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4153,
				  QString(tr("Multichannel transmitter/receiver must have the same number of ConnectionIDs as schema's channel number (number of schema's EquipmentIDs), LogicSchema %2, SchemaItem %3.")
						.arg(schema)
						.arg(schemaItem)));
	}

	/// IssueCode: ALP4154
	///
	/// IssueType: Error
	///
	/// Title:		Property ConnectionID is empty (LogicSchema %1, SchemaItem %2).
	///
	/// Parameters:
	///		%1 Logic Schema ID
	///		%2 Receiver item description
	///
	/// Description:
	///		Property ConnectionID for Receiver/Transmitter must not be empty.
	///
	void IssueLogger::errALP4154(QString schema, QString schemaItem, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 4154, itemUuid);

		LOG_ERROR(IssueType::AlParsing,
				  4154,
				  QString(tr("Property ConnectionID for is empty, for Receiver/Transmitter it must not be empty (LogicSchema %1, SchemaItem %2).")
						.arg(schema)
						.arg(schemaItem)));
	}


	// ALC			Application logic compiler				5000-5999
	//

	/// IssueCode: ALC5000
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is not found in Application Signals (Logic schema %2).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Signal idendifier is not found in application signals.
	///
	void IssueLogger::errALC5000(QString appSignalID, QUuid itemUuid, QString schemaID)
	{
		if (schemaID.isEmpty() == true)
		{
			addItemsIssues(OutputMessageLevel::Error, 5000, itemUuid);
		}
		else
		{
			addItemsIssues(OutputMessageLevel::Error, 5000, itemUuid, schemaID);
		}

		LOG_ERROR(IssueType::AlCompiler,
				  5000,
				  tr("Signal %1 is not found in Application Signals (Logic schema %2).").arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5001
	///
	/// IssueType: Warning
	///
	/// Title: Application logic for module %1 is not found.
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
				  tr("Application logic for module %1 is not found.").arg(logicModuleID));
	}


	/// IssueCode: ALC5002
	///
	/// IssueType: Error
	///
	/// Title: Value of signal %1 is undefined (Logic schema %2).
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
		addItemsIssues(OutputMessageLevel::Error, 5002, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5002,
				  tr("Value of signal %1 is undefined (Logic schema %2).").
				  arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5003
	///
	/// IssueType: Error
	///
	/// Title: Analog output %1.%2 is connected to discrete signal %3.
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
		addItemsIssues(OutputMessageLevel::Error, 5003, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5003,
				  tr("Analog output %1.%2 is connected to discrete signal %3.").arg(afbCaption).arg(output).arg(appSignalID));
	}

	/// IssueCode: ALC5004
	///
	/// IssueType: Error
	///
	/// Title: Output %1.%2 is connected to signal %3 with uncompatible data format.
	///
	/// Parameters:
	///		%1 AFB caption
	///		%2 AFB output
	///		%3 Application signal ID
	///
	/// Description:
	///		Outpuf of AFB is connected to signal with uncompatible data format.
	///
	void IssueLogger::errALC5004(QString afbCaption, QString output, QString appSignalID, QUuid signalUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5004, signalUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5004,
				  tr("Output %1.%2 is connected to signal %3 with uncompatible data format. (Logic schema %4)").
						arg(afbCaption).arg(output).arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5005
	///
	/// IssueType: Error
	///
	/// Title: Output %1.%2 is connected to signal %3 with uncompatible data size.
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
		addItemsIssues(OutputMessageLevel::Error, 5005, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5005,
				  tr("Output %1.%2 is connected to signal %3 with uncompatible data size.").arg(afbCaption).arg(output).arg(appSignalID));
	}

	/// IssueCode: ALC5006
	///
	/// IssueType: Error
	///
	/// Title: Discrete output %1.%2 is connected to analog signal %3.
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
		addItemsIssues(OutputMessageLevel::Error, 5006, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5006,
				  tr("Discrete output %1.%2 is connected to analog signal %3.").arg(afbCaption).arg(output).arg(appSignalID));
	}


	/// IssueCode: ALC5007
	///
	/// IssueType: Error
	///
	/// Title: Discrete signal %1 is connected to analog input %2.%3.
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
		addItemsIssues(OutputMessageLevel::Error, 5007, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5007,
				  tr("Discrete signal %1 is connected to analog input %2.%3.").arg(appSignalID).arg(afbCaption).arg(input));
	}

	/// IssueCode: ALC5008
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is connected to input %2.%3 with uncompatible data format.
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
		addItemsIssues(OutputMessageLevel::Error, 5008, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5008,
				  tr("Signal %1 is connected to input %2.%3 with uncompatible data format. (Logic schema %4)").
					arg(appSignalID).arg(afbCaption).arg(input).arg(schemaID));
	}


	/// IssueCode: ALC5009
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is connected to input %2.%3 with uncompatible data size.
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
		addItemsIssues(OutputMessageLevel::Error, 5009, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5009,
				  tr("Signal %1 is connected to input %2.%3 with uncompatible data size.").arg(appSignalID).arg(afbCaption).arg(input));
	}

	/// IssueCode: ALC5010
	///
	/// IssueType: Error
	///
	/// Title: Analog signal %1 is connected to discrete input %2.%3 (Logic schema %4).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 AFB caption
	///		%3 AFB input
	///		%4 Logic schema ID
	///
	/// Description:
	///		Discrete signal is connected to analog input of AFB.
	///
	void IssueLogger::errALC5010(QString appSignalID, QString afbCaption, QString input, QUuid signalUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5010, signalUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5010,
				  tr("Analog signal %1 is connected to discrete input %2.%3 (Logic schema %4).").
				  arg(appSignalID).arg(afbCaption).arg(input).arg(schemaID));
	}

	/// IssueCode: ALC5011
	///
	/// IssueType: Error
	///
	/// Title: Application item %1 has unknown type, SchemaID %2.
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
		addItemsIssues(OutputMessageLevel::Error, 5011, itemUuid, schemaId);

		LOG_ERROR(IssueType::AlCompiler,
				  5011,
				  tr("Application item %1 has unknown type, SchemaID %2. Contact to the RPCT developers.")
					.arg(itemLabel)
					.arg(schemaId));
	}

	/// IssueCode: ALC5012
	///
	/// IssueType: Warning
	///
	/// Title: Application signal %1 is not bound to any device object.
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
				  tr("Application signal %1 is not bound to any device object.").arg(appSignalID));
	}

	/// IssueCode: ALC5013
	///
	/// IssueType: Error
	///
	/// Title: Application signal %1 is bound to unknown device object %2.
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
				  tr("Application signal %1 is bound to unknown device object %2.").
					arg(appSignalID).arg(equipmentID));
	}

	/// IssueCode: ALC5014
	///
	/// IssueType: Error
	///
	/// Title: Discrete signal %1 must have DataSize equal to 1.
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
				  tr("Discrete signal %1 must have DataSize equal to 1.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5015
	///
	/// IssueType: Error
	///
	/// Title: Analog signal %1 must have DataSize equal to 32.
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
				  tr("Analog signal %1 must have DataSize equal to 32.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5016
	///
	/// IssueType: Error
	///
	/// Title: Application signal identifier %1 is not unique.
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
				  tr("Application signal identifier %1 is not unique.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5017
	///
	/// IssueType: Error
	///
	/// Title: Custom application signal identifier %1 is not unique.
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
				  tr("Custom application signal identifier %1 is not unique.").
					arg(appSignalID));
	}

	/// IssueCode: ALC5018
	///
	/// IssueType: Error
	///
	/// Title: Opto ports %1 and %2 are not compatible (connection %3).
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
				  QString(tr("Opto ports %1 and %2 are not compatible (connection %3).")).
				  arg(port1).arg(port2).arg(connection));
	}

	/// IssueCode: ALC5019
	///
	/// IssueType: Error
	///
	/// Title: Opto port %1 of connection %2 is already used in connection %3.
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
				  QString(tr("Opto port %1 of connection %2 is already used in connection %3.")).
				  arg(port).arg(connection1).arg(connection2));
	}

	/// IssueCode: ALC5020
	///
	/// IssueType: Error
	///
	/// Title: LM's opto port %1 can't work in RS232/485 mode (connection %2).
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
				  QString(tr("LM's opto port %1 can't work in RS232/485 mode (connection %2).")).
				  arg(port).arg(connection));
	}

	/// IssueCode: ALC5021
	///
	/// IssueType: Error
	///
	/// Title: Undefined opto port %1 in the connection %2.
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
				  QString(tr("Undefined opto port %1 in the connection %2.")).
				  arg(port).arg(connection));
	}

	/// IssueCode: ALC5022
	///
	/// IssueType: Error
	///
	/// Title: Opto ports of the same chassis are linked via connection %1.
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
				  QString(tr("Opto ports of the same chassis is linked via connection %1.")).
				  arg(connection));
	}

	/// IssueCode: ALC5023
	///
	/// IssueType: Error
	///
	/// Title: Opto connection ID %1 is not unique.
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
				  QString(tr("Opto connection ID %1 is not unique.")).
				  arg(connection));
	}

	/// IssueCode: ALC5024
	///
	/// IssueType: Error
	///
	/// Title: Transmitter is linked to unknown opto connection %1 (Logic schema %2).
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
		addItemsIssues(OutputMessageLevel::Error, 5024, transmitterUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5024,
				  QString(tr("Transmitter is linked to unknown opto connection %1 (Logic schema %2).")).
				  arg(connection).arg(schemaID));
	}

	/// IssueCode: ALC5025
	///
	/// IssueType: Error
	///
	/// Title: Receiver is linked to unknown opto connection %1.
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
		addItemsIssues(OutputMessageLevel::Error, 5025, receiverUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5025,
				  QString(tr("Receiver is linked to unknown opto connection %1 (Logic schema %2).")).
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
		addItemsIssues(OutputMessageLevel::Error, 5026, transmitterUuid);

		for(QUuid signalID : signalIDs)
		{
			addItemsIssues(OutputMessageLevel::Error, 5026, signalID);
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
	void IssueLogger::errALC5027(QUuid transmitterUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5027, transmitterUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5027,
				  QString(tr("All transmitter inputs must be directly linked to a signals. (Logic schema %1)")).arg(schemaID));
	}

	/// IssueCode: ALC5028
	///
	/// IssueType: Error
	///
	/// Title: Uncompatible constant type (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logis schema ID
	///
	/// Description:
	///		Constant is not compatible to destination.
	///
	void IssueLogger::errALC5028(QUuid constUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5028, constUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5028,
				  QString(tr("Uncompatible constant type (Logic schema %1).").arg(schemaID)));
	}

	/// IssueCode: ALC5030
	///
	/// IssueType: Error
	///
	/// Title: The signal %1 is not associated with LM %2.
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
		addItemsIssues(OutputMessageLevel::Error, 5030, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5030,
				  QString(tr("The signal %1 is not associated with LM %2.").
						  arg(appSignalID).arg(lmEquipmentID)));
	}

	/// IssueCode: ALC5031
	///
	/// IssueType: Error
	///
	/// Title: The signal %1 can be bind only to Logic Module or Equipment Signal.
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
				  QString(tr("The signal %1 can be bind only to Logic Module or Equipment Signal.").
						  arg(appSignalID)));
	}


	/// IssueCode: ALC5032
	///
	/// IssueType: Error
	///
	/// Title: TxData size (%1 words) of opto port %2 exceed value of OptoPortAppDataSize property of module %3 (%4 words).
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
				  QString(tr("TxData size (%1 words) of opto port %2 exceed value of OptoPortAppDataSize property of module %3 (%4 words).")).
						  arg(txDataSize).arg(optoPortID).arg(moduleID).arg(optoPortAppDataSize));
	}


	/// IssueCode: ALC5033
	///
	/// IssueType: Error
	///
	/// Title: Can't find logic module associated with signal %1 (no LM in chassis %2).
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
				  QString(tr("Can't find logic module associated with signal %1 (no LM in chassis %2).").
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
		addItemsIssues(OutputMessageLevel::Error, 5034, transmitterUuid);
		addItemsIssues(OutputMessageLevel::Error, 5034, connectedItemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5034,
				  tr("Non-signal element is connected to transmitter."));
	}


	/// IssueCode: ALC5035
	///
	/// IssueType: Error
	///
	/// Title: RxData size (%1 words) of opto port %2 exceed value of OptoPortAppDataSize property of module %3 (%4 words).
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
				  QString(tr("RxData size (%1 words) of opto port %2 exceed value of OptoPortAppDataSize property of module %3 (%4 words).")).
						  arg(rxDataSize).arg(optoPortID).arg(moduleID).arg(optoPortAppDataSize));
	}

	/// IssueCode: ALC5036
	///
	/// IssueType: Error
	///
	/// Title: Analog signal %1 is connected to discrete signal %3.
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
		addItemsIssues(OutputMessageLevel::Error, 5036, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, 5036, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5036,
				  QString(tr("Analog signal %1 is connected to discrete signal %2.")).
						  arg(srcSignalID).arg(destSignalID));
	}


	/// IssueCode: ALC5037
	///
	/// IssueType: Error
	///
	/// Title: Discrete signal %1 is connected to analog signal %3.
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
		addItemsIssues(OutputMessageLevel::Error, 5037, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, 5037, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5037,
				  QString(tr("Discrete signal %1 is connected to analog signal %2.")).
						  arg(srcSignalID).arg(destSignalID));
	}

	/// IssueCode: ALC5038
	///
	/// IssueType: Error
	///
	/// Title: Signals %1 and %2 have different data format.
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
		addItemsIssues(OutputMessageLevel::Error, 5038, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, 5038, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5038,
				  QString(tr("Signals %1 and %2 have different data format.")).
						  arg(srcSignalID).arg(destSignalID));
	}

	/// IssueCode: ALC5039
	///
	/// IssueType: Error
	///
	/// Title: Signals %1 and %2 have different data size.
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
		addItemsIssues(OutputMessageLevel::Error, 5039, srcUuid);
		addItemsIssues(OutputMessageLevel::Error, 5039, destUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5039,
				  QString(tr("Signals %1 and %2 have different data size.")).
						  arg(srcSignalID).arg(destSignalID));
	}

	/// IssueCode: ALC5040
	///
	/// IssueType: Error
	///
	/// Title: Connection with ID %1 is not found.
	///
	/// Parameters:
	///		%1 connection ID
	///
	/// Description:
	///		Connection with specified identifier is not found. Check connection ID.
	///
	void IssueLogger::errALC5040(QString connectionID, QUuid item)
	{
		addItemsIssues(OutputMessageLevel::Error, 5040, item);

		LOG_ERROR(IssueType::AlCompiler,
				  5040,
				  QString(tr("Connection with ID %1 is not found.")).arg(connectionID));
	}

	/// IssueCode: ALC5041
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 exists in LM %2. No receivers needed.
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
		addItemsIssues(OutputMessageLevel::Error, 5041, receiverUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5041,
				  QString(tr("Signal %1 exists in LM %2. No receivers needed.")).
				  arg(appSignalID).arg(lmID));
	}

	/// IssueCode: ALC5042
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is not exists in connection %2.
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
		addItemsIssues(OutputMessageLevel::Error, 5042, receiverUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5042,
				  QString(tr("Signal %1 is not exists in connection %2 (Logic schema %3).")).
						arg(appSignalID).
						arg(connectionID).
						arg(schemaID));
	}

	/// IssueCode: ALC5043
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter %1.%2 must be greater or equal to 0.
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
		addItemsIssues(OutputMessageLevel::Error, 5043, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5043,
				  QString(tr("Value of parameter %1.%2 must be greater or equal to 0.")).
				  arg(fbCaption).arg(paramCaption));
	}

	/// IssueCode: ALC5044
	///
	/// IssueType: Error
	///
	/// Title: Parameter's calculation for AFB %1 (opcode %2) is not implemented.
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
		addItemsIssues(OutputMessageLevel::Error, 5044, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5044,
				  QString(tr("Parameter's calculation for AFB %1 (opcode %2) is not implemented.")).
				  arg(fbCaption).arg(opcode));
	}

	/// IssueCode: ALC5045
	///
	/// IssueType: Error
	///
	/// Title: Required parameter %1 of AFB %2 is missing.
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
		addItemsIssues(OutputMessageLevel::Error, 5045, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5045,
				  QString(tr("Required parameter %1 of AFB %2 is missing.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5046
	///
	/// IssueType: Error
	///
	/// Title: Parameter %1 of AFB %2 must have type Unsigned Int.
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
		addItemsIssues(OutputMessageLevel::Error, 5046, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5046,
				  QString(tr("Parameter %1 of AFB %2 must have type Unsigned Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5047
	///
	/// IssueType: Error
	///
	/// Title: Parameter %1 of AFB %2 must have type 16-bit Unsigned Int.
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
		addItemsIssues(OutputMessageLevel::Error, 5047, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5047,
				  QString(tr("Parameter %1 of AFB %2 must have type 16-bit Unsigned Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5048
	///
	/// IssueType: Error
	///
	/// Title: Parameter %1 of AFB %2 must have type 32-bit Unsigned Int.
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
		addItemsIssues(OutputMessageLevel::Error, 5048, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5048,
				  QString(tr("Parameter %1 of AFB %2 must have type 32-bit Unsigned Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5049
	///
	/// IssueType: Error
	///
	/// Title: Parameter %1 of AFB %2 must have type 32-bit Signed Int.
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
		addItemsIssues(OutputMessageLevel::Error, 5049, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5049,
				  QString(tr("Parameter %1 of AFB %2 must have type 32-bit Signed Int.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5050
	///
	/// IssueType: Error
	///
	/// Title: Parameter %1 of AFB %2 must have type 32-bit Float.
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
		addItemsIssues(OutputMessageLevel::Error, 5050, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5050,
				  QString(tr("Parameter %1 of AFB %2 must have type 32-bit Float.")).
				  arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5051
	///
	/// IssueType: Error
	///
	/// Title: Value %1 of parameter %2 of AFB %3 is incorrect.
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
		addItemsIssues(OutputMessageLevel::Error, 5051, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5051,
				  QString(tr("Value %1 of parameter %2 of AFB %3 is incorrect.")).
				  arg(paramValue).arg(paramCaption).arg(fbCaption));
	}

	/// IssueCode: ALC5052
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter %1.%2 must be greater then the value of %1.%3.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 parameter 1 caption
	///		%3 parameter 2 caption
	///		%4 application logic item Uuid
	///
	/// Description:
	///		Value of first specified parameter must be greater then the value of second parameneter. Correct prameter's values.
	///
	void IssueLogger::errALC5052(QString fbCaption, QString param1, QString param2, QUuid itemUuid, QString schemaID, QString itemLabel)
	{
		addItemsIssues(OutputMessageLevel::Error, 5052, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5052,
				  QString(tr("Value of parameter %1.%2 must be greater then the value of %1.%3 (Logic schema %4, item %5).")).
					arg(fbCaption).arg(param1).arg(param2).arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5053
	///
	/// IssueType: Warning
	///
	/// Title: Automatic sorting of XY points of FB %1 has been performed.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 application logic item Uuid
	///
	/// Description:
	///		Automatic sorting of XY points of FB %1 has been performed. Check XY points values.
	///
	void IssueLogger::wrnALC5053(QString fbCaption, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5053, itemUuid);

		LOG_WARNING0(IssueType::AlCompiler,
				  5053,
				  QString(tr("Automatic sorting of XY points of FB %1 has been performed.")).arg(fbCaption));
	}

	/// IssueCode: ALC5054
	///
	/// IssueType: Error
	///
	/// Title:	   Parameters %1 and %2 of AFB %3 can't be equal.
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
		addItemsIssues(OutputMessageLevel::Error, 5054, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5054,
				  QString(tr("Parameters %1 and %2 of AFB %3 can't be equal.")).
				  arg(param1).arg(param2).arg(fbCaption));
	}

	/// IssueCode: ALC5055
	///
	/// IssueType: Warning
	///
	/// Title:	   Optical connection %1 is configured manually.
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
				  QString(tr("Optical connection %1 is configured manually.")).arg(connectionID));
	}

	/// IssueCode: ALC5056
	///
	/// IssueType: Error
	///
	/// Title:	   SubsystemID %1 assigned in LM %2 is not found in subsystem list.
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
				  QString(tr("SubsystemID %1 assigned in LM %2 is not found in subsystem list.")).
				  arg(subsystemID).arg(lmEquipmentID));
	}


	/// IssueCode: ALC5057
	///
	/// IssueType: Error
	///
	/// Title:	   Uncompatible data format of analog AFB Signal %1.%2.
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
		addItemsIssues(OutputMessageLevel::Error, 5057, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5057,
				  QString(tr("Uncompatible data format of analog AFB signal %1.%2.")).
				  arg(afbCaption).arg(afbSignal));
	}

	/// IssueCode: ALC5058
	///
	/// IssueType: Error
	///
	/// Title:	   Parameter %1 of AFB %2 can't be 0.
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
		addItemsIssues(OutputMessageLevel::Error, 5058, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5058,
				  QString(tr("Parameter %1 of AFB %2 can't be 0.")).
				  arg(paramCaption).arg(afbCaption));
	}

	/// IssueCode: ALC5059
	///
	/// IssueType: Error
	///
	/// Title:	   Ports of connection %1 are not accessible in LM %2 (Logic schema %3).
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
		addItemsIssues(OutputMessageLevel::Error, 5059, transmitterUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5059,
				  QString(tr("Ports of connection %1 are not accessible in LM %2 (Logic schema %3).")).
				  arg(connectionID).arg(lmID).arg(schemaID));
	}

	/// IssueCode: ALC5060
	///
	/// IssueType: Error
	///
	/// Title:	   Float constant is connected to discrete input (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Float constant is connected to discrete input. Change contant type to IntegerType.
	///
	void IssueLogger::errALC5060(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5060, constantUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5060,
				  QString(tr("Float constant is connected to discrete input (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5061
	///
	/// IssueType: Error
	///
	/// Title:	   Float constant is connected to 16-bit input (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Float constant is connected to 16-bit input. Change contant type to IntegerType.
	///
	void IssueLogger::errALC5061(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5061, constantUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5061,
				  QString(tr("Float constant is connected to 16-bit input (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5062
	///
	/// IssueType: Error
	///
	/// Title:	   Float constant is connected to SignedInt input (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Float constant is connected to SignedInt input. Change contant type to IntegerType.
	///
	void IssueLogger::errALC5062(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5062, constantUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5062,
				  QString(tr("Float constant is connected to SignedInt input (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5063
	///
	/// IssueType: Error
	///
	/// Title:	   Integer constant is connected to Float input (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Integer constant is connected to Float input. Change contant type to FloatType.
	///
	void IssueLogger::errALC5063(QString schemaID, QUuid constantUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5063, constantUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5063,
				  QString(tr("Integer constant is connected to Float input (Logic schema %1).")).arg(schemaID));
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
	/// Title:	   Command MOVEMEM %1, %2, %3 can't write to bit-addressed memory.
	///
	/// Parameters:
	///		%1 Destination address
	///		%2 Source address
	///		%3 Memory size to move
	///
	/// Description:
	///		Command MOVEMEM can't write to bit-addressed memory. Contact to RPCT developers.
	///
	void IssueLogger::errALC5066(int addrTo, int addrFrom, int sizeW)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5066,
				  QString(tr("Command MOVEMEM %1, %2, %3 can't write to bit-addressed memory.")).
					arg(addrTo).arg(addrFrom).arg(sizeW));
	}

	/// IssueCode: ALC5067
	///
	/// IssueType: Error
	///
	/// Title:	   Command MOVBC %1[%2], #%3 can't write out of application bit- or word-addressed memory.
	///
	/// Parameters:
	///		%1 Destination address
	///		%2 Destination bit
	///		%3 Const bit value
	///
	/// Description:
	///		Command MOVBC can't write out of application bit- or word-addressed memory. Contact to RPCT developers.
	///
	void IssueLogger::errALC5067(int addrTo, int bit, int value)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5067,
				  QString(tr("Command MOVBC %1[%2], #%3 can't write out of application bit- or word-addressed memory.")).
					arg(addrTo).arg(bit).arg(value));
	}


	/// IssueCode: ALC5068
	///
	/// IssueType: Error
	///
	/// Title:	   TuningHighBound property of tunable signal %1 must be greate than TuningLowBound
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		TuningHighBound property of tunable signal must be greate than TuningLowBound. Check signal properties.
	///
	void IssueLogger::errALC5068(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5068,
				  QString(tr("TuningHighBound property of tunable signal %1 must be greate than TuningLowBound")).
					arg(appSignalID));
	}


	/// IssueCode: ALC5069
	///
	/// IssueType: Error
	///
	/// Title:	   TuningDefaultValue property of tunable signal %1 must be in range from TuningLowBound to TuningHighBound.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		TuningDefaultValue property of tunable signal must be in range from TuningLowBound to TuningHighBound.
	///		Check signal's TuningDefaultValue property.
	///
	void IssueLogger::errALC5069(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5069,
				  QString(tr("TuningDefaultValue property of tunable signal %1 must be in range from TuningLowBound to TuningHighBound.")).
					arg(appSignalID));
	}


	/// IssueCode: ALC5070
	///
	/// IssueType: Warning
	///
	/// Title:	   Signal %1 has Little Endian byte order.
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
				  QString(tr("Signal %1 has Little Endian byte order.")).
					arg(appSignalID));
	}


	/// IssueCode: ALC5071
	///
	/// IssueType: Error
	///
	/// Title:	   Can't assign value to tunable signal %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Can't assign value to tunable signal. Such signals are read-only.
	///
	void IssueLogger::errALC5071(QString schemaID, QString appSignalID, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5071, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5071,
				  QString(tr("Can't assign value to tunable signal %1 (Logic schema %2).")).
					arg(appSignalID).arg(schemaID));
	}


	/// IssueCode: ALC5072
	///
	/// IssueType: Warning
	///
	/// Title: Possible error. AFB 'Poly' CoefCount = %1, but coefficient %2 is not equal to 0 (Logic schema %3).
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
		addItemsIssues(OutputMessageLevel::Warning0, 5072, itemUuid, schemaID);

		LOG_WARNING1(IssueType::AlCompiler,
				  5072,
				  QString(tr("Possible error. AFB 'Poly' CoefCount = %1, but coefficient %2 is not equal to 0 (Logic schema %3).")).
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
	/// Title: Receiver of connection %1 (port %2) is not associated with LM %3
	///
	/// Parameters:
	///		%1 Connection ID
	///		%2 Connection port EquipmentID
	///		%3 Logic module EquipmentID
	///
	/// Description:
	///		Receiver of connection is not associated with specified LM. Check receiver placement.
	///
	void IssueLogger::errALC5083(QString receiverPortID, QString connectionID, QString lmID, QUuid receiverUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5083, receiverUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5083,
				  QString(tr("Receiver of connection %1 (port %2) is not associated with LM %3.")).
						arg(connectionID).arg(receiverPortID).arg(lmID));
	}

	/// IssueCode: ALC5085
	///
	/// IssueType: Error
	///
	/// Title: Rx data size of RS232/485 port %1 is undefined (connection %2).
	///
	/// Parameters:
	///		%1 Serial port EquipmentID
	///		%2 Serial connection ID
	///
	/// Description:
	///		Receving data size of specified RS232/485 port is undefined. Use Manual Settings to determine Rx data size.
	///
	void IssueLogger::errALC5085(QString portEquipmentID, QString connectionID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5085,
				  QString(tr("Rx data size of RS232/485 port %1 is undefined (connection %2).")).
						arg(portEquipmentID).arg(connectionID));
	}

	/// IssueCode: ALC5086
	///
	/// IssueType: Error
	///
	/// Title: Discrete constant must have value 0 or 1 (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Discrete constant must have value 0 or 1. Check constant value.
	///

	void IssueLogger::errALC5086(QUuid constItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5086, constItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5086,
				  QString(tr("Discrete constant must have value 0 or 1 (Logic schema %1).")).
						arg(schemaID));

	}

	/// IssueCode: ALC5087
	///
	/// IssueType: Error
	///
	/// Title:	   Can't assign value to input signal %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Can't assign value to input signal. Such signals are read-only.
	///
	void IssueLogger::errALC5087(QString schemaID, QString appSignalID, QUuid itemUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 5087, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5087,
				  QString(tr("Can't assign value to input signal %1 (Logic schema %2).")).
					arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5088
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter %1.%2 must be greater then 0.
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
		addItemsIssues(OutputMessageLevel::Error, 5088, itemUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5088,
				  QString(tr("Value of parameter %1.%2 must be greater then 0.")).
				  arg(fbCaption).arg(paramCaption));
	}

	/// IssueCode: ALC5089
	///
	/// IssueType: Error
	///
	/// Title:	   Command MOVB %1[%2], %3[%4] can't write out of application bit- or word-addressed memory.
	///
	/// Parameters:
	///		%1 Destination address
	///		%2 Destination bit
	///		%3 Source address
	///		%4 Source bit
	///
	/// Description:
	///		Command MOVB can't write out of application bit- or word-addressed memory. Contact to RPCT developers.
	///
	void IssueLogger::errALC5089(int addrTo, int bitTo, int addrFrom, int bitFrom)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5089,
				  QString(tr("Command MOVB %1[%2], %3[%4] can't write out of application bit- or word-addressed memory.")).
					arg(addrTo).arg(bitTo).arg(addrFrom).arg(bitFrom));
	}

	/// IssueCode: ALC5090
	///
	/// IssueType: Error
	///
	/// Title:	   Analog signal %1 aperture should be greate then 0.
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
				  QString(tr("Analog signal %1 aperture should be greate then 0.")).
					arg(appSignalID));
	}

	/// IssueCode: ALC5091
	///
	/// IssueType: Error
	///
	/// Title:	   Input/output application signal %1 should be bound to equipment signal.
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
				  QString(tr("Input/output application signal %1 should be bound to equipment signal.")).
					arg(appSignalID));
	}

	/// IssueCode: ALC5092
	///
	/// IssueType: Error
	///
	/// Title:	   Bus type ID %1 of signal %2 is undefined.
	///
	/// Parameters:
	///		%1 Bus type ID
	///		%2 Application signal ID
	///
	/// Description:
	///		Specified BusTypeID of signal is undefined. Check signals BusTypeID property.
	///
	void IssueLogger::errALC5092(QString busTypeID, QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5092,
				  QString(tr("Bus type ID %1 of signal %2 is undefined.")).
						arg(busTypeID).arg(appSignalID));
	}

	/// IssueCode: ALC5093
	///
	/// IssueType: Warning
	///
	/// Title:	   Coarse aperture of signal %1 less then fine aperture.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Coarse aperture of specified signal less then fine aperture. Check signal's properties.
	///
	void IssueLogger::wrnALC5093(QString appSignalID)
	{
		LOG_WARNING0(IssueType::AlCompiler,
				  5093,
				  QString(tr("Coarse aperture of signal %1 less then fine aperture.")).arg(appSignalID));
	}

	/// IssueCode: ALC5094
	///
	/// IssueType: Error
	///
	/// Title:	   Size of in bus analog signal %1 is not multiple 16 bits (bus type %2).
	///
	/// Parameters:
	///		%1 In bus signal ID
	///		%2 Bus type ID
	///
	/// Description:
	///		Size of in bus analog signal is not multiple 16 bits. Check in bus signal properties.
	///
	void IssueLogger::errALC5094(QString inBusSignalID, QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5094,
				  QString(tr("Size of in bus analog signal %1 is not multiple 16 bits (bus type %2).")).
						arg(inBusSignalID).arg(busTypeID));
	}

	/// IssueCode: ALC5095
	///
	/// IssueType: Error
	///
	/// Title:	   The bus size must be a multiple of 2 bytes (1 word) (bus type %1).
	///
	/// Parameters:
	///		%1 Bus type ID
	///
	/// Description:
	///		The bus size must be a multiple of 2 bytes (1 word). Check bus properties.
	///
	void IssueLogger::errALC5095(QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5095,
				  QString(tr("The bus size must be a multiple of 2 bytes (1 word) (bus type %1).")).arg(busTypeID));
	}

	/// IssueCode: ALC5096
	///
	/// IssueType: Error
	///
	/// Title:	   Offset of in bus analog (or bus) signal %1 is not multiple of 2 bytes (1 word) (bus type %2).
	///
	/// Parameters:
	///		%1 In bus signal ID
	///		%2 Bus type ID
	///
	/// Description:
	///		Offset of in bus analog signal is not multiple of 2 bytes (1 word). Check in bus signal properties.
	///
	void IssueLogger::errALC5096(QString inBusSignalID, QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5096,
				  QString(tr("Offset of in-bus analog (or bus) signal %1 is not multiple of 2 bytes (1 word) (bus type %2).")).
							arg(inBusSignalID).arg(busTypeID));
	}

	/// IssueCode: ALC5097
	///
	/// IssueType: Error
	///
	/// Title:	   Bus signals %1 and %2 are overlapped (bus type %3).
	///
	/// Parameters:
	///		%1 Bus signal ID
	///		%2 Bus signal ID
	///		%3 Bus type ID
	///
	/// Description:
	///		Bus signals are overlaped. Check bus signal offsets.
	///
	void IssueLogger::errALC5097(QString signalID1, QString signalID2, QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5097,
				  QString(tr("Bus signals %1 and %2 are overlapped (bus type %3).")).
							arg(signalID1).arg(signalID2).arg(busTypeID));
	}

	/// IssueCode: ALC5098
	///
	/// IssueType: Error
	///
	/// Title:	   Bus signal %1 offset out of range (bus type %2).
	///
	/// Parameters:
	///		%1 Bus signal ID
	///		%2 Bus type ID
	///
	/// Description:
	///		Bus signal offset out of range. Check bus or signal properties.
	///
	void IssueLogger::errALC5098(QString signalID, QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5098,
				  QString(tr("Bus signal %1 offset out of range (bus type %2).")).
							arg(signalID).arg(busTypeID));
	}

	/// IssueCode: ALC5099
	///
	/// IssueType: Error
	///
	/// Title:	   Bus size must be multiple of 2 bytes (bus type %1).
	///
	/// Parameters:
	///		%1 Bus type ID
	///
	/// Description:
	///		Bus size must be multiple of 2 bytes. Check bus properties.
	///
	void IssueLogger::errALC5099(QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5099,
				  QString(tr("Bus size must be multiple of 2 bytes (bus type %1).")).arg(busTypeID));
	}

	/// IssueCode: ALC5100
	///
	/// IssueType: Error
	///
	/// Title:	   Bus type ID %1 is undefined (Logic schema %2).
	///
	/// Parameters:
	///		%1 Bus type ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Bus type ID is undefined. Check UAL item properties.
	///
	void IssueLogger::errALC5100(QString busTypeID, QUuid item, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5100, item, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5100,
				  QString(tr("Bus type ID %1 is undefined (Logic schema %2).")).arg(busTypeID).arg(schemaID));
	}

	/// IssueCode: ALC5102
	///
	/// IssueType: Error
	///
	/// Title:	   Output of bus composer can't be connected to input of another bus composer (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Output of bus composer can't be connected to input of another bus composer. Nested busses is not allowed.
	///
	void IssueLogger::errALC5102(QUuid composer1Guid, QUuid composer2Guid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5102, composer1Guid, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5102, composer2Guid);

		LOG_ERROR(IssueType::AlCompiler,
				  5102,
				  QString(tr("Output of bus composer can't be connected to input of another bus composer (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5103
	///
	/// IssueType: Error
	///
	/// Title:	   Different bus types of bus composer and signal %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Different bus types of bus composer and signal. Harmonize bus type of elements.
	///
	void IssueLogger::errALC5103(QString signalID, QUuid signalUuid, QUuid composerUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5103, signalUuid, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5103, composerUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5103,
				  QString(tr("Different bus types of bus composer and signal %1 (Logic schema %2).")).
						arg(signalID).arg(schemaID));
	}

	/// IssueCode: ALC5104
	///
	/// IssueType: Error
	///
	/// Title:	   Bus composer is connected to non-bus signal %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Bus composer is connected to non-bus signal.
	///
	void IssueLogger::errALC5104(QUuid composerUuid, QString signalID, QUuid signalUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5104, composerUuid, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5104, signalUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5104,
				  QString(tr("Bus composer is connected to non-bus signal %1 (Logic schema %2).")).
						arg(signalID).arg(schemaID));
	}

	/// IssueCode: ALC5105
	///
	/// IssueType: Error
	///
	/// Title:	   Undefined UAL address of signal %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Bus composer is connected to non-bus signal.
	///
	void IssueLogger::errALC5105(QString signalID, QUuid signalUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5105, signalUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5105,
				  QString(tr("Undefined UAL address of signal %1 (Logic schema %2).")).
						arg(signalID).arg(schemaID));
	}

	/// IssueCode: ALC5106
	///
	/// IssueType: Error
	///
	/// Title:	   Pin with caption %1 is not found in schema item (Logic schema %2).
	///
	/// Parameters:
	///		%1 Pin caption
	///		%2 Logic schema ID
	///
	/// Description:
	///		Pin with caption %1 is not found in schema item. Contact with RPCT developers.
	///
	void IssueLogger::errALC5106(QString pinCaption, QUuid schemaItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5106, schemaItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5106,
				  QString(tr("Pin with caption %1 is not found in schema item (Logic schema %2).")).
						arg(pinCaption).arg(schemaID));
	}

	/// IssueCode: ALC5107
	///
	/// IssueType: Error
	///
	/// Title:	   Afb's output cannot be directly connected to the transmitter. Intermediate app signal should be used (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Afb's output cannot be directly connected to the transmitter. Intermediate app signal should be used. Check Afb and transmitter connection.
	///
	void IssueLogger::errALC5107(QUuid afbUuid, QUuid transmitterUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5107, afbUuid, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5107, transmitterUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5107,
				  QString(tr("AFB's output cannot be directly connected to the transmitter. Intermediate app signal should be used (Logic schema %1).")).
								arg(schemaID));
	}

	/// IssueCode: ALC5108
	///
	/// IssueType: Error
	///
	/// Title:	   Cannot identify AFB bus type (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Cannot identify AFB bus type based on its inputs.
	///
	void IssueLogger::errALC5108(QUuid afbUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5108, afbUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5108,
				  QString(tr("Cannot identify AFB bus type (Logic schema %1).").arg(schemaID)));
	}

	/// IssueCode: ALC5109
	///
	/// IssueType: Error
	///
	/// Title:	   Different bus types on AFB inputs (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Busses of different types is connected to AFB's inputs.
	///
	void IssueLogger::errALC5109(QUuid afbUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5109, afbUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5109,
				  QString(tr("Different bus types on AFB inputs (Logic schema %1).").arg(schemaID)));
	}

	/// IssueCode: ALC5110
	///
	/// IssueType: Error
	///
	/// Title:	   Non-bus output is connected to bus input (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Non-bus output cannot be connected to bus input.
	///
	void IssueLogger::errALC5110(QUuid item1, QUuid item2, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5110, item1, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5110, item2);

		LOG_ERROR(IssueType::AlCompiler,
				  5110,
				  QString(tr("Non-bus output is connected to bus input (Logic schema %1).")).
						arg(schemaID));
	}

	/// IssueCode: ALC5111
	///
	/// IssueType: Error
	///
	/// Title:	   Output of type 'Bus' is occured in non-bus processing AFB (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Output of type 'Bus' is occured in non-bus processing AFB. Contact to RPCT developers.
	///
	void IssueLogger::errALC5111(QUuid afbUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5111, afbUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5111,
				  QString(tr("Output of type 'Bus' is occured in non-bus processing AFB (Logic schema %1")).
						arg(schemaID));
	}

	/// IssueCode: ALC5112
	///
	/// IssueType: Error
	///
	/// Title:	   Different bus types of UAL elements (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Different bus types of UAL elements.
	///
	void IssueLogger::errALC5112(QUuid uuid1, QUuid uuid2, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5112, uuid1, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5112, uuid2);

		LOG_ERROR(IssueType::AlCompiler,
				  5112,
				  QString(tr("Different bus types of UAL elements (Logic schema %1).")).
						arg(schemaID));
	}

	/// IssueCode: ALC5113
	///
	/// IssueType: Error
	///
	/// Title:	   Bus output is connected to non-bus input (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Bus output cannot be connected to non-bus input.
	///
	void IssueLogger::errALC5113(QUuid item1, QUuid item2, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5113, item1, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5113, item2);

		LOG_ERROR(IssueType::AlCompiler,
				  5113,
				  QString(tr("Bus output is connected to non-bus input (Logic schema %1).")).
						arg(schemaID));
	}

	/// IssueCode: ALC5114
	///
	/// IssueType: Error
	///
	/// Title:	   Bus size exceed max bus size of input %1.%2 (Logic schema %3).
	///
	/// Parameters:
	///		%1 Logic item caption
	///		%2 Logic input caption
	///		%3 Logic schema ID
	///
	/// Description:
	///		Connected bus size exceed max allowed bus size of specified input.
	///
	void IssueLogger::errALC5114(QString itemCaption, QString inputCaption, QUuid itemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5114, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5114,
				  QString(tr("Bus size exceed max bus size of input %1.%2 (Logic schema %3).")).
						arg(itemCaption).arg(inputCaption).arg(schemaID));
	}

	/// IssueCode: ALC5115
	///
	/// IssueType: Error
	///
	/// Title:	   Uncompatible bus data format of UAL elements (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Uncompatible bus data format of UAL elements.
	///
	void IssueLogger::errALC5115(QUuid uuid1, QUuid uuid2, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5115, uuid1, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5115, uuid2);

		LOG_ERROR(IssueType::AlCompiler,
				  5115,
				  QString(tr("Uncompatible bus data format of UAL elements (Logic schema %1).")).
						arg(schemaID));
	}

	/// IssueCode: ALC5116
	///
	/// IssueType: Error
	///
	/// Title:	   Disallowed connection of UAL elements (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Disallowed connection of UAL elements. Check connection.
	///
	void IssueLogger::errALC5116(QUuid uuid1, QUuid uuid2, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5116, uuid1, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5116, uuid2);

		LOG_ERROR(IssueType::AlCompiler,
				  5116,
				  QString(tr("Disallowed connection of UAL elements (Logic schema %1).")).
						arg(schemaID));
	}

	/// IssueCode: ALC5117
	///
	/// IssueType: Error
	///
	/// Title:	   Uncompatible signals connection (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Uncompatible signals connection. Check signals type and data format.
	///
	void IssueLogger::errALC5117(QUuid uuid1, QString label1, QUuid uuid2, QString label2, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5117, uuid1, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5117, uuid2);

		LOG_ERROR(IssueType::AlCompiler,
				  5117,
				  QString(tr("Uncompatible signals connection (items %1 and %2) (Logic schema %3).")).
						arg(label1).arg(label2).arg(schemaID));
	}

	/// IssueCode: ALC5118
	///
	/// IssueType: Error
	///
	/// Title:	   Signal %1 is not connected to any signal source. (Logic schema %2).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Signal is not connected to any signal source. Signal's value is undefined.
	///
	void IssueLogger::errALC5118(QString appSignalID, QUuid itemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5118, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5118,
				  QString(tr("Signal %1 is not connected to any signal source. (Logic schema %2).")).
						arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5119
	///
	/// IssueType: Error
	///
	/// Title:	   Type of Constant is uncompatible with type of linked schema items (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Constant and linked schema items has different types.
	///
	void IssueLogger::errALC5119(QUuid constItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5119, constItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5119,
				  QString(tr("Type of Constant is uncompatible with type of linked schema items (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5120
	///
	/// IssueType: Error
	///
	/// Title:	   UalSignal is not found for pin %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Schema item pin Guid
	///		%2 Logic schema ID
	///
	/// Description:
	///		UalSignal is not found for pin with specified Uuid. Contact to RPCT developers.
	///
	void IssueLogger::errALC5120(QUuid ualItemUuid, QString ualItemLabel, QString pin, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5120, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5120,
				  QString(tr("UalSignal is not found for pin %1.%2 (Logic schema %3).")).
							arg(ualItemLabel).arg(pin).arg(schemaID));
	}

	/// IssueCode: ALC5121
	///
	/// IssueType: Error
	///
	/// Title:	   Can't assign value to input/tunable/opto/const signal %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 App signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Value of input/tunable/opto/const signals cannot be modified by UAL.
	///
	void IssueLogger::errALC5121(QString appSignalID, QUuid ualItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5121, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5121,
				  QString(tr("Can't assign value to input/tunable/opto/const signal %1 (Logic schema %2).")).
							arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5122
	///
	/// IssueType: Error
	///
	/// Title:	   Different busTypes on AFB output (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Different busTypes on AFB output.
	///
	void IssueLogger::errALC5122(QUuid ualItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5122, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5122,
				  QString(tr("Different busTypes on AFB output (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5123
	///
	/// IssueType: Error
	///
	/// Title:	   Different busTypes on AFB inputs (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Different busTypes on AFB inputs.
	///
	void IssueLogger::errALC5123(QUuid ualItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5123, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5123,
				  QString(tr("Different busTypes on AFB inputs (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5124
	///
	/// IssueType: Error
	///
	/// Title:	   Discrete signal %1 is connected to non-discrete bus input (Logic schema %2)
	///
	/// Parameters:
	///		%1 App signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Different busTypes on AFB inputs.
	///
	void IssueLogger::errALC5124(QString appSignalID, QUuid signalUuid, QUuid ualItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5124, ualItemUuid, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5124, signalUuid);

		LOG_ERROR(IssueType::AlCompiler,
				  5124,
				  QString(tr("Discrete signal %1 is connected to non-discrete bus input (Logic schema %2)")).
								arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5125
	///
	/// IssueType: Error
	///
	/// Title:	   Input %1 of transmitter is connected unnamed signal (Logic schema %2).
	///
	/// Parameters:
	///		%1 Transmitter's input
	///		%2 Logic schema ID
	///
	/// Description:
	///		Different busTypes on AFB inputs.
	///
	void IssueLogger::errALC5125(QString pinCaption, QUuid transmitterUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5125, transmitterUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5125,
				  QString(tr("Input %1 of transmitter is connected unnamed signal (Logic schema %2).")).
								arg(pinCaption).arg(schemaID));
	}

	/// IssueCode: ALC5126
	///
	/// IssueType: Error
	///
	/// Title:	   Signal and bus inputs sizes are not multiples (Logic schema %1).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///
	/// Description:
	///		Signal and bus inputs sizes are not multiples.
	///
	void IssueLogger::errALC5126(QUuid ualItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5126, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5126,
				  QString(tr("Signal and bus inputs sizes are not multiples (Logic schema %1).")).arg(schemaID));
	}

	/// IssueCode: ALC5127
	///
	/// IssueType: Error
	///
	/// Title:	   Output bus type cannot be determined (Logic schema %1, item %2)
	///
	/// Parameters:
	///		%1 Logic schema ID
	///		%2 Schema item label
	///
	/// Description:
	///		Output bus type cannot be determined.
	///
	void IssueLogger::errALC5127(QUuid ualItemUuid, QString itemLabel, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5127, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5127,
				  QString(tr("Output bus type cannot be determined (Logic schema %1, item %2).")).
						arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5128
	///
	/// IssueType: Error
	///
	/// Title:	   All AFB's bus inputs connected to discretes (Logic schema %1, item %2).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///		%2 Schema item label
	///
	/// Description:
	///		All AFB's bus inputs connected to discretes. Output bus type cannot be determined.
	///
	void IssueLogger::errALC5128(QUuid ualItemUuid, QString itemLabel, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5128, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5128,
				  QString(tr("All AFB's bus inputs connected to discretes (Logic schema %1, item %2).")).
						arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5129
	///
	/// IssueType: Error
	///
	/// Title:	   Unknown AFB type (opCode) (Logic schema %1, item %2).
	///
	/// Parameters:
	///		%1 Logic schema ID
	///		%2 Schema item label
	///
	/// Description:
	///		Unknown AFB type (opCode) (Logic schema %1, item %2). Contact to RPCT developers.
	///
	void IssueLogger::errALC5129(QUuid ualItemUuid, QString itemLabel, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5129, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5129,
				  QString(tr("Unknown AFB type (opCode) (Logic schema %1, item %2).")).
						arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5130
	///
	/// IssueType: Error
	///
	/// Title:	   Max instances (%1) of AFB component %2 is used (Logic schema %3, item %4)
	///
	/// Parameters:
	///		%1 Max instances count
	///		%2 AFB component caption
	///		%3 Logic schema ID
	///		%4 Schema item label
	///
	/// Description:
	///		 Max instances of specified AFB component is used.
	///
	void IssueLogger::errALC5130(int maxInstances, QString afbComponentCaption, QUuid ualItemUuid, QString itemLabel, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5130, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5130,
				  QString(tr("Max instances (%1) of AFB component %2 is used (Logic schema %3, item %4)")).
						arg(maxInstances).arg(afbComponentCaption).arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5131
	///
	/// IssueType: Error
	///
	/// Title:	   Type of signal %1 connected to opto port %2 isn't correspond to its type specified in raw data description.
	///
	/// Parameters:
	///		%1 App signal ID
	///		%2 Opto port equipmentID
	///
	/// Description:
	///		 Type of signal connected to opto port isn't correspond to its type specified in raw data description.
	///
	void IssueLogger::errALC5131(QString appSignalID, QString portID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5131,
				  QString(tr("Type of signal %1 connected to opto port %2 isn't correspond to its type specified in raw data description.")).
						arg(appSignalID).arg(portID));
	}

	/// IssueCode: ALC5132
	///
	/// IssueType: Error
	///
	/// Title:	   Can't resolve busses interdependencies: %1
	///
	/// Parameters:
	///		%1 Buses with interdependensies list
	///
	/// Description:
	///		 Specified busses have interdependencies that cannot be resolved.
	///
	void IssueLogger::errALC5132(QString unresolvedBusList)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5132,
				  QString(tr("Can't resolve busses interdependencies: %1")).
						arg(unresolvedBusList));
	}

	/// IssueCode: ALC5133
	///
	/// IssueType: Error
	///
	/// Title:	   Application signal with equipmentID %1 is not found (Logic schema %2, item %3).
	///
	/// Parameters:
	///		%1 Application signal's equipmemtID
	///		%2 Logic schema ID
	///		%3 Schema item label
	///
	/// Description:
	///		Application signal with specified equipmentID is not found.
	///
	void IssueLogger::errALC5133(QString signalEquipmentID, QUuid ualItemUuid, QString itemLabel, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5133, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5133,
				  QString(tr("Application signal with equipmentID %1 is not found (Logic schema %2, item %3).")).
						arg(signalEquipmentID).arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5134
	///
	/// IssueType: Error
	///
	/// Title:	   Integer constant value out of range (Logic schema %1, item %2)
	///
	/// Parameters:
	///		%1 Logic schema ID
	///		%2 Schema item label
	///
	/// Description:
	///		Integer constant value out of range. Check constant value.
	///
	void IssueLogger::errALC5134(QUuid ualItemUuid, QString itemLabel, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5134, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5134,
				  QString(tr("Integer constant value out of range %1..%2 (Logic schema %3, item %4).")).
						arg(std::numeric_limits<qint32>::min()).
						arg(std::numeric_limits<qint32>::max()).
						arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5135
	///
	/// IssueType: Error
	///
	/// Title:	   Float constant value out of range (Logic schema %1, item %2)
	///
	/// Parameters:
	///		%1 Logic schema ID
	///		%2 Schema item label
	///
	/// Description:
	///		Float constant value out of range. Check constant value.
	///
	void IssueLogger::errALC5135(QUuid ualItemUuid, QString itemLabel, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5135, ualItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5135,
				  QString(tr("Float constant value out of range %1..%2 (Logic schema %3, item %4).")).
						arg(std::numeric_limits<float>::min()).
						arg(std::numeric_limits<float>::max()).
						arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5136
	///
	/// IssueType: Error
	///
	/// Title: The input (or output) signal %1 can be bind to Equipment Signal only.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		The signal bind to uncorrect equipment. Set the correct value of signal's EquipmentID property.
	///
	void IssueLogger::errALC5136(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5136,
				  QString(tr("The input (or output) signal %1 can be bind to Equipment Signal only.").
						  arg(appSignalID)));
	}

	/// IssueCode: ALC5137
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 property %2 out of SignedInt32 range.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Signal property name
	///
	/// Description:
	///		Specified signal property value is out of SignedInt32 range. Check property value.
	///
	void IssueLogger::errALC5137(QString appSignalID, QString property)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5137,
				  QString(tr("Signal %1 property %2 out of SignedInt32 range.")).
						  arg(appSignalID).arg(property));
	}

	/// IssueCode: ALC5138
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 property %2 out of Float32 range.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Signal property name
	///
	/// Description:
	///		Specified signal property value is out of Float32 range. Check property value.
	///
	void IssueLogger::errALC5138(QString appSignalID, QString property)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5138,
				  QString(tr("Signal %1 property %2 out of Float32 range.")).
						  arg(appSignalID).arg(property));
	}

	/// IssueCode: ALC5139
	///
	/// IssueType: Warning
	///
	/// Title: Values of parameters %1.%2 and %1.%3 are equal.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 parameter 1 caption
	///		%3 parameter 2 caption
	///
	/// Description:
	///		Values of parameters are equal. Check params values.
	///
	void IssueLogger::wrnALC5139(QString fbCaption, QString param1, QString param2, QUuid itemUuid, QString schemaID, QString itemLabel)
	{
		addItemsIssues(OutputMessageLevel::Warning0, 5139, itemUuid, schemaID);

		LOG_WARNING0(IssueType::AlCompiler,
				  5139,
				  QString(tr("Values of parameters %1.%2 and %1.%3 are equal (Logic schema %4, item %5).")).
					arg(fbCaption).arg(param1).arg(param2).arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5140
	///
	/// IssueType: Error
	///
	/// Title: Undefined ConfigurationService IP-address for software %1.
	///
	/// Parameters:
	///		%1 Software EquipmentID
	///
	/// Description:
	///		ConfigurationService IP-address for software is undefined. Check ConfigurationService ClientrequestIP property.
	///
	void IssueLogger::errALC5140(QString softwareID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5140,
				  QString(tr("Undefined ConfigurationService IP-address for software %1.")).arg(softwareID));
	}

	/// IssueCode: ALC5141
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter %1.%2 must be in range %3 (Logic schema %4)
	///
	/// Parameters:
	///		%1 AFB caption
	///		%2 parameter caption
	/// 	%3 range string (ex: "5..65535")
	///
	/// Description:
	///		Value of AFB parameter should be in specified range. Check parameter value.
	///
	void IssueLogger::errALC5141(QString fbCaption, QString paramCaption, QString rangeStr, QUuid itemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5141, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5141,
				  QString(tr("Value of parameter %1.%2 must be in range %3 (Logic schema %4)")).
					arg(fbCaption).arg(paramCaption).arg(rangeStr).arg(schemaID));
	}

	/// IssueCode: ALC5142
	///
	/// IssueType: Error
	///
	/// Title:	   Duplicate loopback source ID %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Loopback sourceID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Loopback source IDs can't duplicate.
	///
	void IssueLogger::errALC5142(QString loopbackSourceID, QUuid loopbackSourceItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5142, loopbackSourceItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5142,
				  QString(tr("Duplicate loopback source ID %1 (Logic schema %2)")).
								arg(loopbackSourceID).arg(schemaID));
	}

	/// IssueCode: ALC5143
	///
	/// IssueType: Error
	///
	/// Title:	   LoopbackSource is not exists for LoopbackTarget with ID %1 (Logic schema %2).
	///
	/// Parameters:
	///		%1 Loopback ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		LoopbackSource with specified ID is not exists. Check loopback identifier or create LoopbackSource.
	///
	void IssueLogger::errALC5143(QString loopbackID, QUuid loopbackTargetItemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5143, loopbackTargetItemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5143,
				  QString(tr("LoopbackSource is not exists for LoopbackTarget with ID %1 (Logic schema %2)")).
								arg(loopbackID).arg(schemaID));
	}

	/// IssueCode: ALC5144
	///
	/// IssueType: Error
	///
	/// Title:	   Non compatible signals %1 and %2 are connected to same Loopback %3 (Logic schema %4)
	///
	/// Parameters:
	///		%1 Signal 1 ID
	///		%2 Signal 2 ID
	///		%3 Loopback ID
	///		%4 Logic schema ID
	///
	/// Description:
	///		Non compatible signals are connected to same LoopbackTarget.
	///
	void IssueLogger::errALC5144(QString s1ID, QUuid s1Guid, QString s2ID, QUuid s2Guid, QString lbId, QUuid lbGuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5144, s1Guid, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5144, s2Guid, schemaID);
		addItemsIssues(OutputMessageLevel::Error, 5144, lbGuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5144,
				  QString(tr("Non compatible signals %1 and %2 are connected to same Loopback %3 (Logic schema %4)")).
								arg(s1ID).arg(s2ID).arg(lbId).arg(schemaID));
	}

	/// IssueCode: ALC5145
	///
	/// IssueType: Error
	///
	/// Title:	   Input signal %1 is connected to LoopbackTarget (Logic schema %2).
	///
	/// Parameters:
	///		%1 Signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Input signal cannot be connected to loopback target.
	///
	void IssueLogger::errALC5145(QString signalID, QUuid signalGuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5145, signalGuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5145,
				  QString(tr("Input signal %1 is connected to LoopbackTarget (Logic schema %2).")).arg(signalID).arg(schemaID));
	}

	/// IssueCode: ALC5146
	///
	/// IssueType: Error
	///
	/// Title:	   Tunable signal %1 is connected to LoopbackTarget (Logic schema %2).
	///
	/// Parameters:
	///		%1 Signal ID
	///		%2 Logic schema ID
	///
	/// Description:
	///		Tunable signal cannot be connected to loopback target.
	///
	void IssueLogger::errALC5146(QString signalID, QUuid signalGuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5146, signalGuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5146,
				  QString(tr("Tunable signal %1 is connected to LoopbackTarget (Logic schema %2).")).arg(signalID).arg(schemaID));
	}

	/// IssueCode: ALC5147
	///
	/// IssueType: Error
	///
	/// Title:	   Signal %1 is connected to different Loopbacks %2 and %3
	///
	/// Parameters:
	///		%1 Signal ID
	///		%2 Loopback1 ID
	///		%3 Loopback2 ID
	///
	/// Description:
	///		Signal is connected to different Loopbacks. Check signals connections.
	///
	void IssueLogger::errALC5147(QString signalID, QString lbID1, QString lbID2)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5147,
				  QString(tr("Signal %1 is connected to different LoopbackTargets %2 and %3.")).
					arg(signalID).arg(lbID1).arg(lbID2));
	}

	/// IssueCode: ALC5148
	///
	/// IssueType: Warning
	///
	/// Title:	   Internal signal %1 is unused.
	///
	/// Parameters:
	///		%1 Signal ID
	///
	/// Description:
	///		Internal signal has been created but not used in UAL.
	///
	void IssueLogger::wrnALC5148(QString signalID)
	{
		LOG_WARNING2(IssueType::AlCompiler,
				  5148,
				  QString(tr("Internal signal %1 is unused.")).arg(signalID));
	}

	/// IssueCode: ALC5149
	///
	/// IssueType: Error
	///
	/// Title:	   LM- or BVB-family module is not found in chassis %1.
	///
	/// Parameters:
	///		%1 chassis equipmentID
	///
	/// Description:
	///		LM- or BVB-family module is not found in chassis. Check equipment configuration.
	///
	void IssueLogger::errALC5149(QString chassisEquipmentID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5149,
				  QString(tr("LM- or BVB-family module is not found in chassis %1.")).arg(chassisEquipmentID));
	}

	/// IssueCode: ALC5150
	///
	/// IssueType: Error
	///
	/// Title:	   Monitor %1 cannot be connected to TuningService %2 with enabled SingleLmControl mode.
	///
	/// Parameters:
	///		%1 Monitor equipmentID
	///		%2 Tuning service equipmentID
	///
	/// Description:
	///		Monitor cannot be connected to TuningService with enabled SingleLmControl mode. Check TuningService settings.
	///
	void IssueLogger::errALC5150(QString monitorID, QString tuningServiceID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5150,
				  QString(tr("Monitor %1 cannot be connected to TuningService %2 with enabled SingleLmControl mode.")).
						arg(monitorID).arg(tuningServiceID));
	}

	/// IssueCode: ALC5151
	///
	/// IssueType: Error
	///
	/// Title:	   Bus type %1 has not initialized.
	///
	/// Parameters:
	///		%1 Bus type ID
	///
	/// Description:
	///		Bus type %1 has not initialized.
	///
	void IssueLogger::errALC5151(QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5151,
				  QString(tr("Bus type %1 has not initialized.")).arg(busTypeID));
	}

	/// IssueCode: ALC5152
	///
	/// IssueType: Error
	///
	/// Title:	   Bus input signal %1 placement is out of bus size (bus type %2).
	///
	/// Parameters:
	///		%1 Bus input signal ID
	///		%2 Bus type ID
	///
	/// Description:
	///		Bus input signal placement is out of bus size. Check bus input signal properties.
	///
	void IssueLogger::errALC5152(QString inBusSignal, QString busTypeID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5152,
				  QString(tr("Bus input signal %1 placement is out of bus size (bus type %2).")).
							arg(inBusSignal).arg(busTypeID));
	}

	/// IssueCode: ALC5153
	///
	/// IssueType: Error
	///
	/// Title:	   Unknown conversion of signal %1 to inbus signal %2 (Logic schema %3).
	///
	/// Parameters:
	///		%1 Input signal appSignalID
	///		%2 Bus child signal appSignalID
	///		%3 Logic schemaID
	///
	/// Description:
	///		Unknown conversion of signal to inbus signal. Check types of signals.
	///
	void IssueLogger::errALC5153(QString signalID, QString inbusSignalID, QString schemaID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5153,
				  QString(tr("Unknown conversion of signal %1 to inbus signal %2 (Logic schema %3).")).
							arg(signalID).arg(inbusSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5154
	///
	/// IssueType: Error
	///
	/// Title:	   Associated logic module is not found. Signal %1 cannot be processed.
	///
	/// Parameters:
	///		%1 AppSignalID
	///
	/// Description:
	///		Logic module is not found in chassis with application signals. Check hardware configuration and append LM if necessary.
	///
	void IssueLogger::errALC5154(QString signalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5154,
				  QString(tr("Associated logic module is not found. Signal %1 cannot be processed.")).arg(signalID));
	}

	/// IssueCode: ALC5155
	///
	/// IssueType: Error
	///
	/// Title:	   Linked validity signal with EquipmentID %1 is not found (input signal %2).
	///
	/// Parameters:
	///		%1 Validity signal EquipmentID
	///		%2 Input signal AppSignalID
	///
	/// Description:
	///		Validity signal linked to input signal is not found.
	///
	void IssueLogger::errALC5155(QString validitySignalEquipmentID, QString inputSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5155,
				  QString(tr("Linked validity signal with EquipmentID %1 is not found (input signal %2).")).
								arg(validitySignalEquipmentID).arg(inputSignalID));
	}

	/// IssueCode: ALC5156
	///
	/// IssueType: Error
	///
	/// Title:	   Linked validity signal %1 shoud have Discrete Input type (input signal %2).
	///
	/// Parameters:
	///		%1 Validity signal AppSignalID
	///		%2 Input signal AppSignalID
	///
	/// Description:
	///		Validity signal linked to input signal should have discrete input type.
	///
	void IssueLogger::errALC5156(QString validitySignalID, QString inputSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5156,
				  QString(tr("Linked validity signal %1 shoud have Discrete Input type (input signal %2).")).
								arg(validitySignalID).arg(inputSignalID));
	}

	/// IssueCode: ALC5157
	///
	/// IssueType: Error
	///
	/// Title:	   Analog signal %1 aperture should be less then 100.
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Analog signal aperture should be less then 100. Check properties of specified signal.
	///
	void IssueLogger::errALC5157(QString appSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5157,
				  QString(tr("Analog signal %1 aperture should be less then 100.")).
					arg(appSignalID));
	}

	/// IssueCode: ALC5158
	///
	/// IssueType: Error
	///
	/// Title: Value of parameter %1.%2 must be greater or equal then the value of %1.%3.
	///
	/// Parameters:
	///		%1 functional block caption
	///		%2 parameter 1 caption
	///		%3 parameter 2 caption
	///		%4 application logic item Uuid
	///
	/// Description:
	///		Value of first specified parameter must be greater or equal then the value of second parameneter. Correct prameter's values.
	///
	void IssueLogger::errALC5158(QString fbCaption, QString param1, QString param2, QUuid itemUuid, QString schemaID, QString itemLabel)
	{
		addItemsIssues(OutputMessageLevel::Error, 5158, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5158,
				  QString(tr("Value of parameter %1.%2 must be greater or equal then the value of %1.%3 (Logic schema %4, item %5).")).
					arg(fbCaption).arg(param1).arg(param2).arg(schemaID).arg(itemLabel));
	}

	/// IssueCode: ALC5159
	///
	/// IssueType: Error
	///
	/// Title: Receiver has no connection ID (Schema %1, module %2)
	///
	/// Parameters:
	///		%1 schema ID
	///		%2 module ID
	///
	/// Description:
	///		Connection ID should be assigned to Receiver.
	///
	void IssueLogger::errALC5159(QUuid itemUuid, QString schemaID, QString moduleID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5159, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5159,
				  QString(tr("Receiver has no connection ID (Schema %1, module %2)")).
					arg(schemaID).arg(moduleID));
	}

	/// IssueCode: ALC5160
	///
	/// IssueType: Error
	///
	/// Title: Transmitter has no connection ID (Schema %1, module %2)
	///
	/// Parameters:
	///		%1 schema ID
	///		%2 module ID
	///
	/// Description:
	///		At least one connection ID should be assigned to Transmitter.
	///
	void IssueLogger::errALC5160(QUuid itemUuid, QString schemaID, QString moduleID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5160, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5160,
				  QString(tr("Transmitter has no connection ID (Schema %1, module %2)")).
					arg(schemaID).arg(moduleID));
	}

	/// IssueCode: ALC5161
	///
	/// IssueType: Error
	///
	/// Title: Receiver has more than one connections ID (Schema %1, module %2)
	///
	/// Parameters:
	///		%1 schema ID
	///		%2 module ID
	///
	/// Description:
	///		Only one connection ID should be assigned to Receiver.
	///
	void IssueLogger::errALC5161(QUuid itemUuid, QString schemaID, QString moduleID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5161, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5161,
				  QString(tr("Receiver has more than one connections ID (Schema %1, module %2)")).
					arg(schemaID).arg(moduleID));
	}

	/// IssueCode: ALC5162
	///
	/// IssueType: Error
	///
	/// Title: In single-port connection %1 Port2EquipmentID property is not empty.
	///
	/// Parameters:
	///		%1 connection ID
	///
	/// Description:
	///		In single-port connections only Port1EquipmentID property should be assigned.
	///		Clear Port2EquipmentID property or change connection type to port-to-port.
	///
	void IssueLogger::errALC5162(QString connectionID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5162,
				  QString(tr("In single-port connection %1 Port2EquipmentID property is not empty.")).
					arg(connectionID));
	}

	/// IssueCode: ALC5163
	///
	/// IssueType: Error
	///
	/// Title: Port1EquipmentID property is empty in connection %1.
	///
	/// Parameters:
	///		%1 connection ID
	///
	/// Description:
	///		Port1EquipmentID property should be assigned.
	///
	void IssueLogger::errALC5163(QString connectionID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5163,
				  QString(tr("Port1EquipmentID property is empty in connection %1.")).
					arg(connectionID));
	}

	/// IssueCode: ALC5164
	///
	/// IssueType: Error
	///
	/// Title: Port2EquipmentID property is empty in connection %1.
	///
	/// Parameters:
	///		%1 connection ID
	///
	/// Description:
	///		Port2EquipmentID property should be assigned.
	///
	void IssueLogger::errALC5164(QString connectionID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5164,
				  QString(tr("Port2EquipmentID property is empty in connection %1.")).
					arg(connectionID));
	}

	/// IssueCode: ALC5165
	///
	/// IssueType: Warning
	///
	/// Title: Tuning is enabled for module %1 but tunable signals is not found.
	///
	/// Parameters:
	///		%1 LM's equipmentID
	///
	/// Description:
	///		Tuning is enabled for specified module but tunable signals is not found.
	///
	void IssueLogger::wrnALC5165(QString lmEquipmentID)
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5165,
				  QString(tr("Tuning is enabled for module %1 but tunable signals is not found.")).
					arg(lmEquipmentID));
	}

	/// IssueCode: ALC5166
	///
	/// IssueType: Error
	///
	/// Title: Tunable signals is found in module %1 but tuning is not enabled.
	///
	/// Parameters:
	///		%1 LM's equipmentID
	///
	/// Description:
	///		Tunable signals is found in specified module but tuning is not enabled.
	///
	void IssueLogger::errALC5166(QString lmEquipmentID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5166,
				  QString(tr("Tunable signals is found in module %1 but tuning is not enabled.")).
					arg(lmEquipmentID));
	}

	/// IssueCode: ALC5167
	///
	/// IssueType: Warning
	///
	/// Title: Signal %1 is excluded from build.
	///
	/// Parameters:
	///		%1 app signal ID
	///
	/// Description:
	///		Signal is excluded from build.
	///
	void IssueLogger::wrnALC5167(QString appSignalID)
	{
		LOG_WARNING1(IssueType::AlCompiler,
				  5167,
				  QString(tr("Signal %1 is excluded from build.")).
					arg(appSignalID));
	}

	/// IssueCode: ALC5168
	///
	/// IssueType: Warning
	///
	/// Title: Duplicate assigning of signal %1 to flag %2 of signal %3. Signal %4 already assigned to this flag.
	///
	/// Parameters:
	///		%1 flag signal ID
	///		%2 flag type
	///		%3 signal with flag ID
	///		%4 already assigned flag signal ID
	///
	/// Description:
	///		Duplicate assigning of signal to flag of specified signal
	///
	void IssueLogger::wrnALC5168(	QString flagSignalID,
									QString flagTypeStr,
									QString signalWithFlagID,
									QString alreadyAssignedFlagSignalID,
									QUuid itemUuid,
									QString schemaID)
	{
		if (schemaID.isEmpty() == false)
		{
			addItemsIssues(OutputMessageLevel::Warning0, 5168, itemUuid, schemaID);
		}

		LOG_WARNING0(IssueType::AlCompiler,
				  5168,
				  QString(tr("Duplicate assigning of signal %1 to flag %2 of signal %3. Signal %4 already assigned to this flag.")).
						arg(flagSignalID).arg(flagTypeStr).arg(signalWithFlagID).arg(alreadyAssignedFlagSignalID));
	}

	/// IssueCode: ALC5169
	///
	/// IssueType: Warning
	///
	/// Title: No flags assiged on set_flags item %1 (Schema %2)
	///
	/// Parameters:
	///		%1 set_flags item label
	///		%2 app logic schema ID
	///
	/// Description:
	///		No flags assiged on specified set_flags item.
	///
	void IssueLogger::wrnALC5169(QString setFlagsItemLabel, QUuid itemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Warning1, 5169, itemUuid, schemaID);

		LOG_WARNING1(IssueType::AlCompiler,
				  5169,
				  QString(tr("No flags assiged on set_flags item %1 (Schema %2)")).
					arg(setFlagsItemLabel).arg(schemaID));
	}

	/// IssueCode: ALC5170
	///
	/// IssueType: Error
	///
	/// Title: LM's %1 native signal %2 can't be received via opto connection (Logic schema %3)
	///
	/// Parameters:
	///		%1 LM equipmentID
	///		%2 application signal ID
	///
	/// Description:
	///		LM's native signal can't be received via opto connection.
	///
	void IssueLogger::errALC5170(QString lmEquipmentID, QString appSignalID, QUuid itemUuid, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5170, itemUuid, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5170,
				  QString(tr("LM's %1 native signal %2 can't be received via opto connection (Logic schema %3)")).
						arg(lmEquipmentID).arg(appSignalID).arg(schemaID));
	}

	/// IssueCode: ALC5171
	///
	/// IssueType: Error
	///
	/// Title: Internal application signal %1 cannot be linked to equipment input/output signal %2.
	///
	/// Parameters:
	///		%1 application signal ID
	///		%2 equipment signal ID
	///
	/// Description:
	///		Only input/output application signals can be linked to equipment input/output signals.
	///
	///
	void IssueLogger::errALC5171(QString appSignalID, QString equipmentSignalID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5171,
				  QString(tr("Internal application signal %1 cannot be linked to equipment input/output signal %2.")).
						arg(appSignalID).arg(equipmentSignalID));
	}

	//

	/// IssueCode: ALC5186
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 is not found (opto port %2 raw data description).
	///
	/// Parameters:
	///		%1 Application signalID
	///		%2 Opto port equpment ID
	///
	/// Description:
	///		Signal specified in opto port raw data description is not found. Check ID of signal.
	///
	void IssueLogger::errALC5186(QString appSignalID, QString portEquipmentID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5186,
				  QString(tr("Signal %1 is not found (opto port %2 raw data description).")).
						arg(appSignalID).arg(portEquipmentID));
	}

	/// IssueCode: ALC5187
	///
	/// IssueType: Error
	///
	/// Title: Tx data memory areas of opto ports %1 and %2 are overlapped.
	///
	/// Parameters:
	///		%1 Opto port 1 ID
	///		%2 Opto port 2 ID
	///
	/// Description:
	///		Tx data memory areas of specified opto ports are overlapped. Check manual settinggs of opto ports.
	///
	void IssueLogger::errALC5187(QString port1ID, QString port2ID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5187,
				  QString(tr("Tx data memory areas of opto ports %1 and %2 are overlapped.")).
						arg(port1ID).arg(port2ID));
	}

	/// IssueCode: ALC5188
	///
	/// IssueType: Error
	///
	/// Title: Duplicate signal ID %1 in opto port %2 raw data description.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///
	/// Description:
	///		Duplicate signal ID in specified opto port raw data description. Check description.
	///
	void IssueLogger::errALC5188(QString appSignalID, QString portID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5188,
				  QString(tr("Signal ID %1 is duplicate in opto port %2 raw data description.")).
						arg(appSignalID).arg(portID));
	}

	/// IssueCode: ALC5189
	///
	/// IssueType: Error
	///
	/// Title: Tx signal %1 specified in opto port %2 raw data description is not exists in LM %3.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Logic module ID
	///
	/// Description:
	///		Transmitted signal specified in opto port raw data description is not exists in associated LM. Check description or signal ID.
	///
	void IssueLogger::errALC5189(QString appSignalID, QString portID, QString lmID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5189,
				  QString(tr("Tx signal %1 specified in opto port %2 raw data description is not exists in LM %3.")).
						arg(appSignalID).arg(portID).arg(lmID));
	}

	/// IssueCode: ALC5190
	///
	/// IssueType: Error
	///
	/// Title: Rx signal %1 specified in opto port %2 raw data description is not exists in LM %3.
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Logic module ID
	///
	/// Description:
	///		Receiving signal specified in opto port raw data description is not exists in associated LM. Check description or signal ID.
	///
	void IssueLogger::errALC5190(QString appSignalID, QString portID, QString lmID)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5190,
				  QString(tr("Rx signal %1 specified in opto port %2 raw data description is not exists in LM %3.")).
						arg(appSignalID).arg(portID).arg(lmID));
	}

	/// IssueCode: ALC5191
	///
	/// IssueType: Error
	///
	/// Title: Single-port Rx signal %1 is not associated with LM %2 (Logic schema %3).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Logic module ID
	///		%3 Logic schema ID
	///
	/// Description:
	///		Receiving signal specified in opto port raw data description is not exists in associated LM. Check description or signal ID.
	///
	void IssueLogger::errALC5191(QString appSignalID, QString lmID, QUuid itemID, QString schemaID)
	{
		addItemsIssues(OutputMessageLevel::Error, 5191, itemID, schemaID);

		LOG_ERROR(IssueType::AlCompiler,
				  5191,
				  QString(tr("Single-port Rx signal %1 is not associated with LM %2 (Logic schema %3).")).
						arg(appSignalID).arg(lmID).arg(schemaID));
	}

	/// IssueCode: ALC5192
	///
	/// IssueType: Warning
	///
	/// Title: Tx signal %1 specified in port %2 raw data description isn't connected to transmitter (Connection %3).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Connection ID
	///
	/// Description:
	///		Tx signal specified in port raw data description isn't connected to transmitter. Connect signal to transmitter.
	///
	void IssueLogger::wrnALC5192(QString appSignalID, QString portID, QString connectionID)
	{
		LOG_WARNING0(IssueType::AlCompiler,
				  5192,
				  QString(tr("Tx signal %1 specified in port %2 raw data description isn't connected to transmitter (Connection %3).")).
						arg(appSignalID).arg(portID).arg(connectionID));
	}

	/// IssueCode: ALC5193
	///
	/// IssueType: Warning
	///
	/// Title: Rx signal %1 specified in port %2 raw data description isn't assigned to receiver (Connection %3).
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Opto port ID
	///		%3 Connection ID
	///
	/// Description:
	///		Rx signal specified in port raw data description isn't assigned to reciever.
	///
	void IssueLogger::wrnALC5193(QString appSignalID, QString portID, QString connectionID)
	{
		LOG_WARNING0(IssueType::AlCompiler,
				  5193,
				  QString(tr("Rx signal %1 specified in port %2 raw data description isn't assigned to receiver (Connection %3).")).
						arg(appSignalID).arg(portID).arg(connectionID));
	}

	/// IssueCode: ALC5187
	///
	/// IssueType: Warning
	///
	/// Title: Tx data memory areas of ports %1 and %2 with manual settings are overlapped.
	///
	/// Parameters:
	///		%1 Opto port 1 ID
	///		%2 Opto port 2 ID
	///
	/// Description:
	///		Tx data memory areas of specified opto ports are overlapped. Check manual settinggs of opto ports.
	///
	void IssueLogger::wrnALC5194(QString port1ID, QString port2ID)
	{
		LOG_WARNING0(IssueType::AlCompiler,
				  5194,
				  QString(tr("Tx data memory areas of ports %1 and %2 with manual settings are overlapped.")).
						arg(port1ID).arg(port2ID));
	}

	/// IssueCode: ALC5996
	///
	/// IssueType: Error
	///
	/// Title:	   Internal error! %1. File: %1 Line: %2 Function: %3
	///
	/// Parameters:
	///		%1 error message
	///		%2 source file name
	///		%3 source line number
	///		%4 function name
	///
	/// Description:
	///		Internal error in specified function. Contact to the RPCT developers.
	///
	void IssueLogger::errALC5996(QString errorMsg,
								 QString fileName,
								 int lineNo,
								 QString functionName)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5996,
				  QString(tr("Internal error! %1. File: %2 Line: %3 Function: %4")).
								arg(errorMsg).arg(fileName).arg(lineNo).arg(functionName));
	}

	/// IssueCode: ALC5997
	///
	/// IssueType: Error
	///
	/// Title:	   Null pointer occurred! File: %1 Line: %2 Function: %3
	///
	/// Parameters:
	///		%1 source file name
	///		%2 source line number
	///		%3 function name
	///
	/// Description:
	///		Null pointer occurred in specified function. Contact to the RPCT developers.
	///
	void IssueLogger::errALC5997(QString fileName, int lineNo, QString functionName)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5997,
				  QString(tr(" Null pointer occurred! File: %1 Line: %2 Function: %3")).
								arg(fileName).arg(lineNo).arg(functionName));
	}

	/// IssueCode: ALC5998
	///
	/// IssueType: Error
	///
	/// Title:	   Internal error! File: %1 Line: %2 Function: %3
	///
	/// Parameters:
	///		%1 source file name
	///		%2 source line number
	///		%3 function name
	///
	/// Description:
	///		Internal error in specified function. Contact to the RPCT developers.
	///
	void IssueLogger::errALC5998(QString fileName, int lineNo, QString functionName)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5998,
				  QString(tr("Internal error! File: %1 Line: %2 Function: %3")).
								arg(fileName).arg(lineNo).arg(functionName));
	}

	/// IssueCode: ALC5999
	///
	/// IssueType: Error
	///
	/// Title:	   %1 has been finished with errors.
	///
	/// Parameters:
	///		%1 Compilation procedure name
	///
	/// Description:
	///		Specified compilation procedure has been finished with errors.
	///
	void IssueLogger::errALC5999(QString compilationProcedureName)
	{
		LOG_ERROR(IssueType::AlCompiler,
				  5999,
				  QString(tr("%1 has been finished with errors.")).
								arg(compilationProcedureName));
	}

	// EQP			Equipment issues						6000-6999
	//

	/// IssueCode: EQP6000
	///
	/// IssueType: Error
	///
	/// Title: Property Place is less then 0 (Equipment object %1).
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
		addItemsIssues(OutputMessageLevel::Error, 6000, equpmentUuid);

		LOG_ERROR(IssueType::Equipment,
				  6000,
				  tr("Property Place is less then 0 (Equipment object %1).")
				  .arg(equipmemtId)
				  );
	}

	/// IssueCode: EQP6001
	///
	/// IssueType: Error
	///
	/// Title: Two or more equipment objects have the same EquipmentID %1.
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
		addItemsIssues(OutputMessageLevel::Error, 6001, equipmentUuid1);
		addItemsIssues(OutputMessageLevel::Error, 6001, equipmentUuid2);

		LOG_ERROR(IssueType::Equipment,
				  6001,
				  tr("Two or more equipment objects have the same EquipmentID %1.")
				  .arg(equipmemtId)
				  );
	}

	/// IssueCode: EQP6002
	///
	/// IssueType: Error
	///
	/// Title: Two or more equipment objects have the same Uuid %1 (Object1 %2, Object2 %3).
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
		addItemsIssues(OutputMessageLevel::Error, 6002, equipmentUuid);

		LOG_ERROR(IssueType::Equipment,
				  6002,
				  tr("Two or more equipment objects have the same Uuid %1 (Object1 %2, Object2 %3)")
				  .arg(equipmentUuid.toString())
				  .arg(equipmentId1)
				  .arg(equipmentId2)
				  );
	}

	/// IssueCode: EQP6003
	///
	/// IssueType: Error
	///
	/// Title: Ethernet adapters of LMs %1 and %2 has duplicate IP address %3.
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
		addItemsIssues(OutputMessageLevel::Error, 6003, lm1Uuid);
		addItemsIssues(OutputMessageLevel::Error, 6003, lm2Uuid);

		LOG_ERROR(IssueType::Equipment,
				  6003,
				  tr("Ethernet adapters of LMs %1 and %2 has duplicate IP address %3.")
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
		addItemsIssues(OutputMessageLevel::Error, 6004, lmUuid);

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
	/// Title: Child %1 with place %2 is not allowed in parent %3.
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
				  tr("Child %1 with place %2 is not allowed in parent %3.")
                  .arg(childEquipmentId)
                  .arg(childPlace)
                  .arg(equipmentId));
    }

	/// IssueCode: EQP6009
	///
	/// IssueType: Error
	///
	/// Title: Property Place must be 0 (Equipment object %1).
	///
	/// Parameters:
	///		%1 Equipmnet object StrID
	///
	/// Description:
	///		Property Place for Logic Module must be set to 0.
	///
	void IssueLogger::errEQP6009(QString equipmemtId, QUuid equpmentUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 6009, equpmentUuid);

		LOG_ERROR(IssueType::Equipment,
				  6009,
				  tr("Property Place must be 0 (Equipment object %1).")
				  .arg(equipmemtId)
				  );
	}

	/// IssueCode: EQP6020
	///
	/// IssueType: Error
	///
	/// Title: Property LmDescriptionFile is empty, LogicModule %1.
	///
	/// Parameters:
	///		%1 LogicModule EquipmentID
	///
	/// Description:
	///		Property LmDescriptionFile is empty.
	///
	void IssueLogger::errEQP6020(QString lm, QUuid lmUuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 6020, lmUuid);

		LOG_ERROR(IssueType::Equipment,
				  6020,
				  tr("Property LmDescriptionFile is empty, LogicModule %1.")
				  .arg(lm));
	}


	/// IssueCode: EQP6100
	///
	/// IssueType: Error
	///
	/// Title: Unknown software type (Software object StrID %1).
	///
	/// Parameters:
	///		%1 Equipmnet object StrID
	///
	/// Description:
	///		Unknown software type. It is required to set proprety Type to the correct value.
	///
	void IssueLogger::errEQP6100(QString softwareObjectStrId, QUuid uuid)
	{
		addItemsIssues(OutputMessageLevel::Error, 6100, uuid);

		LOG_ERROR(IssueType::Equipment,
				  6100,
				  tr("Unknown software type (Software object StrID %1).")
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

	/// IssueCode: EQP6107
	///
	/// IssueType: Error
	///
	/// Title: Error parsing property %1 specified in software %2.
	///
	/// Parameters:
	///		%1 Property
	///		%2 Software Equipment ID
	///
	/// Description:
	///		Error parsing property in software configuration
	///
	void IssueLogger::errEQP6107(QString property, QString softwareEquipmentId)
	{
		LOG_ERROR(IssueType::Equipment,
				  6107,
				  tr("Error parsing property %1 specified in software %2.")
				  .arg(property)
				  .arg(softwareEquipmentId)
				  );
	}


	/// IssueCode: EQP6108
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 specified in filter %2 in Tuning Client %3 does not exist.
	///
	/// Parameters:
	///		%1 Schema ID
	///		%2 TuningClient Equipment ID
	///
	/// Description:
	///		Signal that is specified in Filter of the Tuning Client does not exist.
	///
	void IssueLogger::errEQP6108(QString appSignalId, QString filter, QString tuningClientEquipmentId)
	{
		LOG_ERROR(IssueType::Equipment,
				  6108,
				  tr("Signal %1 specified in filter %2 in Tuning Client %3 does not exist.")
				  .arg(appSignalId)
				  .arg(filter)
				  .arg(tuningClientEquipmentId)
				  );
	}

	/// IssueCode: EQP6109
	///
	/// IssueType: Error
	///
	/// Title: Equipment ID %1 specified in TuningSourceEquipmentID property of Tuning Client %2 does not exist.
	///
	/// Parameters:
	///		%1 Tuning Source Equipment ID
	///		%2 TuningClient Equipment ID
	///
	/// Description:
	///		Equipment object that is specified in TuningSourceEquipmentID property of the Tuning Client does not exist.
	///
	void IssueLogger::errEQP6109(QString equipmentId, QString tuningClientEquipmentId)
	{
		LOG_ERROR(IssueType::Equipment,
				  6109,
				  tr("Equipment ID %1 specified in TuningSourceEquipmentID property of Tuning Client %2 does not exist.")
				  .arg(equipmentId)
				  .arg(tuningClientEquipmentId)
				  );
	}

	/// IssueCode: EQP6110
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong physical low Limit
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Wrong physical low Limit.
	///
	void IssueLogger::errEQP6110(QString appSignalID)
	{
		LOG_ERROR(IssueType::Equipment,
				  6110,
				  tr("Signal %1 has wrong physical low Limit.")
				  .arg(appSignalID)
				  );
	}

	/// IssueCode: EQP6111
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong physical high Limit
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Wrong physical high Limit.
	///
	void IssueLogger::errEQP6111(QString appSignalID)
	{
		LOG_ERROR(IssueType::Equipment,
				  6111,
				  tr("Signal %1 has wrong physical high Limit.")
				  .arg(appSignalID)
				  );
	}

	/// IssueCode: EQP6112
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 - engeneering low Limit mismatch electrical low Limit: %2, set electrical low Limit: %3
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Wrong electrical low Limit
	///		%3 Correct electrical low Limit
	///
	/// Description:
	///		Only ThermoCouple and ThermoResistor. Engeneering low Limit mismatch electrical low Limit.
	///
	void IssueLogger::errEQP6112(QString appSignalID, QString wrongValue, QString correctValue)
	{
		LOG_ERROR(IssueType::Equipment,
				  6112,
				  tr("Signal %1 - engeneering low Limit mismatch electrical low Limit: %2, set electrical low Limit: %3.")
				  .arg(appSignalID)
				  .arg(wrongValue)
				  .arg(correctValue)
				  );
	}

	/// IssueCode: EQP6113
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 - engeneering high Limit mismatch electrical high Limit: %2, set electrical high Limit: %3
	///
	/// Parameters:
	///		%1 Application signal ID
	///		%2 Wrong electrical low Limit
	///		%3 Correct electrical low Limit
	///
	/// Description:
	///		 Only ThermoCouple and ThermoResistor. Engeneering high Limit mismatch electrical high Limit. Only ThermoCouple and ThermoResistor.
	///
	void IssueLogger::errEQP6113(QString appSignalID, QString wrongValue, QString correctValue)
	{
		LOG_ERROR(IssueType::Equipment,
				  6113,
				  tr("Signal %1 - engeneering high Limit mismatch electrical high Limit: %2, set electrical high Limit: %3.")
				  .arg(appSignalID)
				  .arg(wrongValue)
				  .arg(correctValue)
				  );
	}

	/// IssueCode: EQP6114
	///
	/// IssueType: Error
	///
	/// Title: Signal %1 has wrong R0 (ThermoResistor).
	///
	/// Parameters:
	///		%1 Application signal ID
	///
	/// Description:
	///		Wrong R0. It is required to set R0 of ThermoResistor.
	///
	void IssueLogger::errEQP6114(QString appSignalID)
	{
		LOG_ERROR(IssueType::Equipment,
				  6114,
				  tr("Signal %1 has wrong R0 (ThermoResistor).")
				  .arg(appSignalID)
				  );
	}

	// --
	//
	void IssueLogger::addItemsIssues(OutputMessageLevel level, int issueCode, const std::vector<QUuid>& itemsUuids)
	{
		if ((level == OutputMessageLevel::Warning0 ||
			level == OutputMessageLevel::Warning1 ||
			level == OutputMessageLevel::Warning2) &&
			isIssueSuppressed(issueCode) == true)
		{
			return;
		}

		if (m_buildIssues != nullptr)
		{
			m_buildIssues->addItemsIssues(level, itemsUuids);
		}
	}

	void IssueLogger::addItemsIssues(OutputMessageLevel level, int issueCode, const std::vector<QUuid>& itemsUuids, const QString& schemaID)
	{
		if ((level == OutputMessageLevel::Warning0 ||
			level == OutputMessageLevel::Warning1 ||
			level == OutputMessageLevel::Warning2) &&
			isIssueSuppressed(issueCode) == true)
		{
			return;
		}

		if (m_buildIssues != nullptr)
		{
			m_buildIssues->addItemsIssues(level, itemsUuids, schemaID);
		}
	}

	void IssueLogger::addItemsIssues(OutputMessageLevel level, int issueCode, QUuid itemsUuid)
	{
		if ((level == OutputMessageLevel::Warning0 ||
			level == OutputMessageLevel::Warning1 ||
			level == OutputMessageLevel::Warning2) &&
			isIssueSuppressed(issueCode) == true)
		{
			return;
		}

		if (m_buildIssues != nullptr)
		{
			m_buildIssues->addItemsIssues(level, itemsUuid);
		}
	}

	void IssueLogger::addItemsIssues(OutputMessageLevel level, int issueCode, QUuid itemsUuid, const QString& schemaID)
	{
		if ((level == OutputMessageLevel::Warning0 ||
			level == OutputMessageLevel::Warning1 ||
			level == OutputMessageLevel::Warning2) &&
			isIssueSuppressed(issueCode) == true)
		{
			return;
		}

		if (m_buildIssues != nullptr)
		{
			m_buildIssues->addItemsIssues(level, itemsUuid, schemaID);
		}
	}

	void IssueLogger::addSchemaIssue(OutputMessageLevel level, int issueCode, const QString& schemaID)
	{
		if ((level == OutputMessageLevel::Warning0 ||
			level == OutputMessageLevel::Warning1 ||
			level == OutputMessageLevel::Warning2) &&
			isIssueSuppressed(issueCode) == true)
		{
			return;
		}

		if (m_buildIssues != nullptr)
		{
			m_buildIssues->addSchemaIssue(level, schemaID);
		}
	}

	void IssueLogger::clearItemsIssues()
	{
		if (m_buildIssues != nullptr)
		{
			m_buildIssues->clear();
		}
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
