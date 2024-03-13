#pragma once

#include "../UtilsLib/Address16.h"
#include "../UtilsLib/DomXmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

#include <HardwareLib/LanControllerInfo.h>
#include <HardwareLib/LmDescription.h>
#include <HardwareLib/LogicModulesInfo.h>

#include "Context.h"
#include "DeviceHelper.h"
#include "LanControllerInfoHelper.h"
#include "ModuleLogicCompiler.h"


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

private:
	const Builder::Context& m_context;
};


// void testLogicModulesInfoLoad();
