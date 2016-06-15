#ifndef GLOBALMESSANGER_H
#define GLOBALMESSANGER_H

#include <QObject>
#include <QMutex>
#include "../lib/OutputLog.h"
#include "../lib/DbStruct.h"

struct RunOrderDebugInfo
{
	QString equipmentId;							// LM's ID
	std::map<QUuid, int> schemaItemsRunOrder;		// Key is item's guid, value is run order index
};

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
	void fireShowDeviceApplicationSignals(QStringList deviceStrIds, bool refreshSignalList);

	// Build
	//
	void fireBuildStarted();
	void fireBuildFinished();

	void clearBuildSchemaIssues();
	void swapSchemaIssues(std::map<QUuid, OutputMessageLevel>& data);
	OutputMessageLevel issueForSchemaItem(QUuid itemId) const;

	void clearSchemaItemRunOrder();
	void setRunOrder(const QString& equipmentId, std::map<QUuid, int>& data);
	int schemaItemRunOrder(const QString& equipmentId, const QUuid& itemId) const;

	// Select tab
	//
	void fireChangeCurrentTab(QWidget* tab);

	// --
	//

signals:
	void projectOpened(DbProject project);
	void projectClosed();

	void buildStarted();
	void buildFinished();

	void showDeviceApplicationSignals(QStringList deviceStrIds, bool refreshSignalList);

	void changeCurrentTab(QWidget* tab);

public slots:

private:
	static GlobalMessanger* m_instance;

	// --
	//
	mutable QMutex m_buildResultMutex;

	std::map<QUuid, OutputMessageLevel> m_buildSchemaIssues;
	std::list<RunOrderDebugInfo> m_runOrder;

	//std::map<QUuid, int> m_schemaItemRunOrder;					// Debug information for dispalying on schema in debugMode

	// -- end of data for m_buildResultMutex
	//
};

#endif // GLOBALMESSANGER_H
