#pragma once

#include <QCryptographicHash>


class Md5Hash : public QCryptographicHash
{
public:
	Md5Hash() : QCryptographicHash(QCryptographicHash::Md5) {}

	QString resultStr() const { return QString(result().toHex()); }

	static QByteArray hash(const QByteArray& data) { return QCryptographicHash::hash(data, QCryptographicHash::Md5); }
	static QString hashStr(const QByteArray& data) { return QString(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex()); }
};

