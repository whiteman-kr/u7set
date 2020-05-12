#pragma once

#include <QString>
#include <QHash>

namespace std
{
	//
	// Required for std::unordered_set<QString>
	//

	template<> struct hash<QString>
	{
		std::size_t operator()(const QString& s) const
		{
			return qHash(s);
		}
	};
}
