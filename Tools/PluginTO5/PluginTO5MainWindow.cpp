#include "PluginTO5MainWindow.h"

#include "DialogImport.h"

#include <UiLib/DialogAbout.h>
#include <CommonStdLib/u7_vld.h>
#include <TestSuiteLib/TestReport.h>
#include <UiLib/LogDialog.h>

#include <QTableWidget>
#include <QSplitter>
#include <QToolBar>
#include <QLabel>
#include <QTableWidgetItem>
#include <QSettings>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMenuBar>
#include <QPushButton>
#include <QHeaderView>
#include <QFile>

class NumericDelegate : public QStyledItemDelegate
{
public:
	explicit NumericDelegate(QTableWidget* tableWidget, QObject* parent = nullptr) :
		QStyledItemDelegate(parent),
		m_tableWidget(tableWidget)
	{
	}

	// Override to customize the editor widget
	//
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /* option*/, const QModelIndex& /*index*/) const override
	{
		QLineEdit* editor = new QLineEdit(parent);

		QDoubleValidator* validator = new QDoubleValidator(editor);

		QLocale loc = QLocale::c();
		loc.setNumberOptions(QLocale::RejectGroupSeparator);
		validator->setLocale(loc);

		editor->setValidator(validator);

		return editor;
	}

	QTableWidget* m_tableWidget = nullptr;
};

PluginTO5MainWindow::PluginTO5MainWindow(QWidget* parent) :
	QMainWindow(parent),
	m_logFile{qAppName(),
			  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + PluginTO5Settings::restore().equipmentId},
	m_runner(m_logFile)
{

	// Init translator
	//
	m_translator.addLanguage("en", "English");
	m_translator.addLanguage("uk", "Ukrainian");

	for (const QString& l : m_translator.languagesList())
	{
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/PluginTO5_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/TestSuiteLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/ClientLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/UtilsLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/UiLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/ReportLib_%1.qm").arg(l));
	}

	QString language = QSettings().value("MainWindow/language", "uk").toString();

	if (language != "en")
	{
		QStringList failedTranslations;
		if (m_translator.setLanguage(language, failedTranslations) == false)
		{
			if (failedTranslations.isEmpty() == false)
			{
				m_logFile.writeError("Failed to load translation files:\n" + failedTranslations.join('\n'));
			}
			else
			{
				m_logFile.writeError("Failed to set language: " + language);
			}
		}
	}
	//

	m_comparatorsStorage = ComparatorsStorage();

	m_maskHelp = tr("A mask contains '*' and '?' symbols.\n\
	'*' symbol means any set of symbols on its place, '?' symbol means one symbol on its place.\n\
	Several masks can be separated by semicolon or space.\n\n\
	Examples:\n\n\
	TZB* (mask for Schema Id),\n\
	To apply the filter, enter the mask and press Enter.");
	m_maskHelp.remove('\t');

	createMenu();

	{
		int filterType = QSettings().value("MainWindow/filterType", static_cast<int>(m_filterType)).toInt();
		if (filterType >= 0 && filterType < static_cast<int>(FilterType::Count)) 
		{
			m_filterType = static_cast<FilterType>(filterType);
		}
	}

	createUi();

	setMinimumSize(1024, 768);

	// Restore settings
	//
	QPoint p = QSettings().value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	if (p.x() != -1 && p.y() != -1)
	{
		move(p);
	}
	QByteArray ba = QSettings().value("MainWindow/geometry").toByteArray();
	if (ba.isEmpty() == false)
	{
		restoreGeometry(ba);
	}
	ba = QSettings().value("MainWindow/splitter").toByteArray();
	if (ba.isEmpty() == false)
	{
		m_splitter->restoreState(ba);
	}

	// Load comparators
	//
	m_comparatorsCSV = QSettings().value("fileComparatorsCSV").toString();
	m_comparatorsCSV = QDir::toNativeSeparators(m_comparatorsCSV);

	if (m_comparatorsCSV.isEmpty() == false &&
		loadComparators(m_comparatorsCSV) == true)
	{
		fillFilters();
		updateTable({}, FilterType::BySchemaID);
	}

	// Restore table state
	//
	if (QSettings().value("MainWindow/tableColumnsCount", 0) != m_tableHeaders.size())
	{
		// Hide columns
		//

		auto hideColumn = [this](const QString& header) ->void
		{
			int index = m_tableHeaders.indexOf(header);
			if (index == -1) 
			{
				Q_ASSERT(false);
				return;
			}
			m_tableWidget->setColumnHidden(index, true);
		};

		{
			hideColumn(tr("InputAppSignalCaption"));
			hideColumn(tr("Type"));
			hideColumn(tr("SetpointAppSignalID"));
			hideColumn(tr("SetpointAppSignalCaption"));
		}

		m_tableWidget->resizeColumnsToContents();
	}
	else
	{
		// Restore columns state
		//
		ba = QSettings().value("MainWindow/tableState").toByteArray();
		if (ba.isEmpty() == false)
		{
			m_tableWidget->horizontalHeader()->restoreState(ba);
		}
	}

	m_mainWindowTimerId_250ms = startTimer(250);
}

