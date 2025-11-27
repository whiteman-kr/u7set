#include "SignalSnapshotWidget.h"
#include "../AppSignalLib/IAppSignalManager.h"

#include <AppSignalLists/SignalList.h>
#include <AppSignalLibStd/ISignalDataServer.h>
#include <SchemaClientLib/DialogSignalSnapshot.h>
#include <UiLib/ChooseItemsWidget.h>

#include <ReportLib/ReportObject.h>
#include <ReportLib/TableViewReportGenerator.h>

//
// SnapshotReportGenerator
//
namespace
{
	class SnapshotReportInfo : public ReportLib::ITableViewReportInfo
	{
	public:
		SnapshotReportInfo(const QString& projectName, const QString& softwareEquipmentId);

	protected:
		virtual void generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) const override;

	private:
		QString m_projectName;
		QString m_equipmentId;
	};

	//
	// SnapshotReportInfo
	//
	SnapshotReportInfo::SnapshotReportInfo(const QString& projectName,
										   const QString& softwareEquipmentId) :
		m_projectName(projectName),
		m_equipmentId(softwareEquipmentId)
	{
	}

	void SnapshotReportInfo::generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) const
	{
		ReportLib::ReportFont marginFont{"Arial", 10};

		report.addMarginItem({QObject::tr("Generated: %1").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss")),
							  -1,
							  -1,
							  {marginFont, Qt::AlignLeft | Qt::AlignTop}});
		
		report.addMarginItem({QObject::tr("Signals Snapshot"), -1, -1, {marginFont, Qt::AlignCenter | Qt::AlignTop}});

		report.addMarginItem({QObject::tr("Project: %1").arg(m_projectName), -1, -1, {marginFont, Qt::AlignRight | Qt::AlignTop}});

		report.addMarginItem(
			{QObject::tr("%1: %2").arg(qAppName()).arg(m_equipmentId), -1, -1, {marginFont, Qt::AlignLeft | Qt::AlignBottom}});

		report.addMarginItem({"%PAGE%", -1, -1, {marginFont, Qt::AlignRight | Qt::AlignBottom}});

		ReportLib::ReportFont textFont{"Arial", 12};
		mainSection.addText(" \n", {textFont, Qt::AlignLeft});
	}
} // namespace

//
// SnapshotTableView
//
namespace SchemaClientLib
{
	void SnapshotTableView::mousePressEvent(QMouseEvent* event)
	{
		QTableView::mousePressEvent(event);

		SignalSnapshotModel* snapshotModel = dynamic_cast<SignalSnapshotModel*>(model());
		if (snapshotModel == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		QList<AppSignalParam> appSignalParams;

		QModelIndexList rows = selectionModel()->selectedRows();

		for (QModelIndex& index : rows)
		{
			bool found = false;

			AppSignalParam appSignalParam = snapshotModel->signalParam(index.row(), &found);

			if (found == true)
			{
				appSignalParams.push_back(appSignalParam);
			}
		}

		m_dragDropHelper.onMousePress(event, appSignalParams);

		return;
	}

	void SnapshotTableView::mouseMoveEvent(QMouseEvent* event)
	{
		m_dragDropHelper.onMouseMove(event, this);

		return;
	}
} // namespace SchemaClientLib

//
// DialogSignalSnapshotSettings
//
namespace SchemaClientLib
{
	void DialogSignalSnapshotSettings::restore()
	{
		QSettings s;

		horzHeader = s.value("SignalSnapshotWidget/horzHeader").toByteArray();
		horzHeaderCount = s.value("SignalSnapshotWidget/horzHeaderCount").toInt();

		maskList = s.value("SignalSnapshotWidget/mask").toStringList();

		sortColumn = s.value("SignalSnapshotWidget/sortColumn", sortColumn).toInt();
		sortOrder = static_cast<Qt::SortOrder>(s.value("SignalSnapshotWidget/sortOrder", sortOrder).toInt());
	}

	void DialogSignalSnapshotSettings::store()
	{
		QSettings s;

		s.setValue("SignalSnapshotWidget/horzHeader", horzHeader);
		s.setValue("SignalSnapshotWidget/horzHeaderCount", horzHeaderCount);

		s.setValue("SignalSnapshotWidget/sortColumn", sortColumn);
		s.setValue("SignalSnapshotWidget/sortOrder", static_cast<int>(sortOrder));
	}
} // namespace SchemaClientLib

//
// SignalSnapshotWidget
//
namespace SchemaClientLib
{
	SignalSnapshotWidget::SignalSnapshotWidget(SchemaClientLib::ISignalSnapshotWidget& signalSnapshotVirtFuncDispatcher,
											   IAppSignalManager* appSignalManager,
											   ClientLib::ISignalDataServer* signalDataServer,
											   AppSignalLists::AppSignalListSet* appSignalListSet,
											   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,
											   const QString& projectName,
											   const QString& equipmentId,
											   QWidget* parent) :
		QWidget(parent),
		m_signalSnapshotVirtFuncDispatcher(signalSnapshotVirtFuncDispatcher),
		m_appSignalManager(appSignalManager),
		m_signalDataServer(signalDataServer),
		m_appSignalListSet(appSignalListSet),
		m_model(m_appSignalManager, m_signalDataServer, m_appSignalListSet, this),
		m_appDataServices(appDataServices),
		m_projectName(projectName),
		m_equipmentId(equipmentId)
	{
		if (m_appSignalManager == nullptr)
		{
			Q_ASSERT(m_appSignalManager);
			return;
		}

		m_maskHelp = tr("A mask contains '*' and '?' symbols.\n\
	'*' symbol means any set of symbols on its place, '?' symbol means one symbol on its place.\n\
	Several masks can be separated by semicolon or space.\n\n\
	Examples:\n\n\
	#SF001P014* (mask for AppSignalID),\n\
	T?30T01? (mask for CustomAppSignalID),\n\
	#SYSTEMID_RACK01_CH01_MD?? (mask for Equipment ID).\n\n\
	To apply the filter, enter the mask and press Enter.");
		m_maskHelp.remove('\t');

		m_tagsHelp = tr("Tags for filtering signals.\n\n\
	Several tags can be separated by semicolon or space: \"tag1; tag2\" or \"tag1 tag2\".\n\n\
	To apply the filter, enter tags and press Enter.");
		m_tagsHelp.remove('\t');

		m_settings.restore();

		setAttribute(Qt::WA_DeleteOnClose);
		setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

		setWindowTitle(tr("Signals Snapshot"));

		createControls();
		createMenus();

		initSignalsView();
		initFiltersView();

		if (m_appSignalListSet != nullptr)
		{
			connect(m_appSignalListSet,
					&AppSignalLists::AppSignalListSet::updatePerformed,
					this,
					&SignalSnapshotWidget::fillAppSignalLists);
		}

		m_updateStateTimerId = startTimer(500);

		return;
	}

