#pragma once

#include "ReportSchemaView.h"

namespace ReportLib
{
	//
	// ReportFormat
	//

	class ReportObjectFormat
	{
		// Formatting functions
public:
		ReportObjectFormat() = default;
		ReportObjectFormat(const QFont& font, Qt::Alignment alignment = Qt::AlignLeft);
		ReportObjectFormat(const QString& fontName, double fontPointSize, Qt::Alignment alignment);

		void setFont(const QFont& font);
		void setTextForeground(const QBrush& brush);
		void setTextBackground(const QBrush& brush);
		void setTextAlignment(Qt::Alignment alignment);

		const QTextCharFormat& charFormat() const;
		const QTextBlockFormat& blockFormat() const;

	private:
		QTextCharFormat m_charFormat;
		QTextBlockFormat m_blockFormat;
	};

	// ReportMarginItem

	struct ReportMarginItem
	{
		ReportMarginItem(const QString& text, int pageFrom, int pageTo, const ReportObjectFormat& format);

		QString text;

		int pageFrom = -1;
		int pageTo = -1;

		ReportObjectFormat format;
	};

	//
	// ReportObject
	//
	class ReportObject
	{
	public:
		ReportObject(const ReportObjectFormat& format);

		virtual void renderText(QTextCursor& cursor) const = 0;

	protected:
		// Format
		//
		ReportObjectFormat m_format;
	};

	//
	// ReportSchema
	//

	class ReportSchema : public ReportLib::ReportObject
	{
	public:
		static std::shared_ptr<ReportSchema> create(const QString& caption,
													const ReportObjectFormat& format,
													std::shared_ptr<VFrame30::Schema> schema,
													const std::map<QUuid, ReportSchemaCompareAction>& compareActions);

		ReportSchema(const QString& caption,
					 const ReportObjectFormat& format,
					 std::shared_ptr<VFrame30::Schema> schema,
					 const std::map<QUuid, ReportSchemaCompareAction>& compareActions);

		// Schema functions

		void renderText(QTextCursor& cursor) const override;
		std::shared_ptr<VFrame30::Schema> schema() const;

		const std::map<QUuid, ReportSchemaCompareAction>& compareActions() const;

	private:
		QString m_caption;

		// Schema data
		//
		std::shared_ptr<VFrame30::Schema> m_schema;
		std::map<QUuid, ReportSchemaCompareAction> m_compareActions;
	};

	//
	// ReportTable
	//

	class ReportTable : public ReportObject
	{
	public:
		static std::shared_ptr<ReportTable> create(const QStringList& headerLabels,
												   const std::vector<int>& columnWidths,
												   const ReportObjectFormat& format);
		ReportTable(const QStringList& headerLabels,
					const std::vector<int>& columnWidths,
					const ReportObjectFormat& format);

		int columnCount() const;
		int rowCount() const;

		const QStringList& rowAt(int index) const;

		void insertRow(const QStringList& row);

		void sortByColumn(int column);

		void renderText(QTextCursor& cursor) const override;

	private:
		QStringList m_headerLabels;
		std::vector<int> m_columnWidths;

		std::vector<QStringList> m_rows;
	};

	//
	// ReportText
	//

	class ReportText : public ReportObject
	{
	public:
		static std::shared_ptr<ReportText> create(const QString& text, const ReportObjectFormat& format);
		ReportText(const QString& text, const ReportObjectFormat& format);

		void renderText(QTextCursor& cursor) const override;

	private:
		QString m_text;
	};

}