PluginTO5MainWindow::~PluginTO5MainWindow()
{
	QSettings().setValue("MainWindow/pos", pos());
	QSettings().setValue("MainWindow/geometry", saveGeometry());
	QSettings().setValue("MainWindow/splitter", m_splitter->saveState());

	QSettings().setValue("fileComparatorsCSV", m_comparatorsCSV);

	QSettings().setValue("MainWindow/tableColumnsCount", m_tableHeaders.size());
	QSettings().setValue("MainWindow/tableState", m_tableWidget->horizontalHeader()->saveState());

	QSettings().setValue("MainWindow/filterType", static_cast<int>(m_filterType));
}

void PluginTO5MainWindow::showAppLog()
{ 
	Log::LogFileDialog::view(m_logFile, this); 
}

void PluginTO5MainWindow::showAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());
	return;
}

void PluginTO5MainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" provides the online operator-support system.");

	UiLib::DialogAbout::show(this, text, {});

	return;
}

void PluginTO5MainWindow::onSettings()
{
	QDialog dialog;
	dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	dialog.setWindowModality(Qt::ApplicationModal);
	dialog.setWindowTitle(QObject::tr("Settings"));

	QFormLayout* layout = new QFormLayout(&dialog);

	QLineEdit* testSuiteId = new QLineEdit(&dialog);
	layout->addRow(tr("TestSuite Id:"), testSuiteId);

	QLineEdit* ipAddress1 = new QLineEdit(&dialog);
	layout->addRow(tr("Configurator IP Address 1:"), ipAddress1);

	QLineEdit* port1 = new QLineEdit(&dialog);
	layout->addRow(tr("Configurator Port 1:"), port1);

	QLineEdit* ipAddress2 = new QLineEdit(&dialog);
	layout->addRow(tr("Configurator IP Address 2:"), ipAddress2);
	QLineEdit* port2 = new QLineEdit(&dialog);
	layout->addRow(tr("Configurator Port 2:"), port2);


	m_language = new QComboBox(&dialog);
	m_language->addItems(m_translator.languagesList());
	m_language->setCurrentIndex(m_translator.languagesList().indexOf(QSettings().value("MainWindow/language", "uk").toString()));
	connect(m_language, &QComboBox::currentTextChanged, [this]()
	{
		QSettings().setValue("MainWindow/language", m_language->currentText());
	});
	layout->addRow(tr("Language:"), m_language);
	{
		PluginTO5Settings settings = PluginTO5Settings::restore();

		testSuiteId->setText(settings.equipmentId);
		ipAddress1->setText(settings.ipAddress1);
		port1->setText(settings.port1);
		ipAddress2->setText(settings.ipAddress2);
		port2->setText(settings.port2);
		m_language->setCurrentText(QSettings().value("MainWindow/language", "uk").toString());
	}

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	layout->addRow(buttons);

	connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	connect(&dialog, &QDialog::accepted, [this, testSuiteId, ipAddress1, port1, ipAddress2, port2]()
	{
				PluginTO5Settings settings;

				settings.equipmentId = testSuiteId->text();
				settings.ipAddress1 = ipAddress1->text();
				settings.port1 = port1->text();
				settings.ipAddress2 = ipAddress2->text();
				settings.port2 = port2->text();

				settings.store();
				m_runner.setTestSuiteSettings(settings);
	});	
	
	dialog.exec();
}

void PluginTO5MainWindow::onNewComparators()
{
	m_comparatorsStorage.clear();
	m_comparatorsCSV.clear();

	setWindowTitle(qApp->applicationName());

	fillFilters();
	updateTable({}, FilterType::BySchemaID);
}

void PluginTO5MainWindow::onOpenComparators()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Comparators CSV"), "", tr("CSV Files (*.csv)"));
	if (fileName.isEmpty())
	{
		return;
	}

	if (loadComparators(fileName) == true)
	{
		m_comparatorsCSV = fileName;

		fillFilters();
		updateTable({}, FilterType::BySchemaID);
	}
}

void PluginTO5MainWindow::onSaveComparators()
{
	if (m_comparatorsCSV.isEmpty())
	{
		onSaveAsComparators();
		return;
	}
	m_comparatorsStorage.saveToFile(m_comparatorsCSV);
	m_hasUnsavedChanges = false;
}

