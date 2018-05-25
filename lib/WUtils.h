#pragma once

#include <QtGlobal>

#define ASSERT_RESULT_FALSE_BREAK	assert(false); \
									result = false; \
									break;

#define RESULT_FALSE_BREAK			result = false; \
									break;

#define ASSERT_RETURN_FALSE			assert(false); \
									return false;

#define ASSERT_FALSE_CONTINUE		assert(false); \
									continue;

#define TEST_PTR_RETURN_FALSE(ptr)	if (ptr == nullptr) \
									{	\
										assert(false);	\
										return false; \
									}

#define TEST_PTR_LOG_RETURN_FALSE(ptr, log)	if (ptr == nullptr) \
											{	\
												assert(false);	\
												LOG_NULLPTR_ERROR(log); \
												return false; \
											}

#define TEST_PTR_CONTINUE(ptr)		if (ptr == nullptr) \
									{	\
										assert(false);	\
										continue; \
									}

#define TEST_PTR_RETURN(ptr)		if (ptr == nullptr) \
									{	\
										assert(false);	\
										return; \
									}

#define DEBUG_STOP					{ int a = 0; a++; }


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
