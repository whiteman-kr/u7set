#include "../lib/WidgetUtils.h"

#include "UndoSignalsDialog.h"
#include "SignalsTabPage.h"
#include "AppSignalSetProvider.h"

UndoSignalsDialog::UndoSignalsDialog(SignalsModel* sourceModel, TableDataVisibilityController* columnManager, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle(tr("Undo signal changes"));

	setWindowPosition(this, "UndoSignalsDialog");

	QVBoxLayout* vl = new QVBoxLayout;

	QTableView* signalsView = new QTableView(this);
	m_proxyModel = new CheckedoutSignalsModel(sourceModel, signalsView, this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);
	vl->addWidget(selectAll);

	signalsView->setModel(m_proxyModel);
	signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(signalsView->fontMetrics().height() * 1.4));

	const auto& propertyManager = *AppSignalPropertyManager::getInstance();

	QSettings settings;
	signalsView->setColumnWidth(0, columnManager->getColumnWidth(0) + 30);	// basic column width + checkbox size

	for (int i = 1; i < propertyManager.count(); i++)
	{
		bool visible = columnManager->getColumnVisibility(i);
		signalsView->setColumnHidden(i, !visible);

		if (visible)
		{
			signalsView->setColumnWidth(i, columnManager->getColumnWidth(i));
		}
	}

	signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	signalsView->horizontalHeader()->setHighlightSections(false);

	vl->addWidget(signalsView);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &UndoSignalsDialog::undoSelected);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &UndoSignalsDialog::saveDialogGeometry);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	vl->addWidget(buttonBox);

	setLayout(vl);
}

void UndoSignalsDialog::setCheckStates(QModelIndexList selection, bool fromSourceModel)
{
	if (!selection.isEmpty())
	{
		m_proxyModel->initCheckStates(selection, fromSourceModel);
	}
}

void UndoSignalsDialog::saveDialogGeometry()
{
	saveWindowPosition(this, "UndoSignalsDialog");
}

void UndoSignalsDialog::undoSelected()
{
	saveDialogGeometry();

	AppSignalSetProvider* signalSetProvider = AppSignalSetProvider::getInstance();

	m_undoedSignalsIDs.clear();

	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, 0);

		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();

		m_undoedSignalsIDs.push_back(signalSetProvider->signalID(sourceRow));
	}

	if (m_undoedSignalsIDs.empty())
	{
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}

	signalSetProvider->undoSignalsChanges(m_undoedSignalsIDs);

	accept();
}

void UndoSignalsDialog::closeEvent(QCloseEvent* event)
{
	saveDialogGeometry();

	QDialog::closeEvent(event);
}