	SignalSnapshotWidget::SignalSnapshotWidget(SchemaClientLib::ISignalSnapshotWidget& signalSnapshotVirtFuncDispatcher,
											   IAppSignalManager* appSignalManager,
											   const QString& projectName,
											   const QString& equipmentId,
											   QWidget* parent) :
		SignalSnapshotWidget(signalSnapshotVirtFuncDispatcher, appSignalManager, nullptr, nullptr, {}, projectName, equipmentId, parent)
	{
	}

	SignalSnapshotWidget::~SignalSnapshotWidget()
	{
		killTimer(m_updateStateTimerId);
		
		// Save state
		// 
		if (m_storeType == true)
		{
			m_storedType = static_cast<SnapshotSignalType>(m_typeCombo->currentIndex());
		}

		if (m_storeRole == true)
		{
			m_storedRole = static_cast<SnapshotSignalRole>(m_roleCombo->currentIndex());
		}

		if (m_storeMaskData == true)
		{
			m_storedMaskType = static_cast<SnapshotMaskType>(m_maskTypeCombo->currentIndex());
		}

		m_settings.horzHeader = m_tableView->horizontalHeader()->saveState();
		m_settings.horzHeaderCount = static_cast<int>(SnapshotColumns::ColumnCount);

		m_settings.store();

		return;
	}

	QString SignalSnapshotWidget::projectName() const
	{
		return m_projectName;
	}

	void SignalSnapshotWidget::setProjectName(const QString& projectName)
	{
		m_projectName = projectName;
	}

	const std::vector<AppSignalParam>& SignalSnapshotWidget::specificSignals() const
	{
		return m_specificSignals;
	}

	void SignalSnapshotWidget::setSpecificSignals(const std::vector<AppSignalParam>& specificSignals)
	{
		m_specificSignals = specificSignals;

		onSignalsUpdated();

		return;
	}

	void SignalSnapshotWidget::setLmEquipmentId(const QString& lmEquipmentId)
	{
		if (m_editMask == nullptr || m_maskTypeCombo == nullptr)
		{
			Q_ASSERT(m_editMask);
			Q_ASSERT(m_maskTypeCombo);
			return;
		}

		// Set Mask(s)
		//
		m_editMask->setText(lmEquipmentId);

		maskChanged(false /*addToCompleter*/);

		// Set Mask Type combo to All and prevent saving it in settings
		//
		m_storeMaskData = false;

		m_maskTypeCombo->blockSignals(true); // Block to prevent signals from updating automatically
		m_maskTypeCombo->setCurrentIndex(static_cast<int>(SnapshotMaskType::LmEquipmentId));
		m_maskTypeCombo->blockSignals(false);

		return;
	}

	void SignalSnapshotWidget::setSignalsMask(const QStringList& masks)
	{
		if (m_editMask == nullptr || m_maskTypeCombo == nullptr)
		{
			Q_ASSERT(m_editMask);
			Q_ASSERT(m_maskTypeCombo);
			return;
		}

		// Set Mask(s)
		//
		m_editMask->setText(masks.join(';'));

		maskChanged(false /*addToCompleter*/);

		// Set Mask Type combo to All and prevent saving it in settings
		//
		m_storeMaskData = false;

		m_maskTypeCombo->blockSignals(true); // Block to prevent signals from updating automatically
		m_maskTypeCombo->setCurrentIndex(static_cast<int>(SnapshotMaskType::All));
		m_maskTypeCombo->blockSignals(false);

		return;
	}

	void SignalSnapshotWidget::setSignalsTags(const QStringList& tags)
	{
		if (m_editTags == nullptr)
		{
			Q_ASSERT(m_editTags);
			return;
		}

		m_editTags->setText(tags.join(' '));

		tagsChanged(false /*addToCompleter*/);
	}

