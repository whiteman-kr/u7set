#ifndef GLOBALMESSANGER_H
#define GLOBALMESSANGER_H

#include <QObject>
#include "../include/DbStruct.h"

class GlobalMessanger : public QObject
{
	Q_OBJECT

private:
	GlobalMessanger();

public:
	static GlobalMessanger* instance();
	static void free();

	void fireProjectOpened(DbProject project);
	void fireProjectClosed();

	void fireBuildStarted();
	void fireBuildFinished();

signals:
	void projectOpened(DbProject project);
	void projectClosed();

	void buildStarted();
	void buildFinished();

public slots:

private:
	static GlobalMessanger* m_instance;
};

#endif // GLOBALMESSANGER_H
