#pragma once
#include "../OnlineLib/MatsUsers.h"

class DbController;

namespace Builder
{
	//
	// DbMatsUserStorage
	//
	class DbMatsUserStorage : public OnlineLib::MatsUserStorage
	{
	public:
		DbMatsUserStorage() = default;

		bool load(DbController* db, QString& errorCode);
		bool save(DbController* db, const QString& comment) const;
	};

}
