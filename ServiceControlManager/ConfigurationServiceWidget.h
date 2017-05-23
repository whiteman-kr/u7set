#ifndef CONFIGURATIONSERVICEWIDGET_H
#define CONFIGURATIONSERVICEWIDGET_H

class QStandardItemModel;

#include "BaseServiceStateWidget.h"

class ConfigurationServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	ConfigurationServiceWidget(quint32 ip, int portIndex, QWidget *parent = 0);

public slots:
	void updateStateInfo();

private:
	QStandardItemModel* m_stateTabModel = nullptr;
};

#endif // CONFIGURATIONSERVICEWIDGET_H
