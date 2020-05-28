#pragma once

#include <QtGlobal>
#include <QElapsedTimer>
#include <QDebug>

#define ASSERT_RESULT_FALSE_BREAK	Q_ASSERT(false); \
									result = false; \
									break;

#define RESULT_FALSE_BREAK			result = false; \
									break;

#define ASSERT_RETURN_FALSE			Q_ASSERT(false); \
									return false;

#define ASSERT_FALSE_CONTINUE		Q_ASSERT(false); \
									continue;

#define TEST_PTR_RETURN_FALSE(ptr)	if (ptr == nullptr) \
									{	\
										Q_ASSERT(false);	\
										return false; \
									}

#define TEST_PTR_RETURN_NULLPTR(ptr)	if (ptr == nullptr) \
									{	\
										Q_ASSERT(false);	\
										return nullptr; \
									}

#define TEST_PTR_LOG_RETURN_FALSE(ptr, log)	if (ptr == nullptr) \
											{	\
												Q_ASSERT(false);	\
												LOG_NULLPTR_ERROR(log); \
												return false; \
											}

#define TEST_PTR_LOG_RETURN_NULLPTR(ptr, log)	if (ptr == nullptr) \
											{	\
												Q_ASSERT(false);	\
												LOG_NULLPTR_ERROR(log); \
												return nullptr; \
											}

#define TEST_PTR_LOG_RETURN_NULLPTR(ptr, log)	if (ptr == nullptr) \
												{	\
													Q_ASSERT(false);	\
													LOG_NULLPTR_ERROR(log); \
													return nullptr; \
												}

#define TEST_PTR_LOG_RETURN(ptr, log)		if (ptr == nullptr) \
											{	\
												Q_ASSERT(false);	\
												LOG_NULLPTR_ERROR(log); \
												return; \
											}

#define TEST_PTR_CONTINUE(ptr)		if (ptr == nullptr) \
									{	\
										Q_ASSERT(false);	\
										continue; \
									}

#define TEST_PTR_RETURN(ptr)		if (ptr == nullptr) \
									{	\
										Q_ASSERT(false);	\
										return; \
									}

#define DELETE_IF_NOT_NULL(ptr)		if (ptr != nullptr) \
									{	\
										delete ptr; \
										ptr = nullptr; \
									}


#define DEBUG_STOP					{ int a = 0; a++; }

#define RETURN_IF_FALSE(result)		if (result == false) \
									{ \
										return false; \
									}


#define AUTO_LOCK(mutex) QMutexLocker m(&mutex);


#define C_STR(qstring) qstring.toStdString().c_str()


void swapBytes(const char* src, char* dest, int size);


template <typename TYPE>
TYPE reverseBytes(TYPE value)
{
	TYPE dest;

	swapBytes(reinterpret_cast<const char*>(&value), reinterpret_cast<char*>(&dest), sizeof(TYPE));

	return dest;
}


inline quint16 reverseUint16(quint16 val) { return reverseBytes<quint16>(val); }
inline quint32 reverseUint32(quint32 val) { return reverseBytes<quint32>(val); }
inline quint64 reverseUint64(quint64 val) { return reverseBytes<quint64>(val); }

inline qint32 reverseInt32(qint32 val)	  { return reverseBytes<qint32>(val);  }

inline float reverseFloat(float val)	  { return reverseBytes<float>(val);   }

const char* const RADIY_ORG = "Radiy";

class PrintElapsedTime
{
public:
	PrintElapsedTime(const QString& msg) :
		m_msg(msg)
	{
		m_timer.start();
	}

	~PrintElapsedTime()
	{
		qDebug() << C_STR(m_msg) << m_timer.elapsed();
	}

private:
	QString m_msg;
	QElapsedTimer m_timer;
};

inline quint16 __checkAndCastToQuint16(int value)
{
	Q_ASSERT(value >= std::numeric_limits<quint16>::lowest() && value <= std::numeric_limits<quint16>::max());

	return static_cast<quint16>(value);
}

#define CHECK_AND_CAST_TO_QUINT16(value)  __checkAndCastToQuint16(value)

bool partitionOfInteger(int number, const std::vector<int>& availableParts, std::vector<int>* resultPartition);
bool partitionOfInteger(int number, const QVector<int>& availableParts, QVector<int>* partition);
