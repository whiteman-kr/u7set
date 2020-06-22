#pragma once

#include "Address16.h"
#include "Types.h"
#include "DomXmlHelper.h"
#include "LanControllerInfo.h"

#ifdef IS_BUILDER

#include "XmlHelper.h"
#include "WUtils.h"
#include "ModuleLogicCompiler.h"
#include "DeviceHelper.h"
#include "LmDescription.h"
#include "LanControllerInfoHelper.h"

#endif

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
	std::vector<::LogicModuleInfo> logicModulesInfo;

	bool load(const QString& fileName, QString* errMsg);
	bool load(const QByteArray& xmlData, QString* errMsg);

private:
	bool load(::LogicModuleInfo* lmi, const QDomNode& lmNode, QString* errMsg);
	bool load(LanControllerInfo* lci, const QDomNode& lanControllerNode, QString* errMsg);

protected:
	static const QString ELEM_LOGIC_MODULES;
	static const QString ELEM_LOGIC_MODULE;

	static const QString ELEM_LAN_CONTROLLERS;
	static const QString ELEM_LAN_CONTROLLER;

	static const QString ELEM_TUNING_PARAMS;
	static const QString ELEM_APP_DATA_PARAMS;
	static const QString ELEM_DIAG_DATA_PARAMS;

	static const QString ATTR_TUNING_PROVIDED;
	static const QString ATTR_APP_DATA_PROVIDED;
	static const QString ATTR_DIAG_DATA_PROVIDED;
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

//void testLogicModulesInfoLoad();


