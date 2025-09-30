#include "LicenserMainWindow.h"

#include "PropertyEditorWithUpdate.h"

#include <CommonLib/ConstStrings.h>
#include <UiLib/DialogAbout.h>

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>


LicenserMainWindow::LicenserMainWindow(QWidget* parent, Qt::WindowFlags flags) :
	QMainWindow{parent, flags},
	m_license{std::make_shared<RpctLicenseObject>()}
{
	setWindowTitle(qAppName());
	setMinimumSize(800, 600);

	QWidget* centralWidget = new QWidget{this};
	QVBoxLayout* layout = new QVBoxLayout{centralWidget};

	m_fileLabel = new QLabel{centralWidget};
	m_fileLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

	m_privateKeyLabel = new QLabel{centralWidget};
	m_privateKeyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

	// Add property editor
	//
	m_propertyEditor = new PropertyEditorWithUpdate{centralWidget};
	m_propertyEditor->setObject(m_license);

	layout->addWidget(m_fileLabel);
	layout->addWidget(m_privateKeyLabel);
	layout->addWidget(m_propertyEditor);

	// Add main menu
	//
	QMenuBar* menuBar = new QMenuBar{this};
	QMenu* fileMenu = menuBar->addMenu(tr("File"));

	newAction = fileMenu->addAction(tr("New"));
	newAction->setShortcut(QKeySequence::New);

	loadAction = fileMenu->addAction(tr("Load..."));
	loadAction->setShortcut(QKeySequence::Open);

	selectPrivateKeyAction = fileMenu->addAction(tr("Select Private Key..."));

	saveAction = fileMenu->addAction(tr("Save"));
	saveAction->setShortcut(QKeySequence::Save);

	saveAsAction = fileMenu->addAction(tr("Save as..."));
	saveAsAction->setShortcut(QKeySequence::SaveAs);

	// --
	fileMenu->addSeparator();
	closeFileAction = fileMenu->addAction(tr("Close"));

	// --
	fileMenu->addSeparator();
	exitAction = fileMenu->addAction(tr("Exit"));
	exitAction->setShortcut(QKeySequence::Quit);

	QMenu* helpMenu = menuBar->addMenu(tr("?"));
	aboutAction = helpMenu->addAction(tr("About"));

	QMainWindow::setMenuBar(menuBar);

	connect(newAction, &QAction::triggered, this, &LicenserMainWindow::newLicense);
	connect(loadAction, &QAction::triggered, this, &LicenserMainWindow::loadLicenses);
	connect(selectPrivateKeyAction, &QAction::triggered, this, &LicenserMainWindow::selectPrivateKey);
	connect(saveAction, &QAction::triggered, this, &LicenserMainWindow::saveLicense);
	connect(saveAsAction, &QAction::triggered, this, &LicenserMainWindow::saveAsLicense);
	connect(closeFileAction, &QAction::triggered, this, &LicenserMainWindow::closeFile);
	connect(exitAction, &QAction::triggered, this, &LicenserMainWindow::exitApplication);
	connect(aboutAction, &QAction::triggered, this, &LicenserMainWindow::aboutApplication);

	connect(m_propertyEditor, &PropertyEditorWithUpdate::valueUpdated, this, &LicenserMainWindow::propertyChanged);

	// Add status bar
	//
	QStatusBar* statusBar = new QStatusBar{this};
	setStatusBar(statusBar);

	// Add central widget
	//
	setCentralWidget(centralWidget);

	updateState();

	return;
}

void LicenserMainWindow::closeEvent(QCloseEvent* event)
{
	if (exitApplication() == false)
	{
		event->ignore();
		return;
	}

	event->accept();
	return;
}

