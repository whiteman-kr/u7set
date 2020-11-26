#include "Options.h"

#include <QSettings>

#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CalibratorOption::CalibratorOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorOption::CalibratorOption(const QString& port, int type) :
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CalibratorsOption::CalibratorsOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorsOption::CalibratorsOption(const CalibratorsOption& from, QObject *parent) :
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
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		assert(0);
		return CalibratorOption();
	}

	return m_calibrator[channel];
}

// -------------------------------------------------------------------------------------------------------------------


void CalibratorsOption::setCalibrator(int channel, const CalibratorOption& сalibrator)
{
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		assert(0);
		return;
	}

	m_calibrator[channel] = сalibrator;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorsOption::load()
{
	QSettings s;

	for(int c = 0; c < Metrology::ChannelCount; c++ )
	{
		QString defaultPort = QString("COM%1").arg(QString::number(c+1));

		QString port = s.value(QString("%1Calibrator%2/Port").arg(CALIBRATOR_OPTIONS_KEY).arg(c), defaultPort).toString();
		int type = s.value(QString("%1Calibrator%2/Type").arg(CALIBRATOR_OPTIONS_KEY).arg(c), CALIBRATOR_TYPE_CALYS75).toInt();

		m_calibrator[c].setPort(port);
		m_calibrator[c].setType(type);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorsOption::save()
{
	QSettings s;

	for(int c = 0; c < Metrology::ChannelCount; c++ )
	{
		s.setValue(QString("%1Calibrator%2/Port").arg(CALIBRATOR_OPTIONS_KEY).arg(c), m_calibrator[c].port());
		s.setValue(QString("%1Calibrator%2/Type").arg(CALIBRATOR_OPTIONS_KEY).arg(c), m_calibrator[c].type());
	}
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorsOption& CalibratorsOption::operator=(const CalibratorsOption& from)
{
	for(int c = 0; c < Metrology::ChannelCount; c++ )
	{
		m_calibrator[c] = from.m_calibrator[c];
	}

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SocketClientOption::SocketClientOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

QString SocketClientOption::equipmentID(int serverType) const
{
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return m_connectOption[serverType].equipmentID;
}

// -------------------------------------------------------------------------------------------------------------------

void SocketClientOption::setEquipmentID(int serverType, const QString& equipmentID)
{
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_connectOption[serverType].equipmentID = equipmentID;
}

// -------------------------------------------------------------------------------------------------------------------

QString SocketClientOption::serverIP(int serverType) const
{
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return m_connectOption[serverType].serverIP;
}

// -------------------------------------------------------------------------------------------------------------------

void SocketClientOption::setServerIP(int serverType, const QString& ip)
{
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_connectOption[serverType].serverIP = ip;
}

// -------------------------------------------------------------------------------------------------------------------

int SocketClientOption::serverPort(int serverType) const
{
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_connectOption[serverType].serverPort;
}

// -------------------------------------------------------------------------------------------------------------------

void SocketClientOption::setServerPort(int serverType, int port)
{
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_connectOption[serverType].serverPort = port;
}

// -------------------------------------------------------------------------------------------------------------------

HostAddressPort SocketClientOption::address(int serverType) const
{
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		assert(0);
		return HostAddressPort();
	}

	return HostAddressPort(m_connectOption[serverType].serverIP, m_connectOption[serverType].serverPort);
}

// -------------------------------------------------------------------------------------------------------------------

void SocketClientOption::load()
{
	if (m_type < 0 || m_type >= SOCKET_TYPE_COUNT)
	{
		return;
	}

	QSettings s;

	QString key = SOCKET_OPTIONS_KEY + QString(SocketType[m_type]) + "/";

	for(int t = 0; t < SOCKET_SERVER_TYPE_COUNT; t++)
	{
		m_connectOption[t].equipmentID = s.value(QString("%1EquipmentID%2").arg(key).arg(t), "SYSTEM_RACKID_WS00" + QString(SocketDefaultID[m_type])).toString();

		m_connectOption[t].serverIP = s.value(QString("%1ServerIP%2").arg(key).arg(t), "127.0.0.1").toString();
		m_connectOption[t].serverPort = s.value(QString("%1ServerPort%2").arg(key).arg(t), SocketDefaultPort[m_type]).toInt();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SocketClientOption::save()
{
	if (m_type < 0 || m_type >= SOCKET_TYPE_COUNT)
	{
		return;
	}

	QSettings s;

	QString key = SOCKET_OPTIONS_KEY + QString(SocketType[m_type]) + "/";

	for(int t = 0; t < SOCKET_SERVER_TYPE_COUNT; t++)
	{
		s.setValue(QString("%1EquipmentID%2").arg(key).arg(t), m_connectOption[t].equipmentID);

		s.setValue(QString("%1ServerIP%2").arg(key).arg(t), m_connectOption[t].serverIP);
		s.setValue(QString("%1ServerPort%2").arg(key).arg(t), m_connectOption[t].serverPort);
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool SocketClientOption::init(const MetrologySettings& settings)
{
	if (m_type < 0 || m_type >= SOCKET_TYPE_COUNT)
	{
		return false;
	}

	bool result = true;

	switch(m_type)
	{
		case SOCKET_TYPE_SIGNAL:
			{
				CONNECTION_OPTION& primary = m_connectOption[SOCKET_SERVER_TYPE_PRIMARY];

				primary.isValid = settings.appDataServicePropertyIsValid1;
				primary.equipmentID = settings.appDataServiceID1;
				primary.serverIP = settings.appDataServiceIP1;
				primary.serverPort = settings.appDataServicePort1;

				CONNECTION_OPTION& reserve = m_connectOption[SOCKET_SERVER_TYPE_RESERVE];

				reserve.isValid = settings.appDataServicePropertyIsValid2;
				reserve.equipmentID = settings.appDataServiceID2;
				reserve.serverIP = settings.appDataServiceIP2;
				reserve.serverPort = settings.appDataServicePort2;

				save();
			}

			break;

		case SOCKET_TYPE_TUNING:
			{
				CONNECTION_OPTION& primary = m_connectOption[SOCKET_SERVER_TYPE_PRIMARY];

				primary.isValid = settings.tuningServicePropertyIsValid;
				primary.equipmentID = settings.softwareMetrologyID;
				primary.serverIP = settings.tuningServiceIP;
				primary.serverPort = settings.tuningServicePort;

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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SocketOption::SocketOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

SocketOption::SocketOption(const SocketOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


SocketOption::~SocketOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

SocketClientOption SocketOption::client(int socketType) const
{
	if (socketType < 0 || socketType >= SOCKET_TYPE_COUNT)
	{
		assert(0);
		return SocketClientOption();
	}

	return m_client[socketType];
}

// -------------------------------------------------------------------------------------------------------------------


void SocketOption::setClient(int socketType, const SocketClientOption& socketClient)
{
	if (socketType < 0 || socketType >= SOCKET_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_client[socketType] = socketClient;
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::load()
{
	for(int t = 0; t < SOCKET_TYPE_COUNT; t++)
	{
		m_client[t].setSocketType(t);
		m_client[t].load();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SocketOption::save()
{
	for(int t = 0; t < SOCKET_TYPE_COUNT; t++)
	{
		m_client[t].save();
	}
}

// -------------------------------------------------------------------------------------------------------------------

SocketOption& SocketOption::operator=(const SocketOption& from)
{
	for(int t = 0; t < SOCKET_TYPE_COUNT; t++)
	{
		m_client[t] = from.m_client[t];
	}

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ProjectInfo::ProjectInfo(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ProjectInfo::ProjectInfo(const ProjectInfo& from, QObject *parent) :
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

	result = xml.findElement("BuildInfo");
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

ModuleOption::ModuleOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ModuleOption::ModuleOption(const ModuleOption& from, QObject *parent) :
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
	if (count == 0)
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
	m_warningIfMeasured = from.m_warningIfMeasured;

	m_maxInputCount = from.m_maxInputCount;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LinearityOption::LinearityOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

LinearityOption::LinearityOption(const LinearityOption& from, QObject *parent) :
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

	if (m_measureCountInPoint > MAX_MEASUREMENT_IN_POINT)
	{
		m_measureCountInPoint = MAX_MEASUREMENT_IN_POINT;
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

	if (measureCount > MAX_MEASUREMENT_IN_POINT)
	{
		measureCount = MAX_MEASUREMENT_IN_POINT;
	}

	m_measureCountInPoint = measureCount;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::recalcPoints(int count)
{
	if (m_rangeType != LO_RANGE_TYPE_AUTOMATIC)
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
		m_pointBase.append(MeasurePoint((m_lowLimitRange + m_highLimitRange) / 2));
	}
	else
	{
		double value = static_cast<double>((m_highLimitRange - m_lowLimitRange) / (count - 1));

		for (int p = 0; p < count ; p++)
		{
			m_pointBase.append(MeasurePoint(m_lowLimitRange + (p * value)));
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::load()
{
	QSettings s;

	m_errorLimit = s.value(QString("%1ErrorLimit").arg(LINEARITY_OPTIONS_KEY), 0.2).toDouble();
	m_errorType = s.value(QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), MEASURE_ERROR_TYPE_REDUCE).toInt();
	m_limitType = s.value(QString("%1ShowErrorFromLimit").arg(LINEARITY_OPTIONS_KEY), MEASURE_LIMIT_TYPE_ELECTRIC).toInt();

	m_measureTimeInPoint = s.value(QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), 1).toInt();
	m_measureCountInPoint = s.value(QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), 20).toInt();

	m_rangeType = s.value(QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), LO_RANGE_TYPE_MANUAL).toInt();
	m_lowLimitRange = s.value(QString("%1LowLimitRange").arg(LINEARITY_OPTIONS_KEY), 5).toDouble();
	m_highLimitRange = s.value(QString("%1HighLimitRange").arg(LINEARITY_OPTIONS_KEY), 95).toDouble();

	m_viewType = s.value(QString("%1ViewType").arg(LINEARITY_OPTIONS_KEY), LO_VIEW_TYPE_SIMPLE).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::save()
{
	QSettings s;

	s.setValue(QString("%1ErrorLimit").arg(LINEARITY_OPTIONS_KEY), m_errorLimit);
	s.setValue(QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), m_errorType);
	s.setValue(QString("%1ShowErrorFromLimit").arg(LINEARITY_OPTIONS_KEY), m_limitType);

	s.setValue(QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureTimeInPoint);
	s.setValue(QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureCountInPoint);

	s.setValue(QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), m_rangeType);
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
	m_limitType = from.m_limitType;

	m_measureTimeInPoint = from.m_measureTimeInPoint;
	m_measureCountInPoint = from.m_measureCountInPoint;

	m_rangeType = from.m_rangeType;
	m_lowLimitRange = from.m_lowLimitRange;
	m_highLimitRange = from.m_highLimitRange;

	m_viewType = from.m_viewType;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorOption::ComparatorOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorOption::ComparatorOption(const ComparatorOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


ComparatorOption::~ComparatorOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorOption::load()
{
	QSettings s;

	m_errorLimit = s.value(QString("%1ErrorLimit").arg(COMPARATOR_OPTIONS_KEY), 0.2).toDouble();
	m_startValueForCompare = s.value(QString("%1StartValueForCompare").arg(COMPARATOR_OPTIONS_KEY), 0.1).toDouble();
	m_errorType = s.value(QString("%1ErrorType").arg(COMPARATOR_OPTIONS_KEY), MEASURE_ERROR_TYPE_REDUCE).toInt();
	m_limitType = s.value(QString("%1ShowErrorFromLimit").arg(COMPARATOR_OPTIONS_KEY), MEASURE_LIMIT_TYPE_ELECTRIC).toInt();

	m_startComparatorIndex = s.value(QString("%1StartComparatorNo").arg(COMPARATOR_OPTIONS_KEY), 0).toInt();
	m_enableMeasureHysteresis = s.value(QString("%1EnableMeasureHysteresis").arg(COMPARATOR_OPTIONS_KEY), false).toBool();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorOption::save()
{
	QSettings s;

	s.setValue(QString("%1ErrorLimit").arg(COMPARATOR_OPTIONS_KEY), m_errorLimit);
	s.setValue(QString("%1StartValueForCompare").arg(COMPARATOR_OPTIONS_KEY), m_startValueForCompare);
	s.setValue(QString("%1ErrorType").arg(COMPARATOR_OPTIONS_KEY), m_errorType);
	s.setValue(QString("%1ShowErrorFromLimit").arg(COMPARATOR_OPTIONS_KEY), m_limitType);

	s.setValue(QString("%1StartComparatorNo").arg(COMPARATOR_OPTIONS_KEY), m_startComparatorIndex);
	s.setValue(QString("%1EnableMeasureHysteresis").arg(COMPARATOR_OPTIONS_KEY), m_enableMeasureHysteresis);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorOption& ComparatorOption::operator=(const ComparatorOption& from)
{
	m_errorLimit = from.m_errorLimit;
	m_startValueForCompare = from.m_startValueForCompare;
	m_errorType = from.m_errorType;
	m_limitType = from.m_limitType;

	m_startComparatorIndex = from.m_startComparatorIndex;
	m_enableMeasureHysteresis = from.m_enableMeasureHysteresis;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ToolBarOption::ToolBarOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ToolBarOption::ToolBarOption(const ToolBarOption& from, QObject *parent) :
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

	m_measureTimeout = s.value(QString("%1MeasureTimeout").arg(TOOLBAR_OPTIONS_KEY), 0).toInt();
	m_measureKind = s.value(QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), MEASURE_KIND_ONE_RACK).toInt();
	m_signalConnectionType = s.value(QString("%1SignalConnectionType").arg(TOOLBAR_OPTIONS_KEY), SIGNAL_CONNECTION_TYPE_UNUSED).toInt();

	m_defaultRack = s.value(QString("%1DefaultRack").arg(TOOLBAR_OPTIONS_KEY), "RACK").toString();
	m_defaultSignalId = s.value(QString("%1DefaultSignalId").arg(TOOLBAR_OPTIONS_KEY), "SIGNAL_ID").toString();
}

// -------------------------------------------------------------------------------------------------------------------

void ToolBarOption::save()
{
	QSettings s;

	s.setValue(QString("%1MeasureTimeout").arg(TOOLBAR_OPTIONS_KEY), m_measureTimeout);
	s.setValue(QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), m_measureKind);
	s.setValue(QString("%1SignalConnectionType").arg(TOOLBAR_OPTIONS_KEY), m_signalConnectionType);

	s.setValue(QString("%1DefaultRack").arg(TOOLBAR_OPTIONS_KEY), m_defaultRack);
	s.setValue(QString("%1DefaultSignalId").arg(TOOLBAR_OPTIONS_KEY), m_defaultSignalId);
}

// -------------------------------------------------------------------------------------------------------------------

ToolBarOption& ToolBarOption::operator=(const ToolBarOption& from)
{
	m_measureTimeout = from.m_measureTimeout;
	m_measureKind = from.m_measureKind;
	m_signalConnectionType = from.m_signalConnectionType;

	m_defaultRack = from.m_defaultRack;
	m_defaultSignalId = from.m_defaultSignalId;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption::MeasureViewOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption::MeasureViewOption(const MeasureViewOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------


MeasureViewOption::~MeasureViewOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureViewOption::updateColumnView(int measureType) const
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return false;
	}

	return m_updateColumnView[measureType];
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::setUpdateColumnView(int measureType, bool state)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
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
	int languageType = theOptions.language().languageType();
	if (languageType >= 0 && languageType < LANGUAGE_TYPE_COUNT)
	{
		// init
		//
		MeasureViewHeader header;

		for(int measureType = 0; measureType < MEASURE_TYPE_COUNT; measureType ++)
		{
			header.setMeasureType(measureType);

			for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
			{
				MeasureViewColumn* pColumn = header.column(column);
				if (pColumn == nullptr)
				{
					continue;
				}

				m_column[measureType][languageType][column] = *pColumn;
			}
		}

		// load
		//
		for(int measureType = 0; measureType < MEASURE_TYPE_COUNT; measureType ++)
		{
			for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
			{
				const MeasureViewColumn& c = m_column[measureType][languageType][column];
				if (c.title().isEmpty() == true)
				{
					continue;
				}

				m_column[measureType][languageType][column].setTitle(s.value(QString("%1/Header/%2/%3/%4/Title").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[measureType]).arg(c.uniqueTitle()).arg(LanguageTypeStr[languageType]), c.title()).toString());
				m_column[measureType][languageType][column].setWidth(s.value(QString("%1/Header/%2/%3/%4/Width").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[measureType]).arg(c.uniqueTitle()).arg(LanguageTypeStr[languageType]), c.width()).toInt());
				m_column[measureType][languageType][column].setVisible(s.value(QString("%1/Header/%2/%3/%4/Visible").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[measureType]).arg(c.uniqueTitle()).arg(LanguageTypeStr[languageType]), c.enableVisible()).toBool());
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
	int languageType = theOptions.language().languageType();
	if (languageType >= 0 && languageType < LANGUAGE_TYPE_COUNT)
	{
		for(int measureType = 0; measureType < MEASURE_TYPE_COUNT; measureType ++)
		{
			for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
			{
				const MeasureViewColumn& c = m_column[measureType][languageType][column];

				if (c.title().isEmpty() == true)
				{
					continue;
				}

				s.setValue(QString("%1/Header/%2/%3/%4/Title").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[measureType]).arg(c.uniqueTitle()).arg(LanguageTypeStr[languageType]), c.title());
				s.setValue(QString("%1/Header/%2/%3/%4/Width").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[measureType]).arg(c.uniqueTitle()).arg(LanguageTypeStr[languageType]), c.width());
				s.setValue(QString("%1/Header/%2/%3/%4/Visible").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[measureType]).arg(c.uniqueTitle()).arg(LanguageTypeStr[languageType]), c.enableVisible());
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

MeasureViewOption& MeasureViewOption::operator=(const MeasureViewOption& from)
{
	for(int measureType = 0; measureType < MEASURE_TYPE_COUNT; measureType ++)
	{
		m_updateColumnView[measureType] = from.m_updateColumnView[measureType];

		for(int languageType = 0; languageType < LANGUAGE_TYPE_COUNT; languageType ++)
		{
			for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalInfoOption::SignalInfoOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoOption::SignalInfoOption(const SignalInfoOption& from, QObject *parent) :
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
	m_colorFlagOverflow = s.value(QString("%1ColorFlagOverflow").arg(SIGNAL_INFO_OPTIONS_KEY), COLOR_FLAG_OVERFLOW.rgb()).toUInt();
	m_colorFlagUnderflow = s.value(QString("%1ColorFlagUnderflow").arg(SIGNAL_INFO_OPTIONS_KEY), COLOR_FLAG_OVERBREAK.rgb()).toUInt();

	m_timeForUpdate = s.value(QString("%1TimeForUpdate").arg(SIGNAL_INFO_OPTIONS_KEY), 250).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoOption::save()
{
	QSettings s;

	s.setValue(QString("%1Font").arg(SIGNAL_INFO_OPTIONS_KEY), m_font.toString());

	s.setValue(QString("%1ShowNoValid").arg(SIGNAL_INFO_OPTIONS_KEY), m_showNoValid);
	s.setValue(QString("%1ShowElectricState").arg(SIGNAL_INFO_OPTIONS_KEY), m_showElectricState);

	s.setValue(QString("%1ColorFlagValid").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagValid.rgb());
	s.setValue(QString("%1ColorFlagOverflow").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagOverflow.rgb());
	s.setValue(QString("%1ColorFlagUnderflow").arg(SIGNAL_INFO_OPTIONS_KEY), m_colorFlagUnderflow.rgb());

	s.setValue(QString("%1TimeForUpdate").arg(SIGNAL_INFO_OPTIONS_KEY), m_timeForUpdate);
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoOption& SignalInfoOption::operator=(const SignalInfoOption& from)
{
	m_font.fromString(from.m_font.toString());

	m_showNoValid = from.m_showNoValid;
	m_showElectricState = from.m_showElectricState;

	m_colorFlagValid = from.m_colorFlagValid;
	m_colorFlagOverflow = from.m_colorFlagOverflow;
	m_colorFlagUnderflow = from.m_colorFlagUnderflow;

	m_timeForUpdate = from.m_timeForUpdate;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoOption::ComparatorInfoOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoOption::ComparatorInfoOption(const ComparatorInfoOption& from, QObject *parent) :
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

	m_displayingStateFalse = s.value(QString("%1DisplayingStateFalse").arg(COMPARATOR_INFO_OPTIONS_KEY), "False").toString();
	m_displayingStateTrue = s.value(QString("%1DisplayingStateTrue").arg(COMPARATOR_INFO_OPTIONS_KEY), "True").toString();

	m_colorStateFalse = s.value(QString("%1ColorStateFalse").arg(COMPARATOR_INFO_OPTIONS_KEY), COLOR_COMPARATOR_STATE_FALSE.rgb()).toUInt();
	m_colorStateTrue = s.value(QString("%1ColorStateTrue").arg(COMPARATOR_INFO_OPTIONS_KEY), COLOR_COMPARATOR_STATE_TRUE.rgb()).toUInt();

	m_timeForUpdate = s.value(QString("%1TimeForUpdate").arg(COMPARATOR_INFO_OPTIONS_KEY), 250).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoOption::save()
{
	QSettings s;

	s.setValue(QString("%1Font").arg(COMPARATOR_INFO_OPTIONS_KEY), m_font.toString());

	s.setValue(QString("%1DisplayingStateFalse").arg(COMPARATOR_INFO_OPTIONS_KEY), m_displayingStateFalse);
	s.setValue(QString("%1DisplayingStateTrue").arg(COMPARATOR_INFO_OPTIONS_KEY), m_displayingStateTrue);

	s.setValue(QString("%1ColorStateFalse").arg(COMPARATOR_INFO_OPTIONS_KEY), m_colorStateFalse.rgb());
	s.setValue(QString("%1ColorStateTrue").arg(COMPARATOR_INFO_OPTIONS_KEY), m_colorStateTrue.rgb());

	s.setValue(QString("%1TimeForUpdate").arg(COMPARATOR_INFO_OPTIONS_KEY), m_timeForUpdate);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoOption& ComparatorInfoOption::operator=(const ComparatorInfoOption& from)
{
	m_font = from.m_font;

	m_displayingStateFalse = from.m_displayingStateFalse;
	m_displayingStateTrue = from.m_displayingStateTrue;

	m_colorStateFalse = from.m_colorStateFalse;
	m_colorStateTrue = from.m_colorStateTrue;

	m_timeForUpdate = from.m_timeForUpdate;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(const DatabaseOption& from, QObject *parent) :
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

	m_locationPath = s.value(QString("%1LocationPath").arg(DATABASE_OPTIONS_REG_KEY), QDir::currentPath()).toString();
	m_type = s.value(QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), DATABASE_TYPE_SQLITE).toInt();

	m_onStart = s.value(QString("%1OnStart").arg(DATABASE_OPTIONS_REG_KEY), false).toBool();
	m_onExit = s.value(QString("%1OnExit").arg(DATABASE_OPTIONS_REG_KEY), true).toBool();
	m_backupPath = s.value(QString("%1BackupPath").arg(DATABASE_OPTIONS_REG_KEY), QDir::tempPath()).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void DatabaseOption::save()
{
	QSettings s;

	s.setValue(QString("%1LocationPath").arg(DATABASE_OPTIONS_REG_KEY), m_locationPath);
	s.setValue(QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), m_type);

	s.setValue(QString("%1OnStart").arg(DATABASE_OPTIONS_REG_KEY), m_onStart);
	s.setValue(QString("%1OnExit").arg(DATABASE_OPTIONS_REG_KEY), m_onExit);
	s.setValue(QString("%1BackupPath").arg(DATABASE_OPTIONS_REG_KEY), m_backupPath);
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption& DatabaseOption::operator=(const DatabaseOption& from)
{
	m_locationPath = from.m_locationPath;
	m_type = from.m_type;

	m_onStart = from.m_onStart;
	m_onExit = from.m_onExit;
	m_backupPath = from.m_backupPath;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LanguageOption::LanguageOption(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

LanguageOption::LanguageOption(const LanguageOption& from, QObject *parent) :
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

	m_languageType = s.value(QString("%1Language").arg(LANGUAGE_OPTIONS_REG_KEY), LANGUAGE_TYPE_EN).toInt();
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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool compareDouble(double lDouble, double rDouble)
{
	return std::nextafter(lDouble, std::numeric_limits<double>::lowest()) <= rDouble && std::nextafter(lDouble, std::numeric_limits<double>::max()) >= rDouble;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Options::Options(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Options::Options(const Options& from, QObject *parent) :
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

	m_module.save();
	m_linearity.save();
	m_comparator.save();
}

// -------------------------------------------------------------------------------------------------------------------

bool Options::readFromXml(const QByteArray& fileData)
{
	bool result = false;

	result = m_projectInfo.readFromXml(fileData);
	if (result == false)
	{
		return false;
	}

	XmlReadHelper xmlReader(fileData);

	result = m_settings.readFromXml(xmlReader);
	if (result == false)
	{
		return false;
	}

	for(int t = 0; t < SOCKET_TYPE_COUNT; t++)
	{
		if (t == SOCKET_TYPE_CONFIG)
		{
			continue;
		}

		SocketClientOption sco = m_socket.client(t);

		result &= sco.init(m_settings);

		if (result == false)
		{
			continue;
		}

		m_socket.setClient(t, sco);
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

	m_module = from.m_module;
	m_linearity = from.m_linearity;
	m_comparator = from.m_comparator;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
