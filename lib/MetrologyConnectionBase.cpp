#include "../lib/MetrologyConnectionBase.h"

namespace Metrology
{
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	SignalConnection::SignalConnection()
	{
		clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalConnection::isValid() const
	{
		if (m_handle.type < 0 || m_handle.type >= CONNECTION_TYPE_COUNT)
		{
			return false;
		}

		if (m_handle.inputID == SIGNAL_ID_IS_NOT_FOUND)
		{
			return false;
		}

		if (m_handle.outputID == SIGNAL_ID_IS_NOT_FOUND)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalConnection::clear()
	{
		m_handle.state = EMPTY_CONNECTION_HANDLE;

		for(int t = 0; t < IO_SIGNAL_CONNECTION_TYPE_COUNT; t++)
		{
			m_pSignal[t] = nullptr;
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalConnection::typeStr() const
	{
		if (m_handle.type < 0 || m_handle.type >= CONNECTION_TYPE_COUNT)
		{
			return QString("???");
		}

		return qApp->translate("MetrologyConnectionBase.h", ConnectionType[m_handle.type]);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalConnection::appSignalID(int ioType) const
	{
		if (ioType < 0 || ioType >= IO_SIGNAL_CONNECTION_TYPE_COUNT)
		{
			return QString();
		}

		return m_appSignalID[ioType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalConnection::setAppSignalID(int ioType, const QString& appSignalID)
	{
		if (ioType < 0 || ioType >= IO_SIGNAL_CONNECTION_TYPE_COUNT)
		{
			return;
		}

		m_appSignalID[ioType] = appSignalID;
	}

	// -------------------------------------------------------------------------------------------------------------------

	Metrology::Signal* SignalConnection::signal(int ioType) const
	{
		if (ioType < 0 || ioType >= IO_SIGNAL_CONNECTION_TYPE_COUNT)
		{
			return nullptr;
		}

		return m_pSignal[ioType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalConnection::setSignal(int ioType, ::Signal* pSignal)
	{
		if (ioType < 0 || ioType >= IO_SIGNAL_CONNECTION_TYPE_COUNT)
		{
			return;
		}

		if (pSignal == nullptr)
		{
			switch (ioType)
			{
				case IO_SIGNAL_CONNECTION_TYPE_INPUT:	m_handle.inputID = SIGNAL_ID_IS_NOT_FOUND;	break;
				case IO_SIGNAL_CONNECTION_TYPE_OUTPUT:	m_handle.outputID = SIGNAL_ID_IS_NOT_FOUND;	break;
				default:								assert(0);									break;
			}

			return;
		}

		if (pSignal->isAnalog() == false)
		{
			return;
		}

		m_appSignalID[ioType] = pSignal->appSignalID();

		switch (ioType)
		{
			case IO_SIGNAL_CONNECTION_TYPE_INPUT:	m_handle.inputID = static_cast<quint64>(pSignal->ID());		break;
			case IO_SIGNAL_CONNECTION_TYPE_OUTPUT:	m_handle.outputID = static_cast<quint64>(pSignal->ID());	break;
			default:								assert(0);													break;
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalConnection::setSignal(int ioType, Metrology::Signal* pSignal)
	{
		if (ioType < 0 || ioType >= IO_SIGNAL_CONNECTION_TYPE_COUNT)
		{
			return;
		}

		if (pSignal == nullptr)
		{
			switch (ioType)
			{
				case IO_SIGNAL_CONNECTION_TYPE_INPUT:	m_handle.inputID = SIGNAL_ID_IS_NOT_FOUND;	break;
				case IO_SIGNAL_CONNECTION_TYPE_OUTPUT:	m_handle.outputID = SIGNAL_ID_IS_NOT_FOUND;	break;
				default:								assert(0);									break;
			}

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

		m_appSignalID[ioType] = param.appSignalID();

		m_pSignal[ioType] = pSignal;


		switch (ioType)
		{
			case IO_SIGNAL_CONNECTION_TYPE_INPUT:	m_handle.inputID = static_cast<quint64>(param.ID());	break;
			case IO_SIGNAL_CONNECTION_TYPE_OUTPUT:	m_handle.outputID = static_cast<quint64>(param.ID());	break;
			default:								assert(0);												break;
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	SignalConnection& SignalConnection::operator=(const SignalConnection& from)
	{
		m_handle = from.m_handle;

		for(int type = 0; type < IO_SIGNAL_CONNECTION_TYPE_COUNT; type++)
		{
			m_appSignalID[type] = from.m_appSignalID[type];
			m_pSignal[type] = from.m_pSignal[type];
		}

		return *this;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalConnection::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		int type = CONNECTION_TYPE_UNDEFINED;
		result &= xml.readIntAttribute("Type", &type);
		setType(type);

		for(int t = 0; t < IO_SIGNAL_CONNECTION_TYPE_COUNT; t++)
		{
			result &= xml.readStringAttribute(QString("AppSignalID_%1").arg(IoConnectionType[t]), &m_appSignalID[t]);
		}

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalConnection::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement("Connection");
		{
			xml.writeIntAttribute("Type", type());

			for(int t = 0; t < IO_SIGNAL_CONNECTION_TYPE_COUNT; t++)
			{
				xml.writeStringAttribute(QString("AppSignalID_%1").arg(IoConnectionType[t]), appSignalID(t));
			}
		}
		xml.writeEndElement();
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	ConnectionBase::ConnectionBase(QObject *parent) :
	    QObject(parent)
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	ConnectionBase::ConnectionBase(DbController* db, SignalSet* signalSet, QObject *parent) :
		QObject(parent)
	{
		if (db == nullptr)
		{
			assert(db);
			return;
		}

		if (signalSet == nullptr)
		{
			assert(signalSet);
			return;
		}

		load(db, signalSet);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::clear()
	{
		QMutexLocker l(&m_connectionMutex);

		m_connectionList.clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::count() const
	{
		QMutexLocker l(&m_connectionMutex);

		return m_connectionList.count();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::sort()
	{
		QMutexLocker l(&m_connectionMutex);

		int connectionCount = m_connectionList.count();
		for( int i = 0; i < connectionCount - 1; i++ )
		{
			for( int j = i+1; j < connectionCount; j++ )
			{
				if ( m_connectionList[i].handle().state > m_connectionList[j].handle().state )
				{
					SignalConnection connection = m_connectionList[i];
					m_connectionList[i] = m_connectionList[j];
					m_connectionList[j] = connection;
				}
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::load(DbController* db, SignalSet* signalSet)
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		if (signalSet == nullptr)
		{
			assert(signalSet);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;
		bool result = db->getFileList(&fileList, db->etcFileId(), CONNECTIONS_FILE_NAME, true, nullptr);
		if (result == false || fileList.size() != 1)
		{
			// if it does not exists, then create a file
			//
			std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
			file->setFileName(CONNECTIONS_FILE_NAME);

			result = db->addFile(file, db->etcFileId(), nullptr);
			return result;
		}

		std::shared_ptr<DbFile> file = nullptr;
		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false || file == nullptr)
		{
			return false;
		}

		QByteArray data;
		file->swapData(data);

		m_connectionList.clear();

		// Load from CSV
		//
		QTextStream in(data);
		while (in.atEnd() == false)
		{
			SignalConnection connection;

			QStringList line = in.readLine().split(";");
			for(int column = 0; column < line.count(); column++)
			{
				switch (column)
				{
					case 0:	connection.setType(line[column].toInt());									break;
					case 1:	connection.setAppSignalID(IO_SIGNAL_CONNECTION_TYPE_INPUT, line[column]);	break;
					case 2:	connection.setAppSignalID(IO_SIGNAL_CONNECTION_TYPE_OUTPUT, line[column]);	break;
				}
			}

			// init signals
			//
			for(int type = 0; type < IO_SIGNAL_CONNECTION_TYPE_COUNT; type++)
			{
				if (connection.appSignalID(type).isEmpty() == true)
				{
					continue;
				}

				::Signal* pSignal = signalSet->getSignal(connection.appSignalID(type));
				if (pSignal == nullptr)
				{
					continue;
				}

				connection.setSignal(type, pSignal);
			}

			// append
			//
			m_connectionMutex.lock();

				m_connectionList.append(connection);

			m_connectionMutex.unlock();
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::save(DbController *db, bool checkIn, const QString& comment)
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		// open file
		//
		std::shared_ptr<DbFile> file = nullptr;

		std::vector<DbFileInfo> fileList;

		bool result = db->getFileList(&fileList, db->etcFileId(), CONNECTIONS_FILE_NAME, true, nullptr);

		if (result == false || fileList.size() != 1)
		{
			// if it does not exists, then create a file
			//
			std::shared_ptr<DbFile> newFile = std::make_shared<DbFile>();
			newFile->setFileName(CONNECTIONS_FILE_NAME);

			if (db->addFile(newFile, db->etcFileId(), nullptr) == false)
			{
				return false;
			}

			result = db->getFileList(&fileList, db->etcFileId(), CONNECTIONS_FILE_NAME, true, nullptr);
			if (result == false || fileList.size() != 1)
			{
				return false;
			}
		}

		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false || file == nullptr)
		{
			return false;
		}

		if (file->state() != VcsState::CheckedOut)
		{
			if (db->checkOut(fileList[0], nullptr) == false)
			{
				return false;
			}
		}

		// save file
		//
		QByteArray data = getCSVdata().toLocal8Bit();
		file->swapData(data);

		if (db->setWorkcopy(file, nullptr) == false)
		{
			return false;
		}

		if (checkIn == true)
		{
			if (db->checkIn(fileList[0], comment, nullptr) == false)
			{
				return false;
			}
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::isCheckIn(DbController *db)
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;
		bool result = db->getFileList(&fileList, db->etcFileId(), CONNECTIONS_FILE_NAME, true, nullptr);
		if (result == false || fileList.size() != 1)
		{
			return false;
		}

		std::shared_ptr<DbFile> file = nullptr;
		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false || file == nullptr)
		{
			return false;
		}

		// test CheckedIn
		//
		return  file->state() == VcsState::CheckedIn;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::сheckOut(DbController *db)
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;
		bool result = db->getFileList(&fileList, db->etcFileId(), CONNECTIONS_FILE_NAME, true, nullptr);
		if (result == false || fileList.size() != 1)
		{
			// if it does not exists, then create a file
			//
			std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
			file->setFileName(CONNECTIONS_FILE_NAME);

			result = db->addFile(file, db->etcFileId(), nullptr);
			if (result == false)
			{
				return false;
			}
		}

		std::shared_ptr<DbFile> file = nullptr;
		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false || file == nullptr)
		{
			return false;
		}

		// Check Out
		//
		if (file->state() == VcsState::CheckedOut)
		{
			return true;
		}

		result = db->checkOut(fileList[0], nullptr);
		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::append(const SignalConnection& connection)
	{
		QMutexLocker l(&m_connectionMutex);

		m_connectionList.append(connection);

		return m_connectionList.count() - 1;
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

	SignalConnection ConnectionBase::connection(int index) const
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return SignalConnection();
		}

		return m_connectionList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	SignalConnection* ConnectionBase::connectionPtr(int index)
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return nullptr;
		}

		return &m_connectionList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ConnectionBase::setConnection(int index, const SignalConnection& connection)
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= m_connectionList.count())
		{
			return;
		}

		m_connectionList[index] = connection;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::findConnectionIndex(int ioType, Metrology::Signal* pSignal) const
	{
		if (ioType < 0 || ioType >= IO_SIGNAL_CONNECTION_TYPE_COUNT)
		{
			assert(0);
			return -1;
		}

		if (pSignal == nullptr)
		{
			assert(0);
			return -1;
		}

		int foundIndex = -1;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_connectionList[i].signal(ioType) != pSignal)
			{
				continue;
			}

			foundIndex = i;

			break;
		}

		return foundIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::findConnectionIndex(int connectionType, int ioType, Metrology::Signal* pSignal) const
	{
		if (connectionType < 0 || connectionType >= CONNECTION_TYPE_COUNT)
		{
			assert(0);
			return -1;
		}

		if (ioType < 0 || ioType >= IO_SIGNAL_CONNECTION_TYPE_COUNT)
		{
			assert(0);
			return -1;
		}

		if (pSignal == nullptr)
		{
			assert(0);
			return -1;
		}

		int foundIndex = -1;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_connectionList[i].type() != connectionType)
			{
				continue;
			}

			if (m_connectionList[i].signal(ioType) != pSignal)
			{
				continue;
			}

			foundIndex = i;

			break;
		}

		return foundIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::findConnectionIndex(const SignalConnection& connection) const
	{
		int foundIndex = -1;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_connectionList[i].handle().state == connection.handle().state)
			{
				foundIndex = i;

				break;
			}
		}

		return foundIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::getOutputSignalCount(int connectionType, const QString& InputAppSignalID) const
	{
		if (connectionType < 0 || connectionType >= CONNECTION_TYPE_COUNT)
		{
			return 0;
		}

		if (InputAppSignalID.isEmpty() == true)
		{
			return 0;
		}

		int outputSignalCount = 0;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			const SignalConnection& connection = m_connectionList[i];

			if (connection.type() != connectionType)
			{
				continue;
			}

			if (connection.appSignalID(IO_SIGNAL_CONNECTION_TYPE_INPUT) != InputAppSignalID)
			{
				continue;
			}

			Metrology::Signal* pOutputSignal = m_connectionList[i].signal(IO_SIGNAL_CONNECTION_TYPE_OUTPUT);
			if (pOutputSignal == nullptr || pOutputSignal->param().isValid() == false)
			{
				continue;
			}

			outputSignalCount ++;
		}

		return outputSignalCount;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QVector<Metrology::Signal*> ConnectionBase::getOutputSignals(int connectionType, const QString& InputAppSignalID) const
	{
		if (connectionType < 0 || connectionType >= CONNECTION_TYPE_COUNT)
		{
			return QVector<Metrology::Signal*>();
		}

		if (InputAppSignalID.isEmpty() == true)
		{
			return QVector<Metrology::Signal*>();
		}

		QVector<Metrology::Signal*> outputSignalsList;

		QMutexLocker l(&m_connectionMutex);

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			const SignalConnection& connection = m_connectionList[i];

			if (connection.type() != connectionType)
			{
				continue;
			}

			if (connection.appSignalID(IO_SIGNAL_CONNECTION_TYPE_INPUT) != InputAppSignalID)
			{
				continue;
			}

			Metrology::Signal* pOutputSignal = m_connectionList[i].signal(IO_SIGNAL_CONNECTION_TYPE_OUTPUT);
			if (pOutputSignal == nullptr || pOutputSignal->param().isValid() == false)
			{
				continue;
			}

			outputSignalsList.append(pOutputSignal);
		}

		return outputSignalsList;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ConnectionBase::getCSVdata()
	{
		QString dataStr;

		for(Metrology::SignalConnection connection : m_connectionList)
		{
			dataStr.append(QString::number(connection.type()));
			dataStr.append(";");
			dataStr.append(connection.appSignalID(Metrology::IO_SIGNAL_CONNECTION_TYPE_INPUT));
			dataStr.append(";");
			dataStr.append(connection.appSignalID(Metrology::IO_SIGNAL_CONNECTION_TYPE_OUTPUT));
			dataStr.append(";");
			dataStr.append("\n");
		}

		return dataStr;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ConnectionBase::exportToFile(const QString& fileName)
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

		qint64 writtenBytes = file.write(getCSVdata().toUtf8());
		if (writtenBytes == 0)
		{
			return false;
		}

		file.close();
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
}
