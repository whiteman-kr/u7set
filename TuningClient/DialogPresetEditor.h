#ifndef DIALOGPRESETEDITOR_H
#define DIALOGPRESETEDITOR_H

#include <QDialog>
#include "ObjectFilter.h"
#include "ObjectManager.h"
#include "TuningObject.h"

namespace Ui {
class DialogPresetEditor;
}

class DialogPresetEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogPresetEditor(ObjectFilterStorage* filterStorage, QWidget *parent = 0);
	~DialogPresetEditor();

private slots:
	void on_m_addPreset_clicked();

	void on_m_editPreset_clicked();

	void on_m_removePreset_clicked();

	void on_m_moveUp_clicked();

	void on_m_moveDown_clicked();

	void on_m_add_clicked();

	void on_m_remove_clicked();


	void on_m_presetsTree_doubleClicked(const QModelIndex &index);

private:
	void addChildTreeObjects(const std::shared_ptr<ObjectFilter> &filter, QTreeWidgetItem* parent);


private:
	Ui::DialogPresetEditor *ui;


	bool m_modified = false;

	ObjectFilterStorage* m_filterStorage;
};

#endif // DIALOGPRESETEDITOR_H
