#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Ui/TuningSourcesWidget.h"

class ClientTuningSourcesWidget : public TuningSourcesWidget
{
	Q_OBJECT
public:

	explicit ClientTuningSourcesWidget(TuningTcpClient* tcpClient, bool hasActivationControls, bool hasCloseButton, QWidget* parent);
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
	explicit DialogTuningSources(TuningTcpClient* tcpClient, bool hasActivationControls, QWidget* parent);
	virtual ~DialogTuningSources();

protected:
	virtual void reject() override;

signals:
	void dialogClosed();

private:
	ClientTuningSourcesWidget* m_tuningSourcesWidget = nullptr;


};


#endif // DIALOGTUNINGSOURCES_H
