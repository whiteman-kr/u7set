#pragma once

#include "ReportObject.h"

namespace ReportLib
{
    class ObjectTemplate
	{
    public:
        explicit ObjectTemplate(ReportObject::Type type);
        virtual ~ObjectTemplate();

        bool load(QXmlStreamReader& reader);

        ReportObject::Type type() const;
        QString typeStr() const;

		virtual QString propToText() const = 0;

		const QString& text() const;
        const QString& tag() const;

    private:
        ReportObject::Type m_type{ReportObject::Type::Undefined};
		QString m_text;	// Static text
		QString m_tag;	// Tag to take text dynamically
    };


    class TextTemplate : public ObjectTemplate
	{
    public:
        TextTemplate();
		bool load(QXmlStreamReader& reader);

        const TextFormat& format() const;

		virtual QString propToText() const override;

    private:
        TextFormat m_format;
	};

    class TableTemplate : public ObjectTemplate
	{
    public:
        TableTemplate();
        bool load(QXmlStreamReader& reader);

        const TableFormat& format() const;
        const QString& separator() const;

		virtual QString propToText() const override;

    private:
        TableFormat m_format;
        QString m_separator;
	};

	class MarginTemplate
	{
	public:
		bool load(QXmlStreamReader& reader);

		const ReportMarginItem& marginItem() const;

	private:
		ReportMarginItem m_marginItem;
	};

    class SectionTemplate
	{
    public:
		bool load(QXmlStreamReader& reader);
		bool empty() const;

		const QPageLayout& pageLayout() const;

		const QString& caption() const;
		const QString& tag() const;
        const std::vector<std::shared_ptr<ObjectTemplate>>& objects() const;

    private:
		QPageLayout m_pageLayout = QPageLayout(QPageSize(QPageSize::A4),
											   QPageLayout::Orientation::Portrait,
											   QMarginsF(30, 20, 15, 20),
											   QPageLayout::Unit::Millimeter);

		QString m_caption;
		QString m_tag;	// Section tag
        std::vector<std::shared_ptr<ObjectTemplate>> m_objects;
	};

    class ReportTemplate
	{
	public:
		ReportTemplate();

		bool load(QXmlStreamReader& reader);

		int resolution() const;

		const QString& caption() const;
		const SectionTemplate& header() const;
		const SectionTemplate& footer() const;
		//const SectionTemplate& pageHeader() const;
		//const SectionTemplate& pageFooter() const;

        const std::vector<SectionTemplate>& sections() const;
		const std::vector<MarginTemplate>& margins() const;

	private:
		int m_resolution = 300;

        QString m_caption;

		SectionTemplate m_reportHeader;
		SectionTemplate m_reportFooter;
		//SectionTemplate m_pageHeader;
		//SectionTemplate m_pageFooter;

        std::vector<SectionTemplate> m_sections;
		std::vector<MarginTemplate> m_margins;
	};

	class ReportTemplateStorage
	{
	public:
		void clear();
		bool load(const QByteArray& data, QString* errorCode);
		const std::vector<ReportTemplate>& templates() const;

		const ReportTemplate& templateByCaption(const QString& caption, bool* found) const;

	private:
		std::vector<ReportTemplate> m_templates;
	};
}

