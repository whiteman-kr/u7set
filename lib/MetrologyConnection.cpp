#include "../lib/MetrologyConnection.h"

namespace Metrology
{
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	ConnectionSignal::ConnectionSignal()
	{
		clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionSignal::clear()
	{
		m_appSignalID.clear();
		m_exist = false;

		m_pMetrologySignal = nullptr;	// only for software Metrology
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionSignal::set(::AppSignal* pSignal)
	{
		if (pSignal == nullptr)
		{
			return;
		}

		m_appSignalID = pSignal->appSignalID();			// update appSignalID from real signal
		m_exist = true;									// signal has been found in SignalSetProvider

		m_pMetrologySignal = nullptr;					// only for software Metrology
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionSignal::set(Metrology::Signal* pSignal)
	{
		if (pSignal == nullptr)
		{
			return;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			return;
		}


		m_appSignalID = param.appSignalID();			// update appSignalID from real signal
		m_exist = true;									// signal has been found in SignalBase

		m_pMetrologySignal = pSignal;					// only for software Metrology
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	Connection::Connection()
	{
		clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Connection::isValid() const
	{
		if (ERR_METROLOGY_CONNECTION_TYPE(m_type) == true)
		{
			return false;
		}

		for(int ioType = 0; ioType < ConnectionIoTypeCount; ioType++)
		{
			if (connectionSignal(ioType).isExist() == false)		// signal has not been found in SignalSetProvider
			{
				return false;
			}
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Connection::clear()
	{
		m_restoreID = -1;

		m_type = ConnectionType::NoConnectionType;

		for(int ioType = 0; ioType < ConnectionIoTypeCount; ioType++)
		{
			m_connectionSignal[ioType].clear();
		}

		m_action = VcsItemAction::VcsItemActionType::Unknown;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Connection::strID() const
	{
		QString strID;

		strID =	QString("%1_%2_%3").
				arg(m_type, 3, 10, QChar('0')).
				arg(m_connectionSignal[ConnectionIoType::Source].appSignalID()).
				arg(m_connectionSignal[ConnectionIoType::Destination].appSignalID());

		return strID;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Connection::signalIsOk(const ::AppSignal& signal)
	{
		if (signal.isAnalog() == false)
		{
			return false;
		}

		// Engineering range
		//
		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return false;
		}

		if (signal.lowEngineeringUnits() == 0.0 && signal.highEngineeringUnits() == 0.0)
		{
			return false;
		}

		// Electric range
		//
		if (signal.isInput() == true || signal.isOutput() == true)
		{
			if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
			{
				return false;
			}

			if (signal.electricLowLimit() == 0.0 && signal.electricHighLimit() == 0.0)
			{
				return false;
			}

			if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
			{
				return false;
			}

			if (signal.electricUnit() == E::ElectricUnit::NoUnit)
			{
				return false;
			}
		}

		// Unique fields
		//
		switch (signal.inOutType())
		{
			case E::SignalInOutType::Input:

				if (signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
				{
					return true; // some modules do not have a sensor type
				}

				if (signal.sensorType() == E::SensorType::NoSensor)
				{
					return false;
				}

				break;

			case E::SignalInOutType::Output:

				if (signal.isSpecPropExists(SignalProperties::outputModeCaption) == false)
				{
					return false;
				}

				break;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	ConnectionSignal Connection::connectionSignal(int ioType) const
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			return ConnectionSignal();
		}

		return m_connectionSignal[ioType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Connection::typeStr() const
	{
		if (ERR_METROLOGY_CONNECTION_TYPE(m_type) == true)
		{
			return QT_TRANSLATE_NOOP("MetrologyConnection", "Unknown");
		}

		return ConnectionTypeCaption(m_type);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Connection::appSignalID(int ioType) const
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			return QString();
		}

		return m_connectionSignal[ioType].appSignalID();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Connection::setAppSignalID(int ioType, const QString& appSignalID)
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			return;
		}

		m_connectionSignal[ioType].setAppSignalID(appSignalID);
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Connection::isExist(int ioType) const
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			return false;
		}

		return m_connectionSignal[ioType].isExist();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Connection::setSignal(int ioType, ::AppSignal* pSignal)
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			return;
		}

		m_connectionSignal[ioType].clear();

		if (pSignal == nullptr)
		{
			return;
		}

		if (signalIsOk(*pSignal) == false)
		{
			return;
		}

		m_connectionSignal[ioType].set(pSignal);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Connection::setSignal(int ioType, Metrology::Signal* pSignal)
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			return;
		}

		m_connectionSignal[ioType].clear();

		if (pSignal == nullptr)
		{
			return;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			return;
		}

		if (param.isAnalog() == false)
		{
			return;
		}

		m_connectionSignal[ioType].set(pSignal);
	}

	// -------------------------------------------------------------------------------------------------------------------

	Metrology::Signal* Connection::metrologySignal(int ioType) const
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			return nullptr;
		}

		return m_connectionSignal[ioType].metrologySignal();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Connection::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		int type = ConnectionType::NoConnectionType;
		QString sourceAppSignalID;
		QString destinationAppSignalID;

		result &= xml.readIntAttribute("Type", &type);
		result &= xml.readStringAttribute(QString("SourceAppSignalID"), &sourceAppSignalID);
		result &= xml.readStringAttribute(QString("DestinationAppSignalID"), &destinationAppSignalID);

		setType(type);
		setAppSignalID(ConnectionIoType::Source, sourceAppSignalID);
		setAppSignalID(ConnectionIoType::Destination, destinationAppSignalID);

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Connection::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement("Connection");
		{
			xml.writeIntAttribute("Type", type());
			xml.writeStringAttribute(QString("SourceAppSignalID"), appSignalID(ConnectionIoType::Source));
			xml.writeStringAttribute(QString("DestinationAppSignalID"), appSignalID(ConnectionIoType::Destination));
		}
		xml.writeEndElement();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Connection::operator == (const Connection& connection) const
	{
		if (m_type != connection.m_type)
		{
			return false;
		}

		for(int ioType = 0; ioType < ConnectionIoTypeCount; ioType++)
		{
			if (m_connectionSignal[ioType].appSignalID() != connection.m_connectionSignal[ioType].appSignalID())
			{
				return false;
			}
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	ConnectionBase::ConnectionBase(QObject* parent) :
		QObject(parent)
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::clear()
	{
		QMutexLocker l(&m_connectionMutex);

		m_connectionList.clear();

		m_enableEditBase = true;
		m_userIsAdmin = false;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::count() const
	{
		QMutexLocker l(&m_connectionMutex);

		return m_connectionList.count();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::setSignalSetProvider(SignalSetProvider* signalSetProvider)
	{
		if (signalSetProvider == nullptr)
		{
			Q_ASSERT(signalSetProvider);
			return;
		}

		m_signalSetProvider = signalSetProvider;
	}

	// -------------------------------------------------------------------------------------------------------------------

	std::shared_ptr<DbFile> ConnectionBase::getConnectionFile(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return nullptr;
		}

		bool result = false;

		std::shared_ptr<DbFile> file;
		std::vector<DbFileInfo> fileList;
		int etcFileId = db->systemFileId(DbDir::EtcDir);

		result = db->getFileList(&fileList, etcFileId, CONNECTIONS_FILE_NAME, true, nullptr);
		if (result == false || fileList.size() != 1)
		{
			// if it does not exists, then create a file
			//
			std::shared_ptr<DbFile> newFile = std::make_shared<DbFile>();
			newFile->setFileName(CONNECTIONS_FILE_NAME);

			result = db->addFile(newFile, etcFileId, nullptr);
			if (result == false)
			{
				return nullptr;
			}

			result = db->getFileList(&fileList, etcFileId, CONNECTIONS_FILE_NAME, true, nullptr);
			if (result == false || fileList.size() != 1)
			{
				return nullptr;
			}
		}

		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false || file == nullptr)
		{
			return nullptr;
		}

		file->setState(fileList[0].state());

		return file;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::load(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		//
		//
		if (file->state() == VcsState::CheckedOut)
		{
			int userId = file->userId();

			// currentUser
			//
			if (userId != db->currentUser().userId())
			{
				m_enableEditBase = false;
			}

			m_userIsAdmin = db->currentUser().isAdminstrator();

			// user of file
			//
			m_userName = db->username(userId);

		}

		// read CSV-data from file
		//
		QByteArray data;
		file->swapData(data);

		// load connections from CSV-data
		//
		m_connectionMutex.lock();
			m_connectionList = connectionsFromCsvData(data);
		m_connectionMutex.unlock();

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::save(bool checkIn, const QString& comment)
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return false;
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// only Admin can do changes if base is Checked Out
		//
		if (m_enableEditBase == false)
		{
			if (m_userIsAdmin == false)
			{
				return false;
			}
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		// file must be check out, after save
		//
		if (file->state() != VcsState::CheckedOut)
		{
			return true;
		}

		// delete all connections that marked as VcsItemAction::VcsItemActionType::Deleted
		// update all restoreID
		//
		if (checkIn == true)
		{
			removeAllMarked();
			updateRestoreIDs();
		}

		// create CSV-data and write to file
		//
		QByteArray data = csvDataFromConnections(true);
		file->swapData(data);

		// save file to database
		//
		if (db->setWorkcopy(file, nullptr) == false)
		{
			return false;
		}

		// check in file
		//
		if (checkIn == true)
		{
			if (db->checkIn(*file, comment, nullptr) == false)
			{
				return false;
			}
			else
			{
				m_enableEditBase = true;
			}
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::checkOut()
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return false;
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		// check out file
		//
		if (file->state() == VcsState::CheckedOut)
		{
			return true;
		}

		bool result = db->checkOut(*file, nullptr);
		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::isCheckIn()
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return false;
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		// test checked in
		//
		bool result = file->state() == VcsState::CheckedIn;
		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::findSignal_in_signalSet()
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return;
		}

		QMutexLocker l(&m_connectionMutex);

		int connectionCount = m_connectionList.count();
		for(int i = 0; i < connectionCount; i++)
		{
			Connection& connection = m_connectionList[i];

			// init signals
			//
			for(int ioType = 0; ioType < ConnectionIoTypeCount; ioType++)
			{
				if (connection.appSignalID(ioType).isEmpty() == true)
				{
					continue;
				}

				::AppSignal* pSignal = m_signalSetProvider->getSignalByStrID(connection.appSignalID(ioType));
				if (pSignal == nullptr)
				{
					continue;
				}

				m_signalSetProvider->loadSignal(pSignal->ID());

				connection.setSignal(ioType, pSignal);
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::append(const Connection& connection)
	{
		QMutexLocker l(&m_connectionMutex);

		m_connectionList.append(connection);

		return m_connectionList.count() - 1;
	}

	// -------------------------------------------------------------------------------------------------------------------

	Connection ConnectionBase::connection(int index) const
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return Connection();
		}

		return m_connectionList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	Connection* ConnectionBase::connectionPtr(int index)
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return nullptr;
		}

		return &m_connectionList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::setConnection(int index, const Connection& connection)
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return;
		}

		m_connectionList[index] = connection;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::remove(int index)
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return;
		}

		m_connectionList.remove(index);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::removeAllMarked()
	{
		QMutexLocker l(&m_connectionMutex);

		int connectionCount = m_connectionList.count();
		for(int i = connectionCount - 1; i >= 0; i--)
		{
			if (m_connectionList[i].action() == VcsItemAction::VcsItemActionType::Deleted)
			{
				m_connectionList.remove(i);
				continue;
			}

			m_connectionList[i].setAction(VcsItemAction::VcsItemActionType::Unknown);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::setAction(int index, const VcsItemAction::VcsItemActionType& type)
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return;
		}

		m_connectionList[index].setAction(type);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::updateRestoreIDs()
	{
		QMutexLocker l(&m_connectionMutex);

		int connectionCount = m_connectionList.count();
		for(int index = 0; index < connectionCount; index++)
		{
			m_connectionList[index].setRestoreID(index);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	Connection ConnectionBase::connectionFromChekedIn(int restoreID)
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return Connection();
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return Connection();
		}

		// file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return Connection();
		}

		// get last changeset of file
		//
		std::vector<DbChangeset> changesetList;
		db->getFileHistory(*file, &changesetList, nullptr);

		if (changesetList.size() == 0)
		{
			return Connection();
		}

		// read last Checked In file
		//
		std::shared_ptr<DbFile> fileOut;

		bool result = db->getSpecificCopy(*file, changesetList[0].changeset(), &fileOut, nullptr);
		if (result == false)
		{
			return Connection();
		}

		// load connections from CSV-data
		//
		QByteArray data;
		fileOut->swapData(data);

		QVector<Connection> connectionList = connectionsFromCsvData(data);

		// find connection with restoreID
		//
		Connection connectionForRestore;

		for(const Connection& connection: connectionList)
		{
			if (connection.restoreID() == restoreID)
			{
				connectionForRestore = connection;
				break;
			}
		}

		return connectionForRestore;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::restoreConnection(int restoreID)
	{
		// find connection with restoreID
		//
		Connection connectionForRestore = connectionFromChekedIn(restoreID);

		// if connection is empty
		//
		if (connectionForRestore == Connection())
		{
			return -1;
		}

		// find and update connection
		//
		int connectionIndex = -1;

		QMutexLocker l(&m_connectionMutex);

		int connectionCount = m_connectionList.count();
		for(int index = 0; index < connectionCount; index++)
		{
			if (m_connectionList[index].restoreID() == restoreID)
			{
				m_connectionList[index] = connectionForRestore;

				connectionIndex = index;

				break;
			}
		}

		return connectionIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::sort()
	{
		QMutexLocker l(&m_connectionMutex);

		int connectionCount = m_connectionList.count();
		for( int i = 0; i < connectionCount - 1; i++ )
		{
			for( int k = i+1; k < connectionCount; k++ )
			{
				if (m_connectionList[i].strID() > m_connectionList[k].strID())
				{
					Connection connection = m_connectionList[i];
					m_connectionList[i] = m_connectionList[k];
					m_connectionList[k] = connection;
				}
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::findConnectionIndex(const Connection& connection) const
	{
		QMutexLocker l(&m_connectionMutex);

		int foundIndex = -1;

		int connectionCount = m_connectionList.count();
		for( int index = 0; index < connectionCount; index++ )
		{
			if (m_connectionList[index] == connection)
			{
				foundIndex = index;
				break;
			}
		}

		return foundIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::findConnectionIndex(int ioType, Metrology::Signal* pSignal) const
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			Q_ASSERT(0);
			return -1;
		}

		if (pSignal == nullptr)
		{
			Q_ASSERT(0);
			return -1;
		}

		int foundIndex = -1;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_connectionList[i].metrologySignal(ioType) != pSignal)
			{
				continue;
			}

			foundIndex = i;

			break;
		}

		return foundIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::findConnectionIndex(int ioType, ConnectionType connectionType, Metrology::Signal* pSignal) const
	{
		if (ioType < 0 || ioType >= ConnectionIoTypeCount)
		{
			Q_ASSERT(0);
			return -1;
		}

		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
		{
			Q_ASSERT(0);
			return -1;
		}

		if (pSignal == nullptr)
		{
			Q_ASSERT(0);
			return -1;
		}

		int foundIndex = -1;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();
		for(int i = 0; i < count; i ++)
		{
			const Connection& connection = m_connectionList[i];

			if (connection.type() != connectionType)
			{
				continue;
			}

			if (connection.metrologySignal(ioType) != pSignal)
			{
				continue;
			}

			foundIndex = i;

			break;
		}

		return foundIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------

	std::vector<Metrology::Signal*> ConnectionBase::destinationSignals(const QString& sourceAppSignalID, ConnectionType connectionType) const
	{
		if (sourceAppSignalID.isEmpty() == true)
		{
			return std::vector<Metrology::Signal*>();
		}

		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
		{
			return std::vector<Metrology::Signal*>();
		}

		std::vector<Metrology::Signal*> destSignalList;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();
		for(int i = 0; i < count; i ++)
		{
			const Connection& connection = m_connectionList[i];

			if (connection.type() != connectionType)
			{
				continue;
			}

			if (connection.appSignalID(ConnectionIoType::Source) != sourceAppSignalID)
			{
				continue;
			}

			Metrology::Signal* pDestSignal = m_connectionList[i].metrologySignal(ConnectionIoType::Destination);
			if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
			{
				continue;
			}

			destSignalList.push_back(pDestSignal);
		}

		return destSignalList;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QByteArray ConnectionBase::csvDataFromConnections(bool full)
	{
		QString dataStr;

		for(Metrology::Connection connection : m_connectionList)
		{
			for(int ioType = 0; ioType < ConnectionIoTypeCount; ioType++)
			{
				if (connection.appSignalID(ioType).isEmpty() == true)
				{
					continue;
				}
			}

			dataStr.append(QString::number(connection.type()));
			dataStr.append(";");
			dataStr.append(connection.appSignalID(Metrology::ConnectionIoType::Source));
			dataStr.append(";");
			dataStr.append(connection.appSignalID(Metrology::ConnectionIoType::Destination));
			dataStr.append(";");

			if (full == true)
			{
				dataStr.append(QString::number(connection.action().toInt()));
				dataStr.append(";");
				dataStr.append(QString::number(connection.restoreID()));
				dataStr.append(";");
			}

			dataStr.append("\n");
		}

		return dataStr.toUtf8();
	}

	// -------------------------------------------------------------------------------------------------------------------

	QVector<Connection> ConnectionBase::connectionsFromCsvData(const QByteArray& data) const
	{
		QVector<Connection> connectionList;

		// load record from CSV-data
		//
		QTextStream in(data);
		while (in.atEnd() == false)
		{
			Connection connection;

			QStringList line = in.readLine().split(";");
			for(int column = 0; column < line.count(); column++)
			{
				switch (column)
				{
					case 0:	connection.setType(line[column].toInt());													break;
					case 1:	connection.setAppSignalID(ConnectionIoType::Source, line[column]);							break;
					case 2:	connection.setAppSignalID(ConnectionIoType::Destination, line[column]);						break;
					case 3:	connection.setAction(static_cast<VcsItemAction::VcsItemActionType>(line[column].toInt()));	break;
					case 4:	connection.setRestoreID(line[column].toInt());												break;
				}
			}

			for(int ioType = 0; ioType < ConnectionIoTypeCount; ioType++)
			{
				if (connection.appSignalID(ioType).isEmpty() == true)
				{
					continue;
				}
			}

			// append to m_connectionList
			//
			connectionList.append(connection);
		}

		return connectionList;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::exportConnectionsToFile(const QString& fileName)
	{
		if (fileName.isEmpty() == true)
		{
			return false;
		}

		QFile file;
		file.setFileName(fileName);
		if (file.open(QIODevice::WriteOnly) == false)
		{
			return false;
		}

		QByteArray data = csvDataFromConnections(false);
		if (data.isEmpty() == true)
		{
			return false;
		}

		qint64 writtenBytes = file.write(data);
		if (writtenBytes != data.count())
		{
			return false;
		}

		file.close();

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	ConnectionBase& ConnectionBase::operator=(const ConnectionBase& from)
	{
		QMutexLocker l(&m_connectionMutex);

		m_connectionList = from.m_connectionList;

		return *this;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	QString ConnectionTypeCaption(ConnectionType type)
	{
		QString caption;

		switch (type)
		{
			case Unused:				caption = QT_TRANSLATE_NOOP("MetrologyConnection", "No connections             ");	break;
			case Input_Internal:		caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Input -> Internal");			break;
			case Input_Output:			caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Input -> Output");				break;
			case Input_DP_Internal_F:	caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Input dP -> Internal F");		break;
			case Input_DP_Output_F:		caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Input dP -> Output F");			break;
			case Input_C_Internal_F:	caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Input °С -> Internal °F");		break;
			case Input_C_Output_F:		caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Input °С -> Output °F");		break;
			case Tuning_Output:			caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Tuning -> Output");				break;
			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MetrologyConnection", "Unknown");
		}

		return caption;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
}