void PluginTO5MainWindow::onSaveAsComparators()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Comparators CSV"), "", tr("CSV Files (*.csv)"));
	if (fileName.isEmpty())
	{
		return;
	}
	fileName = QDir::toNativeSeparators(fileName);

	m_comparatorsStorage.saveToFile(fileName);
	m_comparatorsCSV = fileName;
	setWindowTitle(qApp->applicationName() + " - " + m_comparatorsCSV);
	m_hasUnsavedChanges = false;
}

void PluginTO5MainWindow::onImportComparators()
{
	QSettings s{qApp->organizationName(), qApp->applicationName()};
	QString fileComparators = s.value("fileComparators", "").toString();
	QString fileAppSignals = s.value("fileAppSignals", "").toString();

	DialogImport menuDialog(fileComparators, fileAppSignals);

	if (menuDialog.exec() == QDialog::Accepted)
	{
		fileComparators = menuDialog.comparatorsFile();
		fileAppSignals = menuDialog.appSignalsFile();

		s.setValue("fileComparators", fileComparators);
		s.setValue("fileAppSignals", fileAppSignals);

		QString errorMsg;
		auto [ok, ir] = m_comparatorsStorage.importComparatorsSet(fileComparators, fileAppSignals, &errorMsg);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Import data failed: %1").arg(errorMsg));
			return;
		}


		QMessageBox::warning(
			this,
			QObject::tr("New Comparators found"),
			QObject::tr("%1 new comparators have been found, %2 comparators were removed, %3 comparators have overridden value.")
				.arg(ir.newComparatorCount)
				.arg(ir.removedComparatorCount)
				.arg(ir.overriddenValuesComparatorCount));

		fillFilters();
		updateTable({}, FilterType::BySchemaID);

		if (ir.newComparatorCount > 0 || ir.removedComparatorCount > 0)
		{
			m_hasUnsavedChanges = true;
		}
	}
}

void PluginTO5MainWindow::onItemCriteriaChanged(QTableWidgetItem* item)
{
	if (item == nullptr)
	{
		return;
	}

	// Prevent recursive itemChanged signals
	//
	QSignalBlocker blocker(m_tableWidget);

	const QString value = item->text();
	const int column = item->column();

	if (column != m_criteriaColumn)
	{
		return;
	}

	m_tableWidget->setSortingEnabled(false);

	for (const QModelIndex& index : m_tableWidget->selectionModel()->selectedIndexes())
	{
		QTableWidgetItem* changedItem = m_tableWidget->item(index.row(), column);
		if (changedItem != nullptr)
		{
			changedItem->setText(value);
			

			Hash cmpHash = changedItem->data(Qt::UserRole)
							   .toULongLong();

			bool ok = false;
			double v = value.toDouble(&ok);
			Q_ASSERT(ok);

			if (ok == true)
			{
				const auto& cd = m_comparatorsStorage.comparator(cmpHash, &ok);
				if (ok == true)
				{
					v = QString::number(value.toDouble(), 'f', cd.precision).toDouble();
					m_comparatorsStorage.setComparatorCriteria(cmpHash, v);

					if (cd.isValueOverriden() == true)
					{
						changedItem->setBackground(Qt::yellow);
					}
					else
					{
						changedItem->setBackground(Qt::white);
					}
				}
			}
		}
	}

	m_tableWidget->setSortingEnabled(true);

	m_hasUnsavedChanges = true;
}

