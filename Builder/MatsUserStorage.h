#pragma once
#include "../DbLib/DbMatsUser.h"

class DbController;

namespace Builder
{
	//
	// MatsUserStorage
	//
	class MatsUserStorage
	{
	public:
		MatsUserStorage();

		void add(const DbMatsUser& user);
		int count() const;
		const DbMatsUser& get(int index) const;
		void clear();

		const std::vector<DbMatsUser>& users() const;
		QString tuningTags(const QString& login, bool* found = nullptr) const;

		bool load(DbController* db, QString &errorCode);
		bool save(DbController* db, const QString &comment) const;
	
	private:
		std::vector<DbMatsUser> m_users;
		const QString fileName = "MatsUsers.xml";
	};

}
