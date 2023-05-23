#pragma once

#include "Report.h"

namespace ReportLib
{
	//
	// ReportPrinter
	//

	class ReportPrinter : public QObject
	{
	public:
		struct Statistics
		{
			enum Status
			{
				None,
				Rendering,
				Printing
			};

			int sectionCount = 0;
			int sectionIndex = 0;

			int pagesCount = 0;	// Calculated after text rendering
			int pageIndex = 0;

			Status status{None};
		};

	private:
		ReportPrinter() = default;
	public:
		ReportPrinter(std::shared_ptr<ReportSchemaView> reportSchemaView);

		bool print(const Report& report, const QString& fileName, std::atomic_bool& stop);
		bool print(const Report& report, QBuffer& buffer, std::atomic_bool& stop);

		Statistics statistics() const;

	private:
		void printMarginItems(QPdfWriter& pdfWriter,
							  QPainter& painter,
							  const QString& objectName,
							  const std::vector<ReportMarginItem>& marginItems) const;

		void printSection(QPdfWriter&
							  pdfWriter,
							  QPainter& painter,
							  ReportSection& section,
							  const std::vector<ReportMarginItem>& marginItems) const;

		void printSectionSchema(QPdfWriter& pdfWriter,
								QPainter& painter,
								ReportSection& section) const;

	private:
		mutable QMutex m_statisticsMutex;
		mutable Statistics m_statistics;

		std::shared_ptr<ReportSchemaView> m_schemaView;
	};
}
