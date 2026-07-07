#pragma once

#include "../../UtilsLib/LogFile.h"
#include "../../OnlineLib/TcpClientStatistics.h"
#include "ComparatorsStorage.h"

#include "TO5Settings.h"
#include "TO5Runner.h"


#include <QMainWindow>
#include <QTableWidget>

class QSplitter;
class QToolBar;
class QLabel;

enum class ReportType
{
	BySchemaID,
	ByCaseID,
};

class NumericDelegate;

class PluginTO5MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	PluginTO5MainWindow(QWidget* parent = nullptr);
	~PluginTO5MainWindow();

private slots:
	void showAppLog();
	void showAboutQt();
	void showAbout();
	void onSettings();

	void onNewComparators();
	void onOpenComparators();
	void onSaveComparators();
	void onSaveAsComparators();
	void onImportComparators();
	void onItemCriteriaChanged(QTableWidgetItem* item);
	void onReport(ReportType reportType);

	void editMaskReturnPressed();

private:
	void createMenu();
	void createFilterListWidget();
	void createTableWidget();

	void createStatusBar();
	void updateStatusBar();

	void createUi();

	void showHeaderContextMenu(const QPoint& pos);

	bool loadComparators(const QString& fileName);

	void fillFilters(QList<QString> filtersId = QList<QString>(), bool filterMatch = false);

	void updateTable(const QList<QString>& filteredIds, FilterType sortType);

	void showSoftwareConnection(const QString& caption,
								const QString& nameFilter,
								const std::vector<TcpClientStatistics::Statistics>& connectionStatistics,
								QLabel* label);

protected:
	bool eventFilter(QObject* object, QEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	void timerEvent(QTimerEvent* event) override;

private:
	Log::LogFile m_logFile;

	ComparatorsStorage m_comparatorsStorage;
	TO5Runner m_runner;


	// Ui
	//
	QSplitter* m_splitter = nullptr;

	QWidget* m_mainWidget = nullptr;

	QListWidget* m_filtersListWidget = nullptr;
	QTableWidget* m_tableWidget = nullptr;
	NumericDelegate* m_numericDelegate = nullptr;


	QToolBar* m_toolBar = nullptr;

	QComboBox* m_comboBox = nullptr;

	QLineEdit* m_editMask = nullptr;
	QCompleter* m_maskCompleter = nullptr;

	QLabel* m_statusBarProjectInfo = nullptr;
	QLabel* m_statusBarConfigConnection = nullptr;
	QLabel* m_statusBarAppDataConnection = nullptr;
	QLabel* m_statusBarLogAlerts = nullptr;


	// Parameters
	//
	QStringList m_tableHeaders;
	QString m_comparatorsCSV;

	int m_criteriaColumn = -1;
	
	//QString m_listType;
	FilterType m_filterType = FilterType::BySchemaID;

	bool m_hasUnsavedChanges = false;
	QString m_maskHelp;

	int m_mainWindowTimerId_250ms = -1;

};