	void SignalSnapshotWidget::resetSignalsType()
	{
		// Set Type is set automatically
		//
		m_storeType = false;
		m_storeRole = false;
		m_model.setSignalType(SnapshotSignalType::Any);
		m_model.setSignalRole(SnapshotSignalRole::Any);

		m_typeCombo->blockSignals(true);
		m_typeCombo->setCurrentIndex(static_cast<int>(SnapshotSignalType::Any));
		m_typeCombo->blockSignals(false);

		m_roleCombo->blockSignals(true);
		m_roleCombo->setCurrentIndex(static_cast<int>(SnapshotSignalRole::Any));
		m_roleCombo->blockSignals(false);
	}

	std::vector<VFrame30::SchemaDetails> SignalSnapshotWidget::schemasDetails()
	{
		return m_signalSnapshotVirtFuncDispatcher.schemasDetails();
	}

	std::set<QString> SignalSnapshotWidget::schemaAppSignals(const QString& schemaStrId)
	{
		return m_signalSnapshotVirtFuncDispatcher.schemaAppSignals(schemaStrId);
	}

	void SignalSnapshotWidget::showEvent([[maybe_unused]] QShowEvent* event)
	{
		if (m_firstShow == false)
		{
			return;
		}

		m_firstShow = false;
		
		fillSignals();

		return;
	}

	void SignalSnapshotWidget::keyPressEvent(QKeyEvent* event)
	{
		int key = event->key();

		if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Escape)
		{
			event->ignore();
		}
		else
		{
			QWidget::keyPressEvent(event);
		}

		return;
	}

	void SignalSnapshotWidget::timerEvent(QTimerEvent* event)
	{
		if (event->timerId() == m_updateStateTimerId)
		{
			if (m_buttonFixate->isChecked() == false && m_model.rowCount() > 0)
			{
				updateTableItems();
			}
		}
	}

	void SignalSnapshotWidget::onSignalsUpdated()
	{
		bool emptyModel = m_model.rowCount() == 0;

		// Set new signals to the model
		//
		if (specificSignals().empty() == true)
		{
			std::vector<AppSignalParam> allSignals = m_appSignalManager->signalList();
			m_model.setSignals(allSignals);
		}
		else
		{
			std::vector<AppSignalParam> specSignals = specificSignals();
			m_model.setSignals(specSignals);
		}

		// Refresh signals list
		//

		fillSignals();

		if (emptyModel == true && m_model.rowCount() > 0)
		{
			m_tableView->resizeColumnsToContents();
		}

		emit signalsUpdated();

		return;
	}

