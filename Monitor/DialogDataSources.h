#ifndef DIALOGDATASOURCES_H
#define DIALOGDATASOURCES_H

#include "../lib/Ui/AppDataSourcesWidget.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Ui/TuningSourcesWidget.h"

class DialogDataSources : public QDialog
{
	Q_OBJECT
public:
	explicit DialogDataSources(TcpAppSourcesState* tcpAppSourceState, bool showTuningWidget, TuningTcpClient* tcpTuningClient, bool hasActivationControls, QWidget* parent);
	virtual ~DialogDataSources();

protected:
	virtual void reject() override;

signals:
	void dialogClosed();

private:
	AppDataSourcesWidget* m_appDataSourcesWidget = nullptr;
	TuningSourcesWidget* m_tuningSourcesWidget = nullptr;

};

#endif // DIALOGDATASOURCES_H
