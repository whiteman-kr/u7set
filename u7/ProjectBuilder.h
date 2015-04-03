#ifndef PROJECTBUILDER_H
#define PROJECTBUILDER_H

#include "../include/Signal.h"

class QThread;
class OutputLog;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class McFirmwareOld;
}

namespace VFrame30
{
	class LogicScheme;
	class SchemeLayer;
}

class BuildWorkerThread : public QThread
{
	Q_OBJECT

private:
	virtual void run() override;

	// Get Equipment from the prokect database
	//
	bool getEquipment(DbController* db, Hardware::DeviceObject* parent);

	// Expand Devices StrId
	//
	bool expandDeviceStrId(Hardware::DeviceObject* device);

	// Generate Modules Configurations Firmwares
	//
	bool generateModulesConfigurations(DbController* db, Hardware::DeviceObject* root);

	// Compile Application Logic
	//
	bool applicationLogic(DbController* db);
	bool loadApplicationLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicScheme>>* out);

	bool compileApplicationLogicScheme(VFrame30::LogicScheme* logicScheme);
	bool compileApplicationLogicLayer(VFrame30::LogicScheme* logicScheme, VFrame30::SchemeLayer* layer);

	// What's the next compilation task?
	//

signals:
	void resultReady(const QString &s);

	// Properties
	//
public:
	QString projectName() const;
	void setProjectName(const QString& value);

	QString serverIpAddress() const;
	void setServerIpAddress(const QString& value);

	int serverPort() const;
	void setServerPort(int value);

	QString serverUsername() const;
	void setServerUsername(const QString& value);

	QString serverPassword() const;
	void setServerPassword(const QString& value);

	void setOutputLog(OutputLog* value);

	QString projectUserName() const;
	void setProjectUserName(const QString& value);

	QString projectUserPassword() const;
	void setProjectUserPassword(const QString& value);

	bool debug() const;
	void setDebug(bool value);

	bool release() const;

	// Data
	//
private:
	mutable QMutex m_mutex;

	QString m_projectName;

	QString m_serverIpAddress;
	int m_serverPort = 0;
	QString m_serverUsername;
	QString m_serverPassword;

	QString m_projectUserName;
	QString m_projectUserPassword;

	bool m_debug = false;				// if true then don't get workcopy of checked out files, use unly checked in copy

	OutputLog* m_log;					// Probably it's better to make it as shared_ptr
};

class SignalSetObject : public QObject
{
	Q_OBJECT

private:
	SignalSet m_signalSet;

public:
	void loadSignalsFromDb(DbController* db);
	Q_INVOKABLE QObject* getSignalByDeviceStrID(const QString& deviceStrID);
};

class ProjectBuilder : public QObject
{
	Q_OBJECT

public:
	ProjectBuilder(OutputLog* log);
	virtual ~ProjectBuilder();

	// Public methods
	//
public:
	bool start(QString projectName,
			   QString serverIp,
			   int serverPort,
			   QString serverUserName,
			   QString serverPassword,
			   QString projectUserName,
			   QString projectUserPassword,
			   bool debug);

	void stop();

	bool isRunning() const;

	// Signlas
	//
signals:
	void buildStarted();
	void buildFinished();

	// Slots
	//
protected slots:
	void handleResults(QString result);

	// Properties
	//
private:
	bool debug() const;

	// Data
	//
private:
	BuildWorkerThread* m_thread = nullptr;
	OutputLog* m_log = nullptr;
};


#endif // PROJECTBUILDER_H
