#ifndef SIMSIGNALINFO_H
#define SIMSIGNALINFO_H

#include "../../VFrame30/AppSignalController.h"
#include "../TuningConnectionStub.h"

#include <SchemaClientLib/DialogSignalInfo.h>
#include <Simulator/SimAppSignalManager.h>

class SimWidget;
class SimIdeSimulator;

class SimSignalInfo : public DialogSignalInfo
{
	Q_OBJECT

public:
	static bool showDialog(QString appSignalId,
						   SimIdeSimulator* simuator,
						   SimWidget* simWidget);

private:
	SimSignalInfo(const AppSignalParam& signal,
				  SimIdeSimulator* simuator,
				  SimWidget* simWidget);

private slots:
	void onSignalParamAndUnitsArrived();

signals:
	void openSchema(QString schemaId, QStringList highlightIds);

private:
	virtual QStringList schemasByAppSignalId(const QString& appSignalId) override;
	virtual void setSchema(QString schemaId, QStringList highlightIds) override;
	virtual std::optional<AppSignal> getSignalExt(const AppSignalParam& appSignalParam) override;

private:
	SimIdeSimulator* m_simuator = nullptr;

	TuningConnectionStub m_tuningConnection;
	TuningAuthorizationStub m_tuningAuthorization;
};

#endif // SIMSIGNALINFO_H
