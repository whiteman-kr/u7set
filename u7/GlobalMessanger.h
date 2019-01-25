#ifndef GLOBALMESSANGER_H
#define GLOBALMESSANGER_H

#include "../lib/OutputLog.h"
#include "../lib/DbStruct.h"
#include "../Builder/IssueLogger.h"
#include "../Builder/RunOrder.h"


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
	GlobalMessanger(const GlobalMessanger&) = delete;
	GlobalMessanger& operator=(const GlobalMessanger&) = delete;

public:
	static GlobalMessanger& instance();

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
	void fireSearchSchemaForLm(QString equipmentId);

	// Build
	//
	void fireBuildStarted();
	void fireBuildFinished(int errorCount);

	void clearBuildSchemaIssues();

	OutputMessageLevel issueForSchemaItem(const QUuid& itemId) const;
	Builder::BuildIssues::Counter issueForSchema(const QString& schemaId) const;

	void clearSchemaItemRunOrder();
	std::pair<int, int> schemaItemRunOrder(const QString& equipmentId, const QUuid& itemId) const;

	// Select tab
	//
	void fireChangeCurrentTab(QWidget* tab);

	// CompareObject
	//
	void fireCompareObject(DbChangesetObject object, CompareData compareData);

public slots:
	void runOrderReady(Builder::RunOrder runOrder);

	// --
	//
signals:
	void projectOpened(DbProject project);
	void projectClosed();

	void buildStarted();
	void buildFinished(int errorCount);

	void showDeviceApplicationSignals(QStringList deviceStrIds, bool refreshSignalList);

	void addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile);
	void searchSchemaForLm(QString equipmentId);

	void changeCurrentTab(QWidget* tab);

	void compareObject(DbChangesetObject object, CompareData compareData);

public:
	Builder::BuildIssues& buildIssues();
	const Builder::BuildIssues& buildIssues() const;

private:
	static GlobalMessanger* m_instance;

	// --
	//
	Builder::BuildIssues m_buildIssues;

	mutable QMutex m_buildResultMutex;
	Builder::RunOrder m_runOrder;

	// -- end of data for m_buildResultMutex
	//
};

#endif // GLOBALMESSANGER_H
