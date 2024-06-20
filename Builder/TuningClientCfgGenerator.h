#ifndef TUNINGCLIENTCFGGENERATOR_H
#define TUNINGCLIENTCFGGENERATOR_H

#include "SoftwareCfgGenerator.h"

#include "../OnlineLib/SoftwareSettings.h"

#include <TuningLib/TuningUiItem.h>
#include <ClientLib/TuningSignalManager.h>

namespace Builder
{

	class TuningClientCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TuningClientCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		SubsystemStorage* m_subsystems = nullptr;

		bool createTuningEquipmentList(QStringList* equipmentList);
		bool createEquipmentLists(const QStringList& equipmentList,const ISignalManager& tuningSignalManager);
		bool createSchemasLists(const ISignalManager& tuningSignalManager);

		
		bool writeTuningUi(const QStringList& appSignalListIds, const QStringList& appSignalListMasks, const QStringList& appSignalListTags);
		bool writeTuningSchemas();
		bool writeGlobalScript();
		bool writeTuningClientBehavior();
	};
}

#endif // TUNINGCLIENTCFGGENERATOR_H