void PluginTO5MainWindow::onReport(ReportType reportType)
{ 
	QDialog dialog;

	dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	dialog.setWindowModality(Qt::ApplicationModal);
	dialog.setWindowTitle(QObject::tr("Report"));
	dialog.setMinimumSize(1024, 768);

	QFormLayout* layout = new QFormLayout(&dialog);

	QLineEdit* reportFileEdit = new QLineEdit;

	QPushButton* reportPathBrowse = new QPushButton(tr("Browse..."));
	
	QHBoxLayout* reportLayout = new QHBoxLayout;
	reportLayout->setContentsMargins(0, 0, 0, 0);
	reportLayout->addWidget(reportFileEdit);
	reportLayout->addWidget(reportPathBrowse);

	QString defaultReportFile;
	QString reportFile;
	switch (reportType)
	{
	case ReportType::BySchemaID:
		defaultReportFile = QCoreApplication::applicationDirPath() + QDir::separator() + "TO5_BySchemaID.pdf";
		reportFile = QDir::toNativeSeparators(QSettings().value("reportFile/BySchemaID", defaultReportFile).toString());
		break;
	case ReportType::ByCaseID:
		defaultReportFile = QCoreApplication::applicationDirPath() + QDir::separator() + "TO5_ByCaseID.pdf";
		reportFile = QDir::toNativeSeparators(QSettings().value("reportFile/ByCaseID", defaultReportFile).toString());
		break;
	default:
		Q_ASSERT(false);
		return;
	}

	reportFileEdit->setText(reportFile);

	layout->addRow(tr("Path to save report:"), reportLayout);

	QTextEdit* logWidget = new QTextEdit();
	logWidget->setReadOnly(true);

	layout->addRow(logWidget);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("Create"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	layout->addRow(buttons);

	connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	connect(reportPathBrowse,
			&QPushButton::clicked,
			this,
			[this, &dialog, reportFileEdit]()
			{
				QString file =
				QFileDialog::getSaveFileName(&dialog, tr("Select Report File"), reportFileEdit->text(), tr("PDF Files (*.pdf)"));

				if (file.isEmpty() == false)
				{
					reportFileEdit->setText(QDir::toNativeSeparators(file));
				}
			});


	// OK button
	//
	connect(buttons->button(QDialogButtonBox::Ok),
			&QPushButton::clicked,
			&dialog,
			[&, reportFileEdit, logWidget, buttons]()
			{
				if (reportFileEdit->text().isEmpty() == true)
				{
					return;
				}

				logWidget->clear();

				QString reportFilter;
				switch (reportType)
				{
				case ReportType::BySchemaID:
					reportFilter = "testTO5BySchemaID";
					break;
				case ReportType::ByCaseID:
					reportFilter = "testTO5ByCaseID";
					break;
				default:
					Q_ASSERT(false);
					return;
				}

				QString destinationFile = tr("%1%2PluginTO5.csv")
											  .arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
											  .arg(QDir().separator());

				destinationFile = QDir::toNativeSeparators(destinationFile);


				if (QFile::exists(destinationFile) == true)
				{
					if (QFile(destinationFile).remove() == false) 
					{
						QMessageBox::critical(&dialog, qAppName(), tr("Failed to delete file %1!").arg(destinationFile));
						return;
					}
				}

				if (m_comparatorsStorage.saveToFile(destinationFile) == false) 
				{
					QMessageBox::critical(&dialog, qAppName(), tr("Failed to save comparators file to %1!").arg(destinationFile));
					return;
				}

				buttons->button(QDialogButtonBox::Ok)->setEnabled(false);

				if (m_runner.execute(reportFilter) == false) 
				{
					buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
					QMessageBox::critical(&dialog, qAppName(), tr("Report generating failed, invalid configuration parameters!"));
				}
			});

	// Add new items to log
	//
	connect(&m_runner,
			&TO5Runner::logItem,
			logWidget,
			[logWidget](const QString& text, TestSuite::TestLogItemType type)
			{
				QString color;
				switch (type)
				{
				case TestSuite::TestLogItemType::Message:
				case TestSuite::TestLogItemType::Text:
					color = "black";
					break;
				case TestSuite::TestLogItemType::Warning:
					color = "#F87217";
					break;
				case TestSuite::TestLogItemType::Error:
					color = "#D00000";
					break;
				}

				QString html = QString("<font face=\"Courier\" size=\"4\" color=%1>%2</font>").arg(color).arg(text);

				logWidget->append(html);
			});

	// Finish the report
	//
	connect(&m_runner,
			&TO5Runner::done,
			&dialog,
			[this, &dialog, reportFileEdit, buttons](int result)
			{
				buttons->button(QDialogButtonBox::Ok)->setEnabled(true);

				if (result == 0)
				{
					bool ok = TestSuite::TestReport::generateReport(m_runner.configData().reportTemplates,
																	m_runner.testLog(),
																	"РАЕС КСБ ТО-5",
																	reportFileEdit->text(),
																	&dialog);
					Q_UNUSED(ok);
				}
				else
				{
					QMessageBox::critical(&dialog, qAppName(), tr("Report generating failed."));
				}
			
			});


	dialog.exec();

	if (m_runner.isRunning() == true) 
	{
		m_runner.stop();
	}
	
}

void PluginTO5MainWindow::editMaskReturnPressed()
{
	QString mask = m_editMask->text();
	if (mask.isEmpty() == true)
	{
		fillFilters();
		return;
	}

	bool filterMatch = false;
	QList<QString> filtersId;
	for (int row = 0; row < m_filtersListWidget->count(); ++row)
	{
		QString id = m_filtersListWidget->item(row)->text();
		
		if (mask.contains('*') == true || mask.contains('?') == true)
		{
			// Process wildcard
			//
			QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(mask.trimmed()));

			if (rx.match(id).hasMatch() == true)
			{
				filtersId.append(id);
				filterMatch = true;
			}
		}
		else
		{
			// Process substring
			//
			if (id.contains(mask, Qt::CaseInsensitive) == true)
			{
				filtersId.append(id);
				filterMatch = true;
			}
		}

	}

	fillFilters(filtersId, filterMatch);
}

