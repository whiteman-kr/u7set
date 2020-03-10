#ifndef DIALOGCLIENTBEHAVIOR_H
#define DIALOGCLIENTBEHAVIOR_H

#include "../lib/DbController.h"
#include "../lib/PropertyEditor.h"
#include "../lib/ClientBehavior.h"

class TagsToColorDelegate: public QItemDelegate
{
	Q_OBJECT

public:
	TagsToColorDelegate(QWidget *parent);
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
	void editingFinished(const QModelIndex& index) const;

private:
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;

private:
	mutable QStringList m_existingTags;

	QWidget* m_parentWidget = nullptr;
};

class MonitorBehaviorEditWidget : public QWidget
{
	Q_OBJECT

public:
	MonitorBehaviorEditWidget(QWidget* parent);
	~MonitorBehaviorEditWidget() = default;

	void setBehavior(std::shared_ptr<MonitorBehavior> mb);

public:
	enum class Columns
	{
		Tag,
		Color1,
		Color2
	};

signals:
	void behaviorModified();

private slots:
	void onAddTag();
	void onRemoveTag();
	void onTagUp();
	void onTagDown();
	void onTagEditFinished(const QModelIndex& index);

private:
	void fillTagToColor();
	void addTagTreeItem(const QString& tag);
	void moveTag(int step);

private:
	QTreeWidget* m_tagsTree = nullptr;

	std::shared_ptr<MonitorBehavior> m_behavior;

	TagsToColorDelegate* m_tagsToColorDelegate = nullptr;
};

class TuningClientBehaviorEditWidget : public QWidget
{
	Q_OBJECT
public:
	TuningClientBehaviorEditWidget(QWidget* parent);
	~TuningClientBehaviorEditWidget() = default;

signals:
	void behaviorModified();
};

class DialogClientBehavior : public QDialog
{
	Q_OBJECT
public:
	explicit DialogClientBehavior(DbController* pDbController, QWidget *parent = 0);
	~DialogClientBehavior();

private:
	bool askForSaveChanged();
	bool saveChanges();

	void fillBehaviorList();
	void fillBehaviorProperties();

	void updateBehaviuorItemText(QTreeWidgetItem* item, ClientBehavior* behavior);

	void addBehavior(const std::shared_ptr<ClientBehavior> behavior);

	bool continueWithDuplicateId();

	bool loadFileFromDatabase(DbController* db, const QString& fileName, QString *errorCode, QByteArray* data);
	bool saveFileToDatabase(const QByteArray& data, DbController* db, const QString& fileName, const QString &comment);

protected:
	virtual void keyPressEvent(QKeyEvent *evt);
	virtual void showEvent(QShowEvent* event) override;
	virtual void closeEvent(QCloseEvent* e) override;

private slots:
	void onAddClicked();
	void onAddMonitorBehavior();
	void onAddTuningClientBehavior();
	void onRemoveClicked();
	void onCloneClicked();
	void accept() override;
	void reject() override;
	void onBehaviorSelectionChanged();
	void onBehaviorSortIndicatorChanged(int column, Qt::SortOrder order);
	void onBehaviorPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);
	void onBehaviorModified();

public:
	enum class Columns
	{
		ID,
		Type
	};

private:
	ClientBehaviorStorage m_behaviorStorage;

	bool m_modified = false;

	DbController* db();
	DbController* m_dbController = nullptr;

	QTreeWidget* m_behaviorTree = nullptr;

	QSplitter* m_hSplitter = nullptr;

	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;

	QSplitter* m_vSplitter = nullptr;

	MonitorBehaviorEditWidget* m_monitorBehaviorEditWidget = nullptr;
	TuningClientBehaviorEditWidget* m_tuningClientBehaviorEditWidget = nullptr;
	QLabel* m_emptyEditorWidget = nullptr;
};

#endif // DIALOGCLIENTBEHAVIOR_H
