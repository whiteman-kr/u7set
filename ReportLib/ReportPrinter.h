#pragma once

#include "Report.h"

namespace ReportLib
{



	class PrintObject
	{
	public:
		enum class Type
		{
			Undefined,
			Text,
			Schema,
			NewPage
		};

		virtual QRect contentRect() const = 0;

		virtual void print(QPdfWriter& pdfWriter, QPainter& painter, int vOffset) = 0;

	protected:
		Type m_type{Type::Undefined};
	};

	class PrintText : public PrintObject
	{
	public:
		PrintText(QSizeF pageSize);

		QTextCursor& textCursor();

		virtual QRect contentRect() const override;

		virtual void print(QPdfWriter& pdfWriter, QPainter& painter, int vOffset) override;

	private:
		QTextDocument m_textDocument;
		QTextCursor m_textCusror;
	};

	class PrintNewPage : public PrintObject
	{
	public:
		PrintNewPage();

		virtual QRect contentRect() const override;

		virtual void print(QPdfWriter& pdfWriter, QPainter& painter, int vOffset) override;
	};


	class PrintSchema: public PrintObject
	{
	public:
		PrintSchema(const std::shared_ptr<ReportSchemaView>& schemaView,
					const std::shared_ptr<VFrame30::Schema>& schema,
					const std::map<QUuid, ReportSchemaCompareAction>& compareActions);

		virtual QRect contentRect() const override;

		virtual void print(QPdfWriter& pdfWriter, QPainter& painter, int vOffset) override;

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

	private:
		ReportPrinter() = default;
	public:
		ReportPrinter(std::shared_ptr<ReportSchemaView> reportSchemaView);

		bool print(const Report& report, const QString& fileName, std::atomic_bool& stop);
		bool print(const Report& report, QBuffer& buffer, std::atomic_bool& stop);

		Statistics statistics() const;

	private:
		/*void printMarginItems(QPdfWriter& pdfWriter,
							  QPainter& painter,
							  const QString& objectName,
							  const std::vector<ReportMarginItem>& marginItems) const;*/


		/*
		void printSection(QPdfWriter&
							  pdfWriter,
							  QPainter& painter,
							  ReportSection& section,
							  const std::vector<ReportMarginItem>& marginItems) const;

		void printSectionSchema(QPdfWriter& pdfWriter,
								QPainter& painter,
								ReportSection& section) const;*/

	private:
		std::vector<std::shared_ptr<PrintObject>> m_printObjects;

		mutable QMutex m_statisticsMutex;
		mutable Statistics m_statistics;

		std::shared_ptr<ReportSchemaView> m_schemaView;
	};
}
