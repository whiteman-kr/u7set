#include "SimSelectBuildDialog.h"
#include "ui_SimulatorSelectBuildDialog.h"
#include <QDir>
#include "../Settings.h"

SimSelectBuildDialog::SimSelectBuildDialog(QString currentProject,
													   BuildType buildType,
													   QString buildPath,
													   QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SimulatorSelectBuildDialog),
	m_projectName(currentProject.toLower())
{
	ui->setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
	assert(okButton);

	okButton->setEnabled(false);

	// buildType, buildPath of the currently selected project
	//
	if (buildType == BuildType::Debug)
	{
		ui->debugButton->setChecked(true);
	}
	else
	{
		ui->releaseButton->setChecked(true);
	}

	// --
	//
	connect(ui->buildList, &QListWidget::currentRowChanged, this, &SimSelectBuildDialog::buildListSelectionChanged);
	connect(ui->buildList, &QListWidget::itemDoubleClicked, this, &SimSelectBuildDialog::buildListItemDoubleClicked);

	connect(ui->debugButton, &QRadioButton::toggled, this, [this](bool)	{fillBuildList("");});

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
	QString configurationType = ui->debugButton->isChecked() ? QLatin1String("debug") : QLatin1String("release");

	QString buildSearchPath = QString("%1%2%3-%4")
							  .arg(theSettings.buildOutputPath())
							  .arg(QDir::separator())
							  .arg(m_projectName)
							  .arg(configurationType);

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

SimSelectBuildDialog::BuildType SimSelectBuildDialog::resultBuildType() const
{
	return ui->debugButton->isChecked() ? BuildType::Debug : BuildType::Release;
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
