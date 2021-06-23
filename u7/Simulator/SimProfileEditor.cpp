#include "SimProfileEditor.h"
#include "../../UtilsLib/Ui/UiTools.h"
#include "GlobalMessanger.h"


SimProfileEditor* SimProfileEditor::m_simProfileEditor = nullptr;

const QString SimProfileEditor::m_exampleText =
R"(// Syntaxis:
//
// [Profile1]: Define profile "Profile1"
//
// EQUIPMENTID.PropertyCaption = "TextValue";	// Set property value to text
// EQUIPMENTID.PropertyCaption = 10;			// Set property value to num
//

//
// Profile example to set communiaction of IDE Simulator and AppDataService and TuningService on localhost
//
[Local]

// Overwrites real AppDataService.AppDataReceivingIP address to loacalhost IP address
//
USER_APP_DATA_SERVICE_ID.AppDataReceivingIP = "127.0.0.1";

// Sets AppDataService.AppDataReceivingNetmask to prevent 'different Netmask error' of real LMs AppDataIP addresses and
// overwritten AppDataService.AppDataReceivingIP address
//
USER_APP_DATA_SERVICE_ID.AppDataReceivingNetmask = "0.0.0.0";

// Overwrites real TuningService.TuningDataIP address to loacalhost IP address
//
USER_TUNING_SERVICE_ID.TuningDataIP = "127.0.0.1";

// Sets TuningService.TuningDataNetmask property to prevent 'different Netmask error' of real LMs TuningDataIP and
// overwritten TuningService.TuningDataIP
//
USER_TUNING_SERVICE_ID.TuningDataNetmask = "0.0.0.0";

// Sets Simulator tuning interface IP address to localhost.
// Usually this is not required since 127.0.0.1 is a default value of this property
//
USER_TUNING_SERVICE_ID.TuningSimIP = "127.0.0.1";

// To run communication of IDE Simulator and AppDataService and TuningService on localhost next steps should be done:
//
// 1) In cmd line of CfgService specify parameters: -mode=simulation -profile=Local.
// 2) Run CfgService, AppDataService, TuningService, TuningClient, Monitor.
// 3) Load last successful build in Simulator.
// 4) Press 'Allow LogicModules' Application data transmittion to AppDataSrv' button on Simulator toolbar.
// 5) Press 'Run simulation for complete project' button on Simulator toolbar.
//	  After that states of AppData signals will be displayed in Monitor.
// 6) Enable Tuning mode by pressing 'Arming key' and 'Tuning key' buttons on control tab of required LM
// 7) Activate Control of required LM in TuningClient. After that TuningClient should indicate
//	  successfull communication with selected LM and display valid values of tunable signals.
)";

void SimProfileEditor::run(DbController* dbController, QWidget* parent)
{
	if (m_simProfileEditor == nullptr)
	{
		m_simProfileEditor = new SimProfileEditor(dbController, parent);
		m_simProfileEditor->show();
	}
	else
	{
		m_simProfileEditor->activateWindow();
		UiTools::adjustDialogPlacement(m_simProfileEditor);
	}
}

SimProfileEditor::SimProfileEditor(DbController* dbController, QWidget* parent)	:
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
	m_db(dbController)
{
	if (m_db == nullptr)
	{
		Q_ASSERT(m_db);
		return;
	}

	setWindowTitle(tr("Simulator Profiles"));

	setAttribute(Qt::WA_DeleteOnClose);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SimProfileEditor::projectClosed);

	// Create Text Editor
	//

	m_textEdit = new QsciScintilla(this);

#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
#else
		QFont f = QFont("Courier");
#endif

	// Set up lexer
	//
	m_lexer.setDefaultFont(f);
	m_textEdit->setLexer(&m_lexer);

	m_textEdit->setFont(f);
	m_textEdit->setTabWidth(4);
	m_textEdit->setAutoIndent(true);

	connect(m_textEdit, &QsciScintilla::textChanged, this, &SimProfileEditor::textChanged);

	// Buttons
	//
	QPushButton* buttonCheck = new QPushButton(tr("Check"));
	QPushButton* buttonExample = new QPushButton(tr("Example"));
	QPushButton* buttonSave = new QPushButton(tr("Save"));
	QPushButton* buttonOK = new QPushButton(tr("OK"));
	QPushButton* buttonCancel = new QPushButton(tr("Cancel"));

	connect(buttonCheck, &QPushButton::clicked, this, &SimProfileEditor::checkProfiles);
	connect(buttonExample, &QPushButton::clicked, this, &SimProfileEditor::example);
	connect(buttonSave, &QPushButton::clicked, this, &SimProfileEditor::saveChanges);
	connect(buttonOK, &QPushButton::clicked, this, &SimProfileEditor::accept);
	connect(buttonCancel, &QPushButton::clicked, this, &SimProfileEditor::reject);

	// Layouts
	//
	QHBoxLayout* buttonLayout = new QHBoxLayout();

	buttonLayout->addWidget(buttonCheck);
	buttonLayout->addWidget(buttonExample);
	buttonLayout->addStretch();
	buttonLayout->addWidget(buttonSave);
	buttonLayout->addWidget(buttonOK);
	buttonLayout->addWidget(buttonCancel);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(m_textEdit);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	// Load file from database
	//
	std::vector<DbFileInfo> fileList;

	bool ok = m_db->getFileList(&fileList, DbDir::EtcDir, Db::File::SimProfilesFileName, true, this);

	if (ok == true && fileList.size() == 1)
	{
		std::shared_ptr<DbFile> file;

		if (m_db->getLatestVersion(fileList[0], &file, this) == true)
		{
			QString text(file->data());

			m_textEdit->blockSignals(true);
			m_textEdit->setText(text);
			m_textEdit->blockSignals(false);

			m_startText = text;
		}
	}

	// Resize dialog
	//
	QSettings settings;

	QPoint pos = settings.value("SimProfileEditor/pos", QPoint(-1, -1)).toPoint();
	QByteArray geometry = settings.value("SimProfileEditor/geometry").toByteArray();

	if (pos.x() != -1 && pos.y() != -1)
	{
		move(pos);
		restoreGeometry(geometry);
	}
	else
	{
		QRect screen = QDesktopWidget().availableGeometry(parentWidget());

		resize(static_cast<int>(screen.width() * 0.35),
			   static_cast<int>(screen.height() * 0.35));
		move(screen.center() - rect().center());
	}

	return;
}

