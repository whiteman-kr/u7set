#pragma once
#include <VFrame30/SchemaManager.h>


namespace TrendLib
{
	class TrendSignalParam;
	struct OneHourData;
} // namespace TrendLib

namespace SimUi
{
	class SimIdeSimulator;


	class SimSchemaManager : public VFrame30::SchemaManager
	{
		Q_OBJECT

	public:
		explicit SimSchemaManager(SimIdeSimulator* simulator, QObject* parent = nullptr);

	protected:
		virtual std::shared_ptr<VFrame30::Schema> loadSchema(const QString& schemaId) override;

		// RealTime Trends (ITrendDataProvider)
		//
	public:
		virtual bool trendData(QUuid trendUuid,
							   const TrendLib::TrendSignalParam& trendSignal,
							   QDateTime from,
							   QDateTime to,
							   E::TimeType timeType,
							   E::TrendMode mode,
							   std::list<std::shared_ptr<const TrendLib::OneHourData>>* outData) const override;

	public:
		SimIdeSimulator* simulator();
		const SimIdeSimulator* simulator() const;

		// Data
		//
	private:
		SimIdeSimulator* m_simulator = nullptr;
	};
} // namespace SimUi