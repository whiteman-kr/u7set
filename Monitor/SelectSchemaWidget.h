#ifndef SELECTSCHEMAWIDGET_H
#define SELECTSCHEMAWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include "MonitorCentralWidget.h"
#include "MonitorConfigController.h"

struct SelectSchemaItem
{
	QString schemaId;
	QString caption;
};

class SelectSchemaTable;

// SelectSchemaWidget -- widget to add to ToolBar
//
class SelectSchemaWidget : public QWidget
{
	Q_OBJECT

public:
	SelectSchemaWidget(MonitorConfigController* configController, MonitorCentralWidget* centralWidget);

protected:
	void clear();
	void addSchema(QString schemaId, QString caption);

	bool setCurrentSchema(QString schemaId);
	QString currentSchemaId() const;

signals:
	void selectionChanged(QString schemaId);

protected slots:
	void slot_configurationArrived(ConfigSettings);
	void slot_schemaChanged(QString schemaId);

	void slot_buttonClicked();

	// Data
	//
private:
	QString m_currentSchemaId;
	std::vector<SelectSchemaItem> m_schemas;

	QPushButton* m_button = nullptr;

	MonitorConfigController* m_configController = nullptr;
	MonitorCentralWidget* m_centraWidget = nullptr;

	int m_lastPopupHeight = 0;
	int m_lastPopupWidth = 0;
};


// SelectSchemaPopup -- popup dialog with schema list and filter edit box
//
class SelectSchemaPopup : public QDialog
{
	Q_OBJECT

public:
	SelectSchemaPopup(QString defaultSchemaId, const std::vector<SelectSchemaItem>& schemas, QWidget* parent);

	QString selectedSchemaId() const;

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual	void keyPressEvent(QKeyEvent* event) override;
	void fillList(QString selectSchemaId);

private slots:
	void filterTextChanged();
	void listCellClicked(const QModelIndex& index);

private:
	std::vector<SelectSchemaItem> m_schemas;

	QLineEdit* m_edit = nullptr;
	SelectSchemaTable* m_tableWidget = nullptr;

	QString m_selectedSchemaId;

	static int m_lastTimeHeigh;
};

// SelectSchemaModel -- Simple model for SelectSchemaTable
//
class SelectSchemaModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit SelectSchemaModel(const std::vector<SelectSchemaItem>& schemas, QObject* parent = nullptr);

public:
	virtual int rowCount(const QModelIndex &parent) const override;
	virtual int columnCount(const QModelIndex &parent) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;

public:
	int applyFilter(QString filterText, QString defaultSchemaId);

private:
	const std::vector<SelectSchemaItem>& m_schemas;
	std::vector<int> m_filteredItems;
};


// SelectSchemaTable -- Tbale widget for selection schema
//
class SelectSchemaTable : public QTableView
{
	Q_OBJECT

public:
	SelectSchemaTable(const std::vector<SelectSchemaItem>& schemas, QString defaultSchemaId, QWidget* parent = nullptr);

	void applyFilter(QString filter, QString defaultSchemaId);

protected slots:
	void mouseOverItem(const QModelIndex& index);
	virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;

private:
	const std::vector<SelectSchemaItem>& m_schemas;
	SelectSchemaModel* m_model = nullptr;
};


#endif // SELECTSCHEMAWIDGET_H