void LicenserMainWindow::updateState()
{
	setWindowTitle(qAppName() + (m_modified ? "*" : ""));

	QString fileName = m_openFileName.isEmpty() ? tr("Untitled.rls") : m_openFileName;
	m_fileLabel->setText(tr("License File: ") + fileName);
	m_fileLabel->setDisabled(m_license->isNull());

	m_privateKeyLabel->setText(tr("Private Key: %1").arg(m_privateKeyFileName.isEmpty() ? "Not Selected" : m_privateKeyFileName));
	m_privateKeyLabel->setDisabled(m_license->isNull());

	m_propertyEditor->setDisabled(m_license->isNull());


	newAction->setEnabled(true);
	loadAction->setEnabled(true);
	saveAction->setEnabled(m_license->isNull() == false);
	saveAsAction->setEnabled(m_license->isNull() == false);

	return;
}

void LicenserMainWindow::updatePropertyEditor(const QString& property)
{
	if (property.isEmpty() == false)
	{
		m_propertyEditor->updatePropertyValue(property);
	}
	else
	{
		m_propertyEditor->updatePropertiesValues();
	}
}

void LicenserMainWindow::newLicense()
{
	if (m_license->isNull() == false && m_modified == true)
	{
		// Ask user to save the current license
		//
		QMessageBox::StandardButton button = QMessageBox::question(this,
																   tr("Save license?"),
																   tr("Do you want to save the current license?"),
																   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (button == QMessageBox::Yes)
		{
			// Save the license
			//
			bool saved = saveLicense();
			if (saved == false)
			{
				// Save error, do not close app, let user fix the error.
				//
				return;
			}
		}
		else
		{
			if (button == QMessageBox::Cancel)
			{
				return;
			}
		}
	}

	m_license->clear();
	m_license->setUuid(QUuid::createUuid());
	m_license->setStartDate(QDate::currentDate());
	m_license->setEndDate(m_license->startDate().addYears(1));
	m_license->setWorkplaceCheckType(LicenseLib::WorkplaceCheckType::Relaxed);
	m_license->setIssueDate(QDate::currentDate());

	m_modified = true;
	m_openFileName.clear();

	updatePropertyEditor();
	updateState();

	return;
}

void LicenserMainWindow::loadLicenses()
{
	closeFile();

	// Ask for a file name.
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open License"), QString{}, tr("RPCT License (*.rls)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	// Open file and read data.
	//
	QFile file{fileName};
	if (file.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to open file: ") + file.fileName() + ", error: " + file.errorString());
		return;
	}

	QByteArray data = file.readAll();
	if (data.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to load license from file: ") + fileName);
		return;
	}

	QString errorMessage;
	auto license = LicenseLib::RpctLicense::fromRawData(data, QString{":/LicenseLib/public_key_inst1.pem"}, &errorMessage);

	if (errorMessage.isEmpty() == false || license.isNull() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to parse license from file: ") + fileName + ", ErrorMessage: " + errorMessage);
		return;
	}

	m_license->setLicense(license);

	m_openFileName = fileName;
	m_modified = false;

	updatePropertyEditor();
	updateState();
	return;
}

void LicenserMainWindow::selectPrivateKey()
{
	// Get private key file name
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select Private Key"), QString{}, tr("Private Key (*.pem)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	// Ask for a password
	//
	bool ok = false;
	QString password = QInputDialog::getText(this,
											 tr("Private Key Password"),
											 tr("Enter password for the private key:"),
											 QLineEdit::Password,
											 QString{},
											 &ok);

	if (ok == false)
	{
		return;
	}

	// Test the password
	//
	{
		RpctLicenseObject license;
		license.setUuid(QUuid::createUuid()); // it makes license not null

		QString errorMessage;
		QByteArray data = license.toRawData(fileName, password, &errorMessage);

		if (data.isEmpty() == true || errorMessage.isEmpty() == false)
		{
			QMessageBox::critical(this, tr("Error"), tr("Cannot load private key: ") + errorMessage);
			return;
		}
	}

	// Save password
	//
	m_privateKeyFileName = fileName;
	m_privateKeyLabel->setText(tr("Private Key: ") + fileName);

	m_privateKeyPassword = password;

	return;
}

bool LicenserMainWindow::saveLicense()
{
	if (m_privateKeyFileName.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("Private key file name is empty!"));
		return false;
	}

	if (m_openFileName.isEmpty() == true)
	{
		bool ok = saveAsLicense();
		return ok;
	}

	bool ok = save(m_openFileName);

	if (ok == true)
	{
		m_modified = false;
	}

	updateState();
	return ok;
}

