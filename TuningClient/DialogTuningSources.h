#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Ui/TuningSourcesWidget.h"

class ClientTuningSourcesWidget : public TuningSourcesWidget
{
	Q_OBJECT
public:

	explicit ClientTuningSourcesWidget(std::vector<TuningTcpClient*> tcpClients, bool hasActivationControls, bool hasCloseButton, QWidget* parent);
	virtual ~ClientTuningSourcesWidget();

protected:

	virtual bool login() override;
};


//
// DialogTuningSources
//

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(std::vector<TuningTcpClient*> tcpClients, bool hasActivationControls, QWidget* parent);
	virtual ~DialogTuningSources();

	void setTuningSources(std::vector<TuningTcpClient*> tcpClients);

protected:
	virtual void reject() override;

signals:
	void dialogClosed();

private:
	ClientTuningSourcesWidget* m_tuningSourcesWidget = nullptr;


};


#endif // DIALOGTUNINGSOURCES_H
