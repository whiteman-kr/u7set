#pragma once

#include "ReportSchemaView.h"
#include "../VFrame30/IViewVariables.h"

#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextCursor>


namespace ReportLib
{
	class ReportSection;

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
	// ReportVariables
	//
	class ReportVariables : public VFrame30::IViewVariables
	{
	public:
		// IViewVariables implementation
		//
		[[nodiscard]] bool variableExists(const QString& name) const override;
		[[nodiscard]] QVariant variable(const QString& name) const override;
		void setVariable(const QString& name, const QVariant& value) override;

		//
		void setVariables(const std::map<QString, QString>& variables);

	private:
		std::map<QString, QString> m_variables;
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

	class ReportTagStorage
	{

	public:
		ReportTagStorage(const std::map<QString, std::shared_ptr<ReportSection> >& allSections);

		QString processTags(const QString& str) const;

	public:
		// Usage: $PAGE(SECTIONCAPTION), replaces SECTIONCAPTION with section start page
		static inline QLatin1String tagSectionStartPage{"$PAGE"};

	private:
		const std::map<QString, std::shared_ptr<ReportSection>>& m_allSections;
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

		virtual void renderText(QTextCursor& cursor, double fontScaling, const ReportTagStorage& tagStorage) const = 0;

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

		void renderText(QTextCursor& cursor, double fontScaling, const ReportTagStorage& tagStorage) const override;
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

		ReportTable(const TableFormat& format);

		bool htmlEscaped() const;
		void setHtmlEscaped(bool value);

		int columnCount() const;
		int rowCount() const;

		const QStringList& rowAt(int index) const;
		void insertRow(const QStringList& row);

		void sortByColumn(int column);

		void renderText(QTextCursor& cursor, double fontScaling, const ReportTagStorage& tagStorage) const override;

	private:
		TableFormat m_format;
		std::vector<QStringList> m_rows;
		bool m_htmlEscaped = true;	// If set to false, HTML signs (<>) are NOT replaced automatically, User is responsible for that
	};

	//
	// ReportText
	//

	class ReportText : public ReportObject
	{
	public:
		static std::shared_ptr<ReportText> create(const QString& text, const TextFormat &format);
		ReportText(const QString& text, const TextFormat& format);

		void renderText(QTextCursor& cursor, double fontScaling, const ReportTagStorage& tagStorage) const override;

	private:
		TextFormat m_format;
		QString m_text;
	};
}
