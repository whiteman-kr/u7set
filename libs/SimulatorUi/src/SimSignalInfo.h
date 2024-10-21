#pragma once

#include <ClientLib/TuningConnectionStub.h>
#include <SchemaClientLib/DialogSignalInfo.h>

#include "../../AppSignalLib/AppSignal.h"

#include <optional>


namespace SimUi
{
	class SimWidgetPrivate;
	class SimIdeSimulator;


	class SimSignalInfo : public DialogSignalInfo
	{
		Q_OBJECT

	public:
		static bool showDialog(QString appSignalId, SimIdeSimulator* simuator, SimWidgetPrivate* simWidget);

	private:
		SimSignalInfo(const AppSignalParam& signal, SimIdeSimulator* simuator, SimWidgetPrivate* simWidget);

	private slots:
		void onSignalParamAndUnitsArrived();

	signals:
		void openSchema(QString schemaId, QStringList highlightIds);

	private:
		virtual QStringList schemasByAppSignalId(const QString& appSignalId) override;
		virtual void setSchema(QString schemaId, QStringList highlightIds) override;
		virtual std::optional<AppSignal> getSignalExt(const AppSignalParam& appSignalParam) override;

	private:
		SimIdeSimulator* m_simulator = nullptr;

		ClientLib::TuningConnectionStub m_tuningConnection;
		TuningAuthorizationStub m_tuningAuthorization;
	};
} // namespace SimUi
