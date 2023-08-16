#pragma once

class DbController;

class SignalHistoryDialog : public QDialog
{
	Q_OBJECT
public:
	SignalHistoryDialog(DbController* dbController, const QString& appSignalId, int signalId, QWidget *parent = nullptr);

protected:
	void closeEvent(QCloseEvent* event);

private:
	DbController* m_dbController = nullptr;
	QStandardItemModel* m_historyModel = nullptr;
	int m_signalId = -1;
};

