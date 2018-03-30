#include "SourceBase.h"

#include <assert.h>
#include <QMessageBox>
#include <QFile>

#include "../../lib/XmlHelper.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceItem::SourceItem() :
	m_pWorker(nullptr)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

SourceItem::SourceItem(const SourceItem& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

SourceItem::SourceItem(const SourceInfo& si)
{
	m_pWorker = nullptr;
	m_si = si;
}

// -------------------------------------------------------------------------------------------------------------------

SourceItem::~SourceItem()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SourceItem::clear()
{
	stop();
	m_si.clear();
}

// -------------------------------------------------------------------------------------------------------------------

bool SourceItem::run()
{
	if (m_pWorker != nullptr)
	{
		return false;
	}

	SourceWorker* pWorker = new SourceWorker(this);
	if (pWorker == nullptr)
	{
		return false;
	}

	QThread* pThread = new QThread;
	if (pThread == nullptr)
	{
		delete pWorker;
		return false;
	}

	pWorker->moveToThread(pThread);

	connect(pThread, SIGNAL(started()), pWorker, SLOT(process()));	// on start thread run method process()
	connect(pWorker, SIGNAL(finished()), pThread, SLOT(quit()));	// on finish() run slot quit()

	connect(pWorker, SIGNAL(finished()), pWorker, SLOT(deleteLater()));
	connect(pThread, SIGNAL(finished()), pThread, SLOT(deleteLater()));

	pThread->start();												// run thread that runs process()

	m_pWorker = pWorker;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SourceItem::stop()
{
	if (m_pWorker == nullptr)
	{
		return false;
	}

	m_pWorker->finish();
	m_pWorker = nullptr;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

SourceItem& SourceItem::operator=(const SourceItem& from)
{
	m_sourceMutex.lock();

		m_pWorker = from.m_pWorker;
		m_si = from.m_si;

	m_sourceMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceBase theSourceBase;

// -------------------------------------------------------------------------------------------------------------------

SourceBase::SourceBase(QObject *parent) :
	QObject(parent),
	m_sourcesIsRunning(false)
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

int SourceBase::readFromXml()
{
	clear();

	QString path = theOptions.source().path();
	if (path.isEmpty() == true)
	{
		QMessageBox::information(nullptr, "Loading sources", tr("Please, input path to sources file!"));
		return 0;
	}

	QFile fileXml(path);
	if (fileXml.exists() == false)
	{
		QMessageBox::information(nullptr, "Loading sources", tr("File of sources is not found!"));
		return 0;
	}

	if (fileXml.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::information(nullptr, "Loading sources", tr("File of sources is not opened!"));
		return 0;
	}

	QByteArray&& fileData = fileXml.readAll();

	fileXml.close();



	XmlReadHelper xml(fileData);

	while(xml.findElement("DataSource") == true)
	{
		SourceInfo si;

		xml.readStringAttribute("LmDataType", &si.m_dataType);
		xml.readStringAttribute("LmEquipmentID", &si.m_equipmentID);
		xml.readIntAttribute("LmModuleType", &si.m_moduleType);
		xml.readStringAttribute("LmSubsystem", &si.m_subSystem);
		xml.readStringAttribute("LmCaption", &si.m_caption);
		xml.readStringAttribute("LmDataIP", &si.m_ip);
		xml.readIntAttribute("LmDataPort", &si.m_port);

		si.m_frameCount = 1;

		append(si);
	}

	return count();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::append(const SourceItem& source)
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

SourceItem SourceBase::source(int index) const
{
	SourceItem source;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			source = m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return source;
}

// -------------------------------------------------------------------------------------------------------------------

SourceItem* SourceBase::sourcePtr(int index)
{
	SourceItem* pSource = nullptr;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			pSource = &m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return pSource;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::setSource(int index, const SourceItem &source)
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

		m_sourcesIsRunning = true;

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

		m_sourcesIsRunning = false;

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

	SourceItem* pSource = sourceAt(row);
	if (pSource == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter;
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

QString SourceTable::text(int row, int column, SourceItem* pSource) const
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
		case SOURCE_LIST_COLUMN_DATA_TYPE:		result = pSource->dataType();												break;
		case SOURCE_LIST_COLUMN_EQUIPMENT_ID:	result = pSource->equipmentID();											break;
		case SOURCE_LIST_COLUMN_MODULE_TYPE:	result = QString::number(pSource->moduleType());							break;
		case SOURCE_LIST_COLUMN_SUB_SYSTEM:		result = pSource->subSystem();												break;
		case SOURCE_LIST_COLUMN_CAPTION:		result = pSource->caption();												break;
		case SOURCE_LIST_COLUMN_IP:				result = pSource->ip() + " (" + QString::number(pSource->port()) + ")";		break;
		case SOURCE_LIST_COLUMN_FRAME_COUNT:	result = QString::number(pSource->frameCount());							break;
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

SourceItem* SourceTable::sourceAt(int index) const
{
	SourceItem* pSource = nullptr;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			 pSource = m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return pSource;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::set(const QVector<SourceItem*> list_add)
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
