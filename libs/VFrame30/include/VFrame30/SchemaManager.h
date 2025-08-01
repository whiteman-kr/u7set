#pragma once

#include <TrendView/ITrendDataProvider.h>
#include <VFrame30/Schema.h>

namespace VFrame30
{
	class SchemaManager : public QObject,
						  public TrendLib::ITrendDataProvider
	{
		Q_OBJECT

	public:
		explicit SchemaManager(QObject* parent = nullptr);
		virtual ~SchemaManager();

		// Methods to use
		//
	public:
		void clear();

		// Get loaded schema
		//
		std::shared_ptr<VFrame30::Schema> schema(QString schemaId, std::shared_ptr<Context> context);

		virtual int schemaCount() const;
		virtual std::shared_ptr<VFrame30::Schema> schemaByIndex(int schemaIndex, std::shared_ptr<Context> context);

		virtual QString schemaCaptionById(const QString& schemaId) const;
		virtual QString schemaCaptionByIndex(int schemaIndex) const;
		virtual QString schemaIdByIndex(int schemaIndex) const;

		// RealTime Trends (ITrendDataProvider)
		//
		virtual bool trendData(QUuid trendUuid,
							   const TrendLib::TrendSignalParam& trendSignal,
							   QDateTime from,
							   QDateTime to,
							   E::TimeType timeType,
							   E::TrendMode mode,
							   std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const override;

		virtual TimeStamp maxTimeStamp(QUuid trendUuid, E::TimeType timeType) const override;

	protected:
		// Load schema, must be overriden to perform loading schema appropriate to client.
		// Default implementation: assert(false);
		//
		virtual std::shared_ptr<VFrame30::Schema> loadSchema(const QString& schemaId);

		// Signals
		//
	signals:
		void schemasWereReseted();

	private:
	};

} // namespace VFrame30
