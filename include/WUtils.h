#pragma once


#define ASSERT_RESULT_FALSE_BREAK	assert(false); \
									result = false; \
									break;

#define RESULT_FALSE_BREAK			result = false; \
									break;

#define ASSERT_RETURN_FALSE			assert(false); \
									return false;

#define TEST_PTR_RETURN_FALSE(ptr)	if (ptr == nullptr) \
									{	\
										assert(false);	\
										return false; \
									}


void swapBytes(const char* src, char* dest, int size);


template <typename TYPE>
TYPE reverseBytes(TYPE value)
{
	TYPE dest;

	swapBytes(reinterpret_cast<const char*>(&value), reinterpret_cast<char*>(&dest), sizeof(TYPE));

	return dest;
}
