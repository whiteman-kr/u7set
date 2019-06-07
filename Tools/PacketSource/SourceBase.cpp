#include "SourceBase.h"

#include <assert.h>
#include <QMessageBox>
#include <QFile>

#include "../../lib/XmlHelper.h"
#include "../../Builder/CfgFiles.h"
#include "../../lib/DataSource.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::SourceInfo::SourceInfo()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

void PS::SourceInfo::clear()
{
	index = -1;

	caption.clear();
	equipmentID.clear();

	moduleType = 0;
	subSystem.clear();
	frameCount = 0;
	dataID = 0;

	lmAddress.clear();
	serverAddress.clear();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::Source::Source() :
	m_pThread(nullptr),
	m_pWorker(nullptr)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source::Source(const Source& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source::Source(const PS::SourceInfo& si)
{
	m_si = si;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source::~Source()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Source::clear()
{
	m_sourceMutex.lock();

		m_si.clear();
		m_signalList.clear();
		m_frameBase.clear();

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::run()
{
	if (m_pWorker != nullptr)
	{
		return false;
	}

	return createWorker();
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::stop()
{
	if (m_pWorker == nullptr)
	{
		return false;
	}

	m_pWorker->finish();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::isRunning()
{
	if (m_pWorker == nullptr)
	{
		return false;
	}

	return m_pWorker->isRunnig();
}

// -------------------------------------------------------------------------------------------------------------------

int PS::Source::sentFrames()
{
	if (m_pWorker == nullptr)
	{
		return 0;
	}

	return m_pWorker->sentFrames();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source& PS::Source::operator=(const PS::Source& from)
{
	m_sourceMutex.lock();

		m_pThread = from.m_pThread;
		m_pWorker = from.m_pWorker;

		m_si = from.m_si;
		m_signalList = from.m_signalList;
		m_frameBase = from.m_frameBase;

	m_sourceMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::createWorker()
{
	if (m_pWorker != nullptr)
	{
		return false;
	}

	m_pWorker = new SourceWorker(this);
	if (m_pWorker == nullptr)
	{
		return false;
	}

	if (m_pThread != nullptr)
	{
		return false;
	}

	m_pThread = new QThread;
	if (m_pThread == nullptr)
	{
		delete m_pWorker;
		return false;
	}

	m_pWorker->moveToThread(m_pThread);

	connect(m_pThread, &QThread::started, m_pWorker, &SourceWorker::process);	// on start thread run method process()
	connect(m_pWorker, &SourceWorker::finished, m_pThread, &QThread::quit);		// on finish() run slot quit()

	//connect(m_pThread, SIGNAL(finished()), m_pThread, SLOT(deleteLater()));
	//connect(pWorker, SIGNAL(finished()), pWorker, SLOT(deleteLater()));
	connect(m_pWorker, &SourceWorker::finished, this, &Source::deleteWorker);

	m_pThread->start();														// run thread that runs process()

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Source::deleteWorker()
{
	if (m_pThread != nullptr)
	{
		m_pThread->wait(10000);
		delete m_pThread;
		m_pThread = nullptr;
	}

	if (m_pWorker != nullptr)
	{
		delete m_pWorker;
		m_pWorker = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceBase::SourceBase(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

SourceBase::~SourceBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::clear()
{
	m_sourceMutex.lock();

		m_sourceList.clear();

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::count() const
{
	int count = 0;

	m_sourceMutex.lock();

		count = m_sourceList.count();

	m_sourceMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::readFromFile(const QString& path, const SignalBase& signalBase)
{
	clear();

	QString msgTitle = tr("Loading sources");

	if (path.isEmpty() == true)
	{
		QMessageBox::information(nullptr, msgTitle, tr("Please, input path to sources directory!"));
		return 0;
	}

	// read Server IP and Server Port
	//

	QString fileCfg = path + "/" + Builder::FILE_CONFIGURATION_XML;

	QFile fileCfgXml(fileCfg);
	if (fileCfgXml.exists() == false)
	{
		QMessageBox::information(nullptr, msgTitle, tr("File %1 is not found!").arg(Builder::FILE_CONFIGURATION_XML));
		return 0;
	}

	if (fileCfgXml.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::information(nullptr, msgTitle, tr("File %1 is not opened!").arg(Builder::FILE_CONFIGURATION_XML));
		return 0;
	}

	QByteArray&& cfgData = fileCfgXml.readAll();

	fileCfgXml.close();

	XmlReadHelper xmlCfg(cfgData);

	HostAddressPort serverAddress;

	if (xmlCfg.readHostAddressPort("AppDataReceivingIP", "AppDataReceivingPort", &serverAddress) == false)
	{
		QMessageBox::information(nullptr, msgTitle, tr("IP-address of AppDataSrv is not found in file %1!").arg(Builder::FILE_CONFIGURATION_XML));
		return 0;
	}

	// read Data Sources
	//

	QString fileSource = path + "/" + Builder::FILE_APP_DATA_SOURCES_XML;

	QFile fileSourceXml(fileSource);
	if (fileSourceXml.exists() == false)
	{
		QMessageBox::information(nullptr, msgTitle, tr("File %1 is not found!").arg(Builder::FILE_APP_DATA_SOURCES_XML));
		return 0;
	}

	if (fileSourceXml.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::information(nullptr, msgTitle, tr("File %1 is not opened!").arg(Builder::FILE_APP_DATA_SOURCES_XML));
		return 0;
	}

	QByteArray&& sourceData = fileSourceXml.readAll();

	fileSourceXml.close();

	int indexSource = 0;
	XmlReadHelper xmlSource(sourceData);

	while(xmlSource.atEnd() == false)
	{
		if (xmlSource.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlSource.name() == DataSource::ELEMENT_DATA_SOURCE)
		{
			// Source Info
			//
			PS::SourceInfo si;

			QString ip;
			int port = 0;
			QString dataID;

			xmlSource.readStringAttribute("LmEquipmentID", &si.equipmentID);
			xmlSource.readIntAttribute("LmModuleType", &si.moduleType);
			xmlSource.readStringAttribute("LmSubsystem", &si.subSystem);
			xmlSource.readStringAttribute("LmCaption", &si.caption);
			xmlSource.readStringAttribute("LmDataIP", &ip);
			xmlSource.readIntAttribute("LmDataPort", &port);
			xmlSource.readIntAttribute("LmRupFramesQuantity", &si.frameCount);
			xmlSource.readStringAttribute("LmDataID", &dataID);

			si.index = indexSource++;

			si.dataID = dataID.toUInt(nullptr, 16);

			si.lmAddress.setAddressPort(ip, port);
			si.serverAddress.setAddressPort(serverAddress.addressStr(), serverAddress.port());

			//
			//
			PS::Source source;
			source.info() = si;

			//
			//
			source.frameBase().setFrameCount(si.frameCount);

			// Source Signals
			//
			if (xmlSource.readNextStartElement() == false)
			{
				continue;
			}

			if (xmlSource.name() == DataSource::ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS)
			{
				int signalCount = 0;
				xmlSource.readIntAttribute(DataSource::PROP_COUNT , &signalCount);

				QString strAssociatedSignalIDs;
				xmlSource.readStringElement(DataSource::ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS, &strAssociatedSignalIDs);
				QStringList associatedSignalList = strAssociatedSignalIDs.split(",", QString::SkipEmptyParts);

				if (associatedSignalList.count() != signalCount)
				{
					assert(0);
					continue;
				}

				//

				for (int i = 0; i < signalCount; i++)
				{
					PS::Signal signal;

					int signalIndex = signalBase.findIndex(associatedSignalList[i]);
					if (signalIndex == -1)
					{
						signal.setAppSignalID(associatedSignalList[i]);
					}
					else
					{
						signal = signalBase.signal(signalIndex);
						signal.calcOffset();

						int frameIndex = signal.frameIndex();
						if (frameIndex >= 0 && frameIndex < source.info().frameCount)
						{
							PS::FrameData* pFrameData = source.frameBase().frameDataPtr(frameIndex);
							if (pFrameData != nullptr)
							{
								if (signal.frameOffset() >= 0 && signal.frameOffset() < Rup::FRAME_DATA_SIZE)
								{
									signal.setValueData(&pFrameData->data()[signal.frameOffset()]);
								}
							}
						}
					}

					source.signalList().append(signal);
				}
			}

			append(source);
		}
	}

	return count();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::append(const PS::Source& source)
{
	int index = -1;

	m_sourceMutex.lock();

		m_sourceList.append(source);
		index = m_sourceList.count() - 1;

	m_sourceMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::remove(int index)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList.remove(index);
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source SourceBase::source(int index) const
{
	PS::Source source;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			source = m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return source;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source* SourceBase::sourcePtr(int index)
{
	PS::Source* pSource = nullptr;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			pSource = &m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return pSource;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::setSource(int index, const PS::Source &source)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList[index] = source;
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

SourceBase& SourceBase::operator=(const SourceBase& from)
{
	m_sourceMutex.lock();

		m_sourceList = from.m_sourceList;

	m_sourceMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runSourece(int index)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList[index].run();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::stopSourece(int index)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList[index].stop();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runAllSoureces()
{
	m_sourceMutex.lock();

		int count = m_sourceList.count();
		for(int i = 0; i < count; i++)
		{
			m_sourceList[i].run();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::stopAllSoureces()
{
	m_sourceMutex.lock();

		int count = m_sourceList.count();
		for(int i = 0; i < count; i++)
		{
			m_sourceList[i].stop();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceTable::SourceTable(QObject *)
{
}

// -------------------------------------------------------------------------------------------------------------------

SourceTable::~SourceTable()
{
	m_sourceMutex.lock();

		m_sourceList.clear();

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceTable::columnCount(const QModelIndex&) const
{
	return SOURCE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SourceTable::rowCount(const QModelIndex&) const
{
	return sourceCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SourceTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SOURCE_LIST_COLUMN_COUNT)
		{
			result = SourceListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SourceTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= sourceCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > SOURCE_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	PS::Source* pSource = sourceAt(row);
	if (pSource == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter;
	}

	if (role == Qt::TextColorRole)
	{
		if (column == SOURCE_LIST_COLUMN_STATE)
		{
			if (pSource->isRunning() == false)
			{
				return QColor(0xFF, 0x00, 0x00);
			}
		}

		return QVariant();
	}

//	if (role == Qt::BackgroundColorRole)
//	{
//		if (column == SOURCE_LIST_COLUMN_ID)
//		{
//			if (option->isConnected() == false)
//			{
//				return QColor(0xFF, 0xA0, 0xA0);
//			}
//		}

//		if (column == SOURCE_LIST_COLUMN_RECEIVED)
//        {
//			if (option->isNoReply() == true)
//            {
//                return QColor(0xFF, 0xA0, 0xA0);
//            }
//        }

//		if (column == SOURCE_LIST_COLUMN_SKIPPED)
//		{
//			if ( (double) option->skippedBytes() * 100.0 / (double) option->receivedBytes() > MAX_SKIPPED_BYTES)
//			{
//				return QColor(0xFF, 0xA0, 0xA0);
//			}
//		}

//		return QVariant();
//	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSource);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SourceTable::text(int row, int column, PS::Source* pSource) const
{
	if (row < 0 || row >= sourceCount())
	{
		return QString();
	}

	if (column < 0 || column > SOURCE_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pSource == nullptr)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case SOURCE_LIST_COLUMN_CAPTION:		result = pSource->info().caption;												break;
		case SOURCE_LIST_COLUMN_EQUIPMENT_ID:	result = pSource->info().equipmentID;											break;
		case SOURCE_LIST_COLUMN_MODULE_TYPE:	result = QString::number(pSource->info().moduleType);							break;
		case SOURCE_LIST_COLUMN_SUB_SYSTEM:		result = pSource->info().subSystem;												break;
		case SOURCE_LIST_COLUMN_FRAME_COUNT:	result = QString::number(pSource->info().frameCount);							break;
		case SOURCE_LIST_COLUMN_LM_IP:			result = pSource->info().lmAddress.addressStr() + " (" + QString::number(pSource->info().lmAddress.port()) + ")";			break;
		case SOURCE_LIST_COLUMN_SERVER_IP:		result = pSource->info().serverAddress.addressStr() + " (" + QString::number(pSource->info().serverAddress.port()) + ")";	break;
		case SOURCE_LIST_COLUMN_STATE:			result = pSource->isRunning() ? QString::number(pSource->sentFrames()) : tr("Stopped");										break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::updateColumn(int column)
{
	if (column < 0 || column >= SOURCE_LIST_COLUMN_COUNT)
	{
		return;
	}

	int count = rowCount();

	for (int row = 0; row < count; row ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

int SourceTable::sourceCount() const
{
	int count = 0;

	m_sourceMutex.lock();

		count = m_sourceList.count();

	m_sourceMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source* SourceTable::sourceAt(int index) const
{
	PS::Source* pSource = nullptr;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			 pSource = m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return pSource;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::set(const QVector<PS::Source*> list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceList = list_add;

		m_sourceMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::clear()
{
	int count = m_sourceList.count();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceList.clear();

		m_sourceMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
