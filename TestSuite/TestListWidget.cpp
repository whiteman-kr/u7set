#include "TestListWidget.h"

TestListWidget::TestListWidget(QWidget* parent):
	QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout;

	m_testsPathLabel = new QLabel("Tests Path: Not loaded");
	m_testsPathLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
	m_testsPathLabel->setTextFormat(Qt::RichText);
	m_testsPathLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_testsPathLabel->setOpenExternalLinks(true);

	m_treeWidget = new QTreeWidget;
	m_treeWidget->setUniformRowHeights(true);
	m_treeWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	m_treeWidget->setRootIsDecorated(false);
	QStringList headerLabels;
	headerLabels << tr("Test");
	m_treeWidget->setHeaderLabels(headerLabels);
	m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QByteArray headerState = QSettings().value("TestsListWidget/headerState").toByteArray();
	m_treeWidget->header()->restoreState(headerState);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &TestListWidget::testItemDoubleClicked);

	layout->addWidget(m_testsPathLabel);
	layout->addWidget(m_treeWidget);

	setLayout(layout);
}

void TestListWidget::updateTestsList(const QStringList& tests)
{
	clearTestsList();

	for (const QString& fileName : tests)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << fileName);
		item->setCheckState(0, Qt::Checked);
		m_treeWidget->addTopLevelItem(item);
		item->setData(0, Qt::UserRole, fileName);
	}
}

void TestListWidget::clearTestsList()
{
	m_treeWidget->clear();
}


QStringList TestListWidget::selectedTests() const
{
	QStringList result;
	result.reserve(m_treeWidget->topLevelItemCount());

	for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked)
		{
			QString fileName = item->data(0, Qt::UserRole).toString();
			result.push_back(fileName);
		}
	}

	return result;
}

void TestListWidget::testItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if (item == nullptr)
	{
		return;
	}
	emit testItemClicked(item->text(0));
}
