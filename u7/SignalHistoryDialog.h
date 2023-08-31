#pragma once

class DbController;

class AppSignal;
class AppSignalSetProvider;
class AppSignalPropertyManager;

class SignalHistoryDialog : public QDialog
{
	Q_OBJECT
public:
	SignalHistoryDialog(const AppSignal& s, QWidget* parent = nullptr);

protected:
	void closeEvent(QCloseEvent* event);

private:
	AppSignalSetProvider* m_signalSetProvider = nullptr;
	AppSignalPropertyManager* m_propManager = nullptr;

	QStandardItemModel* m_historyModel = nullptr;
};
