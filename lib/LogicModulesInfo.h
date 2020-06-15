#pragma once

#ifdef IS_BUILDER

#include "Connection.h"
#include "../Builder/OptoModule.h"
#include "XmlHelper.h"
#include "WUtils.h"
#include "ModuleLogicCompiler.h"
#include "../lib/DeviceHelper.h"
#include "LmDescription.h"
#include "LanControllerInfoHelper.h".h

#endif

#include "Address16.h"
#include "Types.h"
#include "DomXmlHelper.h"

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
	LogicModulesInfoWriter(const QVector<Builder::ModuleLogicCompiler *>& moduleCompilers,
						   const Hardware::EquipmentSet& equipmentSet);

	bool fill();
	void save(QByteArray* xmlFileData) const;

private:
	bool fill(LogicModuleInfo* lmInfo, Builder::ModuleLogicCompiler& mc);

	bool save(const LogicModuleInfo& lmInfo, XmlWriteHelper& xml) const;

	bool save(const LanControllerInfo& lci, XmlWriteHelper& xml) const;

private:
	const QVector<Builder::ModuleLogicCompiler *>& m_moduleCompilers;
	const Hardware::EquipmentSet& m_equipmentSet;
};

#endif


