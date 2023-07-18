#pragma once

#include "ReportSchemaView.h"

#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextCursor>

namespace ReportLib
{
	struct ReportFont
	{
		QString family;
		int pointSize{0};
		QFont::Weight weight{QFont::Normal};

		operator QFont() const
		{
			return QFont{family, pointSize, weight};
		}


		QFont operator()(double scaling) const
		{
			return QFont{family, static_cast<int>(pointSize * scaling), weight};
		}
	};

	//
	// Object Formats
	//

	struct TextFormat
	{
	public:
		TextFormat() = default;
		TextFormat(const ReportFont& font, Qt::Alignment alignment);

		const ReportFont& font() const;
		Qt::Alignment alignment() const;

	private:
		ReportFont m_font{"Arial", 12, QFont::Normal};
		Qt::Alignment m_alignment{Qt::AlignLeft};
	};

	struct TableFormat
	{
	public:
		struct ColumnFormat
		{
			QString caption;
			int width{30};                          // column width in percent
			Qt::Alignment alignment{Qt::AlignLeft};
		};

		TableFormat() = default;

		TableFormat(const ReportFont& font,
					const QStringList& headerLabels,
					const std::vector<int> columnWidths,
					Qt::Alignment alignment);

		TableFormat(const ReportFont& font, const std::vector<ColumnFormat>& columnsFormat);

		const ReportFont& font() const;
		const std::vector<ColumnFormat>& columnsFormat() const;

	private:
		ReportFont m_font{"Arial", 12, QFont::Normal};
		std::vector<ColumnFormat> m_columnsFormat;
	};

	struct SchemaFormat
	{
	};

	//
	// ReportMarginItem
	//

	struct ReportMarginItem
	{
		ReportMarginItem() = default;
		ReportMarginItem(const QString& text, int pageFrom, int pageTo, const TextFormat &format);

		QString text;

		int pageFrom = -1;
		int pageTo = -1;

		TextFormat format;
	};

	//
	// ReportObject
	//
	class ReportObject
	{
	public:
		enum class Type
		{
			Undefined,
			Text,
			Table,
			Schema
		};


		ReportObject(Type type);

		Type type() const;

		virtual void renderText(QTextCursor& cursor, double fontScaling) const = 0;

	protected:
		Type m_type;
	};

	//
	// ReportSchema
	//

	class ReportSchema : public ReportLib::ReportObject
	{
	public:
		static std::shared_ptr<ReportSchema> create(const QString& caption,
													const SchemaFormat& format,
													std::shared_ptr<VFrame30::Schema> schema,
													const std::map<QUuid, ReportSchemaCompareAction>& compareActions);

		ReportSchema(const QString& caption,
					 const SchemaFormat& format,
					 std::shared_ptr<VFrame30::Schema> schema,
					 const std::map<QUuid, ReportSchemaCompareAction>& compareActions);

		// Schema functions

		void renderText(QTextCursor& cursor, double fontScaling) const override;
		std::shared_ptr<VFrame30::Schema> schema() const;

		const std::map<QUuid, ReportSchemaCompareAction>& compareActions() const;

	private:
		QString m_caption;
		SchemaFormat m_format;

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
		static std::shared_ptr<ReportTable> create(const TableFormat& format);

		/*ReportTable(const QString& fontName,
					double fontPointSize,
					const QStringList& headerLabels,
					const std::vector<int>& columnWidths,
					Qt::Alignment alignment);*/

		ReportTable(const TableFormat& format);

		int columnCount() const;
		int rowCount() const;

		const QStringList& rowAt(int index) const;

		void insertRow(const QStringList& row);

		void sortByColumn(int column);

		void renderText(QTextCursor& cursor, double fontScaling) const override;

	private:
		TableFormat m_format;
		std::vector<QStringList> m_rows;
	};

	//
	// ReportText
	//

	class ReportText : public ReportObject
	{
	public:
		static std::shared_ptr<ReportText> create(const QString& text, const TextFormat &format);
		ReportText(const QString& text, const TextFormat& format);

		void renderText(QTextCursor& cursor, double fontScaling) const override;

	private:
		TextFormat m_format;
		QString m_text;
	};
}