void PluginTO5MainWindow::createMenu()
{
	auto fileMenu = menuBar()->addMenu(tr("&File"));

	fileMenu->addSeparator();

	QAction* newAction = new QAction(tr("New"), this);
	connect(newAction, &QAction::triggered, this, &PluginTO5MainWindow::onNewComparators);
	fileMenu->addAction(newAction);

	QAction* open = new QAction(tr("Open..."), this);
	connect(open, &QAction::triggered, this, &PluginTO5MainWindow::onOpenComparators);
	fileMenu->addAction(open);

	QAction* save = new QAction(tr("Save"), this);
	connect(save, &QAction::triggered, this, &PluginTO5MainWindow::onSaveComparators);
	fileMenu->addAction(save);

	QAction* saveAs = new QAction(tr("Save As..."), this);
	connect(saveAs, &QAction::triggered, this, &PluginTO5MainWindow::onSaveAsComparators);
	fileMenu->addAction(saveAs);

	QAction* a = new QAction(tr("Exit"), this);
	connect(a, &QAction::triggered, this, &QMainWindow::close);
	fileMenu->addAction(a);


	auto reportsMenu = menuBar()->addMenu(tr("&Reports"));
	QAction* reportByCase = new QAction(tr("Report by CaseID"), this);
	connect(reportByCase,
			&QAction::triggered,
			this,
			[this]()
			{
				onReport(ReportType::ByCaseID);
			});
	reportsMenu->addAction(reportByCase);

	QAction* reportBySchema = new QAction(tr("Report by SchemaID"), this);
	connect(reportBySchema,
			&QAction::triggered,
			this,
			[this]()
			{
				onReport(ReportType::BySchemaID);
			});
	reportsMenu->addAction(reportBySchema);

	auto toolsMenu = menuBar()->addMenu(tr("&Tools"));

	QAction* import = new QAction(tr("Import..."), this);
	connect(import, &QAction::triggered, this, &PluginTO5MainWindow::onImportComparators);
	toolsMenu->addAction(import);

	a = new QAction(tr("Settings..."), this);
	connect(a, &QAction::triggered, this, &PluginTO5MainWindow::onSettings);
	toolsMenu->addAction(a);

	auto helpMenu = menuBar()->addMenu(tr("&?"));

	a = new QAction(tr("Application Log..."), this);
	a->setStatusTip(tr("Show application log"));
	connect(a, &QAction::triggered, this, &PluginTO5MainWindow::showAppLog);
	helpMenu->addAction(a);

	a = new QAction(tr("About..."), this);
	connect(a, &QAction::triggered, this, &PluginTO5MainWindow::showAbout);
	helpMenu->addAction(a);
}

void PluginTO5MainWindow::createFilterListWidget() 
{
	QWidget* listContainer = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(listContainer);

	m_filtersListWidget = new QListWidget();
	m_filtersListWidget->setMinimumWidth(100);

	m_filtersListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_filtersListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_filtersListWidget->setFocusPolicy(Qt::StrongFocus);


	connect(m_filtersListWidget,
			&QListWidget::itemSelectionChanged,
			this,
			[this]()
			{
				QList<QString> schemaIds;

				for (const QModelIndex& index : m_filtersListWidget->selectionModel()->selectedRows(0))
				{
					schemaIds.append(index.data().toString());
				}

				updateTable(schemaIds, static_cast<FilterType>(m_comboBox->currentData().toInt()));
			});
	layout->addWidget(m_filtersListWidget);

	//

	m_comboBox = new QComboBox();

	m_comboBox->addItem(m_comparatorsStorage.FilterTypeStr[static_cast<int>(FilterType::BySchemaID)],
						static_cast<int>(FilterType::BySchemaID));
	m_comboBox->addItem(m_comparatorsStorage.FilterTypeStr[static_cast<int>(FilterType::ByCaseID)], 
						static_cast<int>(FilterType::ByCaseID));

	m_comboBox->setCurrentIndex(QSettings().value("MainWindow/comboBoxIndex", 0).toInt());

	connect(m_comboBox,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			[this]()
			{
				m_filterType = static_cast<FilterType>(m_comboBox->currentData().toInt());
				
				fillFilters();
				
				updateTable({}, m_filterType);
			});
	layout->addWidget(m_comboBox);

	// Mask
	//
	layout->addWidget(new QLabel(tr("SchemaID/CaseID Mask")));

	// Mask field and type combo
	{
		QHBoxLayout* maskLayout = new QHBoxLayout(listContainer);
		maskLayout->setContentsMargins(0, 0, 0, 0);

		m_editMask = new QLineEdit();
		m_editMask->setPlaceholderText(tr("Enter mask (\"*,?\") here"));
		m_editMask->setToolTip(m_maskHelp);
		connect(m_editMask, &QLineEdit::returnPressed, this, &PluginTO5MainWindow::editMaskReturnPressed);
		maskLayout->addWidget(m_editMask);
		m_editMask->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

		layout->addLayout(maskLayout);
	}

	m_splitter->addWidget(listContainer);
}

