#include "SimSelectBuildDialog.h"
#include "ui_SimSelectBuildDialog.h"
#include "../Settings.h"

SimSelectBuildDialog::SimSelectBuildDialog(QString currentProject,
										   QString buildPath,
										   QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SimSelectBuildDialog),
	m_projectName(currentProject.toLower())
{
	ui->setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
	assert(okButton);

	okButton->setEnabled(false);

	// --
	//
	connect(ui->buildList, &QListWidget::currentRowChanged, this, &SimSelectBuildDialog::buildListSelectionChanged);
	connect(ui->buildList, &QListWidget::itemDoubleClicked, this, &SimSelectBuildDialog::buildListItemDoubleClicked);

	// --
	//
	fillBuildList(buildPath);

	return;
}

SimSelectBuildDialog::~SimSelectBuildDialog()
{
	delete ui;
}

QString SimSelectBuildDialog::buildsPath()
{
	QString buildSearchPath = theSettings.buildOutputPath() + QDir::separator() + m_projectName;
	return buildSearchPath;
}

void SimSelectBuildDialog::fillBuildList(QString currentBuildPath)
{
	ui->buildList->clear();

	QDir dir(buildsPath());
	if (dir.exists() == false)
	{
		return;
	}

	QFileInfoList buildDirsList = dir.entryInfoList(QStringList("build*"), QDir::Dirs | QDir::NoSymLinks, QDir::Name);

	QListWidgetItem* currentItem = nullptr;

	for (const QFileInfo& fi : buildDirsList)
	{
		QListWidgetItem* item = new QListWidgetItem(fi.fileName(), ui->buildList);
		item->setData(Qt::UserRole, fi.absoluteFilePath());

		QDir d1(fi.absoluteFilePath());
		QDir d2(currentBuildPath);

		if (d1 == d2)
		{
			currentItem = item;
		}
	}

	if (ui->buildList->count() != 0)
	{
		if (currentItem != nullptr)
		{
			ui->buildList->setCurrentItem(currentItem);
		}
		else
		{
			ui->buildList->setCurrentRow(0);
		}
	}

	return;
}

void SimSelectBuildDialog::buildListSelectionChanged(int currentRow)
{
	QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
	assert(okButton);

	okButton->setEnabled(currentRow != -1);

	return;
}

void SimSelectBuildDialog::buildListItemDoubleClicked(QListWidgetItem*)
{
	assert(ui->buildList);
	QList<QListWidgetItem*> selected = ui->buildList->selectedItems();

	if (selected.isEmpty() == true)
	{
		return;
	}

	accept();
	return;
}

void SimSelectBuildDialog::accept()
{
	QDialog::accept();
	return;
}

QString SimSelectBuildDialog::resultBuildPath() const
{
	assert(ui->buildList);
	QList<QListWidgetItem*> selected = ui->buildList->selectedItems();

	if (selected.isEmpty() == true)
	{
		return QString();
	}

	QListWidgetItem* item = selected.front();
	QString buildPath = item->data(Qt::UserRole).toString();

	return buildPath;
}
