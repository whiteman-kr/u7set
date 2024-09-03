#pragma once

#include "Report.h"
#include <QMutex>
#include <QTextCursor>
#include <QTextDocument>

class QBuffer;
class QPdfWriter;
class QPainter;


namespace ReportLib
{
	class ReportPrinter;

	class PrintObject
	{
	public:
		virtual ~PrintObject() = default;

		enum class Type
		{
			Undefined,
			Text,
			Schema
		};

		virtual QRect contentRect() const = 0;

		Type type() const {return m_type;}

		virtual void print(Report& report,
						   ReportPrinter& printer,
						   QPdfWriter& pdfWriter,
						   QPainter& painter,
						   int& pageIndex,
						   QMutex& pageCounterMutex) = 0;

		virtual int pageCount() const = 0;

		virtual QString tag() const {return m_tag;}

	protected:
		Type m_type{Type::Undefined};
		int m_verticalOffset{0};
		bool m_newPageBefore{false};
		QString m_tag;	// Text printer in margin with %TAG% text
	};

	class PrintText : public PrintObject
	{
	public:
		PrintText(QSizeF pageSize, int verticalOffset, bool newPageBefore, const QString& tag);

		QTextCursor& textCursor();

		virtual QRect contentRect() const override;

		virtual void print(Report& report,
						   ReportPrinter& printer,
						   QPdfWriter& pdfWriter,
						   QPainter& painter,
						   int& pageIndex,
						   QMutex& pageCounterMutex) override;

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
					bool newPageBefore,
					const QString& tag);

		virtual QRect contentRect() const override;

		virtual void print(Report& report,
						   ReportPrinter& printer,
						   QPdfWriter& pdfWriter,
						   QPainter& painter,
						   int& pageIndex,
						   QMutex& pageCounterMutex) override;

		virtual int pageCount() const override;

	private:
		// Schema data
		//
		std::shared_ptr<ReportSchemaView> m_schemaView;
		std::shared_ptr<VFrame30::Schema> m_schema;
		std::map<QUuid, ReportSchemaCompareAction> m_compareActions;
	};

	//
	// RenderedSection
	//
	struct RenderedSection
	{
		RenderedSection(std::shared_ptr<ReportSection> section):
			m_section(section)
		{
		}

		int pagesCount() const
		{
			int result = 0;
			for (const std::shared_ptr<PrintObject>& po : m_printObjects)
			{
				result += po->pageCount();
			}
			return result;
		}

		// ReportSection access
		//
		std::shared_ptr<ReportSection>& section()
		{
			return m_section;
		}
		const std::shared_ptr<ReportSection>& section() const
		{
			return m_section;
		}

		// Data access
		//
		const QPageLayout& pageLayout() const
		{
			return m_section->pageLayout();
		}

		// Rendered objects access
		//
		std::vector<std::shared_ptr<PrintObject>>& printObjects()
		{
			return m_printObjects;
		}
		const std::vector<std::shared_ptr<PrintObject>>& printObjects() const
		{
			return m_printObjects;
		}

	private:
		std::shared_ptr<ReportSection> m_section;
		std::vector<std::shared_ptr<PrintObject>> m_printObjects;
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
				Preview,
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

		bool preview(const Report& report, std::vector<RenderedSection>& renderedSections, std::atomic_bool& stop);

		bool print(Report& report, const QString& fileName, std::atomic_bool& stop);
		bool print(Report& report, QBuffer& buffer, std::atomic_bool& stop);

		Statistics statistics() const;

		void printMarginItems(const Report& report,
							  QPdfWriter& pdfWriter,
							  QPainter& painter,
							  const QString& tag) const;

	private:
		[[nodiscard]] bool createRenderedSections(const Report& report, std::vector<RenderedSection>& renderedSections, Statistics::Status status, std::atomic_bool& stop);
		[[nodiscard]] bool printRenderedSections(Report& report, const std::vector<RenderedSection>& renderedSections, QBuffer& buffer, std::atomic_bool& stop);

		mutable QMutex m_statisticsMutex;
		mutable Statistics m_statistics;

		std::shared_ptr<ReportSchemaView> m_schemaView;
	};
}
