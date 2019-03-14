#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Ui/TuningSourcesWidget.h"

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
	TuningSourcesWidget* m_tuningSourcesWidget = nullptr;


};


#endif // DIALOGTUNINGSOURCES_H
