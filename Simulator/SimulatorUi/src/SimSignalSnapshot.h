#pragma once

#include <SchemaClientLib/DialogSignalSnapshot.h>

namespace VFrame30
{
	class AppSignalController;
}

namespace SimUi
{
	class SimWidgetPrivate;
	class SimIdeSimulator;

	class SimDialogSignalSnapshot : public SchemaClientLib::DialogSignalSnapshot
	{
		Q_OBJECT
	public:
		static bool showDialog(SimIdeSimulator* simulator,
							   VFrame30::AppSignalController* appSignalController,
							   QString lmEquipmentId,
							   SimWidgetPrivate* simWidget);

	private:
		SimDialogSignalSnapshot(SimIdeSimulator* simulator,
								IAppSignalManager* appSignalManager,
								QString projectName,
								QString softwareEquipmentId,
								QString lmEquipmentId,
								QWidget* parent);

	private slots:
		void projectUpdated();

	private:
		virtual std::vector<VFrame30::SchemaDetails> schemasDetails() override;
		virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) override;

	private:
		SimIdeSimulator* m_simulator = nullptr;
	};
} // namespace SimUi