#include "SignalHistoryDialog.h"
#include "../lib/WidgetUtils.h"
#include "../DbLib/DbController.h"
#include "AppSignalSetProvider.h"
#include "Settings.h"

SignalHistoryDialog::SignalHistoryDialog(DbController* dbController, const QString& appSignalId, int signalId, QWidget* parent) :
	QDialog(parent),
	m_dbController(dbController),
	m_signalId(signalId)
{
	// Initial data
	//
	std::vector<DbChangeset> signalChanges;
	dbController->getSignalHistory(signalId, &signalChanges, this);

	QVector<std::pair<QString, std::function<QVariant (DbChangeset&)>>> changesetColumnDescription =
	{
		{"Changeset", [](DbChangeset& c) { return c.changeset(); }},
		{"User", [](DbChangeset& c) { return c.username(); }},
		{"Date", [](DbChangeset& c) { return c.date().toString("dd MMM yyyy HH:mm:ss"); }},
		{"Comment", [](DbChangeset& c) { return c.comment();}},
	};

	int changesetColumnCount = static_cast<int>(changesetColumnDescription.size());

	// Interface
	//
	setWindowTitle(tr("History - ") + appSignalId);

	setWindowPosition(this, "SignalHistoryDialog");

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

	QVBoxLayout* vl = new QVBoxLayout;

	setLayout(vl);

	m_historyModel = new QStandardItemModel(static_cast<int>(signalChanges.size()), changesetColumnCount, this);

	QTableView* historyView = new QTableView(this);
	historyView->setModel(m_historyModel);
	vl->addWidget(historyView);

	historyView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	historyView->setAlternatingRowColors(false);
	historyView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");
	historyView->setEditTriggers(QTableView::NoEditTriggers);

	historyView->verticalHeader()->setDefaultSectionSize(static_cast<int>(historyView->fontMetrics().height() * 1.4));
	historyView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	historyView->horizontalHeader()->setHighlightSections(false);
	historyView->horizontalHeader()->setDefaultSectionSize(150);
	historyView->horizontalHeader()->setStretchLastSection(true);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::clicked, this, &QDialog::accept);
	vl->addWidget(buttonBox);

	// Changeset details
	//
	QVector<int> defaultColumns;

	for (int i = 0; i < changesetColumnCount; i++)
	{
		m_historyModel->setHeaderData(i, Qt::Horizontal, changesetColumnDescription[i].first);
		defaultColumns.push_back(i);
	}

	QVector<AppSignal> signalInstances;
	signalInstances.reserve(static_cast<int>(signalChanges.size()));
	std::vector<int> signalIds = { signalId };
	std::vector<AppSignal> signalInstance;

	AppSignalPropertyManager* pSignalPropertyManager = AppSignalPropertyManager::getInstance();
	pSignalPropertyManager->reloadPropertyBehaviour();

	int row = 0;
	for (DbChangeset& changeset : signalChanges)
	{
		for (int i = 0; i < changesetColumnCount; i++)
		{
			m_historyModel->setData(m_historyModel->index(row, i), changesetColumnDescription[i].second(changeset));
		}

		dbController->getSpecificSignals(&signalIds, changeset.changeset(), &signalInstance, this);

		if (signalInstance.size() == 1)
		{
			signalInstances.push_back(signalInstance[0]);
			pSignalPropertyManager->detectNewProperties(&signalInstance[0]);
			signalInstance.clear();
		}
		else
		{
			assert(false);
		}

		row++;
	}

	// Signal instances details
	//
	for (int propertyIndex = 0; propertyIndex < pSignalPropertyManager->count(); propertyIndex++)
	{
		if (signalInstances.count() == 0)
		{
			break;
		}

		bool isExpert = theSettings.isExpertMode();

		QVariant previousValue = pSignalPropertyManager->value(&signalInstances[0], propertyIndex, isExpert);

		QList<QStandardItem*> column;
		int columnIndex = m_historyModel->columnCount();

		for (int signalIndex = 0; signalIndex < signalInstances.count(); signalIndex++)
		{
			QVariant currentValue = pSignalPropertyManager->value(&signalInstances[signalIndex], propertyIndex, isExpert);
			QStandardItem* newItem = new QStandardItem(currentValue.toString());

			if (currentValue != previousValue)
			{
				column.last()->setData(QColor(Qt::yellow), Qt::BackgroundRole);
				previousValue = currentValue;
				if (defaultColumns.contains(columnIndex) == false)
				{
					defaultColumns.push_back(columnIndex);
				}
			}
			column.push_back(newItem);
		}

		m_historyModel->appendColumn(column);
		m_historyModel->setHeaderData(columnIndex, Qt::Horizontal, pSignalPropertyManager->caption(propertyIndex));
	}

	new TableDataVisibilityController(historyView, "SignalHistoryDialog", defaultColumns);
}

void SignalHistoryDialog::closeEvent(QCloseEvent* event)
{
	saveWindowPosition(this, "SignalHistoryDialog");

	QDialog::closeEvent(event);
}


