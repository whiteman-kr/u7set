#pragma once


class MatsUsersEditorDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	MatsUsersEditorDelegate(QObject *parent);
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

};

class DialogMatsUsersEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogMatsUsersEditor(DbController* pDbController, QWidget *parent = 0);
	virtual ~DialogMatsUsersEditor();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	bool askForSaveChanged();
	bool saveChanges();
	DbController* db();

protected:
	virtual void closeEvent(QCloseEvent* e) override;

private slots:
	void onAddClicked();
	void onRemoveClicked();
	void onOkClicked();
	void onCancelClicked();
	void onListItemChanged(QTreeWidgetItem* item, int column);
	void onListItemDoubleClicked(QTreeWidgetItem* item, int column);

public:
	enum class Columns
	{
		Login,
		Description,
		Enabled,
		TuningTags
	};

private:
	QTreeWidget* m_list = nullptr;
	MatsUsersEditorDelegate* m_editorDelegate = nullptr;

	bool m_modified = false;

	DbController* m_dbController;
};

