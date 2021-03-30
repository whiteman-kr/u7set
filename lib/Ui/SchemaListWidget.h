#pragma once

#include <memory>
#include <QTreeWidget>
#include "../../VFrame30/Schema.h"


class TagSelectorWidget;

enum class SchemaListTreeColumns
{
	SchemaID,
	Caption,
	Tags,
	Modules
};

//
//
//		SimSchemaListView - Tree View
//
//
class SchemaListTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	SchemaListTreeWidget(std::vector<SchemaListTreeColumns> columns, QWidget* parent);
	virtual ~SchemaListTreeWidget();

	void setDetails(VFrame30::SchemaDetailsSet details);

private slots:
	void fillList();
	void slot_doubleClicked(const QModelIndex& /*index*/);

signals:
	void openSchemaRequest(QString schemaId, QStringList highlightIds);

public:
	QString filter() const;
	void setFilter(QString value);

	QStringList tagFilter() const;
	void setTagFilter(const QStringList& tags);

	int filterCount() const;

	std::vector<QTreeWidgetItem*> searchFor(QString searchText);

	void searchAndSelect(QString searchText);

private:
	VFrame30::SchemaDetailsSet m_details;
	std::vector<SchemaListTreeColumns> m_columns;

	QString m_filter;
	QStringList m_tagFilter;

	int m_filterCount = 0;
};

//
//
//		SchemaListWidget
//
//
class SchemaListWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SchemaListWidget(std::vector<SchemaListTreeColumns> columns, bool showTags, QWidget* parent);
	virtual ~SchemaListWidget() = default;

public:
	void setDetails(VFrame30::SchemaDetailsSet details);

private slots:
	void ctrlF();
	void search();
	void filter();
	void resetFilter();
	void tagSelectorHasChanges();

	void treeContextMenu(const QPoint& pos);

signals:
	void openSchemaRequest(QString schemaId, QStringList highlightIds);

private:
	SchemaListTreeWidget* m_treeWidget = nullptr;

	QAction* m_searchAction = nullptr;
	QLineEdit* m_searchEdit = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QCompleter* m_searchCompleter = nullptr;
	QPushButton* m_searchButton = nullptr;
	QPushButton* m_filterButton = nullptr;
	QPushButton* m_resetFilterButton = nullptr;

	TagSelectorWidget* m_tagSelector = nullptr;
};

