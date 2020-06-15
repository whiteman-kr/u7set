#pragma once

#ifdef IS_BUILDER

#include "Connection.h"
#include "../Builder/OptoModule.h"
#include "XmlHelper.h"
#include "WUtils.h"
#include "ModuleLogicCompiler.h"

#endif

#include "Address16.h"
#include "Types.h"
#include "DomXmlHelper.h"

class LogicModuleInfo
{
public:
	QString equipmentID;

	//std::vector<ConnectionPortInfo> ports;
};

class LogicModulesInfo
{
public:
	std::vector<LogicModuleInfo> logicModulesInfo;

	bool load(const QString& fileName, QString* errMsg);
	bool load(const QByteArray& xmlData, QString* errMsg);

private:
	bool load(LogicModuleInfo* lmi, const QDomNode& node, QString* errMsg);

/*	bool load(ConnectionInfo* ci, const QDomNode& node, QString* errMsg);
	bool load(ConnectionPortInfo* cpi, const QDomElement& connectionElement, int prtNo, QString* errMsg);
	bool load(ConnectionTxRxSignal* cs, const QDomElement& txRxSignalElem, QString* errMsg);*/

protected:
	static const QString ELEM_LOGIC_MODULES;
	static const QString ELEM_LOGIC_MODULE;

	static const QString ATTR_COUNT;
	static const QString ATTR_ID;
	static const QString ATTR_EQUIPMENT_ID;
	static const QString ATTR_DATA_ID;
	static const QString ATTR_HEX_DATA_ID;
};

#ifdef IS_BUILDER

class LogicModulesInfoWriter : public LogicModulesInfo
{
public:
	bool fill(const QVector<Builder::ModuleLogicCompiler*>& moduleCompilers);
	void save(QByteArray* xmlFileData) const;

private:
	bool fill(LogicModuleInfo* lmInfo, const Builder::ModuleLogicCompiler& mc);

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


