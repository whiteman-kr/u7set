#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Ui/TuningSourcesWidget.h"
#include "../ClientLib/TuningUserManager.h"

class ClientTuningSourcesWidget : public TuningSourcesWidget
{
	Q_OBJECT
public:

	explicit ClientTuningSourcesWidget(std::vector<TuningTcpClient*> tcpClients, ClientLib::TuningUserManager& userManager, bool hasActivationControls, QWidget* parent);
	virtual ~ClientTuningSourcesWidget();

protected:

	virtual bool login() override;

private:
	ClientLib::TuningUserManager& m_userManager;
};


//
// DialogTuningSources
//

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(std::vector<TuningTcpClient*> tcpClients, ClientLib::TuningUserManager& userManager, bool hasActivationControls, QWidget* parent);
	virtual ~DialogTuningSources();

	void setTuningSources(std::vector<TuningTcpClient*> tcpClients);

protected:
	virtual void reject() override;

signals:
	void dialogClosed();

private:
	ClientTuningSourcesWidget* m_tuningSourcesWidget = nullptr;
	QPushButton* m_btnEnableControl = nullptr;
	QPushButton* m_btnDisableControl = nullptr;



};


#endif // DIALOGTUNINGSOURCES_H
