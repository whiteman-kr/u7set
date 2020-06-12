#pragma once

#ifdef IS_BUILDER

#include "Connection.h"
#include "../Builder/OptoModule.h"
#include "XmlHelper.h"
#include "WUtils.h"
#include "ModuleLogicCompiler.h"
#include "../lib/DeviceHelper.h"

#endif

#include "Address16.h"
#include "Types.h"
#include "DomXmlHelper.h"

class LanControllerInfo
{
	QString equipmentID;
	E::LanControllerType lanControllerType;

	//

	bool tuningEnabled = false;
	QHostAddress tuningIP;
	int tuningPort = 0;

	//

	bool appDataEnabled = false;
	QHostAddress appDataIP;
	int appDataPort = 0;

	QString appDataServiceEquipmentID;
	QHostAddress appDataServiceIP;
	int appDataServicePort = 0;

	int appDataSizeW = 0;
	int overrideAppDataWordCount = -1;

	//

	bool diagDataEnabled = false;
	QHostAddress diagDataIP;
	int diagDataPort = 0;

	QString diagDataServiceEquipmentID;
	QHostAddress diagDataServiceIP;
	int diagDataServicePort;

	int diagDataSizeW = 0;
	int overrideDiagDataWordCount = -1;
};

class LogicModuleInfo
{
public:
	QString equipmentID;
	QString caption;

	QString subsystemID;
	int lmNumber = 0;
	QString subsystemChannel;

	QString moduleFamily;
	int moduleVersion = 0;

	QString presetName;
	QString lmDescriptionFile;

	std::vector<LanControllerInfo> lanControllers;
};

class LogicModulesInfo
{
public:
	std::vector<LogicModuleInfo> logicModulesInfo;

	bool load(const QString& fileName, QString* errMsg);
	bool load(const QByteArray& xmlData, QString* errMsg);

private:
	bool load(LogicModuleInfo* lmi, const QDomNode& node, QString* errMsg);

protected:
	static const QString ELEM_LOGIC_MODULES;
	static const QString ELEM_LOGIC_MODULE;

	static const QString ELEM_LAN_CONTROLLERS;
	static const QString ELEM_LAN_CONTROLLER;

	static const QString ATTR_COUNT;
	static const QString ATTR_ID;

	static const QString ATTR_EQUIPMENT_ID;
	static const QString ATTR_CAPTION;

	static const QString ATTR_SUBSYSTEM_ID;
	static const QString ATTR_LM_NUMBER;
	static const QString ATTR_SUBSYSTEM_CHANNEL;

	static const QString ATTR_MODULE_FAMILY;
	static const QString ATTR_MODULE_VERSION;

	static const QString ATTR_PRESET_NAME;
	static const QString ATTR_LM_DESCRIPTION_FILE;

	static const QString ATTR_DATA_ID;
	static const QString ATTR_HEX_DATA_ID;
};

#ifdef IS_BUILDER

class LogicModulesInfoWriter : public LogicModulesInfo
{
public:
	bool fill(const QVector<Builder::ModuleLogicCompiler *> &moduleCompilers);
	void save(QByteArray* xmlFileData) const;

private:
	bool fill(LogicModuleInfo* lmInfo, Builder::ModuleLogicCompiler& mc);
	bool save(const LogicModuleInfo& lmInfo, XmlWriteHelper& xml) const;

	bool save(const LanControllerInfo& lci, XmlWriteHelper& xml) const;

/*
public:
	bool fill(const Hardware::ConnectionStorage& connectionsStorage, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(QByteArray* xmlFileData) const;

private:

	bool fill(ConnectionInfo* ci, Hardware::SharedConnection connection, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(const ConnectionInfo& ci, XmlWriteHelper& xml) const;

	bool fill(ConnectionPortInfo* cpi, Hardware::SharedConnection connection, int prtNo, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(const ConnectionPortInfo& cpi, XmlWriteHelper& xml) const;

	void save(const ConnectionTxRxSignal& cs, XmlWriteHelper& xml) const;*/
};

#endif