void PluginTO5MainWindow::createTableWidget()
{
	m_tableHeaders.clear();
	m_tableHeaders << tr("CaseID") << tr("SchemaID") << tr("InputAppSignalID") << tr("InputAppSignalCaption") << tr("OutputAppSignalID")
				   << tr("OutputAppSignalCaption") << tr("Default") << tr("Criteria") << tr("Type") << tr("SetpointAppSignalID")
				   << tr("SetpointAppSignalCaption");

	m_criteriaColumn = m_tableHeaders.indexOf(tr("Criteria"));
	if (m_criteriaColumn == -1) 
	{
		Q_ASSERT(false);
	}

	QWidget* tableContainer = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(tableContainer);

	m_tableWidget = new QTableWidget();
	m_tableWidget->setMinimumWidth(100);

	m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
	m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_tableWidget->setColumnCount(m_tableHeaders.size());
	m_tableWidget->setHorizontalHeaderLabels(m_tableHeaders);

	connect(m_tableWidget, &QTableWidget::itemChanged, this, &PluginTO5MainWindow::onItemCriteriaChanged);

	layout->addWidget(m_tableWidget);

	///

	m_splitter->addWidget(tableContainer);

	///

	m_numericDelegate = new NumericDelegate(m_tableWidget, this);
}

void PluginTO5MainWindow::createStatusBar()
{
	m_statusBarProjectInfo = new QLabel();
	m_statusBarProjectInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarConfigConnection->setMinimumWidth(100);
	m_statusBarConfigConnection->installEventFilter(this);

	m_statusBarAppDataConnection = new QLabel();
	m_statusBarAppDataConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarAppDataConnection->setMinimumWidth(100);
	m_statusBarAppDataConnection->installEventFilter(this);

	m_statusBarLogAlerts = new QLabel();
	m_statusBarLogAlerts->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->installEventFilter(this);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));

	// --
	//
	statusBar()->addPermanentWidget(m_statusBarProjectInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarAppDataConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);
}

void PluginTO5MainWindow::updateStatusBar()
{
	// Status bar
	//
	Q_ASSERT(m_statusBarConfigConnection);
	Q_ASSERT(m_statusBarAppDataConnection);
	Q_ASSERT(m_statusBarLogAlerts);


	std::vector<TcpClientStatistics::Statistics> stats = TcpClientStatistics::statistics();

	// ConfigService
	//
	{
		// ConfigService
		//
		Tcp::ConnectionState configConnState = m_runner.configConnectionState();

		QString text = tr(" ConfigService: ");

		if (configConnState.isConnected == false)
		{
			text += tr(" no connection");
		}
		else
		{
			text += tr("%1").arg(QString::number(configConnState.replyCount));
		}

		if (text != m_statusBarConfigConnection->text())
		{
			m_statusBarConfigConnection->setText(text);
		}

		// --
		//
		QString tooltip = tr("Address: %1").arg(configConnState.peerAddr.toString());

		if (tooltip != m_statusBarConfigConnection->toolTip())
		{
			m_statusBarConfigConnection->setToolTip(tooltip);
		}
	}

	TestSuite::ConfigSettings configuration = m_runner.configuration();

	// AppDataService connection
	//
	/*if (configuration.appDataServices.empty() == false)
	{
		showSoftwareConnection("AppDataService", "TcpSignal", stats, m_statusBarAppDataConnection);
	}*/

	// Log alerts tool
	//

	
	auto&& [testErrorCount, testWarningCount] = m_runner.getLogIssuesCount();

	int errorsCounter = testErrorCount + m_logFile.errorAckCounter();
	int warningsCounter = testWarningCount + m_logFile.warningAckCounter();

	static int m_logErrorsCounter = -1;
	static int m_logWarningsCounter = -1;

	if (m_logErrorsCounter != errorsCounter || m_logWarningsCounter != warningsCounter)
	{
		m_logErrorsCounter = errorsCounter;
		m_logWarningsCounter = warningsCounter;

		assert(m_statusBarLogAlerts);

		m_statusBarLogAlerts->setText(QString(tr(" Log E: %1 W: %2 ")).arg(m_logErrorsCounter).arg(m_logWarningsCounter));

		if (m_logErrorsCounter == 0 && m_logWarningsCounter == 0)
		{
				m_statusBarLogAlerts->setStyleSheet(m_statusBarProjectInfo->styleSheet());
		}
		else
		{
			if (m_logErrorsCounter == 0)
			{
				m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: #F87217}");
			}
			else
			{
				m_statusBarLogAlerts->setStyleSheet(QString("QLabel {color : white; background-color: #C00000}"));
			}
		}
	}
}

