#pragma once

#include <QObject>
#include <QDebug>
#include "Builder.h"

class BuildTask : public QObject
{
	Q_OBJECT
public:
	BuildTask(QObject* parent = nullptr);

public slots:
	void start();
	void stop();

	bool isRunning() const;

private slots:
	void newLogItem(OutputLogItem logItem);

signals:
	void finished(int result);

	// Properties
	//
public:
	void setDatabaseAddress(QString value);
	void setDatabasePort(int value);
	void setDatabaseUserName(QString value);
	void setDatabasePassword(QString value);
	void setProjectName(QString value);
	void setProjectUserName(QString value);
	void setProjectUserPassword(QString value);
	void setBuildOutputPath(QString value);
	void setBuildType(QString value);
	void setBuildType(Builder::BuildType value);

	// Data
	//
private:
	Builder::Builder m_builder{nullptr, this};

	QString m_databaseAddress{"127.0.0.1"};
	int m_databasePort{5432};
	QString m_databaseUserName{"u7"};
	QString m_databasePassword{"Password"};
	QString m_projectName;
	QString m_projectUserName{"Administrator"};
	QString m_projectUserPassword{"Password"};
	QString m_buildOutputPath{"."};
	Builder::BuildType m_buildType = Builder::BuildType::Debug;
};


