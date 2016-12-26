#pragma once

#include <QDialog>

namespace Ui {
class SchemaLayersDialog;
}

class EditSchemaView;

class SchemaLayersDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemaLayersDialog(EditSchemaView* schemaView, QWidget *parent = 0);
	~SchemaLayersDialog();

protected:
	virtual void showEvent(QShowEvent* event) override;

private slots:
	void onContextMenu(const QPoint &pos);

	void onActiveClick(bool);
	void onShowClick(bool checked);
	void onPrintClick(bool checked);
	void on_m_layersList_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_SchemaLayersDialog_accepted();

private:
	void fillList(int selectedIndex);

private:
	Ui::SchemaLayersDialog *ui;

	int m_activeIndex = -1;
	QList<QString> m_name;
	QList<bool> m_show;
	QList<bool> m_print;

	QAction* m_activeAction = nullptr;
	QAction* m_showAction = nullptr;
	QAction* m_printAction = nullptr;

	EditSchemaView* m_schemaView;

	int m_cmIndex = -1;
};

