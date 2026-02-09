#include "CheckInDialog.h"
#include "ui_CheckInDialog.h"

CheckInDialog::CheckInDialog(const std::vector<DbFileInfo>& files,
							 bool treeCheckIn,
							 QString objectIdJsonName,
							 DbController* dbc,
							 QWidget* parent) :
	QDialog{parent},
	HasDbController{dbc},
	m_files{files},
	m_objectIdJsonName{objectIdJsonName},
	ui{new Ui::CheckInDialog}
{
	ui->setupUi(this);

	ui->childrenCheckBox->setChecked(treeCheckIn);

	// --
	//
	auto columns = QStringList() << tr("Object*") << tr("Other") << tr("FileName") << tr("FileID");
	assert(columns.size() == static_cast<int>(FileListColumn::Count));

	ui->filesTreeWidget->setColumnCount(static_cast<int>(FileListColumn::Count));
	ui->filesTreeWidget->setHeaderLabels(columns);

	updateCheckInFiles(ui->childrenCheckBox->isChecked());

	connect(ui->childrenCheckBox, &QCheckBox::toggled, this, &CheckInDialog::updateCheckInFiles);

	return;
}

CheckInDialog::~CheckInDialog()
{
	delete ui;
}

bool CheckInDialog::checkIn(std::vector<DbFileInfo>& files,
							std::vector<DbFileInfo>* checkedInFiles,
							bool treeCheckIn,
							QString objectIdJsonName,
							DbController* dbc,
							QWidget* parent,
							bool* checkInTree)
{
	if (files.empty() == true)
	{
		return false;
	}

	if (checkedInFiles == nullptr)
	{
		assert(checkedInFiles != nullptr);
		return false;
	}

	CheckInDialog dialog{files, treeCheckIn, objectIdJsonName, dbc, parent};
	dialog.exec();

	if (dialog.result() == Accepted)
	{
		*checkedInFiles = std::move(dialog.m_checkInFiles);

		if (checkInTree != nullptr)
		{
			*checkInTree = dialog.ui->childrenCheckBox->isChecked();
		}
	}
	else
	{
		checkedInFiles->clear();
	}

	return dialog.result();
}

void CheckInDialog::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = parentWidget()->screen()->availableGeometry();

	if (s_lastSize.isValid() == true)
	{
		s_lastSize.rwidth() = std::clamp(s_lastSize.width(), screen.width() / 5, screen.width());
		s_lastSize.rheight() = std::clamp(s_lastSize.height(), screen.height() / 5, screen.height());
		resize(s_lastSize);
	}
	else
	{
		resize(static_cast<int>(screen.width() * 0.30), static_cast<int>(screen.height() * 0.30));
	}

	move(screen.center() - rect().center());
	return;
}

void CheckInDialog::closeEvent(QCloseEvent*)
{
	s_lastSize = size();
	return;
}

void CheckInDialog::on_checkInButton_clicked()
{
	QString comment = ui->commentEdit->toPlainText();

	if (comment.isEmpty() == true)
	{
		QMessageBox mb{this};
		mb.setText(tr("Check In comment cannot be empty!"));
		mb.exec();

		ui->commentEdit->setFocus();
		return;
	}

	bool result = false;
	result = db()->checkIn(m_checkInFiles, comment, this);

	if (result == true)
	{
		accept();
	}
	else
	{
		reject();
	}

	return;
}

void CheckInDialog::on_cancelButton_clicked()
{
	reject();
	return;
}

void CheckInDialog::updateCheckInFiles(bool includeChildren)
{
	m_checkInFiles.clear();
	ui->filesTreeWidget->clear();

	if (includeChildren == true)
	{
		dbc()->getCheckedOutFiles(&m_files, &m_checkInFiles, this);
	}
	else
	{
		std::vector<int> fileIds;
		fileIds.reserve(m_files.size());
		for (const DbFileInfo& file : m_files)
		{
			fileIds.push_back(file.fileId());
		}

		dbc()->getFileInfo(&fileIds, &m_checkInFiles, this);
	}

	// Filter all files items that were checked-out by other users unless current user is an administrator
	//
	bool currentUserIsAdmin = dbc()->currentUser().isAdministrator();
	auto currentUserId = dbc()->currentUser().userId();

	m_checkInFiles.erase(std::remove_if(m_checkInFiles.begin(),
										m_checkInFiles.end(),
										[this, currentUserIsAdmin, currentUserId](const DbFileInfo& fi)
										{
											bool ok = fi.state() == E::VcsState::CheckedOut &&
													  (fi.userId() == currentUserId || currentUserIsAdmin == false);
											return !ok;
										}),
						 m_checkInFiles.end());

	{
		ui->filesTreeWidget->blockSignals(true);

		for (const DbFileInfo& file : m_checkInFiles)
		{
			QString object;
			QString other;
			QString fileName = file.fileName();
			QString fileId = QString::number(file.fileId());

			if (fileName.isEmpty() == true)
			{
				fileName = tr("Unknown");
			}

			QJsonDocument jsdoc = QJsonDocument::fromJson(file.details().toUtf8());
			QJsonObject jsobject = jsdoc.object();

			if (jsobject.isEmpty() == false && jsobject.contains(m_objectIdJsonName))
			{
				object = jsobject.value(m_objectIdJsonName).toString();
			}
			else
			{
				// Fallback to filename
				//
				object = fileName;
			}

			if (jsobject.isEmpty() == false && jsobject.contains("Place"))
			{
				other = QString{"Place: %1"}.arg(jsobject.value("Place").toInt());
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << object << other << fileName << fileId);
			item->setToolTip(static_cast<int>(FileListColumn::Object), tr("The actual object name may differ."));

			ui->filesTreeWidget->addTopLevelItem(item);
		}

		// Resize columns only once
		//
		if (m_resizeDone == false && ui->filesTreeWidget->topLevelItemCount() > 0)
		{
			for (int column = 0; column < ui->filesTreeWidget->columnCount(); ++column)
			{
				ui->filesTreeWidget->resizeColumnToContents(column);
			}

			m_resizeDone = true;
		}
		ui->filesTreeWidget->blockSignals(false);
		ui->filesTreeWidget->update();
	}

	// Set file count
	//
	QString fileCountText = tr("%1 files").arg(ui->filesTreeWidget->topLevelItemCount());
	if (includeChildren == false)
	{
		fileCountText += tr(", children objects are not included.");
	}
	ui->filesLabel->setText(fileCountText);

	// Enable/disable check-in button
	//
	ui->checkInButton->setEnabled(m_checkInFiles.empty() == false);

	return;
}
