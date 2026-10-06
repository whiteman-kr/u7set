#pragma once

#include <QObject>
#include <QThread>

class RvduSimThreadWorker : public QObject
{
	Q_OBJECT
public:
	explicit RvduSimThreadWorker(unsigned int* pHdmiBuffer,
								 const QString& buildFolder,
								 int subsystemKey,
								 int width,
								 int height);

	bool isRunning() const;
	bool isError() const;

signals:
	void logError(const QString& msg);
	void logWarning(const QString& msg);
	void logMessage(const QString& msg);
	
	void rnInitComplete();

	void updateProjectInfo(const QString& projectInfo);

public slots:
	void threadProc();

private:
	QString m_buildFolder;
	int m_subsystemKey = 0;
	
	unsigned int* m_pHdmiBuffer = nullptr;

	int m_width = 0;
	int m_height = 0;

	bool m_rnInitOk = false;
	bool m_rnInitError = false;
};

class RvduSimThreadController : public QObject
{
	Q_OBJECT

public:
	RvduSimThreadController(QObject* parent,
							unsigned int* pHdmiBuffer,
							const QString& buildFolder,
							int subsystemKey,
							int width,
							int height);
	~RvduSimThreadController();

	bool isRunning() const;
	bool isError() const;

signals:
	void logError(const QString& msg);
	void logWarning(const QString& msg);
	void logMessage(const QString& msg);
	
	void rnInitComplete();

	void updateProjectInfo(const QString& projectInfo);

private:
	RvduSimThreadWorker* m_worker = nullptr;
	QThread m_workerThread;

};

