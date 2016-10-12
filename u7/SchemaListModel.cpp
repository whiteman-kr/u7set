#include "Stable.h"
#include "SchemaListModel.h"
#include "CheckInDialog.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"


SchemaListModel::SchemaListModel(QObject* parent) :
	QAbstractTableModel(parent)
{
}

SchemaListModel::~SchemaListModel()
{
}

int SchemaListModel::rowCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
	return static_cast<int>(m_files.size());
}

int SchemaListModel::columnCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
	return ColumnCount;
}

QVariant SchemaListModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
	unsigned int row = index.row();
	unsigned int col = index.column();

	switch (role)
	{
	case Qt::DisplayRole:
		{
			if (row >= m_files.size())
			{
				assert(false);
				return QVariant();
			}

			const std::shared_ptr<DbFileInfo>& fileInfo = m_files[row];
			int fileId = fileInfo->fileId();

			switch (col)
			{
			case FileNameColumn:
				return QVariant(fileInfo->fileName());

			case FileCaptionColumn:
				return QVariant(fileCaption(fileId));

			case FileStateColumn:
				if (fileInfo->state() == VcsState::CheckedIn)
				{
					return QVariant();
				}
				else
				{
					return QVariant(fileInfo->state().text());
				}

			case FileUserColumn:
				return QVariant(usernameById(fileInfo->userId()));

			case FileActionColumn:
				return QVariant(fileInfo->action().text());

//			case FileLastCheckInColumn:
//				return QVariant(fileInfo->lastCheckIn().toString());

//			case FileIdColumn:
//				return QVariant(fileInfo->fileId());

			case FileIssuesColumn:
				{
					QStringList fn = fileInfo->fileName().split('.');

					if (fn.isEmpty() == false)
					{
						auto gm = GlobalMessanger::instance();
						auto issueCount = gm->issueForSchema(fn.front());

						if (issueCount.errors == 0 && issueCount.warnings == 0)
						{
							return QString();
						}

						if (issueCount.errors > 0 && issueCount.warnings == 0)
						{
							return QString("ERR: %1").arg(issueCount.errors);
						}

						if (issueCount.errors > 0 && issueCount.warnings > 0)
						{
							return QString("ERR: %1, WRN: %2").arg(issueCount.errors).arg(issueCount.warnings);
						}

						if (issueCount.errors == 0 && issueCount.warnings > 0)
						{
							return QString("WRN: %2").arg(issueCount.warnings);
						}

						assert(false);
						return QVariant();
					}
					else
					{
						assert(fn.isEmpty() == false);		// Empty file name?
						return QVariant();
					}

					assert(false);
				}
				return QVariant();

			case FileDetailsColumn:
				return QVariant(detailsColumnText(fileInfo->fileId()));

			default:
				return QVariant();
			}
		}
		break;

	case Qt::BackgroundRole:
		{
			if (row >= m_files.size())
			{
				assert(false);
				return QVariant();
			}

			const std::shared_ptr<DbFileInfo>& fileInfo = m_files[row];

			if (fileInfo->state() == VcsState::CheckedOut)
			{
				QBrush b(QColor(0xFF, 0xFF, 0xFF));

				switch (static_cast<VcsItemAction::VcsItemActionType>(fileInfo->action().toInt()))
				{
				case VcsItemAction::Added:
					b.setColor(QColor(0xF9, 0xFF, 0xF9));
					break;
				case VcsItemAction::Modified:
					b.setColor(QColor(0xF4, 0xFA, 0xFF));
					break;
				case VcsItemAction::Deleted:
					b.setColor(QColor(0xFF, 0xF4, 0xF4));
					break;
				}

				return b;
			}
		}
		break;

	case Qt::TextColorRole:
		{
			if (row >= m_files.size())
			{
				assert(false);
				return QVariant();
			}

			const std::shared_ptr<DbFileInfo>& fileInfo = m_files[row];

			if (col == FileIssuesColumn)
			{
				QStringList fn = fileInfo->fileName().split('.');

				if (fn.isEmpty() == false)
				{
					auto gm = GlobalMessanger::instance();
					auto issueCount = gm->issueForSchema(fn.front());

					if (issueCount.errors > 0)
					{
						return QBrush(QColor(0xE0, 0x33, 0x33, 0xFF));
					}

					if (issueCount.warnings > 0)
					{
						return QBrush(QColor(0xE8, 0x72, 0x17, 0xFF));
					}

					return QVariant();
				}
				else
				{
					assert(fn.isEmpty() == false);		// Empty file name?
					return QVariant();
				}
			}
			else
			{
				return QVariant();
			}
		}
		break;
	}

	return QVariant();
}

