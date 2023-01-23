#include "TestResultLog.h"

TestLogItem::TestLogItem(const QString& test)
	:m_message(test)
{

}

QString TestLogItem::toText() const
{
	return m_message;
}

TestResultLog::TestResultLog()
{
	qRegisterMetaType<TestLogItem>();
}

void TestResultLog::addMessage(const QString& text)
{
	TestLogItem ti(text);
	m_items.push_back(ti);
	emit newLogItem(ti);
}
