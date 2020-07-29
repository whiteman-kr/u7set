#ifndef SIMSIGNALSNAPSHOT_H
#define SIMSIGNALSNAPSHOT_H

#include "../../lib/Ui/DialogSignalSnapshot.h"
#include "../../VFrame30/AppSignalController.h"

class SimWidget;
class SimIdeSimulator;

class SimDialogSignalSnapshot : public DialogSignalSnapshot
{
	Q_OBJECT
public:
	static bool showDialog(SimIdeSimulator* simuator,
						   VFrame30::AppSignalController* appSignalController,
						   QString lmEquipmentId,
						   SimWidget* simWidget);

private:
	explicit SimDialogSignalSnapshot(SimIdeSimulator* simuator,
									 IAppSignalManager* appSignalManager,
									 QString projectName,
									 QString softwareEquipmentId,
									 QString lmEquipmentId,
									 QWidget *parent);

private slots:
	void projectUpdated();

private:
	virtual std::vector<VFrame30::SchemaDetails> schemasDetails() override;
	virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) override;

private:
	SimIdeSimulator* m_simuator = nullptr;
};

#endif // SIMSIGNALSNAPSHOT_H
