#include "SignalLogDialog.h"

SignalLogDialog* SignalLogDialog::s_instance = nullptr;


SignalLogDialog::SignalLogDialog(const ClientLib::SignalLog& signalLog, QWidget* parent) :
	QDialog{parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint},
	m_signalLog{signalLog}
{
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowTitle(tr("Signal Log"));

	setMinimumSize(400, 200);
}

SignalLogDialog::~SignalLogDialog()
{
	s_instance = nullptr;
}

SignalLogDialog* SignalLogDialog::createDialog(const ClientLib::SignalLog& signalLog, QWidget* parent)
{
	if (s_instance == nullptr)
	{
		s_instance = new SignalLogDialog{signalLog, parent};
		s_instance->show();
		return s_instance;
	}
	else
	{
		s_instance->raise(); // Bring to front
	}

	return s_instance;
}

void SignalLogDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
}

void SignalLogDialog::closeEvent(QCloseEvent* event)
{
	QDialog::closeEvent(event);
}
