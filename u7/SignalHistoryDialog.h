#pragma once

class DbController;

class AppSignal;
class AppSignalSetProvider;
class AppSignalPropertyManager;

class SignalHistoryDialog : public QDialog
{
	Q_OBJECT
public:
	SignalHistoryDialog(DbController* db, const AppSignal& s, QWidget* parent = nullptr);

protected:
	void closeEvent(QCloseEvent* event);

private:
	DbController* m_db = nullptr;
	AppSignalPropertyManager* m_propManager = nullptr;

	QStandardItemModel* m_historyModel = nullptr;
};
