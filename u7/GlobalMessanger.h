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

	// Build
	//
	void fireBuildStarted();
	void fireBuildFinished();

	void clearBuildSchemeIssues();
	void swapSchemeIssues(std::map<QUuid, OutputMessageLevel>& data);
	OutputMessageLevel issueForSchemeItem(const QUuid itemId) const;

	// --
	//

signals:
	void projectOpened(DbProject project);
	void projectClosed();

	void buildStarted();
	void buildFinished();

public slots:

private:
	static GlobalMessanger* m_instance;

	// --
	//
	mutable QMutex m_buildResultMutex;
	std::map<QUuid, OutputMessageLevel> m_buildSchemeIssues;

	// -- end of data for m_buildResultMutex
	//
};

#endif // GLOBALMESSANGER_H
