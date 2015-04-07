#ifndef SCHEMELAYERSDIALOG_H
#define SCHEMELAYERSDIALOG_H

#include <QDialog>

namespace Ui {
class SchemeLayersDialog;
}

class EditSchemeView;

class SchemeLayersDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemeLayersDialog(EditSchemeView* schemeView, QWidget *parent = 0);
	~SchemeLayersDialog();

private slots:
	void onContextMenu(const QPoint &pos);

	void onActiveClick(bool);
	void onCompileClick(bool checked);
	void onShowClick(bool checked);
	void onPrintClick(bool checked);
	void on_m_layersList_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_SchemeLayersDialog_accepted();

private:
	void fillList(int selectedIndex);

private:
	Ui::SchemeLayersDialog *ui;

	int m_activeIndex = -1;
	QList<QString> m_name;
	QList<bool> m_compile;
	QList<bool> m_show;
	QList<bool> m_print;

	QAction* m_activeAction = nullptr;
	QAction* m_compileAction = nullptr;
	QAction* m_showAction = nullptr;
	QAction* m_printAction = nullptr;

	EditSchemeView* m_schemeView;

	int m_cmIndex = -1;
};

#endif // SCHEMELAYERSDIALOG_H
