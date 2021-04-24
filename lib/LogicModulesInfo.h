#pragma once

#include "../UtilsLib/Address16.h"
#include "../CommonLib/Types.h"
#include "../UtilsLib/DomXmlHelper.h"
#include "LanControllerInfo.h"

#ifdef IS_BUILDER

#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "ModuleLogicCompiler.h"
#include "DeviceHelper.h"
#include "LmDescription.h"
#include "LanControllerInfoHelper.h"
#include "Context.h"

#endif

class LogicModuleInfo
{
public:
	QString equipmentID;
	QString caption;

	QString subsystemID;
	int subsystemKey = 0;
	int lmNumber = 0;
	QString subsystemChannel;

	quint64 lmUniqueID = 0;

	QString moduleFamily;
	int moduleFamilyID = 0;
	int moduleVersion = 0;

	QString presetName;
	QString lmDescriptionFile;

	bool appDataEnable = false;
	int appDataSizeBytes = 0;
	quint32 appDataUID = 0;

	bool diagDataEnable = false;
	int diagDataSizeBytes = 0;
	quint32 diagDataUID = 0;

	std::vector<LanControllerInfo> lanControllers;

	int moduleType() const { return (moduleFamilyID & 0xFF00) | (moduleVersion & 0x00FF); }
};

class LogicModulesInfo
{
public:
	std::vector<::LogicModuleInfo> logicModulesInfo;

	bool load(const QString& fileName, QString* errMsg);
	bool load(const QByteArray& xmlData, QString* errMsg);

	std::optional<::LogicModuleInfo> get(QString equipmentId) const;

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
	LogicModulesInfoWriter(const Builder::Context& context);

	bool fill();
	void save(QByteArray* xmlFileData) const;

private:
	Builder::IssueLogger* log() const { return m_context.m_log; }

	bool fill(const Hardware::DeviceModule* lmModule, LogicModuleInfo* lmInfo);

	bool save(const LogicModuleInfo& lmInfo, XmlWriteHelper& xml) const;

	bool save(const LanControllerInfo& lci, XmlWriteHelper& xml) const;

private:
	const Builder::Context& m_context;
};

#endif

//void testLogicModulesInfoLoad();


