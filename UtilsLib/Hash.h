#pragma once
#include <assert.h>
#include <type_traits>
#include <QString>
#include <QUuid>
#include <QtGlobal>

using Hash = quint64;

#define UNDEFINED_HASH 0x0000000000000000ULL

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

constexpr Hash calcHash(const QChar* str)
{
	Hash hash = 0;
	const QChar* prt = str;

	while (prt->unicode())
	{
		hash += (hash << 5) + prt->unicode();
		prt++;
	}

	return hash;
}


inline Hash calcHash(const QByteArray& data)
{
	Hash hash = 0;
	const char* prt = data.constData();
	int dataSize = data.size();

	for (int i = 0; i < dataSize; ++i)
	{
		hash += (hash << 5) + *prt;
		prt++;
	}

	return hash;
}

inline Hash calcHash(const void* data, size_t byteSize)
{
	Hash hash = 0;
	const char* prt = (const char*)data;

	for (size_t i = 0; i < byteSize; ++i)
	{
		hash += (hash << 5) + *prt;
		prt++;
	}

	return hash;
}

// Custom specialization of std::hash for QUuid
//
namespace std
{
	template<>
	struct hash<QUuid>
	{
		std::size_t operator()(const QUuid& u) const noexcept
		{
			return ::calcHash(u.toByteArray());
		}
	};
}

#include <QCryptographicHash>


class Md5Hash : public QCryptographicHash
{
public:
	Md5Hash() : QCryptographicHash(QCryptographicHash::Md5) {}

	QString resultStr() const { return QString(result().toHex()); }

	static QByteArray hash(const QByteArray& data) { return QCryptographicHash::hash(data, QCryptographicHash::Md5); }
	static QString hashStr(const QByteArray& data) { return QString(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex()); }
};

