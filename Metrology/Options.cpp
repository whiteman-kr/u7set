#include "Options.h"

#include "Database.h"

#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CalibratorOption::CalibratorOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorOption::CalibratorOption(const QString& port, CalibratorType type) :
	m_port(port),
	m_type(type)
{
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorOption::isValid() const
{
	if (m_port.isEmpty() == true)
	{
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CalibratorsOption::CalibratorsOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorsOption::CalibratorsOption(const CalibratorsOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorsOption::~CalibratorsOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorOption CalibratorsOption::calibrator(int channel) const
{
	if (ERR_CHANNEL(channel) == true)
	{
		assert(0);
		return CalibratorOption();
	}

	return m_calibrator[channel];
}

// -------------------------------------------------------------------------------------------------------------------


void CalibratorsOption::setCalibrator(int channel, const CalibratorOption& calibrator)
{
	if (ERR_CHANNEL(channel) == true)
	{
		assert(0);
		return;
	}

    m_calibrator[channel] = calibrator;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorsOption::load()
{
	QSettings s;

	for(int c = 0; c < Metrology::CHANNEL_COUNT; c++ )
	{
		QString defaultPort = QString("COM%1").arg(QString::number(c+1));

		QString port = s.value(QString("%1Calibrator%2/Port").arg(CALIBRATOR_OPTIONS_KEY).arg(c), defaultPort).toString();
		CalibratorType type = static_cast<CalibratorType>(s.value(QString("%1Calibrator%2/Type").arg(CALIBRATOR_OPTIONS_KEY).arg(c), CalibratorType::Calys75).toInt());

		if (ERR_CALIBRATOR_TYPE(type) == true)
		{
			type = CalibratorType::Calys75;
		}

		m_calibrator[c].setPort(port);
		m_calibrator[c].setType(type);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorsOption::save()
{
	QSettings s;

	for(int c = 0; c < Metrology::CHANNEL_COUNT; c++ )
	{
		s.setValue(QString("%1Calibrator%2/Port").arg(CALIBRATOR_OPTIONS_KEY).arg(c), m_calibrator[c].port());
		s.setValue(QString("%1Calibrator%2/Type").arg(CALIBRATOR_OPTIONS_KEY).arg(c), m_calibrator[c].type());
	}
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorsOption& CalibratorsOption::operator=(const CalibratorsOption& from)
{
	for(int c = 0; c < Metrology::CHANNEL_COUNT; c++ )
	{
		m_calibrator[c] = from.m_calibrator[c];
	}

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ServerConnection::ServerConnection()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ServerConnection::load(OT::ServerType serverType)
{
	if (ERR_SERVER_TYPE(serverType) == true)
	{
		return;
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return;
	}

	QSettings s;

	QString key = SOCKET_OPTIONS_KEY + OT::serverCaption(serverType) + "/";

	m_equipmentID = s.value(QString("%1EquipmentID%2").arg(key).arg(m_priority), "SYSTEM_RACKID_WS00" + OT::serverDefaultID(serverType)).toString();

	m_serverIP = s.value(QString("%1ServerIP%2").arg(key).arg(m_priority), "127.0.0.1").toString();
	m_serverPort = s.value(QString("%1ServerPort%2").arg(key).arg(m_priority), OT::serverDefaultPort(serverType)).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void ServerConnection::save(OT::ServerType serverType)
{
	if (ERR_SERVER_TYPE(serverType) == true)
	{
		return;
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return;
	}

	QSettings s;

	QString key = SOCKET_OPTIONS_KEY + OT::serverCaption(serverType) + "/";

	s.setValue(QString("%1EquipmentID%2").arg(key).arg(m_priority), m_equipmentID);

	s.setValue(QString("%1ServerIP%2").arg(key).arg(m_priority), m_serverIP);
	s.setValue(QString("%1ServerPort%2").arg(key).arg(m_priority), m_serverPort);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ServerOption::ServerOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

QString ServerOption::equipmentID(OT::ServerPriority priority) const
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return QString();
	}

	return m_connection[priority].equipmentID();
}

// -------------------------------------------------------------------------------------------------------------------

void ServerOption::setEquipmentID(OT::ServerPriority priority, const QString& equipmentID)
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return;
	}

	m_connection[priority].setEquipmentID(equipmentID);
}

// -------------------------------------------------------------------------------------------------------------------

QString ServerOption::serverIP(OT::ServerPriority priority) const
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return QString();
	}

	return m_connection[priority].serverIP();
}

// -------------------------------------------------------------------------------------------------------------------

void ServerOption::setServerIP(OT::ServerPriority priority, const QString& ip)
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return;
	}

	m_connection[priority].setServerIP(ip);
}

// -------------------------------------------------------------------------------------------------------------------

int ServerOption::serverPort(OT::ServerPriority priority) const
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return 0;
	}

	return m_connection[priority].serverPort();
}

// -------------------------------------------------------------------------------------------------------------------

void ServerOption::setServerPort(OT::ServerPriority priority, int port)
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return;
	}

	m_connection[priority].setServerPort(port);
}

// -------------------------------------------------------------------------------------------------------------------

HostAddressPort ServerOption::address(OT::ServerPriority priority) const
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return HostAddressPort();
	}

	return HostAddressPort(m_connection[priority].serverIP(), m_connection[priority].serverPort());
}

// -------------------------------------------------------------------------------------------------------------------

