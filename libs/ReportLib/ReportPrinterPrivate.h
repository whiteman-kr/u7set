#pragma once

#include <ReportLib/Report.h>
#include <ReportLib/ReportObject.h>
#include <ReportLib/ReportPrinter.h>

class QPrinter;
class QPagedPaintDevice;
class QPageLayout;

namespace ReportLib
{
	class ReportPrinter;
	class ReportSchemaView;

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
						   ReportPrinterPrivate& printer,
						   QPagedPaintDevice& pdfWriter,
						   QPainter& painter,
						   int& pageIndex,
						   QMutex& pageCounterMutex,
						   std::atomic_bool& stop) = 0;

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
						   ReportPrinterPrivate& printer,
						   QPagedPaintDevice& pdfWriter,
						   QPainter& painter,
						   int& pageIndex,
						   QMutex& pageCounterMutex,
						   std::atomic_bool& stop) override;

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
						   ReportPrinterPrivate& printer,
						   QPagedPaintDevice& pdfWriter,
						   QPainter& painter,
						   int& pageIndex,
						   QMutex& pageCounterMutex,
						   std::atomic_bool& stop) override;

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

	class ReportPrinterPrivate// : public QObject
	{
	public:
		ReportPrinterPrivate() = default;	// Call this constructor if you do not need to print schemas
		ReportPrinterPrivate(std::shared_ptr<ReportSchemaView> reportSchemaView); // Call this constructor if your report contains schemas
		~ReportPrinterPrivate();

		bool preview(const Report& report, std::vector<RenderedSection>& renderedSections, std::atomic_bool& stop);

		bool save(Report& report, const QString& fileName, std::atomic_bool& stop);
		bool save(Report& report, QBuffer& buffer, std::atomic_bool& stop);

		bool print(Report& report, QPrinter& printer, std::atomic_bool& stop);

		Statistics statistics() const;

		void printMarginItems(const Report& report, QPagedPaintDevice& pdfWriter, QPainter& painter, const QString& tag) const;

	private:

		[[nodiscard]] bool createRenderedSections(const Report& report,
												  std::vector<RenderedSection>& renderedSections,
												  Statistics::Status status,
												  std::atomic_bool& stop);
		[[nodiscard]] bool saveRenderedSections(Report& report, const std::vector<RenderedSection>& renderedSections, QBuffer& buffer, std::atomic_bool& stop);
		[[nodiscard]] bool printRenderedSections(Report& report,
												 const std::vector<RenderedSection>& renderedSections,
												 QPrinter& printer,
												 std::atomic_bool& stop);

		mutable QMutex m_statisticsMutex;
		mutable Statistics m_statistics;

		std::shared_ptr<ReportSchemaView> m_schemaView;
	};
}
