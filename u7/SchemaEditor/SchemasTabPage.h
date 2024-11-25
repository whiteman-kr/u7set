#pragma once

#include <UiLib/ClickableLabel.h>
#include "MainTabPage.h"

class AppSignalSetProvider;
class SchemaControlTabPage;

//
//
// SchemasTabPageEx - the main tab page added to IDE
//
//
class SchemasTabPage : public MainTabPage
{
	Q_OBJECT

public:
	explicit SchemasTabPage(DbController* dbc,
							AppSignalSetProvider* signalSetProvider,
							UiLib::ClickableLabel* statusBarLayerLabel,
							UiLib::ClickableLabel* statusBarZoomLabel,
							QWidget* parent);
	virtual ~SchemasTabPage();

public:
	[[nodiscard]] bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();
	bool resetModified();

	void saveSession() const;			// Save schema list (on project close)
	void restoreSession();

	void refreshControlTabPage();

protected:
	void showEvent(QShowEvent* event) override;
	void timerEvent(QTimerEvent* event) override;

public slots:
	void projectOpened();
	void projectClosed();

protected slots:
	void tabCloseRequested(int index);
	void currentTabChanged(int index);

	void statusBarLayerClicked();
	void statusBarZoomClicked();

	// Data
	//
protected:
	UiLib::ClickableLabel* m_statusBarLayerLabel = nullptr;
	UiLib::ClickableLabel* m_statusBarZoomLabel = nullptr;

	QTabWidget* m_tabWidget = nullptr;
	SchemaControlTabPage* m_controlTabPage = nullptr;

	QString m_fileExtension;
	QString m_templFileExtension;

	QAction* m_showControlTabAccelerator = nullptr;

	// Postpone restore session to showEvent()
	//
	bool m_requireRestoreSession = false;
};