bool LicenserMainWindow::saveAsLicense()
{
	// Default file name is a license uuid.
	//
	QUuid uuid = m_license->uuid();
	QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
	QString defaultFileName = uuid.isNull() ? tr("Untitled.rls") : m_license->organization() + "-" + now + "-" + uuid.toString() + ".rls";
	defaultFileName.remove(QChar::fromLatin1('{'));
	defaultFileName.remove(QChar::fromLatin1('}'));

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save License"), defaultFileName, tr("RPCT License (*.rls)"));
	if (fileName.isEmpty() == true)
	{
		return false;
	}

	if (m_privateKeyFileName.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("Private key file name is empty!"));
		return false;
	}

	bool ok = save(fileName);
	if (ok == true)
	{
		m_openFileName = fileName;
		m_modified = false;
	}

	updateState();
	return ok;
}

bool LicenserMainWindow::save(QString fileName)
{
	Q_ASSERT(m_license);
	Q_ASSERT(m_license->isNull() == false);

	if (m_license->isNull() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("License is empty!"));
		return false;
	}

	if (m_license->workplaceId().trimmed().isEmpty() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("WorkplaceID is empty!"));
		return false;
	}

	QString v = qApp->applicationName() + " v" + qApp->applicationVersion();
	m_license->setLicenserVersion(v);
	m_license->setIssueDate(QDate::currentDate());

	QString errorMessage;
	QByteArray data = m_license->toRawData(m_privateKeyFileName, m_privateKeyPassword, &errorMessage);

	if (errorMessage.isEmpty() == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to save license: %1").arg(errorMessage));
		return false;
	}

	// Save data to file
	//
	QFile file{fileName};
	if (file.open(QIODevice::WriteOnly) == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to open file: ") + file.fileName() + ", error: " + file.errorString());
		return false;
	}

	auto savedBytes = file.write(data);
	file.close();

	return savedBytes != 0;
}

void LicenserMainWindow::closeFile()
{
	if (m_license->isNull() == false && m_modified == true)
	{
		// Ask user to save the current license
		//
		QMessageBox::StandardButton button = QMessageBox::question(this,
																   tr("Save license?"),
																   tr("Do you want to save the current license?"),
																   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (button == QMessageBox::Yes)
		{
			// Save the license
			//
			bool saved = saveLicense();
			if (saved == false)
			{
				// Save error, do not close app, let use fix the error.
				//
				return;
			}
		}
		else
		{
			if (button == QMessageBox::Cancel)
			{
				return;
			}
		}
	}

	m_openFileName.clear();
	m_license->clear();
	m_modified = false;

	updatePropertyEditor();
	updateState();
	return;
}

bool LicenserMainWindow::exitApplication()
{
	// Check if license is modified
	//
	if (m_license->isNull() == false && m_modified == true)
	{
		// Ask user to save the current license
		//
		QMessageBox::StandardButton button = QMessageBox::question(this,
																   tr("Save license?"),
																   tr("Do you want to save the current license?"),
																   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (button == QMessageBox::Yes)
		{
			// Save the license
			//
			bool saved = saveLicense();
			if (saved == false)
			{
				// Save error, do not close app.
				//
				return false;
			}
		}
		else
		{
			if (button == QMessageBox::Cancel)
			{
				return false;
			}
		}
	}

	QApplication::quit();
	return true;
}

void LicenserMainWindow::aboutApplication()
{
	UiLib::DialogAbout::show(this,
							 tr("RPCT Licenser - application for managing licenses."),
							 ":/Logo/RadiyLogo.png",
							 Manufacturer::RADIY_ORGANIZATION);
}

void LicenserMainWindow::propertyChanged()
{
	m_modified = m_license->isNull() ? false : true;
	updateState();
}