SimProfileEditor::~SimProfileEditor()
{
	QSettings settings;
	settings.setValue("SimProfileEditor/pos", pos());
	settings.setValue("SimProfileEditor/geometry", saveGeometry());

	m_simProfileEditor = nullptr;
}

void SimProfileEditor::closeEvent(QCloseEvent* e)
{
	if (askForSaveChanged() == true)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}

	return;
}

bool SimProfileEditor::askForSaveChanged()
{
	if (m_textEdit->text() == m_startText)
	{
		m_modified = false;
		return true;
	}

	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, qAppName(), "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (result == QMessageBox::Yes)
	{
		return saveChanges();
	}

	if (result == QMessageBox::No)
	{
		return true;
	}

	return false;
}

bool SimProfileEditor::saveChanges()
{
	if (m_modified == false)
	{
		return true;
	}

	// Check correctness
	//
	Sim::Profiles profiles;
	QString errorMsg;

	if (profiles.parse(m_textEdit->text(), &errorMsg) == false)
	{
		int mbResult = QMessageBox::warning(this,
											qAppName(),
											tr("There are errors in the document. Are you sure you want to save it anyway?"),
											QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

		if (mbResult == QMessageBox::No)
		{
			return false;
		}
	}

	// Save
	//
	bool ok = false;

	QString comment = QInputDialog::getText(this, qAppName(),
											tr("Please enter comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return false;
	}

	if (comment.isEmpty())
	{
		QMessageBox::warning(this, qAppName(), tr("No comment supplied! Please provide a comment."));
		return false;
	}

	// save to db
	//
	std::vector<DbFileInfo> fileList;

	ok = m_db->getFileList(&fileList, DbDir::EtcDir, Db::File::SimProfilesFileName, true, this);
	if (ok == false || fileList.size() != 1)
	{
		// create a file, if it does not exists
		//
		std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
		pf->setFileName(Db::File::SimProfilesFileName);

		if (m_db->addFile(pf, DbDir::EtcDir, this) == false)
		{
			return false;
		}

		ok = m_db->getFileList(&fileList, DbDir::EtcDir, Db::File::SimProfilesFileName, true, this);
		if (ok == false || fileList.size() != 1)
		{
			return false;
		}
	}

	std::shared_ptr<DbFile> file = nullptr;

	ok = m_db->getLatestVersion(fileList[0], &file, this);
	if (ok == false || file == nullptr)
	{
		return false;
	}

	if (file->state() != E::VcsState::CheckedOut)
	{
		if (m_db->checkOut(fileList[0], this) == false)
		{
			return false;
		}
	}

	QByteArray data = m_textEdit->text().toUtf8();

	file->swapData(data);

	if (m_db->setWorkcopy(file, this) == false)
	{
		return false;
	}

	if (m_db->checkIn(fileList[0], comment, this) == false)
	{
		return false;
	}

	m_modified = false;

	m_startText = m_textEdit->text();

	return true;
}

void SimProfileEditor::checkProfiles()
{
	Sim::Profiles profiles;

	QString errorMsg;

	if (profiles.parse(m_textEdit->text(), &errorMsg) == true)
	{
		//QString dump = profiles.dump();

		QMessageBox::information(this, qAppName(), tr("Profiles loaded successfully.")/* + dump*/);
	}
	else
	{
		QMessageBox::critical(this, qAppName(), tr("Profiles loading error!\n\n") + errorMsg);
	}

	return;
}

void SimProfileEditor::example()
{
	if (m_textEdit->text().isEmpty() == false)
	{
		m_textEdit->setText(m_textEdit->text() + "\n");
	}

	m_textEdit->setText(m_textEdit->text() + m_exampleText);
	//textChanged();

	return;
}

void SimProfileEditor::textChanged()
{
	m_modified = true;
}

void SimProfileEditor::accept()
{
	if (m_textEdit->text() == m_startText)
	{
		m_modified = false;
	}

	if (m_modified == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}

	QDialog::accept();

	return;
}

void SimProfileEditor::reject()
{
	if (askForSaveChanged() == true)
	{
		QDialog::reject();
	}

	return;
}

void SimProfileEditor::projectClosed()
{
	m_modified = false;

	QDialog::reject();

	return;
}
