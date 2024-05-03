#include "../UtilsLib/Ui/WidgetUtils.h"

#include "CheckinSignalsDialog.h"
#include "SignalsTabPage.h"
#include "AppSignalSetProvider.h"

CheckinSignalsDialog::CheckinSignalsDialog(const QModelIndexList& selectionSrcIndexes,
											SignalsModel* signalsModel,
											const TableDataVisibilityController& columnManager,
											QWidget* parent) :
	QDialog(parent),
	m_signalsModel(signalsModel)
{
	setWindowTitle("Check In Signal(s)");

	m_splitter = new QSplitter(Qt::Vertical, this);

	QVBoxLayout* vl1 = new QVBoxLayout;
	QVBoxLayout* vl2 = new QVBoxLayout;

	vl2->setContentsMargins(0, 0, 0, 0);

	m_signalsView = new QTableView(this);
	m_checkedOutModel = new CheckedOutSignalsModel(signalsModel, m_signalsView, this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_checkedOutModel, &CheckedOutSignalsModel::setAllCheckStates);


	if (selectionSrcIndexes.isEmpty() == false)
	{
		m_checkedOutModel->initCheckStates(selectionSrcIndexes);
	}
	else
	{
		selectAll->setChecked(true);
	}

	m_commentEdit = new QPlainTextEdit(this);

	vl2->addWidget(selectAll);

	m_signalsView->setModel(m_checkedOutModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_signalsView->horizontalHeader()->setHighlightSections(false);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));

	int signalPropertyCount = AppSignalPropertyManager::getInstance()->count();

	QSettings settings;

	m_signalsView->setColumnWidth(0, columnManager.getColumnWidth(0) + 30);	// basic column width + checkbox size

	for (int i = 1; i < signalPropertyCount; i++)
	{
		bool visible = columnManager.getColumnVisibility(i);
		m_signalsView->setColumnHidden(i, !visible);

		if (visible)
		{
			m_signalsView->setColumnWidth(i, columnManager.getColumnWidth(i));
		}
	}

	vl2->addWidget(m_signalsView);

	QWidget* w = new QWidget(this);

	w->setLayout(vl2);

	m_splitter->addWidget(m_commentEdit);
	m_splitter->addWidget(w);

	vl1->addWidget(new QLabel(tr("Check In Comment:"), this));
	vl1->addWidget(m_splitter);

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addStretch();

	QPushButton* checkinSelectedButton = new QPushButton(tr("Check In"), this);
	connect(checkinSelectedButton, &QPushButton::clicked, this, &CheckinSignalsDialog::checkinSelected);
	hl->addWidget(checkinSelectedButton);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, &QPushButton::clicked, this, &CheckinSignalsDialog::cancel);
	hl->addWidget(cancelButton);

	vl1->addLayout(hl);

	setLayout(vl1);

	m_splitter->setChildrenCollapsible(false);

	setWindowPosition(this, "CheckinSignalsDialog");

	QList<int> list = m_splitter->sizes();
	list[0] = height();
	list[1] = m_commentEdit->height();
	m_splitter->setSizes(list);

	m_splitter->restoreState(settings.value("CheckinSignalsDialog/splitterPosition", m_splitter->saveState()).toByteArray());
}

void CheckinSignalsDialog::checkinSelected()
{
	saveDialogGeometry();

	QString commentText = m_commentEdit->toPlainText();

	if (commentText.isEmpty())
	{
		QMessageBox::warning(m_signalsModel->parentWidget(), tr("Warning"), tr("Checkin comment is empty"));
		return;
	}

	std::vector<int> IDs;

	AppSignalSetProvider* signalSetProvider = AppSignalSetProvider::getInstance();

	for (int i = 0; i < m_checkedOutModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_checkedOutModel->index(i, 0);

		if (m_checkedOutModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}

		int sourceRow = m_checkedOutModel->mapToSource(proxyIndex).row();

		IDs.push_back(signalSetProvider->signalID(sourceRow));
	}

	if (IDs.size() == 0)
	{
		QMessageBox::warning(m_signalsModel->parentWidget(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}

	signalSetProvider->checkinSignals(IDs, commentText);

	accept();
}

void CheckinSignalsDialog::cancel()
{
	saveDialogGeometry();

	reject();
}

void CheckinSignalsDialog::closeEvent(QCloseEvent* event)
{
	saveDialogGeometry();

	QDialog::closeEvent(event);
}

void CheckinSignalsDialog::saveDialogGeometry()
{
	saveWindowPosition(this, "CheckinSignalsDialog");

	QSettings settings;
	settings.setValue("CheckinSignalsDialog/splitterPosition", m_splitter->saveState());
}

