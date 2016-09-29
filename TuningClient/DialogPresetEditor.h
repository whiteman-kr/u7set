#ifndef DIALOGPRESETEDITOR_H
#define DIALOGPRESETEDITOR_H

#include <QDialog>
#include "ObjectFilter.h"
#include "ObjectManager.h"
#include "TuningObject.h"
#include "TuningPage.h"

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

	void on_m_signalTypeCombo_currentIndexChanged(int index);

private:

	enum class TreeItemType
	{
		Filter,
		Signal
	};

	enum class SignalType
	{
		All,
		Analog,
		Discrete
	};

	bool isFilter(QTreeWidgetItem* item);
	bool isSignal(QTreeWidgetItem* item);

	void addChildTreeObjects(const std::shared_ptr<ObjectFilter> &filter, QTreeWidgetItem* parent);

	void setTreeItemText(QTreeWidgetItem* item, ObjectFilter* filter);

	void fillObjectsList();


private:
	Ui::DialogPresetEditor *ui;

	TuningItemModel *m_model = nullptr;

	bool m_modified = false;

	ObjectFilterStorage* m_filterStorage;

	std::vector<int> m_objectsIndexes;
};

#endif // DIALOGPRESETEDITOR_H
