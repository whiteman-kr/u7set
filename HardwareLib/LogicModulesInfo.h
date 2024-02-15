#pragma once

#include "LanControllerInfo.h"

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
	quint32 rupAppDataUID = 0;

//	quint64 tunDataUID = 0;

	bool diagDataEnable = false;
	int diagDataSizeBytes = 0;
	quint32 rupDiagDataUID = 0;

	LanControllersInfo lanControllers;

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
};

//void testLogicModulesInfoLoad();


