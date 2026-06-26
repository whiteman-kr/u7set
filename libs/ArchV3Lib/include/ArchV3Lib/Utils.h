#pragma once

#include <QString>
#include <QRegularExpression>

namespace ArchV3
{
	inline QString sanitizeID(QString id)
	{
		id.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
		id.replace(QRegularExpression("_+"), "_");
		id.remove(QRegularExpression("^_+"));
		id.remove(QRegularExpression("_+$"));

		return id;
	}
}