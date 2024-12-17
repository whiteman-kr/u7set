#pragma once

class QBuffer;

namespace ReportLib
{
	class TaggedReportPrivate;
	class ReportTemplate;
	struct Statistics;

	//
	// ITaggedReportDataProvider
	//
	class ITaggedReportDataProvider
	{
	public:
		virtual int count() const = 0;
		virtual int count(const QString& tag) const = 0;

		virtual QString text(int index, QString* tag) const = 0;
		virtual QString text(const QString& tag, bool* ok) const = 0;
	};

	//
	// TaggedReportGenerator
	//
	class TaggedReportGenerator : public QObject
	{
	public:
		explicit TaggedReportGenerator(const ReportTemplate& reportTemplate, const ITaggedReportDataProvider& dataProvider);
		virtual ~TaggedReportGenerator() = default;

		bool generate(QBuffer& buffer, std::atomic_bool& stop);

		Statistics statistics() const;

	private:
		TaggedReportPrivate* m_impl = nullptr;
	};
} // namespace ReportLib
