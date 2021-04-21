#pragma once

#include "../lib/SignalSetProvider.h"
#include "../lib/MetrologyConnection.h"

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
		void setSignalSetProvider(SignalSetProvider* signalSetProvider);

		//
		//
		std::shared_ptr<DbFile> getConnectionFile(DbController* db);

		bool load(DbController* db);
		bool save(bool checkIn, const QString &comment);

		bool checkOut();
		bool isCheckIn();

		void findSignal_in_signalSet();

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

		SignalSetProvider* m_signalSetProvider = nullptr;

		QString m_userName;
		bool m_enableEditBase = true;
		bool m_userIsAdmin = false;
	};

	// ==============================================================================================
}