void PluginTO5MainWindow::createUi()
{
	QWidget* centralWidget = new QWidget(this);

	QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

	m_splitter = new QSplitter(Qt::Horizontal);
	
	// Create main widgets
	//
	createFilterListWidget();
	createTableWidget();

	// Setup window layout
	//
	m_splitter->setSizes({200, 400});
	mainLayout->addWidget(m_splitter);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addStretch();

	QPushButton* btnReportByCase = new QPushButton(tr("Report by CaseID"), this);
	connect(btnReportByCase,
			&QPushButton::pressed,
			this,
			[this]()
			{
				onReport(ReportType::ByCaseID);
			});
	layout->addWidget(btnReportByCase);

	QPushButton* btnReportBySchema = new QPushButton(tr("Report by SchemaID"), this);
	connect(btnReportBySchema,
			&QPushButton::pressed,
			this,
			[this]()
			{
				onReport(ReportType::BySchemaID);
			});
	layout->addWidget(btnReportBySchema);

	createStatusBar();

	mainLayout->addLayout(layout);
	
	setCentralWidget(centralWidget);

	QHeaderView* header = m_tableWidget->horizontalHeader();

	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, &QHeaderView::customContextMenuRequested, this, &PluginTO5MainWindow::showHeaderContextMenu);

	// Initialize main widgets
	//
	fillFilters();
}

void PluginTO5MainWindow::showHeaderContextMenu(const QPoint& pos)
{
	QMenu menu(this);

	QHeaderView* header = m_tableWidget->horizontalHeader();

	for (int col = 0; col < m_tableWidget->columnCount(); ++col)
	{
		QString columnName = m_tableWidget->horizontalHeaderItem(col)->text();

		QAction* action = menu.addAction(columnName);
		action->setCheckable(true);
		action->setChecked(m_tableWidget->isColumnHidden(col) == false);

		connect(action,
				&QAction::toggled,
				this,
				[this, col](bool visible)
				{
					m_tableWidget->setColumnHidden(col, visible == false);
				});
	}

	menu.exec(header->mapToGlobal(pos));
}

bool PluginTO5MainWindow::loadComparators(const QString& fileName)
{
	QString errorMsg;

	if (m_comparatorsStorage.loadFromFile(fileName, &errorMsg) == true)
	{
		setWindowTitle(qApp->applicationName() + " - " + fileName);
	}
	else
	{
		QMessageBox::critical(this, qAppName(), tr("File %1 load error: %2").arg(fileName).arg(errorMsg));

		setWindowTitle(qApp->applicationName());
		return false;
	}

	return true;
}


void PluginTO5MainWindow::fillFilters(QList<QString> filtersId, bool filterMatch)
{
	// Fill filters list
	//
	m_filtersListWidget->clear();

	if (filtersId.isEmpty() == true || filterMatch == false) 
	{
		// No mask or filter is set
		//
		QList<QString> filteredIds;
		if (m_filterType == FilterType::BySchemaID)
		{
			filteredIds = m_comparatorsStorage.schemaIds();
		}
		else if (m_filterType == FilterType::ByCaseID)
		{
			filteredIds = m_comparatorsStorage.caseIds();
		}
		else
		{
			Q_ASSERT(false);
			return;
		}

		for (int row = 0; row < filteredIds.size(); ++row)
		{
			m_filtersListWidget->addItem(new QListWidgetItem(filteredIds[row]));
		}
	}
	else 
	{
		// A filter or a mask is set
		//

		for (int row = 0; row < filtersId.size(); ++row)
		{
			m_filtersListWidget->addItem(new QListWidgetItem(filtersId[row]));
		}
	}
}

static QTableWidgetItem* createTableItem(const QString& text, bool editable)
{
	QTableWidgetItem* item = new QTableWidgetItem(text);
	if (editable == false)
	{
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	}
	return item;
}

