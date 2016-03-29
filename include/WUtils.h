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