	void SignalSnapshotWidget::headerColumnContextMenuRequested(const QPoint& pos)
	{
		QMenu menu(this);

		QList<QAction*> actions;

		std::vector<std::pair<SnapshotColumns, QString>> actionsData;
		actionsData.reserve(static_cast<int>(SnapshotColumns::ColumnCount));

		SignalSnapshotModel* model = dynamic_cast<SignalSnapshotModel*>(m_tableView->model());
		if (model == nullptr)
		{
			Q_ASSERT(model);
			return;
		}

		QStringList columns = model->columnsNames();

		for (int i = 0; i < columns.size(); i++)
		{
			actionsData.emplace_back(static_cast<SnapshotColumns>(i), columns[i]);
		}

		for (std::pair<SnapshotColumns, QString> ad : actionsData)
		{
			QAction* action = new QAction(ad.second, this);
			action->setData(QVariant::fromValue(ad.first));
			action->setCheckable(true);
			action->setChecked(!m_tableView->horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

			if (m_tableView->horizontalHeader()->count() - m_tableView->horizontalHeader()->hiddenSectionCount() == 1 &&
				action->isChecked() == true)
			{
				action->setEnabled(false); // Impossible to uncheck the last column
			}

			connect(action, &QAction::toggled, this, &SignalSnapshotWidget::headerColumnToggled);

			actions << action;
		}

		menu.exec(actions, mapToGlobal(pos), 0, this);
		return;
	}

	void SignalSnapshotWidget::headerColumnToggled(bool checked)
	{
		QAction* action = dynamic_cast<QAction*>(sender());

		if (action == nullptr)
		{
			Q_ASSERT(action);
			return;
		}

		int column = action->data().value<int>();

		if (column >= static_cast<int>(SnapshotColumns::ColumnCount))
		{
			Q_ASSERT(column < static_cast<int>(SnapshotColumns::ColumnCount));
			return;
		}

		if (checked == true)
		{
			m_tableView->showColumn(column);
		}
		else
		{
			m_tableView->hideColumn(column);
		}

		return;
	}

	void SignalSnapshotWidget::contextMenuRequested(const QPoint& pos)
	{
		Q_UNUSED(pos);

		int row = m_tableView->currentIndex().row();
		if (row == -1)
		{
			return;
		}

		int rowIndex = m_tableView->currentIndex().row();

		bool found = false;

		const AppSignalParam& s = m_model.signalParam(rowIndex, &found);

		if (found == false)
		{
			return;
		}

		QStringList list;
		list << s.appSignalId();

		// Check analog format options

		m_formatAutoSelect->setChecked(m_model.analogFormat() == E::AnalogFormat::g_9_or_9e ||
									   m_model.analogFormat() == E::AnalogFormat::G_9_or_9E);
		m_formatDecimal->setChecked(m_model.analogFormat() == E::AnalogFormat::f_9);
		m_formatExponential->setChecked(m_model.analogFormat() == E::AnalogFormat::e_9e ||
										m_model.analogFormat() == E::AnalogFormat::E_9E);

		m_precisionDefault->setChecked(m_model.analogPrecision() == -1);

		for (int i = 0; i < static_cast<int>(m_precisionActions.size()); i++)
		{
			m_precisionActions[i]->setChecked(m_model.analogPrecision() == i);
		}

		//

		emit signalContextMenu(list, QList<QMenu*>() << &m_formatMenu);
	}

	void SignalSnapshotWidget::tableViewdoubleClicked(const QModelIndex& index)
	{
		Q_UNUSED(index);

		int row = m_tableView->currentIndex().row();
		if (row == -1)
		{
			return;
		}

		int rowIndex = m_tableView->currentIndex().row();

		bool found = false;

		const AppSignalParam& s = m_model.signalParam(rowIndex, &found);

		if (found == false)
		{
			return;
		}

		QTimer::singleShot(10,
						   [this, s]
						   {
							   emit signalInfo(s.appSignalId());
						   });
	}

	void SignalSnapshotWidget::sortIndicatorChanged(int column, Qt::SortOrder order)
	{
		m_settings.sortColumn = column;
		m_settings.sortOrder = order;
	}

	void SignalSnapshotWidget::typeComboCurrentIndexChanged(int index)
	{
		m_storeType = true;
		m_model.setSignalType(static_cast<SnapshotSignalType>(index));

		fillSignals();
	}

	void SignalSnapshotWidget::roleComboCurrentIndexChanged(int index)
	{
		m_storeRole = true;
		m_model.setSignalRole(static_cast<SnapshotSignalRole>(index));

		fillSignals();
	}

	void SignalSnapshotWidget::editMaskReturnPressed()
	{
		m_storeMaskData = true;
		maskChanged(true /*addToCompleter*/);

		fillSignals();
	}

	void SignalSnapshotWidget::editTagsReturnPressed()
	{
		tagsChanged(true /*addToCompleter*/);

		fillSignals();
	}

	void SignalSnapshotWidget::maskTypeComboCurrentIndexChanged(int index)
	{
		m_storeMaskData = true;

		m_model.setMaskType(static_cast<SnapshotMaskType>(index));

		QString mask = m_editMask->text();
		if (mask.isEmpty() == true)
		{
			return;
		}

		fillSignals();
	}

	void SignalSnapshotWidget::serverComboIndexChanged(int /*index*/)
	{
		m_model.setDataServiceId(m_serverCombo->currentData().toString());
		fillSignals();
	}

	void SignalSnapshotWidget::signalListComboIndexChanged(int /*index*/) 
	{
		m_model.setAppSignalList(m_signalListCombo->currentData().toString());
		fillSignals();
	}

	void SignalSnapshotWidget::buttonExportClicked()
	{
		if (m_model.rowCount() == 0)
		{
			QMessageBox::warning(this, qAppName(), tr("Nothing to export."));
			return;
		}

		static QString path{"."};

		QFileDialog dialog(
			this,
			tr("Save File"),
			path + QDir::separator() + "untitled.pdf",
			tr("Portable Document Format (*.pdf);;CSV Files, semicolon separated (*.csv);;Plaintext (*.txt);;HTML (*.html)"));
		dialog.setOption(QFileDialog::DontUseNativeDialog, true);
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setViewMode(QFileDialog::List);

		// Create your checkbox
		//
		QCheckBox* customCheck = new QCheckBox(tr("Export Selected Only"), &dialog);
		customCheck->setEnabled(m_tableView->selectionModel()->hasSelection() == true);

		// Access the dialog's layout and insert the checkbox
		//
		QGridLayout* layout = qobject_cast<QGridLayout*>(dialog.layout());
		if (layout)
		{
			int row = layout->rowCount();
			layout->addWidget(customCheck, row, 0, 1, layout->columnCount());
		}

		// Execute dialog
		if (dialog.exec() != QDialog::Accepted)
		{
			return;
		}
		QStringList files = dialog.selectedFiles();
		if (files.isEmpty())
		{
			return;
		}
		QString fileName = files[0];
		path = QFileInfo(fileName).path(); // store path for next time

		bool exportSelected = customCheck->isChecked();

		QFileInfo fileInfo(fileName);
		QString extension = fileInfo.completeSuffix();

		if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0 ||
			extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 ||
			extension.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
			extension.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 ||
			extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
		{
			QPageLayout pageLayout(QPageSize(QPageSize::A4),
								   QPageLayout::Orientation::Portrait,
								   QMarginsF(25, 20, 15, 20),
								   QPageLayout::Unit::Millimeter);

			pageLayout = ReportLib::TableViewReportGenerator::loadPageLayoutFromSettings("SnapshotExportPageLayout", pageLayout);

			SnapshotReportInfo ri(m_projectName, m_equipmentId);

			ReportLib::TableViewReportGenerator generator(this, *m_tableView, ri, pageLayout, exportSelected);

			generator.exportTable(fileName);

			pageLayout = generator.pageLayout();
			ReportLib::TableViewReportGenerator::savePageLayoutToSettings(pageLayout, "SnapshotExportPageLayout");
		}
		else
		{
			QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));
		}

