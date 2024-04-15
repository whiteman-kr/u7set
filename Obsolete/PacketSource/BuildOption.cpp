#include "BuildOption.h"

#include <QSettings>

#include "../../OnlineLib/SocketIO.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

BuildOption::BuildOption(QObject* parent) :
	QObject(parent)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption::BuildOption(const BuildOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption::~BuildOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::clear()
{
	m_cfgSrvEquipmentID.clear();
	m_cfgSrvIP.clear();
	m_appDataSrvEquipmentID.clear();
	m_ualTesterIP.clear();

	m_signalsStatePath.clear();
	m_sourcesForRunList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::load()
{
	QSettings s;

	//
	//
	m_cfgSrvEquipmentID = s.value(QString("%1СfgSrvEquipmentID").arg(BUILD_REG_KEY), QString("SYSTEM_ID_CFG")).toString();

	m_cfgSrvIP = HostAddressPort(	s.value(QString("%1СfgSrvIP").arg(BUILD_REG_KEY), QString("127.0.0.1")).toString(),
														s.value(QString("%1СfgSrvPort").arg(BUILD_REG_KEY), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt());


	m_appDataSrvEquipmentID = s.value(QString("%1AppDataSrvEquipmentID").arg(BUILD_REG_KEY), QString("SYSTEM_ID_ADS")).toString();

	m_ualTesterIP = HostAddressPort(	s.value(QString("%1UalTesterIP").arg(BUILD_REG_KEY), QString("127.0.0.1")).toString(),
										s.value(QString("%1UalTesterPort").arg(BUILD_REG_KEY), PORT_TUNING_SERVICE_CLIENT_REQUEST).toInt());

	m_signalsStatePath  = s.value(QString("%1SignalsStatePath").arg(BUILD_REG_KEY), QString("SignalStates.csv")).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildOption::save()
{
	QSettings s;

	//
	//
	s.setValue(QString("%1СfgSrvEquipmentID").arg(BUILD_REG_KEY), m_cfgSrvEquipmentID);

	s.setValue(QString("%1СfgSrvIP").arg(BUILD_REG_KEY), m_cfgSrvIP.address().toString());
	s.setValue(QString("%1СfgSrvPort").arg(BUILD_REG_KEY), m_cfgSrvIP.port());

	s.setValue(QString("%1AppDataSrvEquipmentID").arg(BUILD_REG_KEY), m_appDataSrvEquipmentID);

	s.setValue(QString("%1UalTesterIP").arg(BUILD_REG_KEY), m_ualTesterIP.address().toString());
	s.setValue(QString("%1UalTesterPort").arg(BUILD_REG_KEY), m_ualTesterIP.port());

	s.setValue(QString("%1SignalsStatePath").arg(BUILD_REG_KEY), m_signalsStatePath);
}

// -------------------------------------------------------------------------------------------------------------------

BuildOption& BuildOption::operator=(const BuildOption& from)
{
	m_cfgSrvEquipmentID = from.m_cfgSrvEquipmentID;
	m_cfgSrvIP = from.m_cfgSrvIP;
	m_appDataSrvEquipmentID = from.m_appDataSrvEquipmentID;
	m_ualTesterIP = from.m_ualTesterIP;

	m_signalsStatePath = from.m_signalsStatePath;
	m_sourcesForRunList = from.m_sourcesForRunList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
