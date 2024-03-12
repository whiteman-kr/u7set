#pragma once


#include <DbLib/DbController.h>
#include "../Metrology/MetrologyConnection.h"

namespace Metrology
{
	// ==============================================================================================

	class DbConnectionBase : public ConnectionBase
	{

		Q_OBJECT

	public:

		DbConnectionBase(QObject* parent = nullptr);
		virtual ~DbConnectionBase() override {}

	public:

		void clear() override;

		//
		//
		void setDbController(DbController* dbController);

		//
		//
		std::shared_ptr<DbFile> getConnectionFile();

		bool load();
		bool save(bool checkIn, const QString &comment);

		bool checkOut();
		bool isCheckIn();

		// resolution on edit
		//
		QString userName() { return m_userName; }
		bool enableEditBase() { return m_enableEditBase; }
		bool userIsAdmin() { return m_userIsAdmin; }

		// modify
		//
		void setAction(int index, const E::VcsItemAction& type);

		void removeAllMarked();

		//
		//
		void updateRestoreIDs();									// will be update all resotoreID  on check in during save connection
		Connection connectionFromChekedIn(int restoreID);			// get connection from last check in
		int restoreConnection(int restoreID);						// restore connection from last check in, return index of restore connection

	private:

		DbController* m_dbController = nullptr;

		QString m_userName;
		bool m_enableEditBase = true;
		bool m_userIsAdmin = false;
	};

	// ==============================================================================================
}