QVariant SchemaListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case FileNameColumn:
				return QObject::tr("File Name");

			case FileCaptionColumn:
				return QObject::tr("Caption");

			case FileStateColumn:
				return QObject::tr("State");

			case FileUserColumn:
				return QObject::tr("User");

			case FileActionColumn:
				return QObject::tr("Action");

//			case FileLastCheckInColumn:
//				return QObject::tr("Last Check In");

//			case FileIdColumn:
//				return QObject::tr("FileID");

			case FileIssuesColumn:
				return QObject::tr("Issues");

			case FileDetailsColumn:
				return QObject::tr("Details");

			default:
				assert(false);
			}
		}
	}
	return QVariant();
}

void SchemaListModel::sort(int column, Qt::SortOrder order/* = Qt::AscendingOrder*/)
{
	if (m_files.empty() == true)
	{
		return;
	}

	emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

	QModelIndexList pers = persistentIndexList();
	std::vector<std::shared_ptr<DbFileInfo>> oldFileOrder(m_files);

	switch (column)
	{
	case FileNameColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString n1 = f1->fileName();
			QString n2 = f2->fileName();

			if (order == Qt::AscendingOrder)
			{
				return n2 < n1;
			}
			else
			{
				return n1 < n2;
			}
		});
		break;

	case FileCaptionColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order, this](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString c1 = this->fileCaption(f1->fileId());
			QString c2 = this->fileCaption(f2->fileId());

			if (order == Qt::AscendingOrder)
			{
				return c2 < c1;
			}
			else
			{
				return c1 < c2;
			}
		});
		break;

	case FileStateColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString s1 = f1->state().text();
			QString s2 = f2->state().text();

			if (order == Qt::AscendingOrder)
			{
				return s2 < s1;
			}
			else
			{
				return s1 < s2;
			}
		});
		break;

	case FileUserColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order, this](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			int uid1 = f1->userId();
			int uid2 = f2->userId();

			QString u1 = this->usernameById(uid1);
			QString u2 = this->usernameById(uid2);

			if (order == Qt::AscendingOrder)
			{
				return u2 < u1;
			}
			else
			{
				return u1 < u2;
			}
		});
		break;

	case FileActionColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString a1 = f1->action().text();
			QString a2 = f2->action().text();

			if (order == Qt::AscendingOrder)
			{
				return a2 < a1;
			}
			else
			{
				return a1 < a2;
			}
		});
		break;

//	case FileLastCheckInColumn:
//		std::sort(m_files.begin(), m_files.end(),
//				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
//		{
//			QDateTime c1 = f1->created();
//			QDateTime c2 = f2->created();

//			if (order == Qt::AscendingOrder)
//			{
//				return c2 < c1;
//			}
//			else
//			{
//				return c1 < c2;
//			}
//		});
//		break;

	case  FileDetailsColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order, this](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString s1 = this->detailsColumnText(f1->fileId());
			QString s2 = this->detailsColumnText(f2->fileId());

			if (order == Qt::AscendingOrder)
			{
				return s2 < s1;
			}
			else
			{
				return s1 < s2;
			}
		});
		break;

	default:
		assert(false);
	}

	// Move pers indexes
	//
	for (QModelIndex& oldIndex : pers)
	{
		int oldIndexRow = oldIndex.row();
		if (oldIndexRow < 0 || oldIndex.row() >= static_cast<int>(oldFileOrder.size()))
		{
			assert(oldIndex.row() < static_cast<int>(oldFileOrder.size()));
			continue;
		}

		std::shared_ptr<DbFileInfo> oldFile = oldFileOrder.at(oldIndexRow);
		QModelIndex newIndex = index(getFileRow(oldFile->fileId()), oldIndex.column());

		if (oldIndex != newIndex)
		{
			changePersistentIndex(oldIndex, newIndex);
		}
	}

	emit layoutChanged();
}

