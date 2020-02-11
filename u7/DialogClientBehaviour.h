#ifndef DIALOGCLIENTBEHAVIOUR_H
#define DIALOGCLIENTBEHAVIOUR_H

#include "../lib/DbController.h"
#include "../lib/PropertyEditor.h"
#include "../lib/ClientBehaviour.h"


class DialogClientBehaviour : public QDialog
{
	Q_OBJECT
public:
	explicit DialogClientBehaviour(DbController* pDbController, QWidget *parent = 0);
	~DialogClientBehaviour();

private:
	bool askForSaveChanged();
	bool saveChanges();

	void fillBehaviourList();
	void fillBehaviourProperties();

	void updateBehaviuorItemText(QTreeWidgetItem* item, ClientBehaviour* behaviour);

	void addBehaviour(const std::shared_ptr<ClientBehaviour> behaviour);

	bool continueWithDuplicateId();

	bool loadFileFromDatabase(DbController* db, const QString& fileName, QString *errorCode, QByteArray* data);
	bool saveFileToDatabase(const QByteArray& data, DbController* db, const QString& fileName, const QString &comment);

protected:
	virtual void keyPressEvent(QKeyEvent *evt);
	virtual void showEvent(QShowEvent* event) override;
	virtual void closeEvent(QCloseEvent* e) override;

private slots:
	void on_add_clicked();
	void on_addMonitorBehaviour();
	void on_addTuningClientBehaviour();
	void on_remove_clicked();
	void on_clone_clicked();
	void accept() override;
	void reject() override;
	void on_behaviourSelectionChanged();
	void on_behaviourSortIndicatorChanged(int column, Qt::SortOrder order);

	void on_behaviourPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

public:
	enum class Columns
	{
		ID,
		Type
	};

private:
	ClientBehaviourStorage m_behaviourStorage;

	bool m_modified = false;

	DbController* db();
	DbController* m_dbController;

	QTreeWidget* m_behaviourTree = nullptr;

	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;
};

#endif // DIALOGCLIENTBEHAVIOUR_H
