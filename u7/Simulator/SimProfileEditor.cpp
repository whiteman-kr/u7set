#include "SimProfileEditor.h"
#include "SimScopedLog.h"
#include "../../lib/Ui/UiTools.h"
#include "GlobalMessanger.h"


SimProfileEditor* SimProfileEditor::m_simProfileEditor = nullptr;

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

SimProfileEditor::SimProfileEditor(DbController* dbController, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
	  m_db(dbController)
{
	if (m_db == nullptr)
	{
		Q_ASSERT(m_db);
		return;
	}

	setWindowTitle("Simulator Profiles");

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

	QPushButton* buttonCheck = new QPushButton(tr("Check"));
	QPushButton* buttonOK = new QPushButton(tr("OK"));
	QPushButton* buttonCancel = new QPushButton(tr("Cancel"));

	connect(buttonCheck, &QPushButton::clicked, this, &SimProfileEditor::checkProfiles);
	connect(buttonOK, &QPushButton::clicked, this, &SimProfileEditor::accept);
	connect(buttonCancel, &QPushButton::clicked, this, &SimProfileEditor::reject);

	// Layouts

	QHBoxLayout* buttonLayout = new QHBoxLayout();

	buttonLayout->addWidget(buttonCheck);
	buttonLayout->addStretch();
	buttonLayout->addWidget(buttonOK);
	buttonLayout->addWidget(buttonCancel);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(m_textEdit);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	// Load file from database
	//
	std::vector<DbFileInfo> fileList;

	bool ok = m_db->getFileList(&fileList, m_db->etcFileId(), Db::File::SimProfilesFileName, true, nullptr);

	if (ok == true && fileList.size() == 1)
	{
		std::shared_ptr<DbFile> file;

		if (m_db->getLatestVersion(fileList[0], &file, nullptr) == true)
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
	}

	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, qAppName(), "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (result == QMessageBox::Yes)
	{
		if (saveChanges() == false)
		{
			return false;
		}
		return true;
	}

	if (result == QMessageBox::No)
	{
		return true;
	}

	return false;
}

bool SimProfileEditor::saveChanges()
{
	Sim::Profiles profiles;

	QString errorMsg;

	if (profiles.parse(m_textEdit->text(), &errorMsg) == false)
	{
		if (QMessageBox::warning(this,
								 qAppName(),
								 tr("There are errors in the document. Are you sure you want to save it anyway?"),
								 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
		{
			return false;
		}
	}

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
		QMessageBox::warning(this, qAppName(), "No comment supplied! Please provide a comment.");
		return false;
	}

	// save to db
	//
	std::vector<DbFileInfo> fileList;

	ok = m_db->getFileList(&fileList, m_db->etcFileId(), Db::File::SimProfilesFileName, true, nullptr);
	if (ok == false || fileList.size() != 1)
	{
		// create a file, if it does not exists
		//
		std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
		pf->setFileName(Db::File::SimProfilesFileName);

		if (m_db->addFile(pf, m_db->etcFileId(), nullptr) == false)
		{
			return false;
		}

		ok = m_db->getFileList(&fileList, m_db->etcFileId(), Db::File::SimProfilesFileName, true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			return false;
		}
	}

	std::shared_ptr<DbFile> file = nullptr;

	ok = m_db->getLatestVersion(fileList[0], &file, nullptr);
	if (ok == false || file == nullptr)
	{
		return false;
	}

	if (file->state() != VcsState::CheckedOut)
	{
		if (m_db->checkOut(fileList[0], nullptr) == false)
		{
			return false;
		}
	}

	QByteArray data = m_textEdit->text().toUtf8();

	file->swapData(data);

	if (m_db->setWorkcopy(file, nullptr) == false)
	{
		return false;
	}

	if (m_db->checkIn(fileList[0], comment, nullptr) == false)
	{
		return false;
	}

	m_modified = false;

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
