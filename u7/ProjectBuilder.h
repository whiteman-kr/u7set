#ifndef PROJECTBUILDER_H
#define PROJECTBUILDER_H

class QThread;
class OutputLog;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class McFirmware;
}

class BuildWorkerThread : public QThread
{
	Q_OBJECT

private:
	virtual void run() override;

	// Get Equipment from the prokect database
	//
	bool getEquipment(DbController* db, Hardware::DeviceObject* parent);

	// Generate Modules Configurations Firmwares
	//
	bool generateModulesConfigurations(DbController* db, const Hardware::DeviceObject *root);
	bool generateModulesConfigurations(
			DbController* db,
			const Hardware::DeviceObject* parent,
			std::map<QString, std::shared_ptr<Hardware::McFirmware>>* firmwares);

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

	bool onlyCheckedIn() const;
	void setOnlyCheckedIn(bool value);

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

	bool m_onlyCheckedIn = true;		// Don't get workcopy of checked out files, use unly checked in copy

	OutputLog* m_log;					// Probably it's better to make it as shared_ptr
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
			   bool onlyCheckedIn);

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

	// Data
	//
private:
	BuildWorkerThread* m_thread = nullptr;
	OutputLog* m_log = nullptr;
};

#endif // PROJECTBUILDER_H
