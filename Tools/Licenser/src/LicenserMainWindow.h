#pragma once

#include "RpctLicenseObject.h"
#include <QLabel>
#include <QMainWindow>

class PropertyEditorWithUpdate;

class LicenserMainWindow : public QMainWindow
{
public:
	explicit LicenserMainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private slots:
	void updateState();
	void updatePropertyEditor(const QString& property = QString{});

	void newLicense();
	void loadLicenses();
	void selectPrivateKey();
	bool saveLicense();
	bool saveAsLicense();
	bool save(QString fileName);
	void closeFile();
	bool exitApplication();
	void aboutApplication();

	void propertyChanged();

private:
	// UI
	//
	QLabel* m_fileLabel = nullptr;
	QLabel* m_privateKeyLabel = nullptr;
	PropertyEditorWithUpdate* m_propertyEditor = nullptr;

	QAction* newAction = nullptr;
	QAction* loadAction = nullptr;
	QAction* selectPrivateKeyAction = nullptr;
	QAction* saveAction = nullptr;
	QAction* saveAsAction = nullptr;
	// --
	QAction* closeFileAction = nullptr;
	// --
	QAction* exitAction = nullptr;

	QAction* aboutAction = nullptr;

	// State
	//
	QString m_openFileName;

	QString m_privateKeyFileName;
	QString m_privateKeyPassword;

	bool m_modified = false;
	std::shared_ptr<RpctLicenseObject> m_license;
};