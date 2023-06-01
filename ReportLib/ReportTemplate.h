#pragma once

namespace ReportLib
{
    class ObjectTemplate
	{
    public:
        enum class Type
        {
            Unknown,
            Text,
            Table
        };

        explicit ObjectTemplate(Type type);
        virtual ~ObjectTemplate();

        bool load(QXmlStreamReader& reader);
        QString typeStr() const;

        Type objectType{Type::Unknown};
        QString tag;

        QString fontName{"Arial"};
		int fontSize{12};
	};


    class TextTemplate : public ObjectTemplate
	{
    public:
        TextTemplate();
		bool load(QXmlStreamReader& reader);

		Qt::Alignment alignment{Qt::AlignLeft};

	};

    class TableTemplate : public ObjectTemplate
	{
    public:
        TableTemplate();
        bool load(QXmlStreamReader& reader);

		struct Column
		{
			QString caption;
			Qt::Alignment alignment;

		};
		std::vector<Column> columns;
        QString separator;
	};

    class SectionTemplate
	{
    public:
		bool load(QXmlStreamReader& reader);
		bool empty() const;

        QString caption;
		std::vector<std::shared_ptr<ObjectTemplate>> objects;
	};

    class ReportTemplate
	{
	public:
		ReportTemplate();

		bool load(QXmlStreamReader& reader);

        const QString& caption() const;
		const SectionTemplate& header() const;
		const SectionTemplate& footer() const;
		const SectionTemplate& pageHeader() const;
		const SectionTemplate& pageFooter() const;

        const std::vector<SectionTemplate>& sections() const;

	private:
        QString m_caption;

		SectionTemplate m_reportHeader;
		SectionTemplate m_reportFooter;
		SectionTemplate m_pageHeader;
		SectionTemplate m_pageFooter;

        std::vector<SectionTemplate> m_sections;
	};

	class ReportTemplateStorage
	{
	public:
		void clear();
		bool load(const QByteArray& data, QString* errorCode);
		const std::vector<ReportTemplate>& templates() const;

	private:
		std::vector<ReportTemplate> m_templates;
	};
}

