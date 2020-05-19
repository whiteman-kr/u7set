#pragma once

#include <QObject>
#include "SimBasePage.h"

class TagSelectorWidget;


//
//
//		SimSchemaListView - Tree View
//
//
class SimSchemaListView : public QTreeWidget
{
	Q_OBJECT

public:
	SimSchemaListView(SimIdeSimulator* simulator, QWidget* parent);
	virtual ~SimSchemaListView();

private slots:
	void fillList();

	void slot_projectUpdated();
	void slot_doubleClicked(const QModelIndex& /*index*/);

signals:
	void openSchemaRequest(QString schemaId);

public:
	QString filter() const;
	void setFilter(QString value);

	void setTagFilter(const QStringList& tags);

	int filterCount() const;

	std::vector<QTreeWidgetItem*> searchFor(QString searchText);

	void searchAndSelect(QString searchText);

private:
	SimIdeSimulator* m_simulator = nullptr;

	QString m_filter;
	QStringList m_tagFilter;

	int m_filterCount = 0;
};

//
//
//		SimAppLogicSchemasPage - Tab Page
//
//
class SimAppLogicSchemasPage : public SimBasePage
{
	Q_OBJECT

public:
	SimAppLogicSchemasPage(SimIdeSimulator* simulator, QWidget* parent);
	virtual ~SimAppLogicSchemasPage() = default;

protected:
	void updateData();

private slots:
	void ctrlF();
	void search();
	void filter();
	void resetFilter();
	void tagSelectorHasChanges();

	void treeContextMenu(const QPoint& pos);

signals:
	void openSchemaRequest(QString schemaId);

private:
	SimSchemaListView* m_schemasView = nullptr;

	QAction* m_searchAction = nullptr;
	QLineEdit* m_searchEdit = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QCompleter* m_searchCompleter = nullptr;
	QPushButton* m_searchButton = nullptr;
	QPushButton* m_filterButton = nullptr;
	QPushButton* m_resetFilterButton = nullptr;

	TagSelectorWidget* m_tagSelector = nullptr;
};

