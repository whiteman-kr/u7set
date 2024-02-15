#pragma once
#include <cassert>
#include <array>
#include <QString>
#include <QUuid>
#include <QRectF>
#include <QSize>

using Hash = quint64;

#define UNDEFINED_HASH 0x0000000000000000ULL		// Do not change to other value.


inline Hash calcHash(const QString& str, Hash init = 0)
{
	Hash hash = init;
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
	qsizetype dataSize = data.size();

	for (qsizetype i = 0; i < dataSize; ++i)
	{
		hash += (hash << 5) + *prt;
		prt++;
	}

	return hash;
}

inline Hash calcHash(const void* data, size_t byteSize, Hash init = 0)
{
	Hash hash = init;
	const char* prt = (const char*)data;

	while (byteSize--)
	{
		hash += (hash << 5) + *prt;
		prt++;
	}

	return hash;
}

inline Hash calcHash(const QRectF& rect, Hash init = 0)
{
	std::array<double, 4> data{rect.left(), rect.top(), rect.width(), rect.height()};
	return calcHash(data.data(), sizeof(data), init);
}

inline Hash calcHash(QSize size, Hash init = 0)
{
	std::array<int, 2> data{size.width(), size.height()};
	return calcHash(data.data(), sizeof(data), init);
}

inline quint16 calcHash16(const void* src, qsizetype l)
{
	if (src == nullptr)
	{
		assert(src);
		return 0;
	}

	quint16 nHash = 0;
	quint8* p = (quint8*)src;

	while (l--)
	{
		nHash += (nHash<<5) + *p++;
	}

	return nHash;
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


inline quint32 ClassNameHashCode(const std::string& className)
{
	assert(className.empty() == false);

	std::string clearClassName = className;

	auto findResult = className.rfind("::");
	if (findResult != className.npos)
	{
		assert(findResult + 2 < className.size());
		assert((findResult + 2) + (className.size() - findResult - 2) == className.size());

		clearClassName = className.substr(findResult + 2, className.size() - findResult - 2);
	}

	// Do not change this hash function
	// as its' results are stored in files
	//
	quint32 nHash = 0;
	const char* ptr = clearClassName.c_str();

	while (*ptr)
	{
		nHash += (nHash << 5) + *ptr++;
	}

	return nHash;
}

namespace hash_h
{
	template<typename Type>
	struct hasher
	{
		// Calc hasher for unordered_map
		// No need to calc hash on hash for std::unourdered_map, in this way it is significantly faster
		// Usage: std::unordered_map<Hash, AppSignalState, HashHasher> m_states;
		//
		std::size_t operator()(Type u) const noexcept
		{
			return u;
		}
	};
}

template<typename T>
using VoidHasher = hash_h::hasher<T>;

