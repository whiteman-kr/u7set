#include "MetrologyConnection.h"

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

		m_action = E::VcsItemAction::Unknown;
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
		if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
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
			if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
			{
				return false;
			}

			if (signal.electricLowLimit() == 0.0 && signal.electricHighLimit() == 0.0)
			{
				return false;
			}

			if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
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

				if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
				{
					return true; // some modules do not have a sensor type
				}

				if (signal.sensorType() == E::SensorType::NoSensor)
				{
					return false;
				}

				break;

			case E::SignalInOutType::Output:

				if (signal.isSpecPropExists(AppSignalPropNames::OUTPUT_MODE) == false)
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
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ConnectionBase::count() const
	{
		QMutexLocker l(&m_connectionMutex);

		return m_connectionList.count();
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
				dataStr.append(QString::number(static_cast<int>(connection.action())));
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
					case 0:	connection.setType(line[column].toInt());										break;
					case 1:	connection.setAppSignalID(ConnectionIoType::Source, line[column]);				break;
					case 2:	connection.setAppSignalID(ConnectionIoType::Destination, line[column]);			break;
					case 3:	connection.setAction(static_cast<E::VcsItemAction>(line[column].toInt()));		break;
					case 4:	connection.setRestoreID(line[column].toInt());									break;
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
