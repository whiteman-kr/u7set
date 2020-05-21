#ifndef SIMSIGNALINFO_H
#define SIMSIGNALINFO_H

#include "../lib/Ui/DialogSignalInfo.h"
#include "../../VFrame30/AppSignalController.h"

class SimWidget;
class SimIdeSimulator;

class SimSignalInfo : public DialogSignalInfo
{
	Q_OBJECT
public:
	static bool showDialog(QString appSignalId,
						   SimIdeSimulator* simuator,
						   IAppSignalManager* appSignalManager,
						   SimWidget* simWidget);

private:
	SimSignalInfo(const AppSignalParam& signal,
					  SimIdeSimulator* simuator,
					  IAppSignalManager* appSignalManager,
					  SimWidget* simWidget);

signals:
	void openSchema(QString schemaId);

private:
	virtual QStringList schemasByAppSignalId(const QString& appSignalId) override;
	virtual void setSchema(QString schemaId, QStringList highlightIds) override;

private:
	SimIdeSimulator* m_simuator = nullptr;

};

#endif // SIMSIGNALINFO_H
