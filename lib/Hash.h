#pragma once
#include <assert.h>
#include <type_traits>
#include <QString>
#include <QtGlobal>

typedef quint64 Hash;

inline Hash calcHash(const QString& str)
{
	Hash hash = 0;

	const QChar* prt = str.constData();

	while (prt->unicode())
	{
		hash += (hash << 5) + prt->unicode();
		prt++;
	}

	return hash;
}

//inline HASH CalcHash(void* ptr, int nSize)
//{
//	if (ptr == NULL)
//	{
//		ASSERT(ptr);
//		return UNDEFINED_HASH;
//	}

//	HASH nHash = 0;

//	register BYTE* p = (BYTE*)ptr;

//	while (nSize--)
//		nHash += (nHash<<5) + *p++;

//	return nHash;
//}

