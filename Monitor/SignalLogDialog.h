#pragma once

namespace ClientLib
{
	class SignalLog;
}

class SignalLogDialog : public QDialog
{
	Q_OBJECT

private:
	explicit SignalLogDialog(const ClientLib::SignalLog& signalLog, QWidget* parent = nullptr);

public:
	virtual ~SignalLogDialog();

	static SignalLogDialog* createDialog(const ClientLib::SignalLog& signalLog, QWidget* parent);

protected:
	void showEvent(QShowEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

private:
	const ClientLib::SignalLog& m_signalLog;

	static SignalLogDialog* s_instance;
};