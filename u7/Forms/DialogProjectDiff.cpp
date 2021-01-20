#include "DialogProjectDiff.h"
#include "ui_DialogProjectDiff.h"
#include "../../lib/DbController.h"
#include "SelectChangesetDialog.h"

#include <QPageSetupDialog>
#include <QPrinter>

//
// DialogProjectDiffSections
//

DialogProjectDiffSections::DialogProjectDiffSections(const ProjectDiffReportParams& reportParams, QWidget *parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_reportParams(reportParams)
{
	setWindowTitle(tr("Report Sections Page Setup"));
	setMinimumSize(540, 350);

	QGridLayout* gl = new QGridLayout();

	m_treeWidget = new QTreeWidget();

	QStringList l;
	l << tr("Section");
	l << tr("Page Size");
	l << tr("Orientation");
	l << tr("Margins, mm");
	m_treeWidget->setHeaderLabels(l);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item, int column){
		Q_UNUSED(item);
		Q_UNUSED(column);
		pageSetup();
	});

	QPushButton* b = new QPushButton(tr("Page Setup..."));
	connect(b, &QPushButton::clicked, this, &DialogProjectDiffSections::pageSetup);

	gl->addWidget(m_treeWidget, 0, 0);
	gl->addWidget(b, 0, 1, Qt::AlignTop);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();

	b = new QPushButton(tr("OK"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogProjectDiffSections::accept);

	b = new QPushButton(tr("Cancel"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogProjectDiffSections::reject);

	QVBoxLayout* ml = new QVBoxLayout();
	ml->addLayout(gl);
	ml->addLayout(buttonsLayout);

	setLayout(ml);

	fillTree();

	return;
}

ProjectDiffReportParams DialogProjectDiffSections::reportParams() const
{
	return m_reportParams;
}

void DialogProjectDiffSections::pageSetup()
{
	QList<QTreeWidgetItem*> selectedItems =  m_treeWidget->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	int firstIndex = m_treeWidget->indexOfTopLevelItem(selectedItems[0]);
	if (firstIndex < 0 || firstIndex >= m_reportParams.fileTypeParams.size())
	{
		Q_ASSERT(false);
		return;
	}

	const ProjectDiffFileTypeParams& firstFt = m_reportParams.fileTypeParams[firstIndex];

	QPageSize pageSize = firstFt.pageSize;
	QPageLayout::Orientation orientation = firstFt.orientation;
	QMarginsF margins = firstFt.margins;

	QPrinter printer(QPrinter::HighResolution);

	QPageSize::PageSizeId id = QPageSize::id(pageSize.sizePoints(), QPageSize::FuzzyOrientationMatch);
	if (id == QPageSize::Custom)
	{
		id = QPageSize::A4;
	}

	printer.setFullPage(true);
	printer.setPageSize(QPageSize(id));
	printer.setPageOrientation(orientation);
	printer.setPageMargins(margins, QPageLayout::Unit::Millimeter);

	QPageSetupDialog d(&printer, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	id = QPageSize::id(d.printer()->pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);

	for (QTreeWidgetItem* item : selectedItems)
	{
		int itemIndex = m_treeWidget->indexOfTopLevelItem(item);
		if (itemIndex < 0 || itemIndex >= m_reportParams.fileTypeParams.size())
		{
			Q_ASSERT(false);
			return;
		}

		ProjectDiffFileTypeParams& ft = m_reportParams.fileTypeParams[firstIndex];

		ft.pageSize = QPageSize(id);
		ft.orientation = d.printer()->pageLayout().orientation();
		ft.margins = d.printer()->pageLayout().margins();
	}

	fillTree();

	return;
}

void DialogProjectDiffSections::fillTree()
{
	if (m_treeWidget->topLevelItemCount() != m_reportParams.fileTypeParams.size())
	{
		m_treeWidget->clear();

		for (int i = 0; i < m_reportParams.fileTypeParams.size(); i++)
		{
			m_treeWidget->addTopLevelItem(new QTreeWidgetItem());
		}
	}

	int itemIndex = 0;

	for (const ProjectDiffFileTypeParams& ft : m_reportParams.fileTypeParams)
	{
		QTreeWidgetItem* item = m_treeWidget->topLevelItem(itemIndex++);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		QPageSize::PageSizeId id = QPageSize::id(ft.pageSize.sizePoints(), QPageSize::FuzzyOrientationMatch);
		if (id == QPageSize::Custom)
		{
			id = QPageSize::A4;
		}

		item->setText(0, ft.caption);
		item->setText(1, QPageSize(id).name());
		item->setText(2, ft.orientation == QPageLayout::Portrait ? tr("Portrait") : tr("Landscape"));
		item->setText(3, tr("l%1 t%2 r%3 b%4").arg(ft.margins.left()).arg(ft.margins.top()).arg(ft.margins.right()).arg(ft.margins.bottom()));
	}

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	return;
}

//
// DialogProjectDiff
//

QString DialogProjectDiff::m_fileName;
ProjectDiffReportParams DialogProjectDiff::m_reportParams;

DialogProjectDiff::DialogProjectDiff(DbController* db, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogProjectDiff),
	m_db(db)
{
	assert(m_db);

	ui->setupUi(this);

	// --
	//

	ui->reportFileEdit->setText(m_fileName);

	QStringList versionTypes;

	versionTypes << tr("Changeset");			// VersionType::Changeset
	versionTypes << tr("Date");					// VersionType::Date
	versionTypes << tr("Latest Version");		// VersionType::LatestVersion

	ui->sourceTypeComboBox->addItems(versionTypes);
	ui->targetTypeComboBox->addItems(versionTypes);

	ui->targetTypeComboBox->setCurrentIndex(static_cast<int>(CompareVersionType::LatestVersion));

	ui->sourceDateEdit->setDateTime(QDateTime::currentDateTime());
	ui->targetDateEdit->setDateTime(QDateTime::currentDateTime());

	versionTypeChanged();

	// --
	//
	connect(ui->sourceTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogProjectDiff::versionTypeChanged);
	connect(ui->targetTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogProjectDiff::versionTypeChanged);

	// Select default file types if they are not selected
	//
	if (m_reportParams.fileTypeParams.empty() == true)
	{
		m_reportParams.fileTypeParams = ProjectDiffGenerator::defaultFileTypeParams(db);
	}

	// Fill file types list
	//
	for (const ProjectDiffFileTypeParams& ft : m_reportParams.fileTypeParams)
	{
		QListWidgetItem* item = new QListWidgetItem(tr("%1").arg(ft.caption));

		if (ft.selected == true)
		{
			item->setCheckState(Qt::Checked);
		}
		else
		{
			item->setCheckState(Qt::Unchecked);
		}

		ui->categoriesList->addItem(item);
	}

	ui->expertPropertiesCheck->setChecked(m_reportParams.expertProperties == true);

	ui->multipleFilesCheck->setChecked(m_reportParams.multipleFiles == true);

	return;

}

DialogProjectDiff::~DialogProjectDiff()
{
	delete ui;
}

QString DialogProjectDiff::fileName() const
{
	return m_fileName;
}

ProjectDiffReportParams DialogProjectDiff::reportParams() const
{
	return m_reportParams;
}

void DialogProjectDiff::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	resize(static_cast<int>(screen.width() * 0.25),
		   rect().height());

	move(screen.center() - rect().center());

	return;
}

void DialogProjectDiff::versionTypeChanged()
{
	// Source
	//
	CompareVersionType sourceType = static_cast<CompareVersionType>(ui->sourceTypeComboBox->currentIndex());

	switch (sourceType)
	{
	case CompareVersionType::Changeset:
		ui->sourceChangesetLabel->setEnabled(true);
		ui->sourceChangesetLineEdit->setEnabled(true);
		ui->sourceChangesetButton->setEnabled(true);
		ui->sourceDateLabel->setEnabled(false);
		ui->sourceDateEdit->setEnabled(false);
		break;
	case CompareVersionType::Date:
		ui->sourceChangesetLabel->setEnabled(false);
		ui->sourceChangesetLineEdit->setEnabled(false);
		ui->sourceChangesetButton->setEnabled(false);
		ui->sourceDateLabel->setEnabled(true);
		ui->sourceDateEdit->setEnabled(true);
		break;
	case CompareVersionType::LatestVersion:
		ui->sourceChangesetLabel->setEnabled(false);
		ui->sourceChangesetLineEdit->setEnabled(false);
		ui->sourceChangesetButton->setEnabled(false);
		ui->sourceDateLabel->setEnabled(false);
		ui->sourceDateEdit->setEnabled(false);
		break;
	default:
		assert(false);
	}

	// Target
	//
	CompareVersionType targetType = static_cast<CompareVersionType>(ui->targetTypeComboBox->currentIndex());

	switch (targetType)
	{
	case CompareVersionType::Changeset:
		ui->targetChangesetLabel->setEnabled(true);
		ui->targetChangesetLineEdit->setEnabled(true);
		ui->targetChangesetButton->setEnabled(true);
		ui->targetDateLabel->setEnabled(false);
		ui->targetDateEdit->setEnabled(false);
		break;
	case CompareVersionType::Date:
		ui->targetChangesetLabel->setEnabled(false);
		ui->targetChangesetLineEdit->setEnabled(false);
		ui->targetChangesetButton->setEnabled(false);
		ui->targetDateLabel->setEnabled(true);
		ui->targetDateEdit->setEnabled(true);
		break;
	case CompareVersionType::LatestVersion:
		ui->targetChangesetLabel->setEnabled(false);
		ui->targetChangesetLineEdit->setEnabled(false);
		ui->targetChangesetButton->setEnabled(false);
		ui->targetDateLabel->setEnabled(false);
		ui->targetDateEdit->setEnabled(false);
		break;
	default:
		assert(false);
	}

	return;
}

void DialogProjectDiff::on_sourceChangesetButton_clicked()
{
	int changeset = SelectChangesetDialog::getProjectChangeset(m_db, this);

	if (changeset != -1)
	{
		ui->sourceChangesetLineEdit->setText(QString::number(changeset));
	}
}

void DialogProjectDiff::on_targetChangesetButton_clicked()
{
	int changeset = SelectChangesetDialog::getProjectChangeset(m_db, this);

	if (changeset != -1)
	{
		ui->targetChangesetLineEdit->setText(QString::number(changeset));
	}
}

void DialogProjectDiff::done(int r)
{
	if (r == QDialog::Rejected)
	{
		QDialog::done(r);
		return;
	}

	m_fileName = ui->reportFileEdit->text();

	if (m_fileName.isEmpty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Please enter report file name!"));
		ui->reportFileEdit->setFocus();
		return;
	}

	CompareData compareData;

	// Source
	//
	compareData.sourceVersionType = static_cast<CompareVersionType>(ui->sourceTypeComboBox->currentIndex());

	bool sourceChangesetConversionOk = false;
	compareData.sourceChangeset = ui->sourceChangesetLineEdit->text().toInt(&sourceChangesetConversionOk);

	compareData.sourceDate = ui->sourceDateEdit->dateTime();

	// Target
	//
	compareData.targetVersionType = static_cast<CompareVersionType>(ui->targetTypeComboBox->currentIndex());

	bool targetChangesetConversionOk = false;
	compareData.targetChangeset = ui->targetChangesetLineEdit->text().toInt(&targetChangesetConversionOk);

	compareData.targetDate = ui->targetDateEdit->dateTime();

	// Checks
	//
	if (compareData.sourceVersionType == CompareVersionType::Changeset &&
		sourceChangesetConversionOk == false)
	{
		ui->sourceChangesetLineEdit->setFocus();
		ui->sourceChangesetLineEdit->selectAll();
		return;
	}

	if (compareData.sourceVersionType == CompareVersionType::Date &&
		compareData.sourceDate.isValid() == false)
	{
		ui->sourceDateEdit->setFocus();
		ui->sourceDateEdit->selectAll();
		return;
	}

	if (compareData.targetVersionType == CompareVersionType::Changeset &&
		targetChangesetConversionOk == false)
	{
		ui->targetChangesetLineEdit->setFocus();
		ui->targetChangesetLineEdit->selectAll();
		return;
	}

	if (compareData.targetVersionType == CompareVersionType::Date &&
		compareData.targetDate.isValid() == false)
	{
		ui->targetDateEdit->setFocus();
		ui->targetDateEdit->selectAll();
		return;
	}

	if (compareData.sourceVersionType == compareData.targetVersionType &&
		compareData.sourceDate == compareData.targetDate &&
		compareData.sourceChangeset == compareData.targetChangeset)
	{
		QMessageBox::critical(this, qAppName(), tr("Please select different changesets!"));
		return;
	}

	m_reportParams.compareData = compareData;

	int selectedCount = 0;

	if (ui->categoriesList->count() != m_reportParams.fileTypeParams.size())
	{
		Q_ASSERT(false);
		return;
	}

	for (int i = 0; i < ui->categoriesList->count(); i++)
	{
		QListWidgetItem* item = ui->categoriesList->item(i);

		m_reportParams.fileTypeParams[i].selected = item->checkState() == Qt::Checked;
		if (m_reportParams.fileTypeParams[i].selected == true)
		{
			selectedCount++;
		}
	}

	if (selectedCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Please select at least one file category!"));
		return;
	}

	m_reportParams.expertProperties = ui->expertPropertiesCheck->isChecked() == true;
	m_reportParams.multipleFiles = ui->multipleFilesCheck->isChecked() == true;

	QDialog::done(r);
}

void DialogProjectDiff::on_buttonSelectAll_clicked()
{
	for (int i = 0; i < ui->categoriesList->count(); i++)
	{
		QListWidgetItem* item = ui->categoriesList->item(i);
		item->setCheckState(Qt::Checked);
	}
}

void DialogProjectDiff::on_buttonSelectNone_clicked()
{
	for (int i = 0; i < ui->categoriesList->count(); i++)
	{
		QListWidgetItem* item = ui->categoriesList->item(i);
		item->setCheckState(Qt::Unchecked);
	}
}

void DialogProjectDiff::on_categoriesList_itemPressed(QListWidgetItem *item)
{
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	if (item->checkState() == Qt::Checked)
	{
		item->setCheckState(Qt::Unchecked);
	}
	else
	{
		item->setCheckState(Qt::Checked);
	}

	return;
}

void DialogProjectDiff::on_fileBrowseButton_clicked()
{
	// Get filename
	//

	QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Diff Report"),
													"./",
													QObject::tr("PDF documents (*.pdf)"));
	if (fileName.isNull() == true)
	{
		return;
	}

	m_fileName = fileName;
	ui->reportFileEdit->setText(fileName);

	return;
}

void DialogProjectDiff::on_pageSetupButton_clicked()
{
	if (ui->multipleFilesCheck->isChecked() == true)
	{
		DialogProjectDiffSections d(m_reportParams, this);
		if (d.exec() == QDialog::Accepted)
		{
			m_reportParams = d.reportParams();
		}
	}
	else
	{
		// Single-file report

		QPrinter printer(QPrinter::HighResolution);

		QPageSize::PageSizeId id = QPageSize::id(m_reportParams.albumPageSize.sizePoints(), QPageSize::FuzzyOrientationMatch);
		if (id == QPageSize::Custom)
		{
			id = QPageSize::A4;
		}

		printer.setFullPage(true);
		printer.setPageSize(QPageSize(id));
		printer.setPageOrientation(m_reportParams.albumOrientation);
		printer.setPageMargins(m_reportParams.albumMargins, QPageLayout::Unit::Millimeter);

		QPageSetupDialog d(&printer, this);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		id = QPageSize::id(d.printer()->pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);

		m_reportParams.albumPageSize = QPageSize(id);
		m_reportParams.albumOrientation = d.printer()->pageLayout().orientation();
		m_reportParams.albumMargins = d.printer()->pageLayout().margins();
	}

	return;
}