		return;
	}

	void SignalSnapshotWidget::buttonPrintClicked()
	{
		if (m_tableView->selectionModel()->hasSelection() == true)
		{
			QMenu menu;

			QAction* all = new QAction(tr("Print All"), &menu);
			menu.addAction(all);
			connect(all,
					&QAction::triggered,
					this,
					[this]()
					{
						printData(false /*printSelected*/);
					});

			QAction* sel = new QAction(tr("Print Selected"), &menu);
			connect(sel,
					&QAction::triggered,
					this,
					[this]()
					{
						printData(true /*printSelected*/);
					});
			menu.addAction(sel);

			menu.exec(QCursor::pos());
		}
		else
		{
			printData(false /*printSelected*/);
		}

		return;
	}

	void SignalSnapshotWidget::buttonChooseTagsClicked()
	{
		QDialog tagsSelectorDialog{this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint};

		int width = QSettings().value("SignalSnapshotWidget/tagsSelectorDialog/width", 340).toInt();
		int height = QSettings().value("SignalSnapshotWidget/tagsSelectorDialog/height", 400).toInt();
		tagsSelectorDialog.resize(width, height);

		UiLib::ChooseItemsWidget te{m_appSignalManager->tags(), this};
		te.setText(m_editTags->text());

		connect(&te, &UiLib::ChooseItemsWidget::okPressed, &tagsSelectorDialog, &QDialog::accept);
		connect(&te, &UiLib::ChooseItemsWidget::cancelPressed, &tagsSelectorDialog, &QDialog::reject);

		QHBoxLayout l;
		l.addWidget(&te);
		tagsSelectorDialog.setLayout(&l);

		if (tagsSelectorDialog.exec() == QDialog::Accepted)
		{
			m_editTags->setText(te.text());

			tagsChanged(true /*addToCompleter*/);

			fillSignals();
		}

		QSettings().setValue("SignalSnapshotWidget/tagsSelectorDialog/width", tagsSelectorDialog.width());
		QSettings().setValue("SignalSnapshotWidget/tagsSelectorDialog/height", tagsSelectorDialog.height());
	}

	void SignalSnapshotWidget::buttonClearFilterClicked()
	{
		// Type
		//
		m_typeCombo->blockSignals(true);
		m_typeCombo->setCurrentIndex(static_cast<int>(SnapshotSignalType::Any));
		m_typeCombo->blockSignals(false);
		m_model.setSignalType(SnapshotSignalType::Any);

		// Role
		//
		m_roleCombo->blockSignals(true);
		m_roleCombo->setCurrentIndex(static_cast<int>(SnapshotSignalRole::Any));
		m_roleCombo->blockSignals(false);
		m_model.setSignalRole(SnapshotSignalRole::Any);

		// Mask
		//
		m_editMask->blockSignals(true);
		m_editMask->clear();
		m_editMask->blockSignals(false);

		m_maskTypeCombo->blockSignals(true); // Block to prevent signals from updating automatically
		m_maskTypeCombo->setCurrentIndex(static_cast<int>(SnapshotMaskType::All));
		m_maskTypeCombo->blockSignals(false);

		m_model.setMasks({});

		// Server
		//
		m_serverCombo->blockSignals(true);
		m_serverCombo->setCurrentIndex(0);
		m_serverCombo->blockSignals(false);
		m_model.setDataServiceId({});

		// List
		//
		m_signalListCombo->blockSignals(true);
		m_signalListCombo->setCurrentIndex(0);
		m_signalListCombo->blockSignals(false);
		m_model.setAppSignalList({});

		// Tags
		//
		m_editTags->blockSignals(true);
		m_editTags->clear();
		m_editTags->blockSignals(false);

		m_model.setTags({});

		//
		fillSignals();
	}

	void SignalSnapshotWidget::createControls()
	{
		// Filter layout

		QGridLayout* filterLayout = new QGridLayout();

		int row = 0;
		int col = 0;

		filterLayout->addWidget(new QLabel(tr("Type")), row, col++);

		{
			QHBoxLayout* typeRoleLayout = new QHBoxLayout();
			typeRoleLayout->setContentsMargins(0, 0, 0, 0);

			m_typeCombo = new QComboBox();
			connect(m_typeCombo,
					static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this,
					&SignalSnapshotWidget::typeComboCurrentIndexChanged);
			typeRoleLayout->addWidget(m_typeCombo);
			m_typeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			typeRoleLayout->addWidget(new QLabel(tr("Role")));

			m_roleCombo = new QComboBox();
			connect(m_roleCombo,
					static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this,
					&SignalSnapshotWidget::roleComboCurrentIndexChanged);
			typeRoleLayout->addWidget(m_roleCombo);
			m_roleCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			filterLayout->addLayout(typeRoleLayout, row, col++);
		}

		// Mask
		//
		filterLayout->addWidget(new QLabel(tr("Mask")), row, col++);

		// Mask field and type combo
		{
			QHBoxLayout* maskLayout = new QHBoxLayout();
			maskLayout->setContentsMargins(0, 0, 0, 0);

			m_editMask = new QLineEdit();
			m_editMask->setPlaceholderText("Enter mask (\"*,?\") here");
			connect(m_editMask, &QLineEdit::returnPressed, this, &SignalSnapshotWidget::editMaskReturnPressed);
			maskLayout->addWidget(m_editMask);
			m_editMask->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

			m_maskTypeCombo = new QComboBox();
			connect(m_maskTypeCombo,
					static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this,
					&SignalSnapshotWidget::maskTypeComboCurrentIndexChanged);
			maskLayout->addWidget(m_maskTypeCombo);

			filterLayout->addLayout(maskLayout, row, col++);
		}

		// Server
		//
		filterLayout->addWidget(new QLabel(tr("Server")), row, col++);

		// Server Combo
		//
		m_serverCombo = new QComboBox();
		connect(m_serverCombo,
				static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				this,
				&SignalSnapshotWidget::serverComboIndexChanged);
		filterLayout->addWidget(m_serverCombo, row, col++);
		m_serverCombo->setMinimumContentsLength(20);

		row++;
		col = 0;

		// Signal List
		//
		filterLayout->addWidget(new QLabel(tr("List")), row, col++);

		// Signal List Combo
		//
		m_signalListCombo = new QComboBox();
		connect(m_signalListCombo,
				static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				this,
				&SignalSnapshotWidget::signalListComboIndexChanged);
		filterLayout->addWidget(m_signalListCombo, row, col++);
		m_signalListCombo->setMinimumContentsLength(30);
		
		// Tags
		//
		filterLayout->addWidget(new QLabel(tr("Tags")), row, col++);

		// Tags field and button
		//
		{
			QHBoxLayout* tagsLayout = new QHBoxLayout();
			tagsLayout->setSpacing(0);
			tagsLayout->setContentsMargins(0, 0, 0, 0);

			m_editTags = new QLineEdit();
			m_editTags->setPlaceholderText("Signal tags space separated");
			connect(m_editTags, &QLineEdit::returnPressed, this, &SignalSnapshotWidget::editTagsReturnPressed);
			tagsLayout->addWidget(m_editTags);
			m_editTags->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

			m_buttonChooseTags = new QToolButton();
			connect(m_buttonChooseTags, &QToolButton::clicked, this, &SignalSnapshotWidget::buttonChooseTagsClicked);
			m_buttonChooseTags->setText("...");
			tagsLayout->addWidget(m_buttonChooseTags);

			filterLayout->addLayout(tagsLayout, row, col++);
		}

		filterLayout->addWidget(new QWidget(this), row, col++);

		QHBoxLayout* clearFilterLayout = new QHBoxLayout();
		clearFilterLayout->addStretch();

		m_clearFilterButton = new QPushButton(tr("Reset Filter"));
		m_clearFilterButton->setAutoDefault(false);
		clearFilterLayout->addWidget(m_clearFilterButton);
		connect(m_clearFilterButton, &QToolButton::clicked, this, &SignalSnapshotWidget::buttonClearFilterClicked);

		filterLayout->addLayout(clearFilterLayout, row, col++);

		filterLayout->setSpacing(4);

		filterLayout->setColumnStretch(0, 0);
		filterLayout->setColumnStretch(1, 0);
		filterLayout->setColumnStretch(2, 0);
		filterLayout->setColumnStretch(3, 2);
		filterLayout->setColumnStretch(4, 0);
		filterLayout->setColumnStretch(5, 0);

		// Export/Print/Fixate

		QHBoxLayout* exPrintLayout = new QHBoxLayout();

		QPushButton* b = new QPushButton(tr("Export..."));
		b->setAutoDefault(false);
		connect(b, &QPushButton::clicked, this, &SignalSnapshotWidget::buttonExportClicked);
		exPrintLayout->addWidget(b);

		b = new QPushButton(tr("Print..."));
		b->setAutoDefault(false);
		connect(b, &QPushButton::clicked, this, &SignalSnapshotWidget::buttonPrintClicked);
		exPrintLayout->addWidget(b);

		exPrintLayout->addStretch();

		m_buttonFixate = new QPushButton(tr("Fixate"));
		m_buttonFixate->setAutoDefault(false);
		m_buttonFixate->setCheckable(true);
		exPrintLayout->addWidget(m_buttonFixate);

		// Table

		m_tableView = new SnapshotTableView();
		connect(m_tableView, &QTableView::doubleClicked, this, &SignalSnapshotWidget::tableViewdoubleClicked);

		// Main layout

		QVBoxLayout* mainLayout = new QVBoxLayout();

		mainLayout->addLayout(filterLayout);
		mainLayout->addLayout(exPrintLayout);
		mainLayout->addWidget(m_tableView);

		setLayout(mainLayout);

		return;
	}

	void SignalSnapshotWidget::createMenus()
	{
		// Analog Format and precision

		QMenu* menuFormat = m_formatMenu.addMenu(tr("Format"));

		m_formatAutoSelect = new QAction(tr("Auto-select"), this);
		m_formatAutoSelect->setCheckable(true);
		connect(m_formatAutoSelect,
				&QAction::triggered,
				this,
				[this]()
				{
					m_model.setAnalogFormat(E::AnalogFormat::g_9_or_9e);
					updateTableItems();
				});
		menuFormat->addAction(m_formatAutoSelect);

		m_formatDecimal = new QAction(tr("Decimal (as [-]9.9)"), this);
		m_formatDecimal->setCheckable(true);
		connect(m_formatDecimal,
				&QAction::triggered,
				this,
				[this]()
				{
					m_model.setAnalogFormat(E::AnalogFormat::f_9);
					updateTableItems();
				});
		menuFormat->addAction(m_formatDecimal);

		m_formatExponential = new QAction(tr("Exponential (as [-]9.9e[+|-]999)"), this);
		m_formatExponential->setCheckable(true);
		connect(m_formatExponential,
				&QAction::triggered,
				this,
				[this]()
				{
					m_model.setAnalogFormat(E::AnalogFormat::e_9e);
					updateTableItems();
				});
		menuFormat->addAction(m_formatExponential);

		menuFormat->addSeparator();

		m_precisionDefault = new QAction(tr("Default"), this);
		m_precisionDefault->setCheckable(true);
		connect(m_precisionDefault,
				&QAction::triggered,
				this,
				[this]()
				{
					m_model.setAnalogPrecision(-1);
					updateTableItems();
				});
		menuFormat->addAction(m_precisionDefault);

		for (int i = 0; i < 10; i++)
		{
			QAction* a = new QAction(tr(".%1").arg(QString().fill('0', i)), this);
			a->setCheckable(true);
			connect(a,
					&QAction::triggered,
					this,
					[this, i]()
					{
						m_model.setAnalogPrecision(i);
						updateTableItems();
					});
			m_precisionActions << a;
			menuFormat->addAction(a);
		}

		return;
	}

	void SignalSnapshotWidget::initFiltersView()
	{
		// Type combo setup
		//
		m_typeCombo->blockSignals(true);
		m_typeCombo->addItem(tr("Any"), static_cast<int>(SnapshotSignalType::Any));
		m_typeCombo->addItem(tr("Analog"), static_cast<int>(SnapshotSignalType::Analog));
		m_typeCombo->addItem(tr("Discrete"), static_cast<int>(SnapshotSignalType::Discrete));
		m_typeCombo->setCurrentIndex(static_cast<int>(m_storedType));
		m_typeCombo->blockSignals(false);

		// Role combo setup
		//
		m_roleCombo->blockSignals(true);
		m_roleCombo->addItem(tr("Any"), static_cast<int>(SnapshotSignalRole::Any));
		m_roleCombo->addItem(tr("Input"), static_cast<int>(SnapshotSignalRole::Input));
		m_roleCombo->addItem(tr("Output"), static_cast<int>(SnapshotSignalRole::Output));
		m_roleCombo->addItem(tr("Internal"), static_cast<int>(SnapshotSignalRole::Internal));
		m_roleCombo->addItem(tr("Tunable"), static_cast<int>(SnapshotSignalRole::Tunable));
		m_roleCombo->setCurrentIndex(static_cast<int>(m_storedRole));
		m_roleCombo->blockSignals(false);

		// Masks setup
		//
		m_maskCompleter = new QCompleter(m_settings.maskList, this);
		m_maskCompleter->setCaseSensitivity(Qt::CaseInsensitive);

		m_editMask->setCompleter(m_maskCompleter);
		m_editMask->setToolTip(m_maskHelp);

		m_maskTypeCombo->blockSignals(true);
		m_maskTypeCombo->addItem(tr("All"));
		m_maskTypeCombo->addItem(tr("AppSignalID"));
		m_maskTypeCombo->addItem(tr("CustomAppSignalID"));
		m_maskTypeCombo->addItem(tr("EquipmentID"));
		m_maskTypeCombo->addItem(tr("LmEquipmentID"));
		m_maskTypeCombo->setCurrentIndex(static_cast<int>(m_storedMaskType));
		m_maskTypeCombo->blockSignals(false);

		connect(m_editMask,
				&QLineEdit::textEdited,
				[this]()
				{
					m_maskCompleter->complete();
				});
		connect(m_maskCompleter,
				static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted),
				m_editMask,
				&QLineEdit::setText);

		// Servers setup
		//
		m_serverCombo->blockSignals(true);
		m_serverCombo->addItem(tr("All Servers"), QString());
		for (const auto& ads : m_appDataServices)
		{
			m_serverCombo->addItem(ads.shortenId, ads.equipmentId);
		}
		if (m_appDataServices.empty() == true)
		{
			m_serverCombo->setEnabled(false);
		}
		m_serverCombo->blockSignals(false);

		// Signal Lists setup
		//
		fillAppSignalLists();

		// Tags setup
		//
		m_tagsCompleter = new QCompleter(m_storedTags, this);
		m_tagsCompleter->setCaseSensitivity(Qt::CaseInsensitive);

		m_editTags->setCompleter(m_tagsCompleter);
		m_editTags->setToolTip(m_tagsHelp);

		connect(m_editTags,
				&QLineEdit::textEdited,
				[this]()
				{
					m_tagsCompleter->complete();
				});
		connect(m_tagsCompleter,
				static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted),
				m_editTags,
				&QLineEdit::setText);
	}

	void SignalSnapshotWidget::initSignalsView()
	{
		// create models
		//

		std::vector<AppSignalParam> allSignals = m_appSignalManager->signalList();
		m_model.setSignals(allSignals);

		m_model.setSignalType(static_cast<SnapshotSignalType>(m_storedType));
		m_model.setSignalRole(static_cast<SnapshotSignalRole>(m_storedRole));
		m_model.setMaskType(static_cast<SnapshotMaskType>(m_storedMaskType));

		// Table view setup
		//

		m_tableView->setModel(&m_model);
		m_tableView->verticalHeader()->hide();
		m_tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
		m_tableView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
		m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		m_tableView->horizontalHeader()->setStretchLastSection(false);
		m_tableView->setGridStyle(Qt::PenStyle::NoPen);
		m_tableView->setSortingEnabled(true);
		m_tableView->setWordWrap(false);

		int fontHeight = fontMetrics().height() + 4;

		QHeaderView* verticalHeader = m_tableView->verticalHeader();
		verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
		verticalHeader->setDefaultSectionSize(fontHeight);

		connect(m_tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &SignalSnapshotWidget::sortIndicatorChanged);

		m_tableView->horizontalHeader()->setHighlightSections(false);
		m_tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

		connect(m_tableView->horizontalHeader(),
				&QWidget::customContextMenuRequested,
				this,
				&SignalSnapshotWidget::headerColumnContextMenuRequested);

		m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_tableView, &QTreeWidget::customContextMenuRequested, this, &SignalSnapshotWidget::contextMenuRequested);

		if (m_settings.horzHeader.isEmpty() == true || m_settings.horzHeaderCount != static_cast<int>(SnapshotColumns::ColumnCount))
		{
			// First time? Set what is should be hidden by default
			//
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::EquipmentID));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::LmEquipmentID));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::AppSignalID));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Type));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Tags));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::SystemTime));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::LocalTime));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::PlantTime));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Flags));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Valid));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::StateAvailable));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Simulated));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Blocked));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Mismatch));
			m_tableView->hideColumn(static_cast<int>(SnapshotColumns::OutOfLimits));
		}
		else
		{
			m_tableView->horizontalHeader()->restoreState(m_settings.horzHeader);
		}
	}

	void SignalSnapshotWidget::fillAppSignalLists()
	{
		// Remove appSignalList
		//
		if (m_model.appSignalList().isEmpty() == false) 
		{
			m_model.setAppSignalList(QString());
			fillSignals();
		}

		// Refresh AppSignalLists combo
		//
		m_signalListCombo->blockSignals(true);
		
		m_signalListCombo->clear();
		m_signalListCombo->addItem(tr("Not selected"), QString());
		
		if (m_appSignalListSet != nullptr)
		{
			const auto lists = m_appSignalListSet->lists();

			for (const auto& list : lists)
			{
				m_signalListCombo->addItem(tr("[%1] %2").arg(list->id()).arg(list->caption()), list->id());
			}
			if (lists.empty() == true)
			{
				m_signalListCombo->setEnabled(false);
			}
		}
		
		m_signalListCombo->blockSignals(false);
	}

	void SignalSnapshotWidget::fillSignals()
	{
		bool modelWasEmpty = m_model.rowCount() == 0;

		m_model.fillSignals();

		m_tableView->sortByColumn(m_settings.sortColumn, m_settings.sortOrder);

		if (modelWasEmpty == true && m_model.rowCount() > 0)
		{
			m_tableView->resizeColumnsToContents();
		}
	}

	void SignalSnapshotWidget::updateTableItems()
	{
		// Update only visible dynamic items
		//
		int from = m_tableView->rowAt(0);

		int to = m_tableView->rowAt(m_tableView->height() - m_tableView->horizontalHeader()->height());

		if (from == -1)
		{
			from = 0;
		}

		if (to == -1)
		{
			to = m_model.rowCount() - 1;
		}

		// Update signal states
		//
		m_model.updateStates(from, to);

		// Redraw visible table items
		//
		for (int col = 0; col < m_model.columnCount(); col++)
		{
			if (col >= static_cast<int>(SnapshotColumns::SystemTime))
			{
				for (int row = from; row <= to; row++)
				{
					m_tableView->update(m_model.index(row, col));
				}
			}
		}

		return;
	}

	void SignalSnapshotWidget::maskChanged(bool addToCompleter)
	{
		QString maskText = m_editMask->text().trimmed();

		maskText.replace(' ', ';');

		QStringList masks;

		if (maskText.isEmpty() == false)
		{
			masks = maskText.split(';', Qt::SkipEmptyParts);

			if (addToCompleter == true)
			{
				for (const QString& mask : masks)
				{
					// Save filter history
					//
					if (m_settings.maskList.contains(mask) == false)
					{
						m_settings.maskList.append(mask);

						QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_maskCompleter->model());
						if (completerModel == nullptr)
						{
							Q_ASSERT(completerModel);
							return;
						}

						completerModel->setStringList(m_settings.maskList);
					}
				}
			}
		}

		m_model.setMasks(masks);
	}

	void SignalSnapshotWidget::tagsChanged(bool addToCompleter)
	{
		QString tagsText = m_editTags->text().trimmed();
		tagsText.replace(' ', ';');

		QStringList tags;

		if (tagsText.isEmpty() == false)
		{
			tags = tagsText.split(';', Qt::SkipEmptyParts);

			if (addToCompleter == true)
			{
				for (const QString& tag : tags)
				{
					// Save filter history
					//
					if (m_storedTags.contains(tag) == false)
					{
						m_storedTags.append(tag);

						QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_tagsCompleter->model());
						if (completerModel == nullptr)
						{
							Q_ASSERT(completerModel);
							return;
						}

						completerModel->setStringList(m_storedTags);
					}
				}
			}
		}

		m_model.setTags(tags);
	}

	void SignalSnapshotWidget::printData(bool printSelected) 
	{
		QPageLayout pageLayout(QPageSize(QPageSize::A4),
							   QPageLayout::Orientation::Portrait,
							   QMarginsF(10, 10, 10, 10),
							   QPageLayout::Unit::Millimeter);

		pageLayout = ReportLib::TableViewReportGenerator::loadPageLayoutFromSettings("SnapshotPrintPageLayout", pageLayout);

		SnapshotReportInfo ri(m_projectName, m_equipmentId);

		ReportLib::TableViewReportGenerator generator(this, *m_tableView, ri, pageLayout, printSelected);
		connect(this, &SignalSnapshotWidget::signalsUpdated, &generator, &ReportLib::TableViewReportGenerator::stop);

		generator.printTable();

		pageLayout = generator.pageLayout();
		ReportLib::TableViewReportGenerator::savePageLayoutToSettings(pageLayout, "SnapshotPrintPageLayout");
	}

} // namespace SchemaClientLib