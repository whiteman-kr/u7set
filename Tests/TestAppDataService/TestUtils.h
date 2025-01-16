#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <qtestcase.h>

#define FAIL_STR(errorString) \
QFAIL(qPrintable(errorString))

#define VERIFY_STR(statement, errorString) \
QVERIFY2(statement, qPrintable(errorString));

#define VERIFY_RESULT(errorString) \
if (result == false) \
	QFAIL(errorString)

#define VERIFY_RESULT_STR(errorString) \
if (result == false) \
	FAIL_STR(errorString)

#define VERIFY_STATEMENT(statement, errorString) \
result = false;\
QVERIFY2(statement, errorString);\
result = true;

#define VERIFY_STATEMENT_STR(statement, errorString) \
result = false;\
VERIFY_STR(statement, errorString);\
result = true;

#endif // TESTUTILS_H
