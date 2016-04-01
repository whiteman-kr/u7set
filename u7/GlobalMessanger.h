#ifndef GLOBALMESSANGER_H
#define GLOBALMESSANGER_H

#include <QObject>
#include <QMutex>
#include "../include/OutputLog.h"
#include "../include/DbStruct.h"

class GlobalMessanger : public QObject
{
	Q_OBJECT

private:
	GlobalMessanger();

public:
	static GlobalMessanger* instance();
	static void free();

	// Project management
	//
	void fireProjectOpened(DbProject project);
	void fireProjectClosed();

	// Equipment Editor
	//
	void fireShowDeviceApplicationSignals(QStringList deviceStrIds);

	// Build
	//
	void fireBuildStarted();
	void fireBuildFinished();

	void clearBuildSchemaIssues();
	void swapSchemaIssues(std::map<QUuid, OutputMessageLevel>& data);
	OutputMessageLevel issueForSchemaItem(const QUuid itemId) const;

	// --
	//

signals:
	void projectOpened(DbProject project);
	void projectClosed();

	void buildStarted();
	void buildFinished();

	void showDeviceApplicationSignals(QStringList deviceStrIds);

public slots:

private:
	static GlobalMessanger* m_instance;

	// --
	//
	mutable QMutex m_buildResultMutex;
	std::map<QUuid, OutputMessageLevel> m_buildSchemaIssues;

	// -- end of data for m_buildResultMutex
	//
};

#endif // GLOBALMESSANGER_H
