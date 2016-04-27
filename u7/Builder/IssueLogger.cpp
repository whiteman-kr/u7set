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
                  tr("Property '%1'' does not exist in object '%2'.")
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
    ///			Software is not found in Equipment. Default values will be used.
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
    /// Title: Different MaxDifference ADC values (place %1: %2; place %3: %4) in application signal '%5'.
    ///
    /// Parameters:
    ///			%1 Place 1
    ///			%2 MaxDifference 1
    ///			%3 Place 2
    ///			%4 MaxDifference 2
    ///         %5 Signal StrID
    ///
    /// Description:
    ///			MaxDifference ADC values should be equal in one channel in AIM module. Check if physical range,
    ///         ADC range and MaxDifference value are equal for AIM inputs A and B in Applicatiion Signals.
    ///
    void IssueLogger::errCFG3009(int place1, double maxDifference1, int place2, double maxDifference2, QString signalID)
    {
        LOG_ERROR(IssueType::FscConfiguration,
                  3009,
                  tr("Different MaxDifference ADC values (place %1: %2; place %3: %4) in application signal '%5'.")
                  .arg(place1)
                  .arg(maxDifference1)
                  .arg(place2)
                  .arg(maxDifference2)
                  .arg(signalID));
    }

	/// IssueCode: CFG3010
	///
	/// IssueType: Error
	///
	/// Title: Property '%1' has wrong value (%2), valid range is %3..%4 [precision %5](module '%6').
	///
	/// Parameters:
	///         %1 Property Name
	///         %2 Property Value
	///         %3 Min Value
	///         %4 Max Value
	///         %5 Precision
	///			%6 Module StrID
	///
	/// Description:
	///			Occurs if a property value is out of range
	///
	void IssueLogger::errCFG3010(QString name, double value, double min, double max, int precision, QString module)
	{
		QString sValue = QString::number(value, 'f', precision);
		QString sMin = QString::number(min, 'f', precision);
		QString sMax = QString::number(max, 'f', precision);

		LOG_ERROR(IssueType::FscConfiguration,
				  3010,
				  tr("Property '%1'' has wrong value (%2), valid range is %3..%4 (module '%6').")
				  .arg(name)
				  .arg(sValue)
				  .arg(sMin)
				  .arg(sMax)
				  .arg(module));
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
	/// IssueType: Warning
	///
	/// Title: Property DeviceStrIds for Logic Schema is not set (LogicSchema '%1').
	///
	/// Parameters:
	///		%1 Logic schema StrID
	///
	/// Description:
	///		Property DeviceStrIds for an application logic schema is empty. To bind a schema to a Logic Module this field must be set
	///		to the Logic Module StrID. (Note that expanded StrID must be used).
	///
	void IssueLogger::wrnALP4001(QString schema)
	{
		LOG_WARNING(IssueType::AlParsing,
				  4001,
				  tr("Property DeviceStrIds for Logic Schema is not set (LogicSchema '%1').")
				  .arg(schema));
	}

	/// IssueCode: ALP4002
	///
	/// IssueType: Error
	///
	/// Title: EquipmentID '%1' is not found in the project equipment (Logic Schema '%2').
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
				  tr("EquipmentID '%1' is not found in the project equipment (Logic Schema '%2').")
				  .arg(equipmentId)
				  .arg(schema));
	}

	/// IssueCode: ALP4003
	///
	/// IssueType: Error
	///
	/// Title: EquipmentID '%1' must be LM family module type (Logic Schema '%2').
	///
	/// Parameters:
	///		%1 Logic modules StrID
	///		%2 Logic schema StrID
	///
	/// Description:
	///		Logic Schema has property EquipmentIDs but the equipment object with pointed StrID is not a module or is not LM family type.
	///
	void IssueLogger::errALP4003(QString schema, QString equipmentId)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4003,
				  tr("EquipmentID '%1' must be LM family module type (Logic Schema '%2').")
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
	/// Title: There is no any input element in applictaion logic for Logic Module '%1'.
	///
	/// Parameters:
	///		%1 Logic module StrID
	///
	/// Description:
	///		Imposible to set execution order for logic items in logic module as there is no any input element.
	///
	void IssueLogger::errALP4008(QString logicModule)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4008,
				  tr("There is no any input element in applictaion logic for Logic Module '%1'.")
				  .arg(logicModule));
	}

	/// IssueCode: ALP4009
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
	void IssueLogger::errALP4009(QString logicModule, QString schema1, QString schema2, QString schemaItem1, QString schemaItem2, QString signalStrID, const std::vector<QUuid>& itemsUuids)
	{
		addItemsIssues(OutputMessageLevel::Error, itemsUuids);

		LOG_ERROR(IssueType::AlParsing,
				  4009,
				  tr("Duplicate output signal %1, item '%2' on schema '%3', item '%4' on schema '%5' (Logic Module '%6').")
				  .arg(signalStrID)
				  .arg(schemaItem1)
				  .arg(schema1)
				  .arg(schemaItem2)
				  .arg(schema2)
				  .arg(logicModule)
				  );
	}

	/// IssueCode: ALP4010
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
	void IssueLogger::errALP4010(QString schema)
	{
		LOG_ERROR(IssueType::AlParsing,
				  4010,
				  tr("Schema does not have Logic layer (Logic Schema '%1').").arg(schema));
	}

	// ALC			Application logic compiler				5000-5999
	//

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
