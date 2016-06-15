#ifndef RS232SIGNALLISTEDITOR_H
#define RS232SIGNALLISTEDITOR_H

#include <QDialog>

#include "Connection.h"

class QTableWidget;
class DbController;
class QPushButton;

class Rs232SignalListEditor : public QDialog
{
	Q_OBJECT
public:
	explicit Rs232SignalListEditor(DbController* pDbController, QWidget *parent = 0);

protected:

signals:

private slots:
	void checkOut();
	void checkIn();
	void undo();

	void addConnection();
	void editConnection();
	void removeConnection();

	void addSignal();
	void editSignal();
	void removeSignal();

	void onConnectionChanged();

	void reject();

private:
	void fillConnectionsList();
	void fillSignalList(bool forceUpdate = false);
	bool askForSaveChanged();
	bool saveChanges();
	void updateButtons(bool checkOut);
	bool continueWithDuplicateCaptions();


	QPushButton* m_addConnection;
	QPushButton* m_editConnection;
	QPushButton* m_removeConnection;

	QPushButton* m_addSignal;
	QPushButton* m_editSignal;
	QPushButton* m_removeSignal;

	QPushButton* m_checkIn;
	QPushButton* m_checkOut;
	QPushButton* m_undo;

	QTableWidget* m_rs232Connections;
	QTableWidget* m_signalList;
	Hardware::ConnectionStorage m_connections;
	DbController* m_db;

	bool m_modified = false;
	int m_currentConnection = -1;
};

#endif // RS232SIGNALLISTEDITOR_H