void SchemaListModel::addFile(std::shared_ptr<DbFileInfo> file)
{
	if (file->fileName().endsWith(m_filter, Qt::CaseInsensitive) == true)
	{
		emit layoutAboutToBeChanged();
		m_files.push_back(file);
		emit layoutChanged();
	}
}

void SchemaListModel::setFiles(const std::vector<DbFileInfo> &files, const std::vector<DbUser>& users)
{
	emit layoutAboutToBeChanged();

	QModelIndexList pers = persistentIndexList();
	std::vector<std::shared_ptr<DbFileInfo>> oldFileOrder(m_files);

	// --
	//
	m_files.clear();
	m_details.clear();

	for (auto f = files.begin(); f != files.end(); ++f)
	{
		std::shared_ptr<DbFileInfo> spf = std::make_shared<DbFileInfo>(*f);

		if (spf->fileName().endsWith(m_filter, Qt::CaseInsensitive) == true)
		{
			m_files.push_back(spf);

			VFrame30::SchemaDetails details;
			bool parsed = details.parseDetails(spf->details());

			if (parsed == true)
			{
				m_details[spf->fileId()] = details;
			}
		}
	}

	// Move pers indexes
	//
	for (QModelIndex& oldIndex : pers)
	{
		int oldIndexRow = oldIndex.row();
		if (oldIndexRow < 0 || oldIndex.row() >= static_cast<int>(oldFileOrder.size()))
		{
			assert(oldIndex.row() < static_cast<int>(oldFileOrder.size()));
			continue;
		}

		std::shared_ptr<DbFileInfo> oldFile = oldFileOrder.at(oldIndexRow);
		QModelIndex newIndex = index(getFileRow(oldFile->fileId()), oldIndex.column());

		if (oldIndex != newIndex)
		{
			changePersistentIndex(oldIndex, newIndex);
		}
	}

	emit layoutChanged();

	// Set users
	//
	m_users.clear();
	for (const DbUser& u : users)
	{
		m_users[u.userId()] = u.username();
	}

	return;
}

void SchemaListModel::clear()
{
	beginResetModel();
	m_files.clear();
	endResetModel();

	m_users.clear();
	return;
}

std::shared_ptr<DbFileInfo> SchemaListModel::fileByRow(int row)
{
	if (row < 0 || row >= (int)m_files.size())
	{
		assert(row >= 0);
		assert(row < (int)m_files.size());
		return std::shared_ptr<DbFileInfo>();
	}

	return m_files[row];
}

std::shared_ptr<DbFileInfo> SchemaListModel::fileByFileId(int fileId)
{
	if (fileId == -1)
	{
		return std::shared_ptr<DbFileInfo>();
	}

    auto it = std::find_if(m_files.begin(), m_files.end(),
		[fileId](const std::shared_ptr<DbFileInfo>& f)
		{
			return f->fileId() == fileId;
		});

	if (it == m_files.end())
	{
		return std::shared_ptr<DbFileInfo>();
	}
	else
	{
		return *it;
	}
}

int SchemaListModel::getFileRow(int fileId) const
{
	int row = 0;
	for (auto it = m_files.begin(); it != m_files.end(); ++it)
	{
		if (it->get()->fileId() == fileId)
		{
			return row;
		}

		row ++;
	}
	return -1;
}

QString SchemaListModel::filter() const
{
	return m_filter;
}

void SchemaListModel::setFilter(const QString& value)
{
	m_filter = value;
}

const std::vector<std::shared_ptr<DbFileInfo>>& SchemaListModel::files() const
{
	return m_files;
}

QString SchemaListModel::usernameById(int userId) const
{
	auto it = m_users.find(userId);

	if (it == m_users.end())
	{
		return QString("Undefined");
	}
	else
	{
		return it->second;
	}
}

QString SchemaListModel::detailsColumnText(int fileId) const
{
	auto it = m_details.find(fileId);

	if (it == m_details.end())
	{
		return QString();
	}

	const VFrame30::SchemaDetails& d = it->second;

	QString result;

	if (d.m_equipmentId.isEmpty() == false)
	{
		result = d.m_equipmentId;
	}

	return result;
}

QString SchemaListModel::fileCaption(int fileId) const
{
	auto it = m_details.find(fileId);

	if (it == m_details.end())
	{
		return QString();
	}

	const VFrame30::SchemaDetails& d = it->second;

	QString result = d.m_caption;

	return result;
}