void PluginTO5MainWindow::updateTable(const QList<QString>& filteredIds, FilterType filterType)
{
	// Fill comparators table
	//
	m_tableWidget->clearContents();

	m_tableWidget->setSortingEnabled(false);

	std::vector<Hash> comparatorsVec;
	if (filteredIds.isEmpty() == true)
	{
		comparatorsVec = m_comparatorsStorage.comparatorsHashes();
	}
	else
	{
		comparatorsVec = m_comparatorsStorage.filteredComparatorsHashes(filteredIds, filterType);
	}

	m_tableWidget->setRowCount(static_cast<int>(comparatorsVec.size()));

	bool blockState = m_tableWidget->blockSignals(true);

	int row = 0;
	for (Hash hash : comparatorsVec)
	{
		bool ok = false;
		const ComparatorData& data = m_comparatorsStorage.comparator(hash, &ok);
		if (ok == false) 
		{
			Q_ASSERT(ok);
			return;
		}

		int c = 0;

		m_tableWidget->setItem(row, c++, createTableItem(data.caseID, false));
		m_tableWidget->setItem(row, c++, createTableItem(data.schemaID, false));

		m_tableWidget->setItem(row, c++, createTableItem(data.inputCustomAppSignalID, false));
		m_tableWidget->setItem(row, c++, createTableItem(data.inputSignalCaption, false));
		m_tableWidget->setItem(row, c++, createTableItem(data.outputCustomAppSignalID, false));
		m_tableWidget->setItem(row, c++, createTableItem(data.outputSignalCaption, false));


		m_tableWidget->setItem(row, c++, createTableItem(QString::number(data.defaultValue), false));

		QTableWidgetItem* criteriaItem = createTableItem(QString::number(data.currentValue), true);
		criteriaItem->setData(Qt::UserRole, static_cast<qulonglong>(data.hash));
		if (data.isValueOverriden())
		{
			criteriaItem->setBackground(Qt::yellow);
		}
		m_tableWidget->setItem(row, c++, criteriaItem);

		m_tableWidget->setItem(row, c++, createTableItem(data.type, false));


		m_tableWidget->setItem(row, c++, createTableItem(data.setpointCustomAppSignalID, false));
		m_tableWidget->setItem(row, c++, createTableItem(data.setpointAppSignalCaption, false));

		row++;
	}

	m_tableWidget->blockSignals(blockState);

	if (m_criteriaColumn != -1)
	{
		m_tableWidget->setItemDelegateForColumn(m_criteriaColumn, m_numericDelegate);
	}

	m_tableWidget->setSortingEnabled(true);
}

void PluginTO5MainWindow::showSoftwareConnection(const QString& caption,
												 const QString& nameFilter,
												 const std::vector<TcpClientStatistics::Statistics>& connectionStatistics,
												 QLabel* label)
{
	if (label == nullptr)
	{
		Q_ASSERT(label);
		return;
	}

	QString toolTipText = tr("%1:\n").arg(caption);

	if (connectionStatistics.empty() == true)
	{
		toolTipText += tr("Not configured");
	}

	std::vector<Tcp::ConnectionState> states;
	for (const TcpClientStatistics::Statistics& stats : connectionStatistics)
	{
		if (nameFilter.isEmpty() == true || stats.objectName.startsWith(nameFilter) == true)
		{
			states.push_back(stats.state);
		}
	}

	int statusOk = 0;
	qint64 replyCount = 0;
	for (const Tcp::ConnectionState& state : states)
	{
		if (state.isConnected == true)
		{
			statusOk++;
		}

		replyCount += state.replyCount;

		toolTipText += QString("%1 %2 (%3)\n")
						   .arg(state.connectedSoftwareInfo.equipmentID())
						   .arg(state.peerAddr.addressPortStr())
						   .arg(state.isConnected ? tr("ok") : tr("down"));
	}
	toolTipText = toolTipText.trimmed();

	label->setText(caption);
	label->setToolTip(toolTipText);

	QString statusText;

	if (states.size() <= 1)
	{
		statusText = tr("%1: %2 (Replies: %3)").arg(caption).arg(statusOk ? tr("ok") : tr("down")).arg(replyCount);
	}
	else
	{
		statusText = tr("%1: %2/%3 (Replies: %4)").arg(caption).arg(statusOk).arg(states.size()).arg(replyCount);
	}

	label->setText(statusText);

	return;
}

bool PluginTO5MainWindow::eventFilter(QObject* object, QEvent* event)
{
	if (object == m_statusBarLogAlerts && m_statusBarLogAlerts->text().isEmpty() == false && event->type() == QEvent::MouseButtonPress)
	{
		showAppLog();
	}

	return QWidget::eventFilter(object, event);
}

void PluginTO5MainWindow::closeEvent(QCloseEvent* event)
{
	if (m_hasUnsavedChanges == false)
	{
		event->accept();
		return;
	}

	QMessageBox::StandardButton reply = QMessageBox::question(this,
															  tr("Unsaved Changes"),
															  tr("You have unsaved changes.\nDo you want to save them?"),
															  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
															  QMessageBox::Save);

	switch (reply)
	{
	case QMessageBox::Save:
		{
			onSaveComparators();
			event->accept();
			break;
		}

	case QMessageBox::Discard:
		{
			event->accept();
			break;
		}

	case QMessageBox::Cancel:
	default:
		{
			event->ignore();
			break;
		}
	}
}

void PluginTO5MainWindow::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == m_mainWindowTimerId_250ms)
	{
		updateStatusBar();
	}
}