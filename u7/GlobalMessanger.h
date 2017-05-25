#ifndef GLOBALMESSANGER_H
#define GLOBALMESSANGER_H

#include <QObject>
#include <QMutex>
#include "../lib/OutputLog.h"
#include "../lib/DbStruct.h"
#include "Builder/IssueLogger.h"


struct RunOrderDebugInfo
{
	QString equipmentId;										// LM's ID
	std::map<QUuid, std::pair<int, int>> schemaItemsRunOrder;	// Key is item's guid, value is run order index
};


enum class CompareVersionType
{
	Changeset,
	Date,
	LatestVersion
};


struct CompareData
{
	CompareVersionType sourceVersionType = CompareVersionType::Changeset;
	int sourceChangeset = 0;
	QDateTime sourceDate = QDateTime::currentDateTime();

	CompareVersionType targetVersionType = CompareVersionType::LatestVersion;
	int targetChangeset = 0;
	QDateTime targetDate = QDateTime::currentDateTime();
};

Q_DECLARE_METATYPE(CompareData)


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

	// Schema Editor
	//
	void fireAddLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile);

	// Build
	//
	void fireBuildStarted();
	void fireBuildFinished();

	void clearBuildSchemaIssues();
	void swapSchemaIssues(Builder::BuildIssues* buildIssues);

	OutputMessageLevel issueForSchemaItem(const QUuid& itemId) const;
	Builder::BuildIssues::Counter issueForSchema(const QString& schemeId) const;

	void clearSchemaItemRunOrder();
	void setRunOrder(const QString& equipmentId, std::map<QUuid, std::pair<int, int> >& data);
	std::pair<int, int> schemaItemRunOrder(const QString& equipmentId, const QUuid& itemId) const;

	// Select tab
	//
	void fireChangeCurrentTab(QWidget* tab);

	// CompareObject
	//
	void fireCompareObject(DbChangesetObject object, CompareData compareData);

	// --
	//

signals:
	void projectOpened(DbProject project);
	void projectClosed();

	void buildStarted();
	void buildFinished();

	void showDeviceApplicationSignals(QStringList deviceStrIds, bool refreshSignalList);

	void addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile);

	void changeCurrentTab(QWidget* tab);

	void compareObject(DbChangesetObject object, CompareData compareData);

public slots:

private:
	static GlobalMessanger* m_instance;

	// --
	//
	mutable QMutex m_buildResultMutex;

	Builder::BuildIssues m_buildIssues;
	std::list<RunOrderDebugInfo> m_runOrder;

	// -- end of data for m_buildResultMutex
	//
};

#endif // GLOBALMESSANGER_H
