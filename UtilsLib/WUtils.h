#pragma once

#include <QtGlobal>
#include <QElapsedTimer>
#include <QDebug>
#include <cmath>
#include <set>
#include <map>
#include <QTimeZone>

#define ASSERT_RESULT_FALSE_BREAK	Q_ASSERT(false); \
									result = false; \
									break;

#define RESULT_FALSE_BREAK			result = false; \
									break;

#define ASSERT_RETURN_FALSE			Q_ASSERT(false); \
									return false;

#define ASSERT_FALSE_CONTINUE		Q_ASSERT(false); \
									continue;

#define TEST_PTR_CONTINUE(ptr)		if (ptr == nullptr) \
									{	\
										Q_ASSERT(false);	\
										continue; \
									}

#define TEST_PTR_BREAK(ptr)		if (ptr == nullptr) \
									{	\
										Q_ASSERT(false);	\
										break; \
									}


#define TEST_PTR_RETURN(ptr)		if (ptr == nullptr) \
									{	\
										Q_ASSERT(false);	\
										return; \
									}

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

#define TEST_PTR_RETURN_VALUE(ptr, value)	if (ptr == nullptr) \
											{	\
												Q_ASSERT(false);	\
												return value; \
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

#define DELETE_IF_NOT_NULL(ptr)		if (ptr != nullptr) \
									{	\
										delete ptr; \
										ptr = nullptr; \
									}

#define DELETE_ARRAY_IF_NOT_NULL(ptr)		if (ptr != nullptr) \
											{	\
												delete [] ptr; \
												ptr = nullptr; \
											}

#define DEBUG_STOP					{ int a = 0; a++; }

#define RETURN_IF_FALSE(result)		if ((result) == false) \
									{ \
										return false; \
									}

#define ASSERT_RETURN_IF_FALSE(result)		if ((result) == false) \
											{ \
												Q_ASSERT(false); \
												return false; \
											}

#define RETURN_VALUE_IF_FALSE(result, value)		if ((result) == false) \
													{ \
														return value; \
													}

#define CONTINUE_IF_FALSE(result)	if ((result) == false) \
									{ \
										continue; \
									}

#define BREAK_IF_FALSE(result)		if ((result) == false) \
									{ \
										break; \
									}

#define BREAK_IF_TRUE(result)		if ((result) == true) \
									{ \
										break; \
									}


#define AUTO_LOCK(mutex) QMutexLocker _locker_##mutex(&mutex);

#define C_STR(qstring) qstring.toStdString().c_str()

#define CONTAINS_NULLPTR(vector_of_set)	(std::find(vector_of_set.begin(), vector_of_set.end(), nullptr) != vector_of_set.end())


inline void swapBytes(const char* src, char* dest, int size)
{
	Q_ASSERT(src != dest);

	dest += (size - 1);

	for(int i = 0; i < size; i++)
	{
		*dest-- = *src++;
	}
}

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

inline qint16 reverseInt16(qint16 val)	  { return reverseBytes<qint16>(val);  }
inline qint32 reverseInt32(qint32 val)	  { return reverseBytes<qint32>(val);  }
inline qint64 reverseInt64(qint64 val)	  { return reverseBytes<qint64>(val);  }

inline float reverseFloat(float val)	  { return reverseBytes<float>(val);   }
inline double reverseDouble(double val)	  { return reverseBytes<double>(val);  }

// Format time to string:  2022.12.31 06:24:59.239
//
inline QString formatTime_YYYY_MM_DD(int year, int month, int day, int hour, int minute, int second, int millisecond)
{
	QChar zero = QLatin1Char('0');

	return QString("%1.%2.%3 %4:%5:%6.%7").
			arg(year).arg(month, 2, 10, zero).arg(day, 2, 10, zero).
			arg(hour, 2, 10, zero).arg(minute, 2, 10, zero).arg(second, 2, 10, zero).arg(millisecond, 3, 10, zero);
}

inline QString formatTime_YYYY_MM_DD(qint64 timeMs)
{
	QDateTime dt = QDateTime::fromMSecsSinceEpoch(timeMs, QTimeZone::UTC);

	QDate d = dt.date();
	QTime t = dt.time();

	return formatTime_YYYY_MM_DD(d.year(), d.month(), d.day(),
								 t.hour(), t.minute(), t.second(), t.msec());
}

// Format time to string:  31.12.2025 06:24:59.239
//
inline QString formatTime_DD_MM_YYYY(int year, int month, int day, int hour, int minute, int second, int millisecond)
{
	QChar zero = QLatin1Char('0');

	return QString("%1.%2.%3 %4:%5:%6.%7").
		arg(day, 2, 10, zero).arg(month, 2, 10, zero).arg(year).
		arg(hour, 2, 10, zero).arg(minute, 2, 10, zero).arg(second, 2, 10, zero).arg(millisecond, 3, 10, zero);
}

inline QString formatTime_DD_MM_YYYY(qint64 timeMs)
{
	QDateTime dt = QDateTime::fromMSecsSinceEpoch(timeMs, QTimeZone::UTC);

	QDate d = dt.date();
	QTime t = dt.time();

	return formatTime_DD_MM_YYYY(d.year(), d.month(), d.day(),
								 t.hour(), t.minute(), t.second(), t.msec());
}

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

template <class T>
	std::enable_if_t<std::is_same<T, float>::value, bool>	// check that T is type of float
isFloatEquals(T v1, T v2)
{
	return std::nextafter(v1, std::numeric_limits<float>::lowest()) <= v2 &&
							std::nextafter(v1, std::numeric_limits<float>::max()) >= v2;
}

template <class T>
	std::enable_if_t<std::is_same<T, double>::value, bool>	// check that T is type of double
isDoubleEquals(T v1, T v2)
{
	return std::nextafter(v1, std::numeric_limits<double>::lowest()) <= v2 &&
							std::nextafter(v1, std::numeric_limits<double_t>::max()) >= v2;
}

using OptionalBool = std::optional<bool>;
using OptionalQString = std::optional<QString>;

inline QString formatUptime(qint64 uptime)
{
	int s = uptime % 60; uptime /= 60;
	int m = uptime % 60; uptime /= 60;
	int h = uptime % 24; uptime /= 24;

	QString uptimeStr;

	if (uptime != 0 /* days != 0 */)
	{
		uptimeStr = QString("%1d ").arg(uptime);
	}

	uptimeStr += QString("%1:%2:%3").
						arg(h, 2, 10, QChar('0')).
						arg(m, 2, 10, QChar('0')).
						arg(s, 2, 10, QChar('0'));

	return uptimeStr;
}

inline bool stringToBool(const QString& str, bool* ok)
{
	QString boolStr = str.trimmed().toLower();

	static const std::set<QString> trueStr =
	{
		QString("1"),
		QString("true"),
		QString("yes"),
		QString("on"),
	};

	static const std::set<QString> falseStr =
	{
		QString("0"),
		QString("false"),
		QString("no"),
		QString("off"),
	};

	bool _ok = true;
	bool result = false;

	if (trueStr.contains(boolStr) == true)
	{
		result = true;
	}
	else
	{
		if (falseStr.contains(boolStr) == true)
		{
			result = false;
		}
		else
		{
			_ok = false;
		}
	}

	if (ok != nullptr)
	{
		*ok = _ok;
	}

	return result;
}

inline QString boolToString(bool value)
{
	return QString(value ? "true" : "false");
}

inline bool checkInt32Range(qint64 value)
{
	return	value <= std::numeric_limits<qint32>::max() &&
			value >= std::numeric_limits<qint32>::lowest();
}

inline bool checkFloat32Range(double value)
{
	return	value <= std::numeric_limits<float>::max() &&
			value >= std::numeric_limits<float>::lowest();
}

template <typename KEY, typename VALUE>
VALUE getValueOrDefault(const std::map<KEY, VALUE>& map, const KEY& key, const VALUE& def)
{
	auto it = map.find(key);

	if (it == map.end())
	{
		return def;
	}

	return it->second;
}

template <typename KEY, typename VALUE>
VALUE getValueOrNullptr(const std::map<KEY, VALUE>& map, const KEY& key)
{
	return getValueOrDefault<KEY, VALUE>(map, key, nullptr);
}

template <typename KEY, typename VALUE>
std::map<KEY, VALUE>::iterator findOrInsertKey(std::map<KEY, VALUE>& map, const KEY& key)
{
	auto it = map.find(key);

	if (it == map.end())
	{
		auto [newIt, b] = map.emplace(key, VALUE{});

		it = newIt;
	}

	return it;
}

inline QString fineSize(qint64 size)
{
	if (size > 1024 * 1024 * 1024)
	{
		return QString("%1 GBytes").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
	}
	else
	{
		if (size > 1024 * 1024)
		{
			return QString("%1 MBytes").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
		}
		else
		{
			if (size > 1024)
			{
				return QString("%1 KBytes").arg(size / 1024.0, 0, 'f', 1);
			}
		}
	}

	return QString("%1 Bytes").arg(size);
}

#define ROUND_TO(value, roundTo)	(((value + roundTo - 1) / roundTo) * roundTo)

const std::size_t CACHE_LINE_SIZE = 64;				// 64 bytes on x86-64

//#define ROUND_TO_CACHE_LINE_SIZE(size)	(((size + CACHE_LINE_SIZE - 1) / CACHE_LINE_SIZE) * CACHE_LINE_SIZE)

#define ROUND_TO_CACHE_LINE_SIZE(size)	ROUND_TO(size, CACHE_LINE_SIZE)

#define CACHE_ALIGNED					alignas(CACHE_LINE_SIZE)



