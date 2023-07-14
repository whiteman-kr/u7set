#pragma once

#include "Report.h"

#include <QPdfWriter>
#include <QTextDocument>

namespace ReportLib
{

	class ReportPrinter;

	class PrintObject
	{
	public:
		enum class Type
		{
			Undefined,
			Text,
			Schema
		};

		virtual QRect contentRect() const = 0;

		Type type() const {return m_type;}

		virtual void print(ReportPrinter& printer, QPdfWriter& pdfWriter, QPainter& painter, const std::vector<ReportMarginItem>& marginItems,
						   int pageCount, int& pageIndex, QMutex& pageCounterMutex) = 0;

		virtual int pageCount() const = 0;

	protected:
		Type m_type{Type::Undefined};
		int m_verticalOffset{0};
		bool m_newPageBefore{false};
	};

	class PrintText : public PrintObject
	{
	public:
		PrintText(QSizeF pageSize, int verticalOffset, bool newPageBefore);

		QTextCursor& textCursor();

		virtual QRect contentRect() const override;

		virtual void print(ReportPrinter& printer, QPdfWriter& pdfWriter, QPainter& painter, const std::vector<ReportMarginItem>& marginItems,
						   int pageCount, int& pageIndex, QMutex& pageCounterMutex) override;

		virtual int pageCount() const override;

	private:
		QTextDocument m_textDocument;
		QTextCursor m_textCusror;
	};

	class PrintSchema: public PrintObject
	{
	public:
		PrintSchema(const std::shared_ptr<ReportSchemaView>& schemaView,
					const std::shared_ptr<VFrame30::Schema>& schema,
					const std::map<QUuid, ReportSchemaCompareAction>& compareActions,
					int verticalOffset,
					bool newPageBefore);

		virtual QRect contentRect() const override;

		virtual void print(ReportPrinter& printer, QPdfWriter& pdfWriter, QPainter& painter, const std::vector<ReportMarginItem>& marginItems,
						   int pageCount, int& pageIndex, QMutex& pageCounterMutex) override;

		virtual int pageCount() const override;

	private:
		// Schema data
		//
		std::shared_ptr<ReportSchemaView> m_schemaView;
		std::shared_ptr<VFrame30::Schema> m_schema;
		std::map<QUuid, ReportSchemaCompareAction> m_compareActions;
	};

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

	public:
		ReportPrinter() = default;	// Call this constructor if you do not need to print schemas
		ReportPrinter(std::shared_ptr<ReportSchemaView> reportSchemaView); // Call this constructor if your report contains schemas

		bool print(const Report& report, const QString& fileName, std::atomic_bool& stop);
		bool print(const Report& report, QBuffer& buffer, std::atomic_bool& stop);

		Statistics statistics() const;

		void printMarginItems(QPdfWriter& pdfWriter,
							  QPainter& painter,
							  const QString& objectName,
							  const std::vector<ReportMarginItem>& marginItems) const;

	private:
		mutable QMutex m_statisticsMutex;
		mutable Statistics m_statistics;

		std::shared_ptr<ReportSchemaView> m_schemaView;
	};
}
