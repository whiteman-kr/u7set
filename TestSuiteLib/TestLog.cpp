#include "TestLog.h"

TestLogItem::TestLogItem(const QString& text, TestLogItemType type)
	:m_message(text),
	  m_type(type),
	  m_dateTime(QDateTime::currentDateTime())
{

}

QString TestLogItem::toText() const
{
	QString time = m_dateTime.toString("dd.MM.yyyy hh:mm:ss");

	QString type;
	switch(m_type)
	{
	case TestLogItemType::Message:
		type = "MSG";
		break;
	case TestLogItemType::Error:
		type = "ERR";
		break;
	default:
		type = "???";
		Q_ASSERT(false);
	}

	return QObject::tr("%1 [%2] %3").arg(time).arg(type).arg(m_message);
}

TestLog::TestLog()
{
	qRegisterMetaType<TestLogItem>();
}

void TestLog::addMessage(const QString& text)
{
	TestLogItem ti(text, TestLogItemType::Message);
	emit newLogItem(ti);

	QMutexLocker l(&m_itemsMutex);
	m_items.push_back(ti);
}

void TestLog::addError(const QString& text)
{
	TestLogItem ti(text, TestLogItemType::Error);
	emit newLogItem(ti);

	QMutexLocker l(&m_itemsMutex);
	m_items.push_back(ti);
}
