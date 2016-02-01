#ifndef CONFIGURATIONSERVICEWIDGET_H
#define CONFIGURATIONSERVICEWIDGET_H

#include "BaseServiceStateWidget.h"

class ConfigurationServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	ConfigurationServiceWidget(quint32 ip, int portIndex, QWidget *parent = 0);
};

#endif // CONFIGURATIONSERVICEWIDGET_H
