#ifndef DIALOGCLIENTBEHAVIOR_H
#define DIALOGCLIENTBEHAVIOR_H

#include "../lib/DbController.h"
#include "../lib/PropertyEditor.h"
#include "../lib/ClientBehavior.h"


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
	void on_add_clicked();
	void on_addMonitorBehavior();
	void on_addTuningClientBehavior();
	void on_remove_clicked();
	void on_clone_clicked();
	void accept() override;
	void reject() override;
	void on_behaviorSelectionChanged();
	void on_behaviorSortIndicatorChanged(int column, Qt::SortOrder order);

	void on_behaviorPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

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

	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;
};

#endif // DIALOGCLIENTBEHAVIOR_H
