#include "TestSuiteLog.h"

Q_LOGGING_CATEGORY(testsuite_applog, "testsuite.applog")

TestSuiteLogFile::TestSuiteLogFile(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount, bool addAppInfoOnStart)
	:Log::LogFile(fileName, path, maxFileSize, maxFilesCount, addAppInfoOnStart)
{

}

bool TestSuiteLogFile::writeAlert(const QString& text, const QString& /*tag = {}*/)
{
	qCCritical(testsuite_applog).noquote() << text;
	return Log::LogFile::writeAlert(text);
}

bool TestSuiteLogFile::writeError(const QString& text, const QString& /*tag = {}*/)
{
	qCCritical(testsuite_applog).noquote() << text;
	return Log::LogFile::writeError(text);
}

bool TestSuiteLogFile::writeWarning(const QString& text, const QString& /*tag = {}*/)
{
	qCWarning(testsuite_applog).noquote() << text;
	return Log::LogFile::writeWarning(text);
}

bool TestSuiteLogFile::writeMessage(const QString& text, const QString& /*tag = {}*/)
{
	qCInfo(testsuite_applog).noquote() << text;
	return Log::LogFile::writeMessage(text);
}

bool TestSuiteLogFile::writeText(const QString& text, const QString& /*tag = {}*/)
{
	qCInfo(testsuite_applog).noquote() << text;
	return Log::LogFile::writeText(text);
}

TestSuiteTestLogOutput::TestSuiteTestLogOutput()
{
	setHtmlFont("Verdana");
}

QString TestSuiteTestLogOutput::htmlFont() const
{
	return m_htmlFont;
}

void TestSuiteTestLogOutput::setHtmlFont(QString fontName)
{
	m_htmlFont = fontName;
}

bool TestSuiteTestLogOutput::queueIsEmpty() const
{
	QReadLocker l(&m_lock);
	Q_UNUSED(l);
	return m_itemsQueue.empty() == true;
}

void TestSuiteTestLogOutput::pushQueue(const std::vector<TestSuite::TestLogItem>& in)
{
	QWriteLocker l(&m_lock);
	Q_UNUSED(l);

	for (const auto& item: in)
	{
		m_itemsQueue.push(item);
	}
}

void TestSuiteTestLogOutput::popQueue(std::vector<TestSuite::TestLogItem>* out, int maxCount)
{
	if (out == nullptr)
	{
		assert(out);
		return;
	}

	QReadLocker l(&m_lock);
	Q_UNUSED(l);

	while (maxCount > 0 && m_itemsQueue.empty() == false)
	{
		out->push_back(m_itemsQueue.front());
		m_itemsQueue.pop();

		maxCount --;
	}

	return;
}

void TestSuiteTestLogOutput::logItemArrived(const TestSuite::TestLogItem& item)
{
	QWriteLocker l(&m_lock);
	Q_UNUSED(l);
	m_itemsQueue.push(item);
}

