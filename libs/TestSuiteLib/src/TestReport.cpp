#include <TestSuiteLib/TestReport.h>
#include <TestSuiteLib/TestLog.h>
#include <ReportLib/ReportTemplate.h>
#include <UiLib/UiTools.h>

#include <QApplication>
#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

namespace TestSuite
{
	//
	// TestReportDataProvider
	//
	TestReportDataProvider::TestReportDataProvider(const TestLog& testLog) :
		m_items(testLog.items())
	{
	}

	TestReportDataProvider::~TestReportDataProvider() = default;

	int TestReportDataProvider::count() const 
	{
		return static_cast<int>(m_items.size());
	}

	int TestReportDataProvider::count(const QString& /*tag*/) const
	{
		return 0;
	}

	QString TestReportDataProvider::text(int index, QString* tag) const
	{
		if (index < 0 || index >= m_items.size())
		{
			Q_ASSERT(false);
			return QString();
		}

		if (tag != nullptr)
		{
			*tag = m_items[index].tag();
		}

		return m_items[index].message();
	}

	QString TestReportDataProvider::text(const QString& tag, bool* found) const
	{
		if (found == nullptr)
		{
			Q_ASSERT(found);
			return QString();
		}

		if (m_lastTag != tag)
		{
			m_lastTag = tag;
			m_lastIndex = 0;
		}
		else
		{
			m_lastIndex++;
		}

		if (m_items.empty() == true)
		{
			*found = false;
			return QString();
		}

		int count = static_cast<int>(m_items.size());
		for (; m_lastIndex < count; m_lastIndex++)
		{
			if (m_items[m_lastIndex].tag() == tag)
			{
				*found = true;
				return m_items[m_lastIndex].message();
			}
		}

		*found = false;
		return QString();
	}

	//
	// TestReport
	//
	void TestReport::generateReport(const ReportLib::ReportTemplateStorage& templates,
									const TestSuite::TestLog& testLog,
									const QString& caption,
									QWidget* parent)
	{
		QString fileName = QFileDialog::getSaveFileName(parent,
														QObject::tr("Save File"),
														QObject::tr("%1.pdf").arg(caption),
														QObject::tr("PDF Files (*.pdf);;All Files (*.*)"));
		if (fileName.isEmpty() == true)
		{
			return;
		}

		bool found = false;
		const ReportLib::ReportTemplate& templ = templates.templateByCaption(caption, &found);
		if (found == false)
		{
			Q_ASSERT(found);
			return;
		}

		TestReportDataProvider dataProvider(testLog);
		ReportLib::TaggedReportGenerator generator(templ, dataProvider);

		std::atomic_bool stop = false;

		QBuffer buffer;
		if (generator.generate(buffer, stop) == false)
		{
			QMessageBox::critical(parent, qAppName(), QObject::tr("Report '%1' generation error!").arg(caption));
			return;
		}

		QFile f(fileName);
		if (f.open(QFile::WriteOnly) == false || f.write(buffer.data()) == false)
		{
			QMessageBox::critical(parent, qAppName(), QObject::tr("Report file '%1' saving error!").arg(fileName));
		}
		else
		{
			if (QMessageBox::question(parent, qAppName(), QObject::tr("Report generating has been finished.\n\nDo you wish to open it?")) == QMessageBox::Yes)
			{
				UiTools::openPdf(fileName, parent);
			}
		}
	}

	void TestReport::generateReports(const ReportLib::ReportTemplateStorage& templates,
									 const TestSuite::TestLog& testLog,
									 const QString& captionMask, // if empty - generate all
									 const QString& path,
									 ILogFile* appLog)
	{
		if (QDir().mkpath(path) == false)
		{
			appLog->writeError(QObject::tr("Report path '%1' creating error!").arg(path));
			return;
		}

		for (const ReportLib::ReportTemplate& templ : templates.templates())
		{
			bool filterMatch = true;
			if (captionMask.isEmpty() == false)
			{
				QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(captionMask));

				if (rx.match(templ.caption()).hasMatch() == false)
				{
					filterMatch = false;
					break;
				}
			}
			if (filterMatch == false)
			{
				continue;
			}

			TestReportDataProvider dataProvider(testLog);
			ReportLib::TaggedReportGenerator generator(templ, dataProvider);

			std::atomic_bool stop = false;

			QBuffer buffer;
			if (generator.generate(buffer, stop) == false)
			{
				appLog->writeError(QObject::tr("Report '%1' generation error!").arg(templ.caption()));
				return;
			}

			QString fileName = QString("%1%2%3.pdf").arg(path).arg(QDir::separator()).arg(templ.caption());

			QFile f(fileName);
			if (f.open(QFile::WriteOnly) == false || f.write(buffer.data()) == false)
			{
				appLog->writeError(QObject::tr("Report file '%1' saving error!").arg(fileName));
			}
			else
			{
				appLog->writeMessage(QObject::tr("Report file '%1' saved successfully.").arg(fileName));
			}
		}
	}
} // namespace TestSuite