ServerConnection* ServerOption::connection(OT::ServerPriority priority) const
{
	if (ERR_SERVER_PRIORITY(priority) == true)
	{
		assert(0);
		return nullptr;
	}

	return const_cast<ServerConnection*>(&m_connection[priority]);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerOption::load()
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return;
	}

	for(int priority = 0; priority < OT::ServerPriorityCount; priority++)
	{
		m_connection[priority].setServerPriority(static_cast<OT::ServerPriority>(priority));
		m_connection[priority].load(m_type);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ServerOption::save()
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return;
	}

	for(int priority = 0; priority < OT::ServerPriorityCount; priority++)
	{
		m_connection[priority].save(m_type);
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool ServerOption::init(const MetrologySettings& settings)
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return false;
	}

	bool result = true;

	switch(m_type)
	{
		case OT::ServerType::AppDataService:
			{
				ServerConnection& primary = m_connection[OT::ServerPriority::Primary];

				primary.setIsValid(settings.appDataServicePropertyIsValid1);
				primary.setEquipmentID(settings.appDataServiceID1);
				primary.setServerIP(settings.appDataServiceIP1);
				primary.setServerPort(settings.appDataServicePort1);

				ServerConnection& reserve = m_connection[OT::ServerPriority::Reserve];

				reserve.setIsValid(settings.appDataServicePropertyIsValid2);
				reserve.setEquipmentID(settings.appDataServiceID2);
				reserve.setServerIP(settings.appDataServiceIP2);
				reserve.setServerPort(settings.appDataServicePort2);

				save();
			}

			break;

		case OT::ServerType::TuningService:
			{
				ServerConnection& primary = m_connection[OT::ServerPriority::Primary];

				primary.setIsValid(settings.tuningServicePropertyIsValid);
				primary.setEquipmentID(settings.softwareMetrologyID);
				primary.setServerIP(settings.tuningServiceIP);
				primary.setServerPort(settings.tuningServicePort);

				save();
			}

			break;

		default:
			Q_ASSERT(false);
			result = false;
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::serverCaption(OT::ServerType type)
{
	QString caption;

	switch (type)
	{
		case OT::ServerType::ConfigurationService:	caption = "ConfigurationService";	break;
		case OT::ServerType::AppDataService:		caption = "AppDataService";			break;
		case OT::ServerType::TuningService:			caption = "TuningService";			break;

		default:
			assert(0);
			caption = QObject::tr("Unknown");
	}

	return caption;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::serverDefaultID(OT::ServerType type)
{
	QString caption;

	switch (type)
	{
		case OT::ServerType::ConfigurationService:	caption = "_METROLOGY";	break;
		case OT::ServerType::AppDataService:		caption = "_ADS";		break;
		case OT::ServerType::TuningService:			caption = "_METROLOGY";	break;

		default:
			assert(0);
			caption = "_ID";
	}

	return caption;
}

// -------------------------------------------------------------------------------------------------------------------

int OT::serverDefaultPort(OT::ServerType type)
{
	int port;

	switch (type)
	{
		case OT::ServerType::ConfigurationService:	port = PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST;	break;
		case OT::ServerType::AppDataService:		port = PORT_APP_DATA_SERVICE_CLIENT_REQUEST;		break;
		case OT::ServerType::TuningService:			port = PORT_TUNING_SERVICE_CLIENT_REQUEST;			break;

		default:
			assert(0);
			port = Socket::PORT_LOWEST;
	}

	return port;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::serverPriorityCaption(OT::ServerPriority priority)
{
	QString caption;

	switch (priority)
	{
		case OT::ServerPriority::Primary:	caption = QObject::tr("Primary");	break;
		case OT::ServerPriority::Reserve:	caption = QObject::tr("Reserve");	break;

		default:
			assert(0);
			caption = QObject::tr("Unknown");
	}

	return caption;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::serverConnectionParamCaption(OT::ServerConnectionParam param)
{
	QString caption;

	switch (param)
	{
		case OT::sco_Type:			caption = QT_TRANSLATE_NOOP("Options", "Service type");			break;
		case OT::sco_Priority:		caption = QT_TRANSLATE_NOOP("Options", "Server type");			break;
		case OT::sco_EquipmentID:	caption = QT_TRANSLATE_NOOP("Options", "Software EquipmentID");	break;
		case OT::sco_ServerIP:		caption = QT_TRANSLATE_NOOP("Options", "Server IP addres");		break;
		case OT::sco_ServerPort:	caption = QT_TRANSLATE_NOOP("Options", "Server port");			break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SocketOption::SocketOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

SocketOption::SocketOption(const SocketOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


SocketOption::~SocketOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

ServerOption SocketOption::server(OT::ServerType serverType) const
{
	if (ERR_SERVER_TYPE(serverType) == true)
	{
		assert(0);
		return ServerOption();
	}

	return m_server[serverType];
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::setServer(OT::ServerType serverType, const ServerOption& server)
{
	if (ERR_SERVER_TYPE(serverType) == true)
	{
		assert(0);
		return;
	}

	m_server[serverType] = server;
}

// -------------------------------------------------------------------------------------------------------------------

QString SocketOption::equipmentID() const
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return QString();
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return QString();
	}

	ServerConnection* pConnection = m_server[m_type].connection(m_priority);
	if (pConnection == nullptr)
	{
		return QString();
	}

	return pConnection->equipmentID();
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::setEquipmentID(const QString& equipmentID)
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return;
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return;
	}

	ServerConnection* pConnection = m_server[m_type].connection(m_priority);
	if (pConnection == nullptr)
	{
		return;
	}

	pConnection->setEquipmentID(equipmentID);
}

// -------------------------------------------------------------------------------------------------------------------

QString SocketOption::serverIP() const
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return QString();
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return QString();
	}

	ServerConnection* pConnection = m_server[m_type].connection(m_priority);
	if (pConnection == nullptr)
	{
		return QString();
	}

	return pConnection->serverIP();
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::setServerIP(const QString& ip)
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return;
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return;
	}

	ServerConnection* pConnection = m_server[m_type].connection(m_priority);
	if (pConnection == nullptr)
	{
		return;
	}

	pConnection->setServerIP(ip);
}

// -------------------------------------------------------------------------------------------------------------------

int SocketOption::serverPort() const
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return Socket::PORT_LOWEST;
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return Socket::PORT_LOWEST;
	}

	ServerConnection* pConnection = m_server[m_type].connection(m_priority);
	if (pConnection == nullptr)
	{
		return Socket::PORT_LOWEST;
	}

	return pConnection->serverPort();
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::setServerPort(int port)
{
	if (ERR_SERVER_TYPE(m_type) == true)
	{
		return;
	}

	if (ERR_SERVER_PRIORITY(m_priority) == true)
	{
		return;
	}

	ServerConnection* pConnection = m_server[m_type].connection(m_priority);
	if (pConnection == nullptr)
	{
		return;
	}

	pConnection->setServerPort(port);
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::load()
{
	for(int serverType = 0; serverType < OT::ServerTypeCount; serverType++)
	{
		m_server[serverType].setServerType(static_cast<OT::ServerType>(serverType));
		m_server[serverType].load();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::save()
{
	for(int serverType = 0; serverType < OT::ServerTypeCount; serverType++)
	{
		m_server[serverType].save();
	}
}

// -------------------------------------------------------------------------------------------------------------------

SocketOption& SocketOption::operator=(const SocketOption& from)
{
	for(int serverType = 0; serverType < OT::ServerTypeCount; serverType++)
	{
		m_server[serverType] = from.m_server[serverType];
	}

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ProjectInfo::ProjectInfo(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ProjectInfo::ProjectInfo(const ProjectInfo& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

ProjectInfo::~ProjectInfo()
{
}


// -------------------------------------------------------------------------------------------------------------------

void ProjectInfo::save()
{
	QSettings s;

	s.setValue(QString("%1ProjectName").arg(PROJECT_INFO_KEY), m_projectName);
	s.setValue(QString("%1ID").arg(PROJECT_INFO_KEY), m_id);
	s.setValue(QString("%1Date").arg(PROJECT_INFO_KEY), m_date);
	s.setValue(QString("%1Changeset").arg(PROJECT_INFO_KEY), m_changeset);
	s.setValue(QString("%1User").arg(PROJECT_INFO_KEY), m_user);
	s.setValue(QString("%1Workstation").arg(PROJECT_INFO_KEY), m_workstation);
	s.setValue(QString("%1DatabaseVersion").arg(PROJECT_INFO_KEY), m_dbVersion);
	s.setValue(QString("%1CfgFileVersion").arg(PROJECT_INFO_KEY), m_cfgFileVersion);
}

// -------------------------------------------------------------------------------------------------------------------

bool ProjectInfo::readFromXml(const QByteArray& fileData)
{
	bool result = false;

	XmlReadHelper xml(fileData);

	result = xml.findElement(XmlElement::BUILD_INFO);
	if (result == false)
	{
		return false;
	}

	result &= xml.readStringAttribute("Project", &m_projectName);
	result &= xml.readIntAttribute("ID", &m_id);
	result &= xml.readStringAttribute("Date", &m_date);
	result &= xml.readIntAttribute("Changeset", &m_changeset);
	result &= xml.readStringAttribute("User", &m_user);
	result &= xml.readStringAttribute("Workstation", &m_workstation);

	if (result == false)
	{
		return false;
	}

	result = xml.findElement("DatabaseInfo");
	if (result == false)
	{
		return false;
	}

	result &= xml.readIntAttribute("Version", &m_dbVersion);

	if (result == false)
	{
		return false;
	}

	save();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

ProjectInfo& ProjectInfo::operator=(const ProjectInfo& from)
{
	m_projectName = from.m_projectName;
	m_id = from.m_id;
	m_date = from.m_date;
	m_changeset = from.m_changeset;
	m_user = from.m_user;
	m_workstation = from.m_workstation;
	m_dbVersion = from.m_dbVersion;
	m_cfgFileVersion = from.m_cfgFileVersion;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ModuleOption::ModuleOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ModuleOption::ModuleOption(const ModuleOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

ModuleOption::~ModuleOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ModuleOption::setMaxInputCount(int count)
{
	if (count <= 0)
	{
		count = 1;
	}

	m_maxInputCount = count;
}

// -------------------------------------------------------------------------------------------------------------------

void ModuleOption::load()
{
	QSettings s;

	m_suffixSN = s.value(QString("%1SuffixSN").arg(MODULE_OPTIONS_KEY), "_SERIALNO").toString();

	m_measureInterInsteadIn = s.value(QString("%1MeasureInterInsteadIn").arg(MODULE_OPTIONS_KEY), false).toBool();
	m_measureLinAndCmp = s.value(QString("%1MeasureLinAndCmp").arg(MODULE_OPTIONS_KEY), false).toBool();
	m_measureEntireModule = s.value(QString("%1MeasureEntireModule").arg(MODULE_OPTIONS_KEY), false).toBool();
	m_measureShownOnSchemas = s.value(QString("%1MeasureShownOnSchemas").arg(MODULE_OPTIONS_KEY), false).toBool();
	m_warningIfMeasured = s.value(QString("%1WarningIfMeasured").arg(MODULE_OPTIONS_KEY), true).toBool();

	m_maxInputCount = s.value(QString("%1MaxInputCount").arg(MODULE_OPTIONS_KEY), Metrology::InputCount).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void ModuleOption::save()
{
	QSettings s;

	s.setValue(QString("%1SuffixSN").arg(MODULE_OPTIONS_KEY), m_suffixSN);

	s.setValue(QString("%1MeasureInterInsteadIn").arg(MODULE_OPTIONS_KEY), m_measureInterInsteadIn);
	s.setValue(QString("%1MeasureLinAndCmp").arg(MODULE_OPTIONS_KEY), m_measureLinAndCmp);
	s.setValue(QString("%1MeasureEntireModule").arg(MODULE_OPTIONS_KEY), m_measureEntireModule);
	s.setValue(QString("%1MeasureShownOnSchemas").arg(MODULE_OPTIONS_KEY), m_measureShownOnSchemas);
	s.setValue(QString("%1WarningIfMeasured").arg(MODULE_OPTIONS_KEY), m_warningIfMeasured);

	s.setValue(QString("%1MaxInputCount").arg(MODULE_OPTIONS_KEY), m_maxInputCount);
}

// -------------------------------------------------------------------------------------------------------------------

ModuleOption& ModuleOption::operator=(const ModuleOption& from)
{
	m_suffixSN = from.m_suffixSN;

	m_measureInterInsteadIn = from.m_measureInterInsteadIn;
	m_measureLinAndCmp = from.m_measureLinAndCmp;
	m_measureEntireModule = from.m_measureEntireModule;
	m_measureShownOnSchemas = from.m_measureShownOnSchemas;
	m_warningIfMeasured = from.m_warningIfMeasured;

	m_maxInputCount = from.m_maxInputCount;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::ModuleParamCaption(OT::ModuleParam param)
{
	QString caption;

	switch (param)
	{
		case OT::mo_SuffixSN:				caption = QT_TRANSLATE_NOOP("Options", "Suffix to identify signal of module serial number");	break;
		case OT::mo_MeasureInterInsteadIn:	caption = QT_TRANSLATE_NOOP("Options", "Measure Internal signal instead Input signal");			break;
		case OT::mo_MeasureLinAdnCmp:		caption = QT_TRANSLATE_NOOP("Options", "Measure linearity and comparators together");			break;
		case OT::mo_MeasureEntireModule:	caption = QT_TRANSLATE_NOOP("Options", "Measure all signals of module in series");				break;
		case OT::mo_ShowOnSchemas:			caption = QT_TRANSLATE_NOOP("Options", "Measure only signals that are displayed in schemas");	break;
		case OT::mo_WarningIfMeasured:		caption = QT_TRANSLATE_NOOP("Options", "Show warning if signal is already measured");			break;
		case OT::mo_MaxInputs:				caption = QT_TRANSLATE_NOOP("Options", "Maximum number of inputs for input module");			break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LinearityOption::LinearityOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

LinearityOption::LinearityOption(const LinearityOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

LinearityOption::~LinearityOption()
{
	m_pointBase.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int LinearityOption::measureCountInPoint()
{
	if (m_measureCountInPoint == 0)
	{
		m_measureCountInPoint = 1;
	}

	if (m_measureCountInPoint > Measure::MAX_MEASUREMENT_IN_POINT)
	{
		m_measureCountInPoint = Measure::MAX_MEASUREMENT_IN_POINT;
	}

	return m_measureCountInPoint;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::setMeasureCountInPoint(int measureCount)
{
	if (measureCount <= 0)
	{
		measureCount = 1;
	}

	if (measureCount > Measure::MAX_MEASUREMENT_IN_POINT)
	{
		measureCount = Measure::MAX_MEASUREMENT_IN_POINT;
	}

	m_measureCountInPoint = measureCount;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::setLowLimitRange(double lowLimit, bool updateMeasurePoints)
{
	m_lowLimitRange = lowLimit;

	if (updateMeasurePoints == true)
	{
		setMeasurePointsCount();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::setHighLimitRange(double highLimit, bool updateMeasurePoints)
{
	m_highLimitRange = highLimit;

	if (updateMeasurePoints == true)
	{
		setMeasurePointsCount();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::setMeasurePointsCount(int count)
{
	if (m_divisionType != Measure::LT::LinearityDivision::Automatic)
	{
		return;
	}

	if (count == -1)
	{
		count = m_pointBase.count();
	}

	m_pointBase.clear();

	if (count == 0)
	{
		return;
	}

	if (count == 1)
	{
		m_pointBase.append(Measure::Point(0, (m_lowLimitRange + m_highLimitRange) / 2));
	}
	else
	{
		double value = static_cast<double>((m_highLimitRange - m_lowLimitRange) / (count - 1));

		for (int p = 0; p < count ; p++)
		{
			m_pointBase.append(Measure::Point(p, m_lowLimitRange + (p * value)));
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::load()
{
	QSettings s;

	m_errorLimit = s.value(QString("%1ErrorLimit").arg(LINEARITY_OPTIONS_KEY), 0.1).toDouble();
	m_errorType = static_cast<Measure::MT::ErrorType>(s.value(QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), Measure::MT::ErrorType::Reduce).toInt());
	m_calcErrorByRange = static_cast<Measure::MT::CalcErrorRange>(s.value(QString("%1CalcErrorByRange").arg(LINEARITY_OPTIONS_KEY), Measure::MT::CalcErrorRange::By_Electric_Range).toInt());

	m_measureTimeInPoint = s.value(QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), 1).toInt();
	m_measureCountInPoint = s.value(QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), Measure::MAX_MEASUREMENT_IN_POINT).toInt();

	m_divisionType = static_cast<Measure::LT::LinearityDivision>(s.value(QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), Measure::LT::LinearityDivision::Manual).toInt());
	m_lowLimitRange = s.value(QString("%1LowLimitRange").arg(LINEARITY_OPTIONS_KEY), Measure::LinearityRangeLow).toDouble();
	m_highLimitRange = s.value(QString("%1HighLimitRange").arg(LINEARITY_OPTIONS_KEY), Measure::LinearityRangeHigh).toDouble();

	m_viewType = static_cast<OT::LinearityViewType>(s.value(QString("%1ViewType").arg(LINEARITY_OPTIONS_KEY), OT::LinearityViewType::Simple).toInt());
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::save()
{
	QSettings s;

	s.setValue(QString("%1ErrorLimit").arg(LINEARITY_OPTIONS_KEY), m_errorLimit);
	s.setValue(QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), m_errorType);
	s.setValue(QString("%1CalcErrorByRange").arg(LINEARITY_OPTIONS_KEY), m_calcErrorByRange);

	s.setValue(QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureTimeInPoint);
	s.setValue(QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureCountInPoint);

	s.setValue(QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), m_divisionType);
	s.setValue(QString("%1LowLimitRange").arg(LINEARITY_OPTIONS_KEY), m_lowLimitRange);
	s.setValue(QString("%1HighLimitRange").arg(LINEARITY_OPTIONS_KEY), m_highLimitRange);

	s.setValue(QString("%1ViewType").arg(LINEARITY_OPTIONS_KEY), m_viewType);
}

// -------------------------------------------------------------------------------------------------------------------

LinearityOption& LinearityOption::operator=(const LinearityOption& from)
{
	m_pointBase = from.m_pointBase;

	m_errorLimit = from.m_errorLimit;
	m_errorType = from.m_errorType;
	m_calcErrorByRange = from.m_calcErrorByRange;

	m_measureTimeInPoint = from.m_measureTimeInPoint;
	m_measureCountInPoint = from.m_measureCountInPoint;

	m_divisionType = from.m_divisionType;
	m_lowLimitRange = from.m_lowLimitRange;
	m_highLimitRange = from.m_highLimitRange;

	m_viewType = from.m_viewType;

	return *this;
}


// -------------------------------------------------------------------------------------------------------------------

QString OT::LinearityParamCaption(OT::LinearityParam param)
{
	QString caption;

	switch (param)
	{
		case OT::lo_ErrorLimit:			caption = QT_TRANSLATE_NOOP("Options", "Limit of error, %");					break;
		case OT::lo_ErrorType:			caption = QT_TRANSLATE_NOOP("Options", "Error type");							break;
		case OT::lo_CalcErrorByRange:	caption = QT_TRANSLATE_NOOP("Options", "Error is calculated by the range");		break;
		case OT::lo_MeasureTime:		caption = QT_TRANSLATE_NOOP("Options", "Measure time in a point, sec");			break;
		case OT::lo_MaxMeasuresInPoint:	caption = QT_TRANSLATE_NOOP("Options", "Count of measurements in a point");		break;
		case OT::lo_DivisionType:		caption = QT_TRANSLATE_NOOP("Options", "Division of the measure range");		break;
		case OT::lo_PointCount:			caption = QT_TRANSLATE_NOOP("Options", "Count of points");						break;
		case OT::lo_LowLimit:			caption = QT_TRANSLATE_NOOP("Options", "Lower limit of the measure range, %");	break;
		case OT::lo_HighLimit:			caption = QT_TRANSLATE_NOOP("Options", "High limit of the measure range, %");	break;
		case OT::lo_ValuesOfPoints:		caption = QT_TRANSLATE_NOOP("Options", "Points of range");						break;
		case OT::lo_ViewType:			caption = QT_TRANSLATE_NOOP("Options", "Type of measurements list");			break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::LinearityViewTypeCaption(OT::LinearityViewType type)
{
	QString caption;

	switch (type)
	{
		case OT::LinearityViewType::Simple:				caption = QT_TRANSLATE_NOOP("Options", "Simple");				break;
		case OT::LinearityViewType::Extended:			caption = QT_TRANSLATE_NOOP("Options", "Extended");				break;
		case OT::LinearityViewType::Detail_Electric:	caption = QT_TRANSLATE_NOOP("Options", "Detail electric");		break;
		case OT::LinearityViewType::Detail_Engineering:	caption = QT_TRANSLATE_NOOP("Options", "Detail engineering");	break;

		default:
			Q_ASSERT(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
};

// -------------------------------------------------------------------------------------------------------------------

QString OT::LinearityViewTypeCaptionTr(OT::LinearityViewType type)
{
	QString caption;

	switch (type)
	{
		case OT::LinearityViewType::Simple:				caption = QObject::tr("Simple");				break;
		case OT::LinearityViewType::Extended:			caption = QObject::tr("Extended");				break;
		case OT::LinearityViewType::Detail_Electric:	caption = QObject::tr("Detail_Electric");		break;
		case OT::LinearityViewType::Detail_Engineering:	caption = QObject::tr("Detail_Engineering");	break;

		default:
			Q_ASSERT(0);
			caption = QObject::tr("Unknown");
	}

	return caption;
};

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorOption::ComparatorOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorOption::ComparatorOption(const ComparatorOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


ComparatorOption::~ComparatorOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorOption::setStartFromComparator(int index)
{
	if (index <= 0)
	{
		index = 1;
	}

	m_startFromComparator = index - 1;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorOption::load()
{
	QSettings s;

	m_errorLimit = s.value(QString("%1ErrorLimit").arg(COMPARATOR_OPTIONS_KEY), 0.1).toDouble();
	m_errorType = static_cast<Measure::MT::ErrorType>(s.value(QString("%1ErrorType").arg(COMPARATOR_OPTIONS_KEY), Measure::MT::ErrorType::Reduce).toInt());
	m_calcErrorByRange = static_cast<Measure::MT::CalcErrorRange>(s.value(QString("%1CalcErrorByRange").arg(COMPARATOR_OPTIONS_KEY), Measure::MT::CalcErrorRange::By_Electric_Range).toInt());
	m_startValueForCompare = s.value(QString("%1StartValueForCompare").arg(COMPARATOR_OPTIONS_KEY), 0.1).toDouble();

	m_startFromComparator = s.value(QString("%1StartComparatorNo").arg(COMPARATOR_OPTIONS_KEY), 0).toInt();
	m_enableMeasureHysteresis = s.value(QString("%1EnableMeasureHysteresis").arg(COMPARATOR_OPTIONS_KEY), false).toBool();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorOption::save()
{
	QSettings s;

	s.setValue(QString("%1ErrorLimit").arg(COMPARATOR_OPTIONS_KEY), m_errorLimit);
	s.setValue(QString("%1ErrorType").arg(COMPARATOR_OPTIONS_KEY), m_errorType);
	s.setValue(QString("%1CalcErrorByRange").arg(COMPARATOR_OPTIONS_KEY), m_calcErrorByRange);
	s.setValue(QString("%1StartValueForCompare").arg(COMPARATOR_OPTIONS_KEY), m_startValueForCompare);

	s.setValue(QString("%1StartComparatorNo").arg(COMPARATOR_OPTIONS_KEY), m_startFromComparator);
	s.setValue(QString("%1EnableMeasureHysteresis").arg(COMPARATOR_OPTIONS_KEY), m_enableMeasureHysteresis);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorOption& ComparatorOption::operator=(const ComparatorOption& from)
{
	m_errorLimit = from.m_errorLimit;
	m_errorType = from.m_errorType;
	m_calcErrorByRange = from.m_calcErrorByRange;
	m_startValueForCompare = from.m_startValueForCompare;

	m_startFromComparator = from.m_startFromComparator;
	m_enableMeasureHysteresis = from.m_enableMeasureHysteresis;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::ComparatorParamCaption(OT::ComparatorParam param)
{
	QString caption;

	switch (param)
	{
		case OT::co_ErrorLimit:				caption = QT_TRANSLATE_NOOP("Options", "Limit of error, %");							break;
		case OT::co_ErrorType:				caption = QT_TRANSLATE_NOOP("Options", "Error type");									break;
		case OT::co_CalcErrorByRange:		caption = QT_TRANSLATE_NOOP("Options", "Error is calculated by the range");				break;
		case OT::co_StartValue:				caption = QT_TRANSLATE_NOOP("Options", "Start value, %");								break;
		case OT::co_StartFromComparator:	caption = QT_TRANSLATE_NOOP("Options", "Start measurement from the сomparator");		break;
		case OT::co_MeasureHysteresis:		caption = QT_TRANSLATE_NOOP("Options", "Enable to measure hysteresis of comparators");	break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ToolBarOption::ToolBarOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ToolBarOption::ToolBarOption(const ToolBarOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


ToolBarOption::~ToolBarOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ToolBarOption::load()
{
	QSettings s;

	m_measureTimeouts.clear();

	int linearityTimeout = s.value(QString("%1Timeout_Linearity").arg(TOOLBAR_OPTIONS_KEY), 1000).toInt();
	int comparatorsTimeout = s.value(QString("%1Timeout_Comparators").arg(TOOLBAR_OPTIONS_KEY), 1000).toInt();

	m_measureTimeouts[Measure::Linearity] = linearityTimeout;
	m_measureTimeouts[Measure::Comparators] = comparatorsTimeout;

	m_measureKind = s.value(QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), Measure::Kind::OneRack).toInt();
	m_connectionType = s.value(QString("%1ConnectionType").arg(TOOLBAR_OPTIONS_KEY), Metrology::ConnectionType::Unused).toInt();

	m_defaultRack = s.value(QString("%1DefaultRack").arg(TOOLBAR_OPTIONS_KEY), "RACK").toString();
	m_defaultSignalId = s.value(QString("%1DefaultSignalId").arg(TOOLBAR_OPTIONS_KEY), "SIGNAL_ID").toString();
}

// -------------------------------------------------------------------------------------------------------------------

void ToolBarOption::save()
{
	QSettings s;

	for (const auto& [type, timeout] : m_measureTimeouts)
	{
		QString key;

		switch (type)
		{
		case Measure::Linearity:
			key = "Timeout_Linearity";
			break;
		case Measure::Comparators:
			key = "Timeout_Comparators";
			break;
		default:
			continue;
		}

		s.setValue(QString("%1%2").arg(TOOLBAR_OPTIONS_KEY).arg(key), timeout);
	}

	s.setValue(QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), m_measureKind);
	s.setValue(QString("%1ConnectionType").arg(TOOLBAR_OPTIONS_KEY), m_connectionType);

	s.setValue(QString("%1DefaultRack").arg(TOOLBAR_OPTIONS_KEY), m_defaultRack);
	s.setValue(QString("%1DefaultSignalId").arg(TOOLBAR_OPTIONS_KEY), m_defaultSignalId);
}

// -------------------------------------------------------------------------------------------------------------------

ToolBarOption& ToolBarOption::operator=(const ToolBarOption& from)
{
	m_measureTimeout = from.m_measureTimeout;
	m_measureKind = from.m_measureKind;
	m_connectionType = from.m_connectionType;

	m_defaultRack = from.m_defaultRack;
	m_defaultSignalId = from.m_defaultSignalId;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption::MeasureViewOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption::MeasureViewOption(const MeasureViewOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


MeasureViewOption::~MeasureViewOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureViewOption::updateColumnView(Measure::Type measureType) const
{
	if (ERR_MEASURE_TYPE(measureType) == true)
	{
		return false;
	}

	return m_updateColumnView[measureType];
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::setUpdateColumnView(Measure::Type measureType, bool state)
{
	if (ERR_MEASURE_TYPE(measureType) == true)
	{
		return;
	}

	m_updateColumnView[measureType] = state;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::load()
{
	QSettings s;

	// properties of columns
	//
	OT::LanguageType languageType = theOptions.language().languageType();
	if (ERR_LANGUAGE_TYPE(languageType) == false)
	{
		QString language = LanguageTypeCaptionEn(static_cast<OT::LanguageType>(languageType));

		// init
		//
		Measure::ViewHeader header;

		for(int measureType = 0; measureType < Measure::TYPE_COUNT; measureType ++)
		{
			header.setMeasureType(static_cast<Measure::Type>(measureType));

			int coulumnCount = header.count();
			for(int column = 0; column < coulumnCount; column++)
			{
				Measure::HeaderColumn* pColumn = header.column(column);
				if (pColumn == nullptr)
				{
					continue;
				}

				m_column[measureType][languageType][column] = *pColumn;
			}
		}

		// load
		//
		for(int measureType = 0; measureType < Measure::TYPE_COUNT; measureType ++)
		{
			QString caption = Measure::TypeCaption(static_cast<Measure::Type>(measureType));

			for(int column = 0; column < Measure::MaxColumnCount; column++)
			{
				Measure::HeaderColumn& c = m_column[measureType][languageType][column];
				if (c.title().isEmpty() == true)
				{
					continue;
				}

				c.setTitle(s.value(QString("%1/Header/%2/%3/%4/Title").arg(MEASURE_VIEW_OPTIONS_KEY, caption, c.uniqueTitle(), language), c.title()).toString());
				c.setWidth(s.value(QString("%1/Header/%2/%3/Width").arg(MEASURE_VIEW_OPTIONS_KEY, caption, c.uniqueTitle()), c.width()).toInt());
				c.setVisible(s.value(QString("%1/Header/%2/%3/Visible").arg(MEASURE_VIEW_OPTIONS_KEY, caption, c.uniqueTitle()), c.enableVisible()).toBool());
			}
		}
	}

	//
	//
	m_font.fromString(s.value(QString("%1Font").arg(MEASURE_VIEW_OPTIONS_KEY), "Segoe UI, 10").toString());
	m_fontBold = m_font;
	m_fontBold.setBold(true);

	m_colorNotError = s.value(QString("%1ColorNotError").arg(MEASURE_VIEW_OPTIONS_KEY), COLOR_NOT_ERROR.rgb()).toUInt();
	m_colorErrorLimit = s.value(QString("%1ColorErrorLimit").arg(MEASURE_VIEW_OPTIONS_KEY), COLOR_OVER_LIMIT_ERROR.rgb()).toUInt();
	m_colorErrorControl = s.value(QString("%1ColorErrorControl").arg(MEASURE_VIEW_OPTIONS_KEY), COLOR_OVER_CONTROL_ERROR.rgb()).toUInt();

	m_showNoValid = s.value(QString("%1ShowNoValid").arg(MEASURE_VIEW_OPTIONS_KEY), false).toBool();
	m_precesionByCalibrator = s.value(QString("%1ShowPrecesionByCalibrator").arg(MEASURE_VIEW_OPTIONS_KEY), false).toBool();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::save()
{
	QSettings s;

	// properties of columns
	//
	OT::LanguageType languageType = theOptions.language().languageType();
	if (ERR_LANGUAGE_TYPE(languageType) == false)
	{
		QString language = LanguageTypeCaptionEn(static_cast<OT::LanguageType>(languageType));

		for(int measureType = 0; measureType < Measure::TYPE_COUNT; measureType ++)
		{
			QString caption = Measure::TypeCaption(static_cast<Measure::Type>(measureType));

			for(int column = 0; column < Measure::MaxColumnCount; column++)
			{
				const Measure::HeaderColumn& c = m_column[measureType][languageType][column];

				if (c.title().isEmpty() == true)
				{
					continue;
				}

				s.setValue(QString("%1/Header/%2/%3/%4/Title").arg(MEASURE_VIEW_OPTIONS_KEY, caption, c.uniqueTitle(), language), c.title());
				s.setValue(QString("%1/Header/%2/%3/Width").arg(MEASURE_VIEW_OPTIONS_KEY, caption, c.uniqueTitle()), c.width());
				s.setValue(QString("%1/Header/%2/%3/Visible").arg(MEASURE_VIEW_OPTIONS_KEY, caption, c.uniqueTitle()), c.enableVisible());
			}
		}
	}

	//
	//
	s.setValue(QString("%1Font").arg(MEASURE_VIEW_OPTIONS_KEY), m_font.toString());

	s.setValue(QString("%1ColorNotError").arg(MEASURE_VIEW_OPTIONS_KEY), m_colorNotError.rgb());
	s.setValue(QString("%1ColorErrorLimit").arg(MEASURE_VIEW_OPTIONS_KEY), m_colorErrorLimit.rgb());
	s.setValue(QString("%1ColorErrorControl").arg(MEASURE_VIEW_OPTIONS_KEY), m_colorErrorControl.rgb());

	s.setValue(QString("%1ShowNoValid").arg(MEASURE_VIEW_OPTIONS_KEY), m_showNoValid);
	s.setValue(QString("%1ShowPrecesionByCalibrator").arg(MEASURE_VIEW_OPTIONS_KEY), m_precesionByCalibrator);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::saveColumnWidth(Measure::Type measureType, const Measure::HeaderColumn& c)
{
	QSettings s;

	QString caption = Measure::TypeCaption(static_cast<Measure::Type>(measureType));

	s.setValue(QString("%1/Header/%2/%3/Width").arg(MEASURE_VIEW_OPTIONS_KEY, caption, c.uniqueTitle()), c.width());

	OT::LanguageType languageType = theOptions.language().languageType();
	if (ERR_LANGUAGE_TYPE(languageType) == true)
	{
		return;
	}

	for(int column = 0; column < Measure::MaxColumnCount; column++)
	{
		if (m_column[measureType][languageType][column].uniqueTitle() == c.uniqueTitle())
		{
			m_column[measureType][languageType][column].setWidth(c.width());
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption& MeasureViewOption::operator=(const MeasureViewOption& from)
{
	for(int measureType = 0; measureType < Measure::TYPE_COUNT; measureType ++)
	{
		m_updateColumnView[measureType] = from.m_updateColumnView[measureType];

		for(int languageType = 0; languageType < OT::LanguageTypeCount; languageType ++)
		{
			for(int column = 0; column < Measure::MaxColumnCount; column++)
			{
				m_column[measureType][languageType][column] = from.m_column[measureType][languageType][column];
			}
		}
	}

	m_font.fromString(from.m_font.toString());
	m_fontBold = m_font;
	m_fontBold.setBold(true);

	m_colorNotError = from.m_colorNotError;
	m_colorErrorLimit = from.m_colorErrorLimit;
	m_colorErrorControl = from.m_colorErrorControl;

	m_showNoValid = from.m_showNoValid;
	m_precesionByCalibrator = from.m_precesionByCalibrator;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::MeasureViewParamCaption(OT::MeasureViewParam param)
{
	QString caption;

	switch (param)
	{
		case OT::mwo_Font:					caption = QT_TRANSLATE_NOOP("Options", "Font of measurements list");											break;
		case OT::mwo_ColorNoError:			caption = QT_TRANSLATE_NOOP("Options", "Color of measurement that has not error");									break;
		case OT::mwo_ColorErrorOfLimit:		caption = QT_TRANSLATE_NOOP("Options", "Color of measurement over limit error");									break;
		case OT::mwo_ColorErrorOfControl:	caption = QT_TRANSLATE_NOOP("Options", "Color of measurement over control error");									break;
		case OT::mwo_ShowNoValid:			caption = QT_TRANSLATE_NOOP("Options", "Show measuring value if signal is not valid");							break;
		case OT::mwo_PrecesionByCalibrator:	caption = QT_TRANSLATE_NOOP("Options", "Show accuracy for measure value and nominal value from calibrator");	break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalInfoOption::SignalInfoOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoOption::SignalInfoOption(const SignalInfoOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


SignalInfoOption::~SignalInfoOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoOption::load()
{
	QSettings s;

	m_font.fromString(s.value(QString("%1Font").arg(SIGNAL_INFO_OPTIONS_KEY), "Segoe UI, 10").toString());

	m_showNoValid = s.value(QString("%1ShowNoValid").arg(SIGNAL_INFO_OPTIONS_KEY), false).toBool();
	m_showElectricState = s.value(QString("%1ShowElectricState").arg(SIGNAL_INFO_OPTIONS_KEY), false).toBool();

	m_colorFlagValid = s.value(QString("%1ColorFlagValid").arg(SIGNAL_INFO_OPTIONS_KEY), COLOR_FLAG_VALID.rgb()).toUInt();
	m_colorFlagSim = s.value(QString("%1ColorFlagSim").arg(SIGNAL_INFO_OPTIONS_KEY), COLOR_FLAG_SIM.rgb()).toUInt();
	m_colorFlagLock = s.value(QString("%1ColorFlagLock").arg(SIGNAL_INFO_OPTIONS_KEY), COLOR_FLAG_LOCK.rgb()).toUInt();
	m_colorFlagOverflow = s.value(QString("%1ColorFlagOverflow").arg(SIGNAL_INFO_OPTIONS_KEY), COLOR_FLAG_OVERFLOW.rgb()).toUInt();
	m_colorFlagUnderflow = s.value(QString("%1ColorFlagUnderflow").arg(SIGNAL_INFO_OPTIONS_KEY), COLOR_FLAG_OVERBREAK.rgb()).toUInt();

	m_timeForUpdate = s.value(QString("%1TimeForUpdate").arg(SIGNAL_INFO_OPTIONS_KEY), 250).toInt();

	loadColumnsWidth();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoOption::save()
{
	QSettings s;

	s.setValue(QString("%1Font").arg(SIGNAL_INFO_OPTIONS_KEY), m_font.toString());

	s.setValue(QString("%1ShowNoValid").arg(SIGNAL_INFO_OPTIONS_KEY), m_showNoValid);
	s.setValue(QString("%1ShowElectricState").arg(SIGNAL_INFO_OPTIONS_KEY), m_showElectricState);

	s.setValue(QString("%1ColorFlagValid").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagValid.rgb());
	s.setValue(QString("%1ColorFlagSim").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagSim.rgb());
	s.setValue(QString("%1ColorFlagLock").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagLock.rgb());
	s.setValue(QString("%1ColorFlagOverflow").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagOverflow.rgb());
	s.setValue(QString("%1ColorFlagUnderflow").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagUnderflow.rgb());

	s.setValue(QString("%1TimeForUpdate").arg(SIGNAL_INFO_OPTIONS_KEY), m_timeForUpdate);

	saveColumnsWidth();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoOption::loadColumnsWidth()
{
	QSettings s;

	m_columnsWidth = s.value(QString("%1ColumnsWidth").arg(SIGNAL_INFO_OPTIONS_KEY)).value<QMap<QString,int>>();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoOption::saveColumnsWidth()
{
	QSettings s;

	s.setValue(QString("%1ColumnsWidth").arg(SIGNAL_INFO_OPTIONS_KEY), QVariant::fromValue(m_columnsWidth));
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoOption& SignalInfoOption::operator=(const SignalInfoOption& from)
{
	m_font.fromString(from.m_font.toString());

	m_showNoValid = from.m_showNoValid;
	m_showElectricState = from.m_showElectricState;

	m_colorFlagValid = from.m_colorFlagValid;
	m_colorFlagSim = from.m_colorFlagSim;
	m_colorFlagLock = from.m_colorFlagLock;
	m_colorFlagOverflow = from.m_colorFlagOverflow;
	m_colorFlagUnderflow = from.m_colorFlagUnderflow;

	m_timeForUpdate = from.m_timeForUpdate;

	m_columnsWidth = from.m_columnsWidth;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::SignalInfoParamCaption(OT::SignalInfoParam param)
{
	QString caption;

	switch (param)
	{
		case OT::sio_Font:					caption = QT_TRANSLATE_NOOP("Options", "Font of signal information list");				break;
		case OT::sio_ShowNoValid:			caption = QT_TRANSLATE_NOOP("Options", "Show measuring value, if signal is not valid");	break;
		case OT::sio_ShowElectricState:		caption = QT_TRANSLATE_NOOP("Options", "Show electric state");							break;
		case OT::sio_ColorFlagNoValid:		caption = QT_TRANSLATE_NOOP("Options", "Color, if signal has flag \"No validity\"");	break;
		case OT::sio_ColorFlagSim:			caption = QT_TRANSLATE_NOOP("Options", "Color, if signal has flag \"Simulation\"");		break;
		case OT::sio_ColorFlagLock:			caption = QT_TRANSLATE_NOOP("Options", "Color, if signal has flag \"Lock\"");			break;
		case OT::sio_ColorFlagOverflow:		caption = QT_TRANSLATE_NOOP("Options", "Color, if signal has flag \"Overflow\"");		break;
		case OT::sio_ColorFlagUnderflow:	caption = QT_TRANSLATE_NOOP("Options", "Color, if signal has flag \"Underflow\"");		break;
		case OT::sio_TimeForUpdate:			caption = QT_TRANSLATE_NOOP("Options", "Time for updating state of signal, ms");		break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoOption::ComparatorInfoOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoOption::ComparatorInfoOption(const ComparatorInfoOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


ComparatorInfoOption::~ComparatorInfoOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoOption::load()
{
	QSettings s;

	m_font.fromString(s.value(QString("%1Font").arg(COMPARATOR_INFO_OPTIONS_KEY), "Segoe UI, 10").toString());

	m_colorFlagSim = s.value(QString("%1ColorFlagSim").arg(COMPARATOR_INFO_OPTIONS_KEY), COLOR_COMPARATOR_FLAG_SIM.rgb()).toUInt();
	m_colorFlagLock = s.value(QString("%1ColorFlagLock").arg(COMPARATOR_INFO_OPTIONS_KEY), COLOR_COMPARATOR_FLAG_LOCK.rgb()).toUInt();

	m_colorStateFalse = s.value(QString("%1ColorStateFalse").arg(COMPARATOR_INFO_OPTIONS_KEY), COLOR_COMPARATOR_STATE_FALSE.rgb()).toUInt();
	m_colorStateTrue = s.value(QString("%1ColorStateTrue").arg(COMPARATOR_INFO_OPTIONS_KEY), COLOR_COMPARATOR_STATE_TRUE.rgb()).toUInt();

	m_timeForUpdate = s.value(QString("%1TimeForUpdate").arg(COMPARATOR_INFO_OPTIONS_KEY), 250).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoOption::save()
{
	QSettings s;

	s.setValue(QString("%1Font").arg(COMPARATOR_INFO_OPTIONS_KEY), m_font.toString());

	s.setValue(QString("%1ColorFlagSim").arg(COMPARATOR_INFO_OPTIONS_KEY), m_colorFlagSim.rgb());
	s.setValue(QString("%1ColorFlagLock").arg(COMPARATOR_INFO_OPTIONS_KEY), m_colorFlagLock.rgb());

	s.setValue(QString("%1ColorStateFalse").arg(COMPARATOR_INFO_OPTIONS_KEY), m_colorStateFalse.rgb());
	s.setValue(QString("%1ColorStateTrue").arg(COMPARATOR_INFO_OPTIONS_KEY), m_colorStateTrue.rgb());

	s.setValue(QString("%1TimeForUpdate").arg(COMPARATOR_INFO_OPTIONS_KEY), m_timeForUpdate);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoOption& ComparatorInfoOption::operator=(const ComparatorInfoOption& from)
{
	m_font = from.m_font;

	m_colorFlagSim = from.m_colorFlagSim;
	m_colorFlagLock = from.m_colorFlagLock;

	m_colorStateFalse = from.m_colorStateFalse;
	m_colorStateTrue = from.m_colorStateTrue;

	m_timeForUpdate = from.m_timeForUpdate;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::ComparatorInfoParamCaption(OT::ComparatorInfoParam param)
{
	QString caption;

	switch (param)
	{
		case OT::cio_Font:				caption = QT_TRANSLATE_NOOP("Options", "Font of comparator information list");				break;
		case OT::cio_ColorFlagSim:		caption = QT_TRANSLATE_NOOP("Options", "Color, if comparator in the mode \"Simulated\"");	break;
		case OT::cio_ColorFlagLock:		caption = QT_TRANSLATE_NOOP("Options", "Color, if comparator in the mode \"Blocked\"");		break;
		case OT::cio_ColorStateFalse:	caption = QT_TRANSLATE_NOOP("Options", "Color, if comparator has state \"logical 0\"");		break;
		case OT::cio_ColorStateTrue:	caption = QT_TRANSLATE_NOOP("Options", "Color, if comparator has state \"logical 1\"");		break;
		case OT::cio_TimeForUpdate:		caption = QT_TRANSLATE_NOOP("Options", "Time for updating state of comparator, ms");		break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticsOption::StatisticsOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsOption::StatisticsOption(const StatisticsOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


StatisticsOption::~StatisticsOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsOption::load()
{
	QSettings s;

	m_columnsWidth = s.value(QString("%1ColumnsWidth").arg(STATISTICS_OPTIONS_KEY)).value<QMap<QString,int>>();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsOption::save()
{
	QSettings s;

	s.setValue(QString("%1ColumnsWidth").arg(STATISTICS_OPTIONS_KEY), QVariant::fromValue(m_columnsWidth));
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsOption& StatisticsOption::operator=(const StatisticsOption& from)
{
	m_columnsWidth = from.m_columnsWidth;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(const DatabaseOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::~DatabaseOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DatabaseOption::load()
{
	QSettings s;

	m_type = static_cast<OT::DatabaseType>(s.value(QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), OT::SQLite).toInt());
	m_locationPath = s.value(QString("%1LocationPath").arg(DATABASE_OPTIONS_REG_KEY), QDir::currentPath()).toString();

	m_ip = s.value(QString("%1IP").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_IP).toString();
	m_port = s.value(QString("%1Port").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_PORT).toInt();
	m_user = s.value(QString("%1User").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_USER).toString();
	m_password = s.value(QString("%1Password").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_PASSWORD).toString();

	m_onStart = s.value(QString("%1OnStart").arg(DATABASE_OPTIONS_REG_KEY), false).toBool();
	m_onExit = s.value(QString("%1OnExit").arg(DATABASE_OPTIONS_REG_KEY), true).toBool();
	m_backupPath = s.value(QString("%1BackupPath").arg(DATABASE_OPTIONS_REG_KEY), QDir::tempPath()).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void DatabaseOption::save()
{
	QSettings s;

	s.setValue(QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), m_type);
	s.setValue(QString("%1LocationPath").arg(DATABASE_OPTIONS_REG_KEY), m_locationPath);

	s.setValue(QString("%1IP").arg(DATABASE_OPTIONS_REG_KEY), m_ip);
	s.setValue(QString("%1Port").arg(DATABASE_OPTIONS_REG_KEY), m_port);
	s.setValue(QString("%1User").arg(DATABASE_OPTIONS_REG_KEY), m_user);
	s.setValue(QString("%1Password").arg(DATABASE_OPTIONS_REG_KEY), m_password);

	s.setValue(QString("%1OnStart").arg(DATABASE_OPTIONS_REG_KEY), m_onStart);
	s.setValue(QString("%1OnExit").arg(DATABASE_OPTIONS_REG_KEY), m_onExit);
	s.setValue(QString("%1BackupPath").arg(DATABASE_OPTIONS_REG_KEY), m_backupPath);
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption& DatabaseOption::operator=(const DatabaseOption& from)
{
	m_type = from.m_type;
	m_locationPath = from.m_locationPath;

	m_ip = from.m_ip;
	m_port = from.m_port;
	m_user = from.m_user;
	m_password = from.m_password;

	m_onStart = from.m_onStart;
	m_onExit = from.m_onExit;
	m_backupPath = from.m_backupPath;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::DatabaseParamCaption(OT::DatabaseParam param)
{
	QString caption;

	switch (param)
	{
		case OT::dbo_Type:			caption = QT_TRANSLATE_NOOP("Options", "Type");					break;
		case OT::dbo_LocationPath:	caption = QT_TRANSLATE_NOOP("Options", "Location path");		break;
		case OT::dbo_Ip:			caption = QT_TRANSLATE_NOOP("Options", "IP");					break;
		case OT::dbo_Port:			caption = QT_TRANSLATE_NOOP("Options", "Port");					break;
		case OT::dbo_User:			caption = QT_TRANSLATE_NOOP("Options", "User");					break;
		case OT::dbo_Password:		caption = QT_TRANSLATE_NOOP("Options", "Password");				break;
		case OT::dbo_OnStart:		caption = QT_TRANSLATE_NOOP("Options", "On start application"); break;
		case OT::dbo_OnExit:		caption = QT_TRANSLATE_NOOP("Options", "On exit application");	break;
		case OT::dbo_CopyPath:		caption = QT_TRANSLATE_NOOP("Options", "Path for backup");		break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::DatabaseTypeCaption(OT::DatabaseType type)
{
	QString caption;

	switch (type)
	{
		case OT::SQLite:		caption = "SQLite";		break;
		case OT::PostgreSQL:	caption = "PostgreSQL";	break;

		default:
			assert(0);
			caption = QObject::tr("Unknown");
	}

	return caption;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LanguageOption::LanguageOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

LanguageOption::LanguageOption(const LanguageOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

LanguageOption::~LanguageOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void LanguageOption::load()
{
	QSettings s;

	m_languageType = static_cast<OT::LanguageType>(s.value(QString("%1Language").arg(LANGUAGE_OPTIONS_REG_KEY), OT::English).toInt());
}

// -------------------------------------------------------------------------------------------------------------------

void LanguageOption::save()
{
	QSettings s;

	s.setValue(QString("%1Language").arg(LANGUAGE_OPTIONS_REG_KEY), m_languageType);
}

// -------------------------------------------------------------------------------------------------------------------

LanguageOption& LanguageOption::operator=(const LanguageOption& from)
{
	m_languageType = from.m_languageType;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::LanguageTypeCaptionEn(OT::LanguageType type)
{
	QString caption;

	switch (type)
	{
		case OT::English:	caption = "English";	break;
		case OT::Bulgarian:	caption = "Bulgarian";	break;
		case OT::Russian:	caption = "Russian";	break;
		case OT::Ukrainian:	caption = "Ukrainian";	break;

		default:
			Q_ASSERT(0);
			caption = "Unknown";
	}

	return caption;
};

// -------------------------------------------------------------------------------------------------------------------

QString OT::LanguageTypeCaptionTr(OT::LanguageType type)
{
	QString caption;

	switch (type)
	{
		case OT::English:	caption = QObject::tr("English");	break;
		case OT::Bulgarian:	caption = QObject::tr("Bulgarian");	break;
		case OT::Russian:	caption = QObject::tr("Russian");	break;
		case OT::Ukrainian:	caption = QObject::tr("Ukrainian");	break;

		default:
			Q_ASSERT(0);
			caption = QObject::tr("Unknown");
	}

	return caption;
};

// -------------------------------------------------------------------------------------------------------------------

QString OT::LanguageParamCaption(OT::LanguageParam param)
{
	QString caption;

	switch (param)
	{
		case OT::lno_LanguageType:	caption = QT_TRANSLATE_NOOP("Options", "Language");		break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("Options", "Unknown");
	}

	return qApp->translate("Options", caption.toUtf8());
}

// Language initialization
// Ovcharenko 02.09.25 "addition of the Ukrainian language"
// -------------------------------------------------------------------------------------------------------------------

void Options::loadTranslators() const
{
	//

	static const std::map<OT::LanguageType, QString> langMap = {{OT::LanguageType::English, "en"},
																{OT::LanguageType::Bulgarian, "bg"},
																{OT::LanguageType::Russian, "ru"},
																{OT::LanguageType::Ukrainian, "uk"}};

	OT::LanguageType langType = theOptions.language().languageType();

	static QTranslator translatorMetrology;
	static QTranslator translatorUiLib;

	if (langMap.find(langType) != langMap.end() && langType != OT::LanguageType::English)
	{
		QString suffix = langMap.at(langType);

		QString metrologyFile = QApplication::applicationDirPath() + LANGUAGE_OPTIONS_DIR + "/Metrology_" + suffix + ".qm";
		if (translatorMetrology.load(metrologyFile))
		{
			qApp->installTranslator(&translatorMetrology);
		}
		else
		{
			QMessageBox::warning(nullptr,
								 QObject::tr("Language load error"),
								 QString("Didn't load Metrology language file:\n%1").arg(metrologyFile));
		}

		if (langType != OT::LanguageType::Russian)
		{
			QString uiLibFile = QApplication::applicationDirPath() + LANGUAGE_OPTIONS_DIR + "/UiLib_" + suffix + ".qm";
			if (translatorUiLib.load(uiLibFile))
			{
				qApp->installTranslator(&translatorUiLib);
			}
			else
			{
				QMessageBox::warning(nullptr,
									 QObject::tr("Language load error"),
									 QString("Didn't load UiLib language file:\n%1").arg(uiLibFile));
			}
		}
	}
	else
	{
		qDebug() << "Using default English, no translator loaded.";
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool compareDouble(double lDouble, double rDouble)
{
	return std::nextafter(lDouble, std::numeric_limits<double>::lowest()) <= rDouble && std::nextafter(lDouble, std::numeric_limits<double>::max()) >= rDouble;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Options::Options(QObject* parent) :
	QObject(parent)
{
	qRegisterMetaType<QMap<QString,int>>("QMap<QString,int>");
}

// -------------------------------------------------------------------------------------------------------------------

Options::Options(const Options& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

Options::~Options()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Options::load()
{
	m_calibrators.load();
	m_database.load();

	m_socket.load();

	m_language.load();

	m_toolBar.load();
	m_measureView.load();

	m_signalInfo.load();
	m_comparatorInfo.load();
	m_statistics.load();

	m_module.load();
	m_linearity.load();
	m_comparator.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
	m_calibrators.save();
	m_database.save();

	m_socket.save();
	m_projectInfo.save();

	m_language.save();

	m_toolBar.save();
	m_measureView.save();

	m_signalInfo.save();
	m_comparatorInfo.save();
	m_statistics.save();

	m_module.save();
	m_linearity.save();
	m_comparator.save();
}

// -------------------------------------------------------------------------------------------------------------------

bool Options::setMetrologySettings(std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	const MetrologySettings* typedSettingsPtr = dynamic_cast<const MetrologySettings*>(curSettingsProfile.get());

	if (typedSettingsPtr == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	// making modificable (if required) local copy of settings
	//
	m_settings = *typedSettingsPtr;

	return true;
}

bool Options::readFromXml(const QByteArray& fileData)
{
	bool result = false;

	result = m_projectInfo.readFromXml(fileData);

	if (result == false)
	{
		return false;
	}

	for(int type = 0; type < OT::ServerTypeCount; type++)
	{
		OT::ServerType serverType = static_cast<OT::ServerType>(type);
		if (ERR_SERVER_TYPE(serverType) == true)
		{
			continue;
		}

		if (serverType == OT::ServerType::ConfigurationService)
		{
			continue;
		}

		ServerOption sco = m_socket.server(serverType);

		result &= sco.init(m_settings);

		if (result == false)
		{
			continue;
		}

		m_socket.setServer(serverType, sco);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

Options& Options::operator=(const Options& from)
{
	QMutexLocker l(&m_mutex);

	m_calibrators = from.m_calibrators;

	m_socket = from.m_socket;
	m_database = from.m_database;
	m_projectInfo = from.m_projectInfo;

	m_language = from.m_language;

	m_toolBar = from.m_toolBar;
	m_measureView = from.m_measureView;

	m_signalInfo = from.m_signalInfo;
	m_comparatorInfo = from.m_comparatorInfo;
	m_statistics = from.m_statistics;

	m_module = from.m_module;
	m_linearity = from.m_linearity;
	m_comparator = from.m_comparator;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
