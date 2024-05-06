#include "DevToolsSchemaStats.h"
#include "DevToolsSchemaStats.h"
#include <QClipboard>

namespace SchemaClientLib
{
	DevToolsSchemaStats::DevToolsSchemaStats(IDevToolsSchemaStats& provider, QWidget* parent) :
		QWidget{parent},
		m_provider{provider}
	{
		m_treeWidget = new QTreeWidget{this};

		m_treeWidget->setColumnCount(4);
		m_treeWidget->setHeaderLabels({tr("Module"), tr("Item"), tr("Action"), tr("Time, mcs")});
		connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &DevToolsSchemaStats::onTreeWidgetDoubleClicked);

		m_refreshButton = new QPushButton{tr("Refresh"), this};
		m_refreshButton->setShortcut(QKeySequence::Refresh);
		connect(m_refreshButton, &QPushButton::clicked, this, &DevToolsSchemaStats::updateStats);

		m_copyButton = new QPushButton{tr("Copy"), this};
		m_copyButton->setShortcut(QKeySequence::Copy);
		connect(m_copyButton, &QPushButton::clicked, [this] { QApplication::clipboard()->setText(m_lastStats); });

		auto layout = new QGridLayout{this};

		layout->addWidget(m_treeWidget, 0, 0, 1, 2);
		layout->addWidget(m_refreshButton, 1, 0);
		layout->addWidget(m_copyButton, 1, 1);

		setLayout(layout);

		updateStats();

		// Adjust columns width to content.
		//
		for (int i = 0; i < m_treeWidget->columnCount(); ++i)
		{
			m_treeWidget->resizeColumnToContents(i);
		}

		return;
	}

	void DevToolsSchemaStats::onTreeWidgetDoubleClicked(QTreeWidgetItem* item, [[maybe_unused]] int column)
	{
		QString itemLabel = item->text(1).trimmed();
		m_provider.highlightItems(QStringList{} << itemLabel);
		return;
	}

	void DevToolsSchemaStats::updateStats()
	{
		Q_ASSERT(m_treeWidget);

		// Fill tree widget and save text to m_lastStats in csv format (Module;Item;Action;Time)
		//
		using StatRecord = std::tuple<QString, QString, QString, std::uint32_t>;
		std::vector<StatRecord> statRecords;
		statRecords.reserve(4096);

		for (const QString& module : m_provider.modules())
		{
			for (const QString& item : m_provider.items(module))
			{
				auto records = m_provider.itemRecords(module, item);

				for (const auto& record : records)
				{
					statRecords.emplace_back(module, item, record.action, record.time.count());
				}
			}
		}

		std::stable_sort(begin(statRecords), end(statRecords), [](const auto& l, const auto& r) { return std::get<3>(l) > std::get<3>(r); });

		m_treeWidget->clear();
		for (const auto& record : statRecords)
		{
			auto item = new QTreeWidgetItem{m_treeWidget};
			item->setText(0, std::get<0>(record));
			item->setText(1, std::get<1>(record));
			item->setText(2, std::get<2>(record));
			item->setText(3, QString::number(std::get<3>(record)));
		}

		// Save text to m_lastStats in csv format (Module;Item;Action;Time).
		//
		m_lastStats.clear();
		m_lastStats.reserve(4096);

		for (const auto& record : statRecords)
		{
			m_lastStats += QString("%1;%2;%3;%4\n").arg(std::get<0>(record)).arg(std::get<1>(record)).arg(std::get<2>(record)).arg(std::get<3>(record));
		}

		return;
	}
} // namespace SchemaClientLib