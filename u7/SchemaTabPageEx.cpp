#include "SchemaTabPageEx.h"
#include "../lib/StandardColors.h"
#include "CreateSchemaDialog.h"
#include "CheckInDialog.h"
#include "Settings.h"
#include "TagSelectorWidget.h"
#include "Forms/SelectChangesetDialog.h"
#include "Forms/FileHistoryDialog.h"
#include "Forms/CompareDialog.h"
#include "Forms/ComparePropertyObjectDialog.h"
#include "../lib/PropertyEditor.h"
#include "../VFrame30/LogicSchema.h"
#include "../VFrame30/MonitorSchema.h"
#include "../VFrame30/WiringSchema.h"
#include "../VFrame30/DiagSchema.h"
#include "../VFrame30/UfbSchema.h"
#include "../VFrame30/TuningSchema.h"
#include "../VFrame30/FblItemRect.h"

//
//
// SchemaListModelEx
//
//
SchemaListModelEx::SchemaListModelEx(DbController* dbc, QWidget* parentWidget) :
	QAbstractItemModel(parentWidget),
	HasDbController(dbc),
	m_parentWidget(parentWidget)
{
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaListModelEx::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaListModelEx::projectClosed);
}

QModelIndex SchemaListModelEx::index(int row, int column, const QModelIndex& parent/* = QModelIndex()*/) const
{
	if (hasIndex(row, column, parent) == false)
	{
		return {};
	}

	int parentFileId = -1;

	if (parent.isValid() == false)
	{
		parentFileId = m_files.rootFileId();
		Q_ASSERT(parentFileId != -1);
	}
	else
	{
		parentFileId = static_cast<int>(parent.internalId());
		Q_ASSERT(parentFileId != -1);
	}

	// --
	//
	auto file = m_files.child(parentFileId, row);
	if (file == nullptr)
	{
		Q_ASSERT(file);
		return {};
	}

	return createIndex(row, column, static_cast<quintptr>(file->fileId()));
}

QModelIndex SchemaListModelEx::parent(const QModelIndex& index) const
{
	if (index.isValid() == false)
	{
		return {};
	}

	int fileId = static_cast<int>(index.internalId());
	if (fileId == m_files.rootFileId())
	{
		qDebug() << fileId << ",  " << m_files.rootFileId();
		Q_ASSERT(fileId != m_files.rootFileId());
		return {};
	}

	auto file = m_files.file(fileId);
	if (file == nullptr)
	{
		Q_ASSERT(file);
		return {};
	}

	if (file->fileId() != fileId)
	{
		Q_ASSERT(file->fileId() == fileId);
		return {};
	}

	if (file->parentId() == m_files.rootFileId())
	{
		return {};
	}

	auto parentFile = m_files.file(file->parentId());
	if (parentFile == nullptr)
	{
		Q_ASSERT(parentFile);
		return {};
	}

	Q_ASSERT(parentFile->fileId() == file->parentId());

	// Determine the position of the parent in the parent's parent
	//
	int parentRow = m_files.indexInParent(parentFile->fileId());
	if (parentRow == -1)
	{
		Q_ASSERT(parentRow != -1);
		return {};
	}

	return createIndex(parentRow, 0, static_cast<quintptr>(file->parentId()));
}

int SchemaListModelEx::rowCount(const QModelIndex& parentIndex/* = QModelIndex()*/) const
{
	if (m_files.empty() == true ||
		parentIndex.column() > 0)
	{
		return 0;
	}

	if (parentIndex.isValid() == false)
	{
		return m_files.rootChildrenCount();
	}

	int fileId = static_cast<int>(parentIndex.internalId());
	int rowCount = m_files.childrenCount(fileId);

	return rowCount;
}

int SchemaListModelEx::columnCount(const QModelIndex& /*parent*//* = QModelIndex()*/) const
{
	return static_cast<int>(Columns::ColumnCount);
}

QVariant SchemaListModelEx::data(const QModelIndex& index, int role/* = Qt::DisplayRole*/) const
{
	if (index.isValid() == false)
	{
		return {};
	}

	//int row = index.row();
	Columns column = static_cast<Columns>(index.column());

	int fileId = static_cast<int>(index.internalId());
	std::shared_ptr<DbFileInfo> file = m_files.file(fileId);

	bool systemFile = isSystemFile(fileId);

	if (file == nullptr)
	{
		Q_ASSERT(file);
		return {};
	}

	if (role == Qt::DisplayRole)
	{
		switch (column)
		{
		case Columns::FileNameColumn:
			return file->fileName();

		case Columns::CaptionColumn:
			return fileCaption(fileId);

		case Columns::FileStateColumn:
			if (file->state() == VcsState::CheckedIn)
			{
				return {};
			}
			else
			{
				return file->state().text();
			}

		case Columns::FileActionColumn:
			return file->action().text();

		case Columns::ChangesetColumn:
			return (file->state() == VcsState::CheckedIn) ? QVariant{file->changeset()} : QVariant{};

		case Columns::FileUserColumn:
			return usernameById(file->userId());

		case Columns::IssuesColumn:
			{
				if (systemFile == true)
				{
					return {};
				}

				QString result;

				bool excludedThis = false;
				if (excludedFromBuild(file->fileId()) == true)
				{
					excludedThis = true;
				}

				int excludedCount = m_files.calcIf(file->fileId(),
													[this](const DbFileInfo& f) -> int
													{
														return excludedFromBuild(f.fileId()) ? 1 : 0;
													});

				Builder::BuildIssues::Counter issues;

				m_files.calcIf(file->fileId(),
								[&issues](const DbFileInfo& f) -> int
								{
									QStringList fn = f.fileName().split('.');
									if (fn.empty() == false)
									{
										auto issueCount = GlobalMessanger::instance().issueForSchema(fn.front());
										issues.errors += issueCount.errors;
										issues.warnings += issueCount.warnings;
									}
									return 0;
								});

				if (excludedCount != 0 && excludedThis == true)
				{
					excludedCount --;	// match includes file itself
				}

				if (excludedThis)
				{
					result = tr("Excluded");
				}

				if (excludedCount != 0)
				{
					if (result.isEmpty() == false)
					{
						result += tr(", + %1 schema(s)").arg(excludedCount);
					}
					else
					{
						result = tr("Excluded %1 schema(s)").arg(excludedCount);
					}
				}

				// -- Issuese
				//
				if (issues.errors == 0 && issues.warnings == 0)
				{
				}

				if (issues.errors > 0 && issues.warnings == 0)
				{
					if (result.isEmpty() == false)
					{
						result += ", ";
					}

					result += QString("ERR: %1").arg(issues.errors);
				}

				if (issues.errors > 0 && issues.warnings > 0)
				{
					if (result.isEmpty() == false)
					{
						result += ", ";
					}

					result += QString("ERR: %1, WRN: %2").arg(issues.errors).arg(issues.warnings);
				}

				if (issues.errors == 0 && issues.warnings > 0)
				{
					if (result.isEmpty() == false)
					{
						result += ", ";
					}

					result += QString("WRN: %2").arg(issues.warnings);
				}

				return result;
			}

		case Columns::TagsColumn:
			return tagsColumnText(file->fileId());

		case Columns::DetailsColumn:
			return detailsColumnText(file->fileId());

		default:
			Q_ASSERT(false);
		}

		return QVariant{};
	}

	if (role == Qt::BackgroundRole)
	{
		if (file->state() == VcsState::CheckedOut)
		{
			QBrush b{StandardColors::VcsCheckedIn};

			switch (file->action().value())
			{
			case VcsItemAction::Added:
				b.setColor(StandardColors::VcsAdded);
				break;
			case VcsItemAction::Modified:
				b.setColor(StandardColors::VcsModified);
				break;
			case VcsItemAction::Deleted:
				b.setColor(StandardColors::VcsDeleted);
				break;
			default:
				Q_ASSERT(false);
			}

			return {b};
		}
	}

	if (role == Qt::TextColorRole)
	{
		if (column == Columns::IssuesColumn)
		{
//			if (excludedFromBuild(file->fileId()) == true)
//			{
//				return {};
//			}

			QStringList fn = file->fileName().split('.');

			if (fn.isEmpty() == false)
			{
				auto issueCount = GlobalMessanger::instance().issueForSchema(fn.front());

				if (issueCount.errors > 0)
				{
					return QBrush(QColor(0xE0, 0x33, 0x33, 0xFF));
				}

				if (issueCount.warnings > 0)
				{
					return QBrush(QColor(0xE8, 0x72, 0x17, 0xFF));
				}

				return {};
			}
			else
			{
				Q_ASSERT(fn.isEmpty() == false);		// Empty file name?
				return {};
			}
		}
		else
		{
			return {};
		}
	}

	if (role == Qt::DecorationRole)
	{
		if (index.column() == 0 &&
			file->isFolder() == true)
		{
			static QIcon staticFolderIcon(":/Images/Images/SchemaFolder.svg");
			return staticFolderIcon;
		}
		else
		{
			return {};
		}
	}

	if (role == Qt::UserRole)
	{
		return fileId;
	}

	if (role == SearchSchemaRole)
	{
		if (isSystemFile(file->fileId()))
		{
			return false;
		}

		if (file->fileName().contains(m_searchText, Qt::CaseInsensitive) == true)
		{
			return true;
		}

		// Parse details
		//
		if (auto it = m_details.find(fileId);
			it == m_details.end())
		{
			return false;
		}
		else
		{
			const VFrame30::SchemaDetails& details = it->second;
			return details.searchForString(m_searchText);
		}
	}

	if (role == ExcludedSchemaRole)
	{
		bool excluded = excludedFromBuild(file->fileId());
		return excluded;
	}

	return QVariant{};
}

QVariant SchemaListModelEx::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (static_cast<Columns>(section))
			{
			case Columns::FileNameColumn:	return QStringLiteral("File Name");
			case Columns::CaptionColumn:	return QStringLiteral("Caption");
			case Columns::FileStateColumn:	return QStringLiteral("State");
			case Columns::FileActionColumn:	return QStringLiteral("Action");
			case Columns::ChangesetColumn:	return QStringLiteral("Changeset");
			case Columns::FileUserColumn:	return QStringLiteral("User");
			case Columns::IssuesColumn:		return QStringLiteral("Issues");
			case Columns::TagsColumn:		return QStringLiteral("Tags");
			case Columns::DetailsColumn:	return QStringLiteral("Details");
			default:
				Q_ASSERT(false);
			}
		}

		return {};
	}

	return {};
}

std::pair<QModelIndex, bool> SchemaListModelEx::addFile(QModelIndex parentIndex, std::shared_ptr<DbFileInfo> file)
{
	if (file == nullptr)
	{
		Q_ASSERT(file);
		return {{}, false};
	}

	DbFileInfo parentFile = this->file(parentIndex);

	if (file->parentId() != parentFile.fileId())
	{
		Q_ASSERT(file->parentId() == parentFile.fileId());
		return {{}, false};
	}

	if (m_files.hasFile(file->parentId()) == false)
	{
		Q_ASSERT(m_files.hasFile(file->fileId()));
		return {{}, false};
	}

	// --
	//
	if (m_files.empty() == true)
	{
		Q_ASSERT(m_files.empty() == false);
		return {{}, false};		// At least parent must be present
	}

	// We rely that NEW (just created) fileId is always bigger the previously cretated files.
	// It is required to update indexes, and for beginInsertRows to point chich index has been added.
	//
	Q_ASSERT(file->fileId() > m_files.files().crbegin()->second->fileId());

	// --
	//
	int insertIndex = m_files.childrenCount(parentFile.fileId());

	// --
	//
	beginInsertRows(parentIndex, insertIndex, insertIndex);

	m_files.addFile(file);
	if (m_files.hasFile(file->fileId()) == false)
	{
		Q_ASSERT(m_files.hasFile(file->fileId()));
		return {{}, false};
	}

	if (file->directoryAttribute() == false)
	{
		VFrame30::SchemaDetails details;

		bool ok = details.parseDetails(file->details());
		if (ok == true)
		{
			m_details[file->fileId()] = details;
		}
	}

	endInsertRows();

	updateTagsFromDetails();

	//
	QModelIndex addedModelIndex = index(insertIndex, 0, parentIndex);
	Q_ASSERT(addedModelIndex.isValid() == true);

	return {addedModelIndex, true};
}

bool SchemaListModelEx::deleteFilesUpdate(const QModelIndexList& selectedIndexes,
										  const std::vector<std::shared_ptr<DbFileInfo>>& deletedFiles)
{
	std::vector<DbFileInfo> files;
	files.reserve(deletedFiles.size());

	for (const std::shared_ptr<DbFileInfo>& f : deletedFiles)
	{
		files.push_back(*f);
	}

	return updateFiles(selectedIndexes, files);
}

bool SchemaListModelEx::moveFilesUpdate(const QModelIndexList& selectedIndexes,
										int movedToParnetId,
										const std::vector<DbFileInfo>& movedFiles,
										std::vector<QModelIndex>* addedFilesIndexes)
{
	if (addedFilesIndexes == nullptr)
	{
		Q_ASSERT(addedFilesIndexes);

		refresh();
		return false;
	}

	if (movedFiles.empty() == true)
	{
		Q_ASSERT(movedFiles.empty() == false);

		refresh();
		return false;
	}

	if (movedToParnetId == DbFileInfo::Null)
	{
		Q_ASSERT(movedToParnetId != DbFileInfo::Null);
		refresh();
		return false;
	}

	// Remove moved files
	//
	for (const QModelIndex& index : selectedIndexes)
	{
		int fileId = static_cast<int>(index.internalId());

		QModelIndex pi = index.parent();
		int childIndex = m_files.indexInParent(fileId);

		beginRemoveRows(pi, childIndex, childIndex);
		m_files.removeFile(fileId);
		endRemoveRows();
	}

	// Get parent index where files were moved
	//
	QModelIndexList matched = match(index(0, 0),
									Qt::UserRole,
									QVariant::fromValue(movedToParnetId),
									1,
									Qt::MatchExactly | Qt::MatchRecursive);

	if (matched.size() != 1)
	{
		// Cant find ModelIndex for parent
		//
		Q_ASSERT(matched.size() != 1);

		// Mitigate error
		//
		refresh();
		return false;
	}

	QModelIndex movedToParentIndex = matched.front();
	Q_ASSERT(movedToParentIndex.isValid());

	if (movedToParnetId != file(movedToParentIndex).fileId())
	{
		Q_ASSERT(movedToParnetId == file(movedToParentIndex).fileId());

		refresh();
		return false;
	}

	// Add moved files to destination index
	//
	for (const DbFileInfo& f : movedFiles)
	{
		auto[mi, ok] = addFile(movedToParentIndex, std::make_shared<DbFileInfo>(f));

		if (ok == true)
		{
			addedFilesIndexes->push_back(mi);
		}
		else
		{
			Q_ASSERT(ok);
		}
	}

	return true;
}

bool SchemaListModelEx::updateFiles(const QModelIndexList& selectedIndexes, const std::vector<DbFileInfo>& files)
{
	// Q_ASSERT(deletedFiles.size() == selectedIndexes.size()); -- sizes can be different, from deletedFiles
	// could be removed system files before. Do not uncommnet this assertion
	//
	if (files.empty() == true)
	{
		return false;
	}

	// Some files can be completely removed, other could be just marked as deleted
	//
	std::map<int, DbFileInfo> filesMap;
	for (const DbFileInfo& f : files)
	{
		filesMap[f.fileId()] = f;
	}

	// As some rows can be deleted during update model,
	// rowList must be sorted in FileID descending order,
	// to delete first children and then their parents
	//
	QModelIndexList sortedRowList = selectedIndexes;

	qSort(sortedRowList.begin(), sortedRowList.end(),		// Actually, this sort is not required anymore, as rows to remove are stored in map removeRows, which is sorted itslef
		[](QModelIndex& m1, QModelIndex m2)
		{
			return m1.internalId() >= m2.internalId();
		});

	// Update model
	//
	struct RemoveRows
	{
		QModelIndex parentModelIndex;
		std::map<int, int> childrenRows;	// map where key is child row to delete and value if FileId to delete
	};

	std::map<int, RemoveRows> removeRowsMap;	// key - parent.fileid for deleting row. Used fileid as it must be dleted children first

	for (QModelIndex& index : sortedRowList)
	{
		int fileId = static_cast<int>(index.internalId());
		auto file = filesMap[fileId];

		if (file.isNull() == true)
		{
			// It could be system file, which was removed from input deletedFiles
			// No assertion here, just contuinue
			//
			continue;
		}

		if (file.fileId() != fileId)
		{
			Q_ASSERT(file.fileId() == fileId);
			continue;
		}

		if (file.deleted() == true)
		{
			QModelIndex pi = index.parent();
			int childIndex = m_files.indexInParent(fileId);

//			beginRemoveRows(pi, childIndex, childIndex);
//			m_files.removeFile(file);
//			endRemoveRows();

			removeRowsMap[static_cast<int>(pi.internalId())].parentModelIndex = pi;
			removeRowsMap[static_cast<int>(pi.internalId())].childrenRows.insert({childIndex, file.fileId()});
		}
		else
		{
			std::shared_ptr<DbFileInfo> modelFile = m_files.file(file.fileId());

			if (modelFile == nullptr)
			{
				Q_ASSERT(m_files.hasFile(file.fileId()) == true);
				continue;
			}
			else
			{
				modelFile->operator=(file);
			}

			QModelIndex bottomRight = this->index(index.row(), static_cast<int>(Columns::ColumnCount) - 1, index.parent());
			Q_ASSERT(bottomRight.isValid() == true);

			emit dataChanged(index, bottomRight);
		}
	}

	// Removes rows in reverse sequence (row by row), and from high to low fileid (to remove children first)
	//
	for (auto rit = removeRowsMap.rbegin(); rit != removeRowsMap.rend(); ++rit)
	{
		//int parentFileId = rit->first;
		const RemoveRows& removeRows = rit->second;

		for (auto crit = removeRows.childrenRows.rbegin(); crit != removeRows.childrenRows.rend(); ++crit)
		{
			int childrenRow = crit->first;
			int fileId = crit->second;

			beginRemoveRows(removeRows.parentModelIndex, childrenRow, childrenRow);
			m_files.removeFile(fileId);
			endRemoveRows();
		}
	}

	return true;
}

DbFileInfo SchemaListModelEx::file(int fileId) const
{
	auto foundFile = m_files.file(fileId);
	if (foundFile != nullptr)
	{
		return *foundFile.get();
	}
	else
	{
		return {};
	}
}

DbFileInfo SchemaListModelEx::file(const QModelIndex& modelIndex) const
{
	if (modelIndex.isValid() == false)
	{
		return m_parentFile;
	}

	int fileId = static_cast<int>(modelIndex.internalId());
	Q_ASSERT(fileId != -1);

	return file(fileId);
}

std::shared_ptr<DbFileInfo> SchemaListModelEx::fileSharedPtr(const QModelIndex& modelIndex) const
{
	if (modelIndex.isValid() == false)
	{
		return std::make_shared<DbFileInfo>(m_parentFile);
	}

	int fileId = static_cast<int>(modelIndex.internalId());
	Q_ASSERT(fileId != -1);

	return m_files.file(fileId);
}

bool SchemaListModelEx::isFolder(const QModelIndex& modelIndex) const
{
	if (modelIndex.isValid() == false)
	{
		return false;
	}

	int fileId = static_cast<int>(modelIndex.internalId());
	Q_ASSERT(fileId != -1);

	// --
	//
	auto foundFile = m_files.file(fileId);
	if (foundFile != nullptr)
	{
		return foundFile->isFolder();
	}
	else
	{
		return false;
	}
}

QModelIndexList SchemaListModelEx::searchFor(const QString searchText)
{
	m_searchText = searchText;
	return match(index(0, 0), SearchSchemaRole, QVariant::fromValue(true), -1, Qt::MatchExactly | Qt::MatchRecursive);
}

void SchemaListModelEx::setFilter(QString filter)
{
	m_filterText = filter;
	refresh();
	return;
}

void SchemaListModelEx::setTagFilter(const QStringList& tags)
{
	m_tagFilter = tags;
	refresh();
	return;
}

const QStringList& SchemaListModelEx::tagFilter() const
{
	return m_tagFilter;
}

void SchemaListModelEx::applyFilter(DbFileTree* filesTree, const std::map<int, VFrame30::SchemaDetails>& detailsMap)
{
	Q_ASSERT(filesTree);

	// Filetr by filter text
	//
	if (m_filterText.isEmpty() == true)
	{
		return;
	}

	int schemaFilterCount = 0;

	// Apply filter, if parent has any file with filterText, then this parent must be left in tree.
	// So, in tree are files with filterText and they parents
	// System files like Application Logic, Monitor, Tuning.... must be left
	//
	const std::map<int, std::shared_ptr<DbFileInfo>>& files = filesTree->files();
	int rootFileId = filesTree->rootFileId();

	// Filter files
	//
	std::map<int, std::shared_ptr<DbFileInfo>> filteredFiles;

	for (const auto&[fileId, file] : files)
	{
		if (isSystemFile(fileId) ||
			fileId == rootFileId)
		{
			filteredFiles[fileId] = file;
			continue;
		}

		// Filter by text
		//
		if (file->fileName().contains(m_filterText, Qt::CaseInsensitive) == true)
		{
			filteredFiles[fileId] = file;

			schemaFilterCount++;
			continue;
		}

		if (auto dit = detailsMap.find(fileId);
			dit != detailsMap.end())
		{
			const VFrame30::SchemaDetails& details = dit->second;

			if (bool searchResult = details.searchForString(m_filterText);
				searchResult == true)
			{
				filteredFiles[fileId] = file;

				schemaFilterCount++;
				continue;
			}

		}
		else
		{
			Q_ASSERT(dit != detailsMap.end());
		}
	}

	// Add parents
	//
	std::map<int, std::shared_ptr<DbFileInfo>> parentFiles;

	for (auto&[fileId, file] : filteredFiles)
	{
		if (isSystemFile(fileId) ||
			fileId == rootFileId)
		{
			continue;
		}

		auto parentIt = files.find(file->parentId());
		if (parentIt == files.end())
		{
			Q_ASSERT(false);
			continue;
		}

		std::shared_ptr<DbFileInfo> parentFile = parentIt->second;

		while (parentFile != nullptr &&
			   parentFile->isNull() == false &&
			   isSystemFile(parentFile->fileId()) == false)
		{
			parentFiles[parentFile->fileId()] = parentFile;

			auto parentInParentIt = files.find(parentFile->parentId());
			if (parentInParentIt == files.end())
			{
				Q_ASSERT(false);
				parentFile.reset();
			}
			else
			{
				parentFile = parentInParentIt->second;
			}
		}
	}

	for (auto&[fileId, file] : parentFiles)
	{
		filteredFiles[fileId] = file;
	}

	// --
	//
	*filesTree = std::move(DbFileTree{filteredFiles, rootFileId});

	m_schemaFilterCount = schemaFilterCount;

	return;
}

void SchemaListModelEx::applyTagFilter(DbFileTree* filesTree, const std::map<int, VFrame30::SchemaDetails>& detailsMap)
{
	Q_ASSERT(filesTree);

	if (m_tagFilter.isEmpty() == true)
	{
		return;
	}

	// Apply filter, if parent has any file with filterText, then this parent must be left in tree.
	// So, in tree are files with filterText and they parents
	// System files like Application Logic, Monitor, Tuning.... must be left
	//
	const std::map<int, std::shared_ptr<DbFileInfo>>& files = filesTree->files();
	int rootFileId = filesTree->rootFileId();

	// Filter files
	//
	std::map<int, std::shared_ptr<DbFileInfo>> filteredFiles;

	for (const auto&[fileId, file] : files)
	{
		if (isSystemFile(fileId) ||
			fileId == rootFileId)
		{
			filteredFiles[fileId] = file;
			continue;
		}

		if (file->isFolder() == true)
		{
			continue;	// If this folder contains any child it will be added later in "Add parent" part
		}

		if (auto dit = detailsMap.find(fileId);
			dit != detailsMap.end())
		{
			const VFrame30::SchemaDetails& details = dit->second;

			if (bool searchResult = details.hasTag(m_tagFilter);
				searchResult == true)
			{
				filteredFiles[fileId] = file;
				continue;
			}
		}
		else
		{
			Q_ASSERT(dit != detailsMap.end());
		}
	}

	// Add parents
	//
	std::map<int, std::shared_ptr<DbFileInfo>> parentFiles;

	for (auto&[fileId, file] : filteredFiles)
	{
		if (isSystemFile(fileId) ||
			fileId == rootFileId)
		{
			continue;
		}

		auto parentIt = files.find(file->parentId());
		if (parentIt == files.end())
		{
			Q_ASSERT(false);
			continue;
		}

		std::shared_ptr<DbFileInfo> parentFile = parentIt->second;

		while (parentFile != nullptr &&
			   parentFile->isNull() == false &&
			   isSystemFile(parentFile->fileId()) == false)
		{
			parentFiles[parentFile->fileId()] = parentFile;

			auto parentInParentIt = files.find(parentFile->parentId());
			if (parentInParentIt == files.end())
			{
				Q_ASSERT(false);
				parentFile.reset();
			}
			else
			{
				parentFile = parentInParentIt->second;
			}
		}
	}

	for (auto&[fileId, file] : parentFiles)
	{
		filteredFiles[fileId] = file;
	}

	// --
	//
	*filesTree = std::move(DbFileTree{filteredFiles, rootFileId});

	return;
}


bool SchemaListModelEx::isSystemFile(int fileId) const
{
	return m_systemFiles.find(fileId) != m_systemFiles.end();
}

void SchemaListModelEx::updateTagsFromDetails()
{
	m_tags.clear();

	for (auto&[fileId, details] : m_details)
	{
		for (const QString& tag : details.tags())
		{
			m_tags.insert(tag);
		}
	}

	emit tagsChanged();

	return;
}

void SchemaListModelEx::refresh()
{
	if (db()->isProjectOpened() == false)
	{
		projectClosed();
		return;
	}

	// Get file tree
	//
	DbFileTree files;
	bool ok = dbc()->getFileListTree(&files, m_parentFile.fileId(), true, m_parentWidget);

	if (ok == false)
	{
		return;		// do not reset model, just leave it as is
	}

	files.removeFilesWithExtension(Db::File::AlTemplExtension);
	files.removeFilesWithExtension(Db::File::MvsTemplExtension);
	files.removeFilesWithExtension(Db::File::UfbTemplExtension);
	files.removeFilesWithExtension(Db::File::DvsTemplExtension);

	// Parse file details, befor eapplying filter, as we want to keep tags for all schemas
	//
	std::map<int, VFrame30::SchemaDetails> detailsMap;

	for (auto& [fileId, fileInfo] : files.files())
	{
		VFrame30::SchemaDetails details;
		bool parsed = details.parseDetails(fileInfo->details());

		if (parsed == true)
		{
			detailsMap[fileId] = std::move(details);
		}
		else
		{
			//qDebug() << "void SchemaListModelEx::refresh(): File not parsed " << fileId << ", " << fileInfo->fileName();
		}
	}

	// Apply filters
	//
	applyTagFilter(&files, detailsMap);
	applyFilter(&files, detailsMap);

	// Get users
	//
	std::vector<DbUser> users;
	users.reserve(32);

	ok = dbc()->getUserList(&users, m_parentWidget);
	if (ok == false)
	{
		// Clear users, but don't return, we still can show files
		//
		users.clear();
	}

	std::map<int, QString> usersMap;
	for (const DbUser& u : users)
	{
		usersMap[u.userId()] = u.username();
	}

	// Set all data
	//
	beginResetModel();
	m_files = std::move(files);
	m_users = std::move(usersMap);
	m_details = std::move(detailsMap);
	endResetModel();

	updateTagsFromDetails();

	return;
}

void SchemaListModelEx::projectOpened(DbProject /*project*/)
{
	m_parentFile = db()->systemFileInfo(Db::File::SchemasFileName);
	Q_ASSERT(m_parentFile.fileId() != -1);

	std::vector<DbFileInfo> systemFiles = db()->systemFiles();
	for (const DbFileInfo& sf : systemFiles)
	{
		m_systemFiles.insert(sf.fileId());
	}

	refresh();

	return;
}

void SchemaListModelEx::projectClosed()
{
	beginResetModel();
	m_files.clear();
	m_users.clear();
	m_details.clear();
	endResetModel();

	m_parentFile = DbFileInfo();
	m_systemFiles.clear();
	m_schemaFilterCount = 0;
	m_tagFilter.clear();

	updateTagsFromDetails();

	return;
}

QString SchemaListModelEx::usernameById(int userId) const noexcept
{
	auto it = m_users.find(userId);

	if (it == m_users.end())
	{
		return QStringLiteral("Undefined");
	}
	else
	{
		return it->second;
	}
}

QString SchemaListModelEx::tagsColumnText(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return {};
	}

	QString result;
	result.reserve(256);

	const VFrame30::SchemaDetails& d = it->second;
	for (QString tag : d.m_tags)
	{
		if (result.isEmpty() == true)
		{
			result = tag;
		}
		else
		{
			result += QString(", %1").arg(tag);
		}
	}

	return result;
}

QString SchemaListModelEx::detailsColumnText(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return {};
	}

	const VFrame30::SchemaDetails& d = it->second;
	return d.m_equipmentId;
}

QString SchemaListModelEx::fileCaption(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return {};
	}

	const VFrame30::SchemaDetails& d = it->second;
	return d.m_caption;
}

bool SchemaListModelEx::excludedFromBuild(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return false;
	}

	const VFrame30::SchemaDetails& d = it->second;
	return d.m_excludedFromBuild;
}

const DbFileInfo& SchemaListModelEx::parentFile() const
{
	return m_parentFile;
}

int SchemaListModelEx::schemaFilterCount() const
{
	return m_schemaFilterCount;
}

const std::set<QString>& SchemaListModelEx::tags() const
{
	return m_tags;
}


//
// class SchemaProxyListModel
//
SchemaProxyListModel::SchemaProxyListModel(QObject* parent) :
	QSortFilterProxyModel(parent)
{
}


SchemaProxyListModel::~SchemaProxyListModel()
{
}

void SchemaProxyListModel::setSourceModel(QAbstractItemModel* sourceModel)
{
	QSortFilterProxyModel::setSourceModel(sourceModel);

	m_sourceModel = dynamic_cast<SchemaListModelEx*>(sourceModel);
	Q_ASSERT(m_sourceModel != nullptr);

	return;
}

bool SchemaProxyListModel::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
{
	// All folders alway at top
	//
	bool leftIsFolder = m_sourceModel->isFolder(sourceLeft);
	bool rightIsFolder = m_sourceModel->isFolder(sourceRight);

	bool result = false;

	if ((leftIsFolder == true && rightIsFolder == true) ||
		(leftIsFolder == false && rightIsFolder == false))
	{
		result = QSortFilterProxyModel::lessThan(sourceLeft, sourceRight);
	}
	else
	{
		// Relying on sort order helps to kepp folders always at the top
		//
		if (sortOrder() == Qt::AscendingOrder)
		{
			result = (leftIsFolder == true && rightIsFolder == false);
		}
		else
		{
			result = (leftIsFolder == false && rightIsFolder == true);
		}
	}

	return result;
}

DbFileInfo SchemaProxyListModel::file(const QModelIndex& mi) const
{
	QModelIndex mapped = mapToSource(mi);
	if (mapped.isValid() == false)
	{
		return {};
	}

	return m_sourceModel->file(mapped);
}

std::vector<int> SchemaProxyListModel::expandedFileIds(QTreeView* treeView)
{
	std::vector<int> fileIds;
	fileIds.reserve(32);

	QModelIndexList indexes = persistentIndexList();

	for (QModelIndex& mi : indexes)
	{
		int fileId = file(mi).fileId();

		if (treeView->isExpanded(mi) == true &&
			fileId != DbFileInfo::Null)
		{
			fileIds.push_back(fileId);
		}
	}

	return fileIds;
}

//
//
//	SchemaFileView
//
//
SchemaFileViewEx::SchemaFileViewEx(DbController* dbc, QWidget* parent) :
	QTreeView(parent),
	HasDbController(dbc),
	m_filesModel(dbc, this)
{
	Q_ASSERT(dbc != nullptr);

	setUniformRowHeights(true);
	setWordWrap(false);
	setExpandsOnDoubleClick(false);		// DoubleClick signal is used

	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
	//setIndentation(10);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	// --
	//
	createActions();
	createContextMenu();

	// Adjust view
	//
	//m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);
	m_proxyModel.setSourceModel(&m_filesModel);

	setModel(&m_proxyModel);

#ifdef _DEBUG
	[[maybe_unused]]QAbstractItemModelTester* modelTester = new QAbstractItemModelTester(&m_filesModel,
																		 QAbstractItemModelTester::FailureReportingMode::Fatal,
																		 this);
#endif

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaFileViewEx::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaFileViewEx::projectClosed);

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &SchemaFileViewEx::selectionChanged);

	connect(this, &QTreeView::doubleClicked, this, &SchemaFileViewEx::slot_doubleClicked);

	// Timer for updates of WRN/ERR count
	//
	startTimer(50);

	// --
	//
	QByteArray lastState = QSettings{}.value("SchemeEditor/SchemaFileViewEx/State").toByteArray();
	header()->restoreState(lastState);

	return;
}

SchemaFileViewEx::~SchemaFileViewEx()
{
	QSettings{}.setValue("SchemeEditor/SchemaFileViewEx/State", header()->saveState());
}

void SchemaFileViewEx::createActions()
{
	m_newFileAction = new QAction(tr("New Schema..."), parent());
	m_newFileAction->setIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));
	m_newFileAction->setStatusTip(tr("Add new schema to version control..."));
	m_newFileAction->setEnabled(false);
	m_newFileAction->setShortcut(QKeySequence::StandardKey::New);

	m_newFolderAction = new QAction(tr("New Folder..."), parent());
	m_newFolderAction->setIcon(QIcon(":/Images/Images/SchemaAddFolder2.svg"));
	m_newFolderAction->setStatusTip(tr("Add new folder to version control..."));
	m_newFolderAction->setEnabled(false);

	m_cloneFileAction = new QAction(tr("Clone Schema"), parent());
	m_cloneFileAction->setIcon(QIcon(":/Images/Images/SchemaClone.svg"));
	m_cloneFileAction->setStatusTip(tr("Clone file..."));
	m_cloneFileAction->setEnabled(false);

	m_openAction = new QAction(tr("Open Schema"), parent());
	m_openAction->setIcon(QIcon(":/Images/Images/SchemaOpen.svg"));
	m_openAction->setStatusTip(tr("Open file to edit"));
	m_openAction->setEnabled(false);

	m_viewAction = new QAction(tr("View Schema..."), parent());
	m_viewAction->setIcon(QIcon(":/Images/Images/SchemaView.svg"));
	m_viewAction->setStatusTip(tr("Open schema to view"));
	m_viewAction->setEnabled(false);

	m_deleteAction = new QAction(tr("Delete"), parent());
	m_deleteAction->setIcon(QIcon(":/Images/Images/SchemaDelete.svg"));
	m_deleteAction->setStatusTip(tr("Mark file as deleted..."));
	m_deleteAction->setEnabled(false);
	m_deleteAction->setShortcut(QKeySequence::Delete);

	m_moveFileAction = new QAction(tr("Move Schema(s)"), parent());
	m_moveFileAction->setStatusTip(tr("Move Schema(s) to another folder..."));
	m_moveFileAction->setEnabled(false);

	// --
	//
	m_checkOutAction = new QAction(tr("Check Out"), parent());
	m_checkOutAction->setIcon(QIcon(":/Images/Images/SchemaCheckOut.svg"));
	m_checkOutAction->setStatusTip(tr("Check Out for edit..."));
	m_checkOutAction->setEnabled(false);

	m_checkInAction = new QAction(tr("Check In"), parent());
	m_checkInAction->setIcon(QIcon(":/Images/Images/SchemaCheckIn.svg"));
	m_checkInAction->setStatusTip(tr("Check In pending changes..."));
	m_checkInAction->setEnabled(false);

	m_undoChangesAction = new QAction(tr("Undo Changes"), parent());
	m_undoChangesAction->setIcon(QIcon(":/Images/Images/SchemaUndo.svg"));
	m_undoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_undoChangesAction->setEnabled(false);

	m_historyAction = new QAction(tr("History..."), parent());
	m_historyAction->setIcon(QIcon(":/Images/Images/SchemaHistory.svg"));
	m_historyAction->setStatusTip(tr("Show file history..."));
	m_historyAction->setEnabled(false);

	m_recursiveHistoryAction = new QAction(tr("Recursive History..."), parent());
	m_recursiveHistoryAction->setIcon(QIcon(":/Images/Images/SchemaHistory.svg"));
	m_recursiveHistoryAction->setStatusTip(tr("Show file history recursively for all childern..."));
	m_recursiveHistoryAction->setEnabled(false);

	// --
	//
	m_compareAction = new QAction(tr("Compare..."), parent());
	m_compareAction->setStatusTip(tr("Compare file..."));
	m_compareAction->setEnabled(false);

	// --
	//
	m_exportWorkingcopyAction = new QAction(tr("Export Working Copy..."), parent());
	m_exportWorkingcopyAction->setIcon(QIcon(":/Images/Images/SchemaDownload.svg"));
	m_exportWorkingcopyAction->setStatusTip(tr("Export workingcopy file to disk..."));
	m_exportWorkingcopyAction->setEnabled(false);

	m_importWorkingcopyAction = new QAction(tr("Import Working Copy..."), parent());
	m_importWorkingcopyAction->setIcon(QIcon(":/Images/Images/SchemaUpload.svg"));
	m_importWorkingcopyAction->setStatusTip(tr("Import workingcopy file from disk to project file..."));
	m_importWorkingcopyAction->setEnabled(false);

	// --
	//
	m_refreshFileAction = new QAction(tr("Refresh"), parent());
	m_refreshFileAction->setIcon(QIcon(":/Images/Images/SchemaRefresh.svg"));
	m_refreshFileAction->setStatusTip(tr("Refresh file list..."));
	m_refreshFileAction->setEnabled(false);
	m_refreshFileAction->setShortcut(QKeySequence::StandardKey::Refresh);

	m_propertiesAction = new QAction(tr("Properties..."), parent());
	m_propertiesAction->setIcon(QIcon(":/Images/Images/SchemaProperties.svg"));
	m_propertiesAction->setStatusTip(tr("Edit schema properties..."));
	m_propertiesAction->setEnabled(false);

	connect(m_refreshFileAction, &QAction::triggered, this, &SchemaFileViewEx::slot_refreshFiles);
	return;
}

void SchemaFileViewEx::createContextMenu()
{
	setContextMenuPolicy(Qt::ActionsContextMenu);

	addAction(m_openAction);
	addAction(m_viewAction);

	// --
	//
	QAction* separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	addAction(m_newFileAction);
	addAction(m_newFolderAction);
	addAction(m_cloneFileAction);
	addAction(m_deleteAction);
	addAction(m_moveFileAction);

	// --
	//
	separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	addAction(m_checkOutAction);
	addAction(m_checkInAction);
	addAction(m_undoChangesAction);
	addAction(m_historyAction);
	addAction(m_recursiveHistoryAction);
	addAction(m_compareAction);

	// --
	//
	separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	addAction(m_exportWorkingcopyAction);
	addAction(m_importWorkingcopyAction);

	// --
	//
	separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	addAction(m_refreshFileAction);
	addAction(m_propertiesAction);

	return;
}

void SchemaFileViewEx::timerEvent(QTimerEvent* event)
{
	QTreeView::timerEvent(event);

	if (int buildIuuseCount = GlobalMessanger::instance().buildIssues().count();
		buildIuuseCount != m_lastBuildIssueCount)
	{
		m_lastBuildIssueCount = buildIuuseCount;

		// Update and repaint don't work (((
		//
		setUpdatesEnabled(false);
		setRootIsDecorated(false);
		setRootIsDecorated(true);
		setUpdatesEnabled(true);
	}

	return;
}

std::vector<std::shared_ptr<DbFileInfo>> SchemaFileViewEx::selectedFiles() const
{
	std::vector<std::shared_ptr<DbFileInfo>> result;

	QItemSelectionModel* selModel = selectionModel();
	if (selModel->hasSelection() == false)
	{
		return result;
	}

	QModelIndexList	sel = selModel->selectedRows();
	result.reserve(sel.size());

	for (int i = 0; i < sel.size(); i++)
	{
		QModelIndex mi = m_proxyModel.mapToSource(sel[i]);
		auto file = m_filesModel.fileSharedPtr(mi);

		if (file->fileId() == -1)
		{
			Q_ASSERT(file->fileId() != -1);
			return result;
		}

		result.push_back(file);
	}

	return result;
}

void SchemaFileViewEx::refreshFiles()
{
	// Save old selection and expansion
	//
	const QItemSelection proxySelection = selectionModel()->selection();
	const QItemSelection mappedSelection = m_proxyModel.mapSelectionToSource(proxySelection);

	std::vector<int> selectedFilesIds;
	selectedFilesIds.reserve(mappedSelection.size());

	for (QModelIndex mi : mappedSelection.indexes())
	{
		DbFileInfo file = m_filesModel.file(mi);
		if (file.isNull() == false)
		{
			selectedFilesIds.push_back(file.fileId());
		}
	}

	std::vector<int> expandedFileIds = m_proxyModel.expandedFileIds(this);

	selectionModel()->reset();

	// Update model
	//
	m_filesModel.refresh();

	// Restore selection
	//
	selectionModel()->blockSignals(true);

	// Select
	//
	for (int fileId : selectedFilesIds)
	{
		QModelIndexList matched = filesModel().match(m_filesModel.index(0, 0),
													 Qt::UserRole,
													 QVariant::fromValue(fileId),
													 1,
													 Qt::MatchExactly | Qt::MatchRecursive);

		if (matched.size() == 1)
		{
			QModelIndex fileModelIndex = matched.front();
			QModelIndex mappedModelIndex = m_proxyModel.mapFromSource(fileModelIndex);

			selectionModel()->select(mappedModelIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

			QModelIndex expandParent = mappedModelIndex.parent();
			while (expandParent.isValid() == true)
			{
				expand(expandParent);
				expandParent = expandParent.parent();
			}
		}
	}

	// Expand
	//
	for (int fileId : expandedFileIds)
	{
		QModelIndexList matched = filesModel().match(m_filesModel.index(0, 0),
													 Qt::UserRole,
													 QVariant::fromValue(fileId),
													 1,
													 Qt::MatchExactly | Qt::MatchRecursive);

		if (matched.size() == 1)
		{
			QModelIndex fileModelIndex = matched.front();
			QModelIndex mappedModelIndex = m_proxyModel.mapFromSource(fileModelIndex);

			QModelIndex expandIndex = mappedModelIndex;
			while (expandIndex.isValid() == true)
			{
				expand(expandIndex);
				expandIndex = expandIndex.parent();
			}
		}
	}

	selectionModel()->blockSignals(false);

	selectionChanged({}, {});					// To update actions
	return;
}

void SchemaFileViewEx::searchAndSelect(QString searchText)
{
	clearSelection();

	QModelIndexList matched = m_filesModel.searchFor(searchText);
	if (matched.isEmpty() == true)
	{
		return;
	}

	QItemSelection selection;

	for (QModelIndex& fileModelIndex : matched)
	{
		QModelIndex mappedModelIndex = m_proxyModel.mapFromSource(fileModelIndex);

		//selectionModel()->select(mappedModelIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		selection.select(mappedModelIndex, mappedModelIndex);

		QModelIndex expandParent = mappedModelIndex.parent();
		while (expandParent.isValid() == true)
		{
			expand(expandParent);
			expandParent = expandParent.parent();
		}
	}

	selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	// Scroll to somewhere, unfortuanatelly selectedIndexes does not provide sorted list, so it's just scroll somewhere
	//
	if (selection.indexes().empty() == false)
	{
		scrollTo(selection.indexes().front());
	}

	QMessageBox::information(this, qAppName(), tr("Found %1 schema(s)").arg(matched.size()));

	return;
}

void SchemaFileViewEx::setFilter(QString filter)
{
	m_filesModel.setFilter(filter);

	if (filter.trimmed().isEmpty() == false)
	{
		expandAll();
	}

	return;
}

void SchemaFileViewEx::setTagFilter(const QStringList& tags)
{
	m_filesModel.setTagFilter(tags);

	expandAll();

	return;
}

void SchemaFileViewEx::projectOpened()
{
	m_refreshFileAction->setEnabled(true);

	selectionChanged({}, {});

	return;
}

void SchemaFileViewEx::projectClosed()
{
	m_newFileAction->setEnabled(false);
	m_newFolderAction->setEnabled(false);
	m_cloneFileAction->setEnabled(false);
	m_refreshFileAction->setEnabled(false);

	return;
}

void SchemaFileViewEx::slot_refreshFiles()
{
	refreshFiles();
	return;
}

void SchemaFileViewEx::slot_doubleClicked(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	DbFileInfo file = m_proxyModel.file(index);
	if (file.isNull() == true)
	{
		return;
	}

	// If folder then expand/collapse it
	// If file is schema then open or view it
	//
	if (file.directoryAttribute() == true)
	{
		QModelIndex column0Index = index.siblingAtColumn(0);
		setExpanded(column0Index, !isExpanded(column0Index));
	}
	else
	{
		if (file.state() == VcsState::CheckedOut)
		{
			emit openFileSignal(file);
		}
		else
		{
			emit viewFileSignal(file);
		}
	}

	return;
}

void SchemaFileViewEx::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = this->selectedFiles();
	bool selectedOneNonSystemFile = selectedFiles.size() == 1 &&
									db()->systemFileInfo(selectedFiles.front()->fileId()).isNull() == true &&
									selectedFiles.front()->directoryAttribute() == false;

	m_newFileAction->setEnabled(selectedFiles.size() == 1);
	m_newFolderAction->setEnabled(selectedFiles.size() == 1);
	m_cloneFileAction->setEnabled(selectedOneNonSystemFile);

	// --
	//
	int currentUserId = dbc()->currentUser().userId();
	bool currentUserIsAdmin = dbc()->currentUser().isAdminstrator();

	bool hasDeletePossibility = false;
	bool hasMovePossibility = false;
	bool hasCheckOutPossibility = false;
	bool hasCheckInPossibility = false;
	bool hasUndoPossibility = false;
	bool hasAbilityToOpen = false;
	bool hasViewPossibility = false;

	bool canGetWorkcopy = false;
	int canSetWorkcopy = 0;

	bool schemaPoperties = (selectedFiles.empty() == false);

	// hasAbilityToOpen
	//
	if (selectedFiles.size() == 1 &&
		selectedFiles.front()->state() == VcsState::CheckedOut &&
		selectedFiles.front()->directoryAttribute() == false &&
		(selectedFiles.front()->userId() == currentUserId  || currentUserIsAdmin == true))
	{
		hasAbilityToOpen = true;
	}

	// hasViewPossibility
	//
	if (selectedFiles.size() == 1 &&
		selectedFiles.front()->directoryAttribute() == false)
	{
		hasViewPossibility = true;
	}

	for (const std::shared_ptr<DbFileInfo>& file : selectedFiles)
	{
		bool fileIsSystem = dbc()->systemFileInfo(file->fileId()).isNull() == false;

		if (fileIsSystem == true)	// No any possibilty on system files
		{
			continue;
		}

		// hasDeletePossibility
		//
		if ((file->state() == VcsState::CheckedOut && file->userId() == currentUserId) ||
			file->state() == VcsState::CheckedIn)
		{
			hasDeletePossibility = true;
		}

		// hasMovePossibility
		//
		if (file->state() == VcsState::CheckedOut && file->userId() == currentUserId)
		{
			hasMovePossibility = true;
		}

		// hasCheckOutPossibility
		//
		if (file->state() == VcsState::CheckedIn)
		{
			hasCheckOutPossibility = true;
		}

		// hasCheckInPossibility
		//
		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == currentUserId || currentUserIsAdmin == true))
		{
			hasCheckInPossibility = true;
		}

		// hasUndoPossibility
		//
		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == currentUserId || currentUserIsAdmin == true))
		{
			hasUndoPossibility = true;
		}

		// canGetWorkcopy, canSetWorkcopy
		//
		if (file->state() == VcsState::CheckedOut &&
			file->directoryAttribute() == false &&
			file->userId() == currentUserId)
		{
			canGetWorkcopy = true;
			canSetWorkcopy ++;
		}
	}

	// --
	//
	m_openAction->setEnabled(hasAbilityToOpen);
	m_viewAction->setEnabled(hasViewPossibility);

	m_deleteAction->setEnabled(hasDeletePossibility);
	m_moveFileAction->setEnabled(hasMovePossibility);
	m_checkOutAction->setEnabled(hasCheckOutPossibility);
	m_checkInAction->setEnabled(hasCheckInPossibility);
	m_undoChangesAction->setEnabled(hasUndoPossibility);

	m_historyAction->setEnabled(selectedFiles.size() == 1);
	m_recursiveHistoryAction->setEnabled(selectedFiles.size() == 1);
	m_compareAction->setEnabled(selectedFiles.size() == 1);

	m_exportWorkingcopyAction->setEnabled(canGetWorkcopy);
	m_importWorkingcopyAction->setEnabled(canSetWorkcopy == 1);			// can set work copy just for one file

	m_propertiesAction->setEnabled(schemaPoperties);			// can set work copy just for one file

	return;
}

SchemaListModelEx& SchemaFileViewEx::filesModel()
{
	return m_filesModel;
}

SchemaProxyListModel& SchemaFileViewEx::proxyModel()
{
	return m_proxyModel;
}

//const std::vector<std::shared_ptr<DbFileInfo>>& SchemaFileView::files() const
//{
//	return m_filesModel.files();
//}

const DbFileInfo& SchemaFileViewEx::parentFile() const
{
	return m_filesModel.parentFile();
}

int SchemaFileViewEx::parentFileId() const
{
	return m_filesModel.parentFile().fileId();
}

//
//
// SchemasTabPage
//
//
SchemasTabPageEx::SchemasTabPageEx(DbController* dbc, QWidget* parent) :
	MainTabPage(dbc, parent)
{
	m_tabWidget = new QTabWidget{};
	m_tabWidget->setMovable(true);

	QSize sz = fontMetrics().size(Qt::TextSingleLine, "APPLICATION LOGIC");
	sz.setHeight(static_cast<int>(sz.height() * 1.75));

	QString ss = QString("QTabBar::tab{ min-width: %1px; min-height: %2px;}").arg(sz.width()).arg(sz.height());
	m_tabWidget->tabBar()->setStyleSheet(ss);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 6, 0, 0);

	layout->addWidget(m_tabWidget);

	setLayout(layout);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemasTabPageEx::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemasTabPageEx::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);

	// Add control page
	//
	m_controlTabPage = new SchemaControlTabPageEx(dbc);
	m_tabWidget->addTab(m_controlTabPage, tr("Schemas Control"));

	return;
}

SchemasTabPageEx::~SchemasTabPageEx()
{
}

bool SchemasTabPageEx::hasUnsavedSchemas() const
{
	return m_controlTabPage->hasUnsavedSchemas();
}

bool SchemasTabPageEx::saveUnsavedSchemas()
{
	return m_controlTabPage->saveUnsavedSchemas();
}

bool SchemasTabPageEx::resetModified()
{
	return m_controlTabPage->resetModified();
}

void SchemasTabPageEx::refreshControlTabPage()
{
	Q_ASSERT(m_controlTabPage);
	m_controlTabPage->refresh();

	return;
}

void SchemasTabPageEx::projectOpened()
{
	this->setEnabled(true);
}

void SchemasTabPageEx::projectClosed()
{
	GlobalMessanger::instance().clearBuildSchemaIssues();
	GlobalMessanger::instance().clearSchemaItemRunOrder();

	this->setEnabled(false);
	return;
}

//
//
// SchemaControlTabPage
//
//
SchemaControlTabPageEx::SchemaControlTabPageEx(DbController* db) :
		HasDbController(db)
{
	// Create controls
	//
	m_filesView = new SchemaFileViewEx(db, this);

	// --
	//
	m_toolBar = new QToolBar{};

	createToolBar();

	m_toolBar->setStyleSheet("QToolButton { padding-top: 6px; padding-bottom: 6px; padding-left: 6px; padding-right: 6px;}");
	m_toolBar->setIconSize(m_toolBar->iconSize() * 0.9);

	connect(m_filesView->m_openAction, &QAction::triggered, this, &SchemaControlTabPageEx::openSelectedFile);
	connect(m_filesView->m_viewAction, &QAction::triggered, this, &SchemaControlTabPageEx::viewSelectedFile);

	connect(m_filesView->m_newFileAction, &QAction::triggered, this, &SchemaControlTabPageEx::addFile);
	connect(m_filesView->m_newFolderAction, &QAction::triggered, this, &SchemaControlTabPageEx::addFolder);
	connect(m_filesView->m_cloneFileAction, &QAction::triggered, this, &SchemaControlTabPageEx::cloneFile);
	connect(m_filesView->m_deleteAction, &QAction::triggered, this, &SchemaControlTabPageEx::deleteFiles);
	connect(m_filesView->m_moveFileAction, &QAction::triggered, this, &SchemaControlTabPageEx::moveFiles);

	connect(m_filesView->m_checkOutAction, &QAction::triggered, this, &SchemaControlTabPageEx::checkOutFiles);
	connect(m_filesView->m_checkInAction, &QAction::triggered, this, &SchemaControlTabPageEx::checkInFiles);
	connect(m_filesView->m_undoChangesAction, &QAction::triggered, this, &SchemaControlTabPageEx::undoChangesFiles);

	connect(m_filesView->m_historyAction, &QAction::triggered, this, &SchemaControlTabPageEx::showFileHistory);
	connect(m_filesView->m_recursiveHistoryAction, &QAction::triggered, this, &SchemaControlTabPageEx::showFileHistoryRecursive);
	connect(m_filesView->m_compareAction, &QAction::triggered, this, &SchemaControlTabPageEx::compareSelectedFile);

	connect(m_filesView->m_exportWorkingcopyAction, &QAction::triggered, this, &SchemaControlTabPageEx::exportWorkcopy);
	connect(m_filesView->m_importWorkingcopyAction, &QAction::triggered, this, &SchemaControlTabPageEx::importWorkcopy);

	connect(m_filesView->m_propertiesAction, &QAction::triggered, this, &SchemaControlTabPageEx::showFileProperties);

	connect(&m_filesView->filesModel(), &SchemaListModelEx::tagsChanged, this, &SchemaControlTabPageEx::schemaTagsChanged);

	// --
	//
	m_searchAction = new QAction(tr("Edit Search"), this);
	m_searchAction->setShortcut(QKeySequence::Find);
	addAction(m_searchAction);

	m_searchEdit = new QLineEdit(this);
	m_searchEdit->setPlaceholderText(tr("Search Text"));
	m_searchEdit->setClearButtonEnabled(true);

	m_filterEdit = new QLineEdit(this);
	m_filterEdit->setPlaceholderText(tr("Filter Text"));
	m_filterEdit->setClearButtonEnabled(true);

	QStringList completerStringList = QSettings{}.value("SchemaControlTabPageEx/SearchCompleter").toStringList();
	m_searchCompleter = new QCompleter(completerStringList, this);
	m_searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_searchEdit->setCompleter(m_searchCompleter);
	m_filterEdit->setCompleter(m_searchCompleter);

	m_searchButton = new QPushButton(tr("Search"));
	m_filterButton = new QPushButton(tr("Filter"));

	m_resetFilterButton = new QPushButton(tr("Reset Filter"));
	m_resetFilterButton->setDisabled(true);

	m_tagSelector = new TagSelectorWidget(this);
	m_tagSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(m_tagSelector, &TagSelectorWidget::changed, this, &SchemaControlTabPageEx::tagSelectorHasChanges);

	// --
	//
	QGridLayout* layout = new QGridLayout(this);
	layout->setMenuBar(m_toolBar);						// Set ToolBar here as menu, so no gaps and margins
	layout->addWidget(m_filesView, 0, 0, 1, 6);

	layout->addWidget(m_searchEdit, 1, 0, 1, 2);
	layout->addWidget(m_searchButton, 1, 2, 1, 1);

	layout->addWidget(m_filterEdit, 2, 0, 1, 2);
	layout->addWidget(m_filterButton, 2, 2, 1, 1);
	layout->addWidget(m_resetFilterButton, 2, 3, 1, 1);
	layout->addWidget(m_tagSelector, 1, 4, 2, 2);

	layout->setColumnStretch(0, 2);
	layout->setColumnStretch(4, 2);
	layout->setColumnStretch(5, 2);

	layout->setRowStretch(0, 2);
	layout->setRowStretch(1, 0);

	setLayout(layout);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaControlTabPageEx::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaControlTabPageEx::projectClosed);

	connect(m_filesView, &SchemaFileViewEx::openFileSignal, this, &SchemaControlTabPageEx::openFile);
	connect(m_filesView, &SchemaFileViewEx::viewFileSignal, this, &SchemaControlTabPageEx::viewFile);

	connect(m_searchAction, &QAction::triggered, this, &SchemaControlTabPageEx::ctrlF);
	connect(m_searchEdit, &QLineEdit::returnPressed, this, &SchemaControlTabPageEx::search);
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &SchemaControlTabPageEx::filter);
	connect(m_searchButton, &QPushButton::clicked, this, &SchemaControlTabPageEx::search);
	connect(m_filterButton, &QPushButton::clicked, this, &SchemaControlTabPageEx::filter);
	connect(m_resetFilterButton, &QPushButton::clicked, this, &SchemaControlTabPageEx::resetFilter);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::addLogicSchema, this, &SchemaControlTabPageEx::addLogicSchema);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::searchSchemaForLm, this, &SchemaControlTabPageEx::searchSchemaForLm);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &SchemaControlTabPageEx::compareObject);

	return;
}


SchemaControlTabPageEx::~SchemaControlTabPageEx()
{
}

VFrame30::Schema* SchemaControlTabPageEx::createSchema() const
{
	Q_ASSERT(false);
	return nullptr;
}

bool SchemaControlTabPageEx::hasUnsavedSchemas() const
{
	bool result = false;
	for (auto editWidget : m_openedFiles)
	{
		result |= editWidget->modified();
	}

	return result;
}

bool SchemaControlTabPageEx::saveUnsavedSchemas()
{
	bool ok = true;

	for (auto editWidget : m_openedFiles)
	{
		if (editWidget->modified() == true)
		{
			ok &= editWidget->saveWorkcopy();
		}
	}

	return ok;
}

bool SchemaControlTabPageEx::resetModified()
{
	bool ok = true;

	for (auto editWidget : m_openedFiles)
	{
		editWidget->resetModified();
	}

	return ok;
}

void SchemaControlTabPageEx::refresh()
{
	Q_ASSERT(m_filesView);

	if (m_filesView != nullptr)
	{
		m_filesView->refreshFiles();
	}

	return;
}

void SchemaControlTabPageEx::createToolBar()
{
	// Actions created in SchemaVileViewEx
	//
	m_toolBar->addAction(m_filesView->m_openAction);
	m_toolBar->addAction(m_filesView->m_viewAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_filesView->m_newFileAction);
	m_toolBar->addAction(m_filesView->m_newFolderAction);
	m_toolBar->addAction(m_filesView->m_cloneFileAction);
	m_toolBar->addAction(m_filesView->m_deleteAction);
	//m_toolBar->addAction(m_filesView->m_moveFileAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_filesView->m_checkOutAction);
	m_toolBar->addAction(m_filesView->m_checkInAction);
	m_toolBar->addAction(m_filesView->m_undoChangesAction);
	m_toolBar->addAction(m_filesView->m_historyAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_filesView->m_exportWorkingcopyAction);
	m_toolBar->addAction(m_filesView->m_importWorkingcopyAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_filesView->m_refreshFileAction);
	m_toolBar->addAction(m_filesView->m_propertiesAction);

	m_toolBar->addSeparator();

	return;
}

std::shared_ptr<VFrame30::Schema> SchemaControlTabPageEx::createSchema(const DbFileInfo& parentFile) const
{
	if (parentFile.isNull() == true)
	{
		Q_ASSERT(parentFile.isNull() == false);
		return {};
	}

	// If parent  or it's parent... is $root$/Schemas/ApplicatinLogic
	// the create als
	//
	auto createAppLogicSchema =	[]{	return std::make_shared<VFrame30::LogicSchema>();	};
	auto createMonitorSchema =	[]{	return std::make_shared<VFrame30::MonitorSchema>();	};
	auto createTuningSchema =	[]{	return std::make_shared<VFrame30::TuningSchema>();	};
	auto createUfbSchema =		[]{	return std::make_shared<VFrame30::UfbSchema>();		};

	DbFileInfo lookForSystemParent = parentFile;
	do
	{
		if (lookForSystemParent.fileId() == db()->alFileId())
		{
			return createAppLogicSchema();
		}

		if (lookForSystemParent.fileId() == db()->mvsFileId())
		{
			return createMonitorSchema();
		}

		if (lookForSystemParent.fileId() == db()->tvsFileId())
		{
			return createTuningSchema();
		}

		if (lookForSystemParent.fileId() == db()->ufblFileId())
		{
			return createUfbSchema();
		}

		lookForSystemParent = m_filesView->filesModel().file(lookForSystemParent.parentId());
	} while (lookForSystemParent.isNull() == false);

	// What kind of schema suppose to be created?
	//
	Q_ASSERT(false);

	return {};
}

EditSchemaTabPageEx* SchemaControlTabPageEx::findOpenedFile(const DbFileInfo& file, bool readOnly)
{
	for (auto editSchema : m_openedFiles)
	{
		if (readOnly == false)
		{
			if (editSchema->fileInfo().fileId() == file.fileId() &&
				editSchema->readOnly() == false)
			{
				return editSchema;
			}
		}
		else
		{
			if (editSchema->fileInfo().fileId() == file.fileId() &&
				editSchema->readOnly() == true &&
				editSchema->fileInfo().changeset() == file.changeset())
			{
				return editSchema;
			}
		}
	}

	return nullptr;
}

void SchemaControlTabPageEx::removeFromOpenedList(EditSchemaTabPageEx* editTabPage)
{
	if (editTabPage == nullptr)
	{
		Q_ASSERT(editTabPage);
		return;
	}

	m_openedFiles.remove(editTabPage);
	return;
}

void SchemaControlTabPageEx::detachOrAttachWindow(EditSchemaTabPageEx* editTabPage)
{
	if (editTabPage == nullptr)
	{
		Q_ASSERT(editTabPage);
		return;
	}

	// --
	//
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget != nullptr);
		return;
	}

	if (tabWidget->indexOf(editTabPage) != -1)
	{
		// Detach from TabWidget
		//
		tabWidget->removeTab(tabWidget->indexOf(editTabPage));

		editTabPage->setParent(nullptr);
		editTabPage->setWindowFlag(Qt::WindowType::Window);
		editTabPage->setWindowState(Qt::WindowMaximized);
	}
	else
	{
		// Attach to TabWidget
		//
		editTabPage->setWindowFlag(Qt::WindowType::Widget);
		tabWidget->addTab(editTabPage, editTabPage->windowTitle());
		tabWidget->setCurrentWidget(editTabPage);
	}

	editTabPage->updateZoomAndScrolls(false);
	editTabPage->setVisible(true);
	editTabPage->activateWindow();

	return;
}

void SchemaControlTabPageEx::projectOpened()
{
	m_lastSelectedNewSchemaForLmFileId = db()->alFileId();
	setEnabled(true);
}

void SchemaControlTabPageEx::projectClosed()
{
	m_lastSelectedNewSchemaForLmFileId = -1;
	m_tagSelector->clear();
	setEnabled(false);
}

int SchemaControlTabPageEx::showSelectFolderDialog(int parentFileId, int currentSelectionFileId, bool showRootFile)
{
	// Show dialog with file tree to select file, can be used as parent.
	// function returns selected file id or -1 if operation canceled
	//
	QDialog d(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	d.setWindowTitle(tr("Select parent"));

	// --
	//
	QLabel* textLabel = new QLabel(tr("Select parent for new file"));

	QTreeWidget* treeWidget = new QTreeWidget;
	treeWidget->setSortingEnabled(true);
	treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);
	treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	treeWidget->setHeaderLabel("File");

	DbFileTree files;

	if (bool ok = dbc()->getFileListTree(&files, parentFileId, true, this);
		ok == false)
	{
		return -1;
	}

	files.removeIf([](const DbFileInfo& fi)
		{
			return fi.directoryAttribute() == false;
		});

	std::shared_ptr<DbFileInfo> schemaFile = files.rootFile();		// SchemaFile
	Q_ASSERT(schemaFile->directoryAttribute() == true);

	static QIcon staticFolderIcon(":/Images/Images/SchemaFolder.svg");
	const QIcon* const ptrToIcon = &staticFolderIcon;
	QTreeWidgetItem* treeItemToSelect = nullptr;

	std::function<void(std::shared_ptr<DbFileInfo>, QTreeWidgetItem*)> addChilderenFilesFunc =
		[&addChilderenFilesFunc, &files, treeWidget, currentSelectionFileId, &treeItemToSelect, ptrToIcon](std::shared_ptr<DbFileInfo> parent, QTreeWidgetItem* parentTreeItem)
		{
			Q_ASSERT(parent->isNull() == false);

			const auto& childeren = files.children(parent->fileId());

			for (auto file : childeren)
			{
				if (file->isNull() == true ||
					file->directoryAttribute() == false)
				{
					Q_ASSERT(file->isNull() == false);
					Q_ASSERT(file->directoryAttribute() == true);
					return;
				}

				QTreeWidgetItem* treeItem = nullptr;

				if (parentTreeItem == nullptr)
				{
					treeItem = new QTreeWidgetItem(treeWidget, {file->fileName()}, file->fileId()) ;
					treeWidget->addTopLevelItem(treeItem);
				}
				else
				{
					treeItem = new QTreeWidgetItem(parentTreeItem, {file->fileName()}, file->fileId()) ;
				}
				treeItem->setIcon(0, *ptrToIcon);

				addChilderenFilesFunc(file, treeItem);

				if (file->fileId() == currentSelectionFileId)
				{
					treeItem->setSelected(true);
					treeItemToSelect = treeItem;
				}
			}
		};

	QTreeWidgetItem* rootTreeItem = nullptr;
	if (showRootFile == true)
	{
		rootTreeItem = new QTreeWidgetItem(treeWidget, {schemaFile->fileName()}, schemaFile->fileId()) ;
		rootTreeItem->setIcon(0, staticFolderIcon);
		treeWidget->addTopLevelItem(rootTreeItem);

		if (schemaFile->fileId() == currentSelectionFileId)
		{
			rootTreeItem->setSelected(true);
			treeItemToSelect = rootTreeItem;
		}
	}

	addChilderenFilesFunc(schemaFile, rootTreeItem);

	if (treeItemToSelect != nullptr)
	{
		treeWidget->scrollToItem(treeItemToSelect);
	}

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(textLabel);
	layout->addWidget(treeWidget);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	int result = d.exec();
	if (result == QDialog::Accepted)
	{
		auto selected = treeWidget->selectedItems();
		if (selected.size() != 1)
		{
			return -1;
		}

		return selected.front()->type();
	}

	return -1;
}

void SchemaControlTabPageEx::openSelectedFile()
{
	auto selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.size() != 1)
	{
		Q_ASSERT(selectedFiles.size() == 1);
		return;
	}

	std::shared_ptr<DbFileInfo> file = selectedFiles.front();

	return openFile(*file);
}

void SchemaControlTabPageEx::viewSelectedFile()
{
	auto selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.size() != 1)
	{
		Q_ASSERT(selectedFiles.size() == 1);
		return;
	}

	std::shared_ptr<DbFileInfo> file = selectedFiles.front();

	return viewFile(*file);
}

void SchemaControlTabPageEx::openFile(const DbFileInfo& file)
{
	if (file.isNull() == true)
	{
		Q_ASSERT(file.isNull() == false);
		return;
	}

	if (file.state() != VcsState::CheckedOut)
	{
		QMessageBox mb(this);
		mb.setText(tr("Check Out file for edit first."));
		mb.exec();
		return;
	}

	if (file.state() == VcsState::CheckedOut &&
		file.userId() != db()->currentUser().userId())
	{
		QMessageBox mb(this);
		QString username = db()->username(file.userId());
		mb.setText(tr("File %1 is already checked out by user <b>%2</b>.").arg(file.fileName()).arg(username));
		mb.exec();
		return;
	}

	Q_ASSERT(file.state() == VcsState::CheckedOut && file.userId() == db()->currentUser().userId());

	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget != nullptr);
		return;
	}

	// Check if file already open, and activate it if it's so
	//
	if (auto editTabPage = findOpenedFile(file, false);
		editTabPage != nullptr)
	{
		// File already opened, check if it is opened for edit then activate this tab
		//
		if (editTabPage->readOnly() == false &&
			editTabPage->fileInfo().fileId() == file.fileId())
		{
			if (tabWidget->indexOf(editTabPage) != -1)
			{
				tabWidget->activateWindow();
				tabWidget->setCurrentWidget(editTabPage);
			}
			else
			{
				editTabPage->activateWindow();
				editTabPage->raise();
				QApplication::alert(editTabPage, 500);
			}

			return;
		}
	}

	// Get file from the DB
	//
	std::vector<std::shared_ptr<DbFile>> out;
	std::vector<DbFileInfo> files{file};

	bool result = db()->getWorkcopy(files, &out, this);
	if (result == false || out.size() != files.size())
	{
		QMessageBox::critical(this, tr("Error"), "Can't get file from the database.");
		return;
	}

	// Load file
	//
	std::shared_ptr<VFrame30::Schema> vf(VFrame30::Schema::Create(out[0].get()->data()));

	if (vf == nullptr)
	{
		Q_ASSERT(vf != nullptr);
		return;
	}

	// Create TabPage and add it to the TabControl
	//
	DbFileInfo fi(*(out.front().get()));

	EditSchemaTabPageEx* editTabPage = new EditSchemaTabPageEx(tabWidget, vf, fi, db());

	connect(editTabPage, &EditSchemaTabPageEx::vcsFileStateChanged, m_filesView, &SchemaFileViewEx::slot_refreshFiles);
	connect(editTabPage, &EditSchemaTabPageEx::aboutToClose, this, &SchemaControlTabPageEx::removeFromOpenedList);
	connect(editTabPage, &EditSchemaTabPageEx::pleaseDetachOrAttachWindow, this, &SchemaControlTabPageEx::detachOrAttachWindow);

	Q_ASSERT(tabWidget->parent());

	SchemasTabPageEx* schemasTabPage = dynamic_cast<SchemasTabPageEx*>(tabWidget->parent());
	if (schemasTabPage == nullptr)
	{
		Q_ASSERT(dynamic_cast<SchemasTabPageEx*>(tabWidget->parent()));
		return;
	}

	connect(&GlobalMessanger::instance(), &GlobalMessanger::buildStarted, editTabPage, &EditSchemaTabPageEx::saveWorkcopy);

	// Update AFBs/UFBs after creating tab page, so it will be possible to set new (modified) caption
	// to the tab page title
	//
	editTabPage->updateAfbSchemaItems();
	editTabPage->updateUfbSchemaItems();
	editTabPage->updateBussesSchemaItems();

	// Do this ONLY after update, because during updateAfbSchemaItems/updateUfbSchemaItems/updateBussesSchemaItems
	// window can be closed by Ctrl+w, and programm crashes then
	//
	editTabPage->setReadOnly(false);

	tabWidget->addTab(editTabPage, editTabPage->windowTitle());
	tabWidget->setCurrentWidget(editTabPage);

	m_openedFiles.push_back(editTabPage);

	return;
}

void SchemaControlTabPageEx::viewFile(const DbFileInfo& file)
{
	if (file.isNull() == true)
	{
		Q_ASSERT(file.isNull() == false);
		return;
	}

	// --
	//
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget != nullptr);
		return;
	}

	// Show chageset dialog
	//
	int changesetId = SelectChangesetDialog::getFileChangeset(db(), file, this);
	if (changesetId == -1)
	{
		return;
	}

	// Get file with choosen changeset
	//
	std::shared_ptr<DbFile> out;

	bool result = db()->getSpecificCopy(file, changesetId, &out, this);
	if (result == false || out == nullptr)
	{
		return;
	}

	DbFileInfo fi(*out);

	// Load file
	//
	std::shared_ptr<VFrame30::Schema> vf(VFrame30::Schema::Create(out->data()));

	// Find the opened read only file with the same changeset
	//
	if (auto editTabPage = findOpenedFile(fi, true);
		editTabPage != nullptr)
	{
		// File already opened, check if it is opened for edit then activate this tab
		//
		if (editTabPage->readOnly() == true &&
			editTabPage->fileInfo().fileId() == fi.fileId() &&
			editTabPage->fileInfo().changeset() == fi.changeset())
		{
			if (tabWidget->indexOf(editTabPage) != -1)
			{
				tabWidget->activateWindow();
				tabWidget->setCurrentWidget(editTabPage);
			}
			else
			{
				editTabPage->activateWindow();
				editTabPage->raise();
				QApplication::alert(editTabPage, 500);
			}

			return;
		}
	}

	// Create TabPage and add it to the TabControl
	//
	EditSchemaTabPageEx* editTabPage = new EditSchemaTabPageEx(tabWidget, vf, fi, db());

	connect(editTabPage, &EditSchemaTabPageEx::aboutToClose, this, &SchemaControlTabPageEx::removeFromOpenedList);
	connect(editTabPage, &EditSchemaTabPageEx::pleaseDetachOrAttachWindow, this, &SchemaControlTabPageEx::detachOrAttachWindow);

	editTabPage->setReadOnly(true);

	tabWidget->addTab(editTabPage, editTabPage->windowTitle());
	tabWidget->setCurrentWidget(editTabPage);

	m_openedFiles.push_back(editTabPage);

	return;
}

void SchemaControlTabPageEx::addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile)
{
	int parentFileId = showSelectFolderDialog(dbc()->alFileId(), m_lastSelectedNewSchemaForLmFileId, true);
	if (parentFileId == -1)
	{
		return;
	}

	m_lastSelectedNewSchemaForLmFileId = parentFileId;

	// Create new Schema and add it to the vcs
	//
	DbFileInfo parentFile;

	bool ok = db()->getFileInfo(parentFileId, &parentFile, this);
	if (ok == false)
	{
		return;
	}

	std::shared_ptr<VFrame30::Schema> schema = createSchema(parentFile);
	if (schema->isLogicSchema() == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Can add Logic Schema only to '%1' or it's descendands.").arg(Db::File::AlFileName));
		return;
	}

	// Set New Guid
	//
	schema->setGuid(QUuid::createUuid());
	int sequenceNo = db()->nextCounterValue();

	// Set default properties
	//
	schema->setSchemaId("APPSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0'));
	schema->setCaption("Caption "  + QString::number(sequenceNo).rightJustified(6, '0'));

	schema->setDocWidth(420.0 / 25.4);
	schema->setDocHeight(297.0 / 25.4);

	if (VFrame30::LogicSchema* logicSchema = dynamic_cast<VFrame30::LogicSchema*>(schema.get());
		logicSchema != nullptr)
	{
		logicSchema->setEquipmentIdList(deviceStrIds);
		logicSchema->setPropertyValue(Hardware::PropertyNames::lmDescriptionFile, QVariant(lmDescriptionFile));
	}

	// --
	//
	addSchemaFile(schema, Db::File::AlFileExtension, parentFile.fileId());

	GlobalMessanger::instance().fireChangeCurrentTab(this->parentWidget()->parentWidget()->parentWidget());

	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->parentWidget()->parentWidget());
	Q_ASSERT(tabWidget);

	if (tabWidget != nullptr)
	{
		// Activate ControlTabPage (this)
		//
		tabWidget->setCurrentWidget(this);
	}

	m_filesView->setFocus();
	return;
}

void SchemaControlTabPageEx::addFile()
{
    QModelIndexList selectedRows = m_filesView->selectionModel()->selectedRows();
	if (selectedRows.size() != 1)
    {
		Q_ASSERT(selectedRows.size() == 1);
        return;
    }

	QModelIndex selectedModelIndex =  m_filesView->proxyModel().mapToSource(selectedRows.front());
	DbFileInfo selectedFile = m_filesView->filesModel().file(selectedModelIndex);

	DbFileInfo parentFile;

	// If folder selected, then create new file in this bolder
	//
	if (selectedFile.directoryAttribute() == true)
	{
		parentFile = selectedFile;
	}
	else
	{
		// If File selected, the create new file in the same folder as selected one
		//
		parentFile = m_filesView->filesModel().file(selectedFile.parentId());
	}

	if (parentFile.isNull() == true ||
		parentFile.directoryAttribute() == false)
	{
		Q_ASSERT(parentFile.isNull() == false);
		Q_ASSERT(parentFile.directoryAttribute());
		return;
	}

	// Creating new schema depends on parent, if it is ApplicationLogic, then ALS file is created,
	// if Monitor, then MVS, so on
	//
	std::shared_ptr<VFrame30::Schema> schema = createSchema(parentFile);
	if (schema == nullptr)
	{
		Q_ASSERT(schema);
		return;
	}

    // Create new Schema and add it to the vcs
    //

    // Set New Guid
    //
    schema->setGuid(QUuid::createUuid());

    // Set default ID
    //
	int sequenceNo = db()->nextCounterValue();
    QString defaultId = "SCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
	QString extension;

    if (schema->isLogicSchema() == true)
    {
        defaultId = "APPSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = Db::File::AlFileExtension;
    }

    if (schema->isUfbSchema() == true)
    {
        defaultId = "UFBID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = Db::File::UfbFileExtension;
    }

	if (schema->isMonitorSchema() == true)
    {
        defaultId = "MONITORSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = Db::File::MvsFileExtension;
    }

	if (schema->isTuningSchema() == true)
	{
		defaultId = "TUNINGSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = Db::File::TvsFileExtension;
	}

    if (schema->isDiagSchema() == true)
    {
        defaultId = "DIAGSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = Db::File::DvsFileExtension;
    }

	Q_ASSERT(extension.isEmpty() == false);

    schema->setSchemaId(defaultId);

    // Set Caption
    //
    schema->setCaption("Caption "  + QString::number(sequenceNo).rightJustified(6, '0'));

    // Set default EqupmnetIDs for LogicSchema
    //
    if (dynamic_cast<VFrame30::LogicSchema*>(schema.get()) != nullptr)
    {
        VFrame30::LogicSchema* logicSchema = dynamic_cast<VFrame30::LogicSchema*>(schema.get());
        logicSchema->setEquipmentIds("SYSTEMID_RACKID_CH01_MD00");
    }

    // Set Width and Height
    //
    if (schema->unit() == VFrame30::SchemaUnit::Display)
    {
        schema->setDocWidth(1280);
        schema->setDocHeight(1024);
    }
    else
    {
        // A3 Landscape
        //
        if (schema->isUfbSchema() == true)
        {
            schema->setDocWidth(297.0 / 25.4);
            schema->setDocHeight(210.0 / 25.4);
        }
        else
        {
            schema->setDocWidth(420.0 / 25.4);
            schema->setDocHeight(297.0 / 25.4);
        }
    }


	CreateSchemaDialog propertiesDialog(schema, db(), this);

	if (propertiesDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	addSchemaFile(schema, extension, parentFile.fileId());

    return;
}

// Find the QModelIndex for FileID, and call addSchemaFileToDb
//
void SchemaControlTabPageEx::addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, int parentFileId)
{
	QModelIndex parentIndex;
	QModelIndexList matched = m_filesView->filesModel().match(m_filesView->filesModel().index(0, 0),
															  Qt::UserRole,
															  QVariant::fromValue(parentFileId),
															  1,
															  Qt::MatchExactly | Qt::MatchRecursive);

	if (matched.size() != 1)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot find parent item for new file."));
		return;
	}

	parentIndex = matched.front();

	addSchemaFileToDb(schema, fileExtension, parentIndex);

	return;
}

// Add file to DB
//
void SchemaControlTabPageEx::addSchemaFileToDb(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, QModelIndex parentIndex)
{
	if (schema == nullptr)
	{
		Q_ASSERT(schema);
		return;
	}

	//  Save file in DB
	//
	if (fileExtension.isEmpty() == false &&
		fileExtension.startsWith('.') == false)
	{
		fileExtension = '.' + fileExtension;
	}

	QByteArray data;
	schema->saveToByteArray(&data);

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

	file->setFileName(schema->schemaId() + fileExtension);
	file->setDetails(schema->details());
	file->swapData(data);

	int parentFileId = -1;

	if (parentIndex.isValid() == false)
	{
		parentFileId = parentFile().fileId();
	}
	else
	{
		parentFileId = static_cast<int>(parentIndex.internalId());
	}

	if (bool ok = db()->addUniqueFile(file, parentFileId, db()->schemaFileId(), this);
		ok == false)
	{
		return;
	}

	// Add file to the FileModel and select it
	//
	if (file->fileId() != -1)
	{
		// Clear file data, we don't need it anymore, if file will be added to the model with data it will just waste memory
		//
		file->clearData();

		m_filesView->selectionModel()->clear();
		auto [addedModelIndex, addResult] = m_filesView->filesModel().addFile(parentIndex, file);

		if (addResult == true)
		{
			QModelIndex addedProxyIndex = m_filesView->proxyModel().mapFromSource(addedModelIndex);
			QModelIndex parentProxyIndex = addedProxyIndex.parent();

			if (m_filesView->isExpanded(parentProxyIndex) == false)
			{
				m_filesView->expand(parentProxyIndex);
			}

			m_filesView->scrollTo(addedProxyIndex);
			m_filesView->selectionModel()->setCurrentIndex(addedProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);	//
		}
	}

	return;
}

void SchemaControlTabPageEx::addFolder()
{
	// Folder can be created only for another folder
	//
	QModelIndexList selectedRows = m_filesView->selectionModel()->selectedRows();
	if (selectedRows.size() != 1)
	{
		Q_ASSERT(selectedRows.size() == 1);
		return;
	}

	QModelIndex selectedModelIndex =  m_filesView->proxyModel().mapToSource(selectedRows.front());
	DbFileInfo selectedFile = m_filesView->filesModel().file(selectedModelIndex);

	DbFileInfo parentFile;

	// If folder selected, then create new folder in the selected one
	//
	if (selectedFile.directoryAttribute() == true)
	{
		parentFile = selectedFile;
	}
	else
	{
		// If file is selected, the create new folder in the same folder as selected file
		//
		parentFile = m_filesView->filesModel().file(selectedFile.parentId());
	}

	if (parentFile.isNull() == true ||
		parentFile.directoryAttribute() == false)
	{
		Q_ASSERT(parentFile.isNull() == false);
		Q_ASSERT(parentFile.directoryAttribute());
		return;
	}

	// Get folder name
	//
	int sequenceNo = db()->nextCounterValue();
	QString folderName = "FOLDER" + QString::number(sequenceNo).rightJustified(6, '0');

	do
	{
		bool ok = false;
		folderName = QInputDialog::getText(this, tr("Add Folder"), tr("Folder name:"), QLineEdit::Normal, folderName, &ok);

		if (ok == false)
		{
			return;
		}

	} while (folderName.isEmpty() == true);

	// Get ParentModelIndex
	//
	QModelIndex parentIndex;
	QModelIndexList matched = m_filesView->filesModel().match(m_filesView->filesModel().index(0, 0),
															  Qt::UserRole,
															  QVariant::fromValue(parentFile.fileId()),
															  1,
															  Qt::MatchExactly | Qt::MatchRecursive);

	if (matched.size() != 1)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot find parent item for new file."));
		return;
	}

	parentIndex = matched.front();

	// Add folder file to DB and to model
	//
	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

	file->setFileName(folderName);
	file->setDetails("{}");
	file->setDirectoryAttribute(true);
	file->clearData();

	if (bool ok = db()->addFile(file, parentFile.fileId(), this);		// File may not be unique
		ok == false)
	{
		return;
	}

	// Add file to the FileModel and select it
	//
	if (file->isNull() == false)
	{
		m_filesView->selectionModel()->clear();
		auto [addedModelIndex, addResult] = m_filesView->filesModel().addFile(parentIndex, file);

		if (addResult == true)
		{
			QModelIndex addedProxyIndex = m_filesView->proxyModel().mapFromSource(addedModelIndex);
			QModelIndex parentProxyIndex = addedProxyIndex.parent();

			if (m_filesView->isExpanded(parentProxyIndex) == false)
			{
				m_filesView->expand(parentProxyIndex);
			}

			m_filesView->scrollTo(addedProxyIndex);
			m_filesView->selectionModel()->setCurrentIndex(addedProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);	//
		}
	}

	return;
}

void SchemaControlTabPageEx::cloneFile()
{
	auto selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.size() != 1)
	{
		Q_ASSERT(selectedFiles.size() == 1);
		return;
	}

	DbFileInfo fileToClone = *(selectedFiles.front());
	if (fileToClone.fileId() == -1)
	{
		Q_ASSERT(fileToClone.fileId() != -1);
		return;
	}

	// Get file from the DB
	//
	std::shared_ptr<DbFile> out;

	bool result = db()->getLatestVersion(fileToClone, &out, this);
	if (result == false || out == nullptr)
	{
		return;
	}

	// Load file
	//
	std::shared_ptr<VFrame30::Schema> schema(VFrame30::Schema::Create(out->data()));
	if (schema == nullptr)
	{
		Q_ASSERT(schema != nullptr);
		return;
	}

	// Get new SchemaID
	//
	bool ok = false;
	int globalCounter = db()->nextCounterValue();
	QString newSchemaId = QInputDialog::getText(this, qAppName(), tr("New SchemaID <b>(cannot be changed later)</b>:"),
												QLineEdit::Normal,
												schema->schemaId() + QString::number(globalCounter), &ok,
												Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	if (ok == false || newSchemaId.isEmpty() == true)
	{
		return;
	}

	// Set new lables and guids
	//
	schema->setSchemaId(newSchemaId);
	schema->setGuid(QUuid::createUuid());

	std::vector<QUuid> oldGuids = schema->getGuids();
	std::set<QUuid> oldGuidsMap = {oldGuids.begin(), oldGuids.end()};

	for (std::shared_ptr<VFrame30::SchemaLayer> layer : schema->Layers)
	{
		layer->setGuid(QUuid::createUuid());

		for (std::shared_ptr<VFrame30::SchemaItem> item : layer->Items)
		{
			item->setNewGuid();

			if (item->isFblItemRect() == true)
			{
				globalCounter = db()->nextCounterValue();
				item->toFblItemRect()->setLabel(schema->schemaId() + "_" + QString::number(globalCounter));
			}
		}
	}

	// Check if all guids were updated
	//
	std::vector<QUuid> newGuids = schema->getGuids();
	for (const QUuid& guid : newGuids)
	{
		size_t c = oldGuidsMap.count(guid);

		if (c != 0)
		{
			Q_ASSERT(c == 0);
			QMessageBox::critical(this, qAppName(), tr("Cannot clone schema, not all GUIDs were updated. Please, inform developers about this problem."));
			return;
		}

	}

	// Get folder for clonned schema
	//
	int parentFileId = showSelectFolderDialog(dbc()->schemaFileId(), fileToClone.parentId(), false);
	if (parentFileId == -1)
	{
		return;
	}

	addSchemaFile(schema, fileToClone.extension(), parentFileId);

	return;
}

void SchemaControlTabPageEx::deleteFiles()
{
	QModelIndexList	selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi: selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> files = m_filesView->selectedFiles();

	if (files.empty() == true)
	{
		Q_ASSERT(files.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == files.size());

	// --
	//
	std::vector<std::shared_ptr<DbFileInfo>> deleteFiles;
	deleteFiles.reserve(files.size());

	for(const std::shared_ptr<DbFileInfo>& f : files)
	{
		if (dbc()->isSystemFile(f->fileId()) == true)
		{
			continue;
		}

		deleteFiles.push_back(f);
	}

	// Ask user to confirm operation
	//
	QMessageBox mb(this);

	mb.setWindowTitle(qApp->applicationName());
	mb.setText(tr("Are you sure you want to delete selected %1 file(s)").arg(deleteFiles.size()));
	mb.setInformativeText(tr("If files have not been checked in before they will be deleted permanently.\nIf files were checked in at least one time they will be marked as deleted, to confirm operation perform Check In."));

	QString detailedText;
	for(auto f : deleteFiles)
	{
		detailedText += f->fileName() + "\n";
	}
	mb.setDetailedText(detailedText.trimmed());

	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (int mbResult = mb.exec();
		mbResult == QMessageBox::Cancel)
	{
		return;
	}

	// --
	//
	bool ok = db()->deleteFiles(&deleteFiles, this);
	if (ok == false)
	{
		return;
	}

	ok = m_filesView->filesModel().deleteFilesUpdate(selectedIndexes, deleteFiles);
	if (ok == false)
	{
		return;
	}

	// Update open tab pages
	//
	for (auto editWidget : m_openedFiles)
	{
		Q_ASSERT(editWidget);

		for (std::shared_ptr<DbFileInfo> fi : deleteFiles)
		{
			if (editWidget->fileInfo().fileId() == fi->fileId() && editWidget->readOnly() == false)
			{
				editWidget->setReadOnly(true);
				editWidget->setFileInfo(*(fi.get()));
				editWidget->setPageTitle();
				break;
			}
		}
	}

	return;
}

void SchemaControlTabPageEx::moveFiles()
{
	QModelIndexList	selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi: selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> files = m_filesView->selectedFiles();

	if (files.empty() == true)
	{
		Q_ASSERT(files.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == files.size());

	// If schema is opened, can't move it
	//
	for (const auto& file : files)
	{
		auto foundTab = std::find_if(m_openedFiles.begin(), m_openedFiles.end(),
					[&file](const EditSchemaTabPageEx* tabPage)
					{
						Q_ASSERT(tabPage);
						return	tabPage->fileInfo().fileId() == file->fileId() &&
								tabPage->readOnly() == false;
					});

		if (foundTab != m_openedFiles.end())
		{
			EditSchemaTabPageEx* tab = *foundTab;
			QMessageBox::critical(this, qAppName(), tr("Can't move schema %1, as it is opened for edit. Close schema and repeat operation.").arg(tab->schema()->schemaId()));
			return;
		}
	}

	// --
	//
	std::vector<DbFileInfo> filesToMove;
	filesToMove.reserve(files.size());

	for(const std::shared_ptr<DbFileInfo>& f : files)
	{
		if (dbc()->isSystemFile(f->fileId()) == true ||
			f->state() != VcsState::CheckedOut)
		{
			continue;
		}

		filesToMove.push_back(*f);
	}

	if (filesToMove.empty() == true)
	{
		Q_ASSERT(filesToMove.empty() == false);
		return;
	}

	// Get destination folder
	//
	int moveToFileId = showSelectFolderDialog(dbc()->schemaFileId(), filesToMove.front().parentId(), false);
	if (moveToFileId == -1)
	{
		return;
	}

	// Move files in DB
	//
	std::vector<DbFileInfo> movedFiles;

	if (bool ok = db()->moveFiles(filesToMove, moveToFileId, &movedFiles, this);
		ok == false)
	{
		return;
	}

	// Update model/view
	//
	std::vector<QModelIndex> addedIndexes;
	addedIndexes.reserve(selectedIndexes.size());

	if (bool ok = m_filesView->filesModel().moveFilesUpdate(selectedIndexes, moveToFileId, movedFiles, &addedIndexes);
		ok == false)
	{
		return;
	}

	// Expand parent
	//
	QModelIndexList matched = m_filesView->filesModel().match(m_filesView->filesModel().index(0, 0),
															  Qt::UserRole,
															  QVariant::fromValue(moveToFileId),
															  1,
															  Qt::MatchExactly | Qt::MatchRecursive);
	Q_ASSERT(matched.size() == 1);

	if (matched.size() == 1)
	{
		QModelIndex fileModelIndex = matched.front();
		QModelIndex mappedModelIndex = m_filesView->proxyModel().mapFromSource(fileModelIndex);

		QModelIndex expandParent = mappedModelIndex;
		while (expandParent.isValid() == true)
		{
			m_filesView->expand(expandParent);
			expandParent = expandParent.parent();
		}
	}


	// Select moved files
	//
	QItemSelectionModel* selectionModel = m_filesView->selectionModel();
	Q_ASSERT(selectionModel);

	selectionModel->reset();

	for (const QModelIndex& mi : addedIndexes)
	{
		QModelIndex mappedToProxy = m_filesView->proxyModel().mapFromSource(mi);

		selectionModel->select(mappedToProxy, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	return;
}

void SchemaControlTabPageEx::checkOutFiles()
{
	QModelIndexList	selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi: selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> files = m_filesView->selectedFiles();
	if (files.empty() == true)
	{
		Q_ASSERT(files.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == files.size());

	// --
	//
	std::vector<DbFileInfo> checkOutFiles;
	checkOutFiles.reserve(files.size());

	for(const std::shared_ptr<DbFileInfo>& f : files)
	{
		if (dbc()->isSystemFile(f->fileId()) == true)
		{
			continue;
		}

		if (f->state() == VcsState::CheckedIn)
		{
			checkOutFiles.emplace_back(*f);
		}
	}

	if (checkOutFiles.empty() == true)
	{
		return;
	}

	bool ok = db()->checkOut(checkOutFiles, this);
	if (ok == false)
	{
		return;
	}

	ok = m_filesView->filesModel().updateFiles(selectedIndexes, checkOutFiles);
	if (ok == false)
	{
		return;
	}

	m_filesView->selectionChanged({}, {});		// To update actions

	return;
}

void SchemaControlTabPageEx::checkInFiles()
{
	QModelIndexList	selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi: selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.empty() == true)
	{
		Q_ASSERT(selectedFiles.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == selectedFiles.size());

	// --
	//
	std::vector<DbFileInfo> checkInFiles;
	checkInFiles.reserve(selectedFiles.size());

	for(const std::shared_ptr<DbFileInfo>& file : selectedFiles)
	{
		if (dbc()->isSystemFile(file->fileId()) == true)
		{
			continue;
		}

		if (file->state() == VcsState::CheckedIn)
		{
			continue;
		}

		if (file->userId() == db()->currentUser().userId() ||
			db()->currentUser().isAdminstrator() == true)
		{
			checkInFiles.push_back(*file);
		}
	}

	if (checkInFiles.empty() == true)
	{
		return;
	}

	// Save file if it is open
	//
	for (auto editWidget : m_openedFiles)
	{
		if (editWidget == nullptr)
		{
			Q_ASSERT(editWidget);
			continue;
		}

		if (editWidget->readOnly() == true || editWidget->modified() == false)
		{
			continue;
		}

		auto it = std::find_if(checkInFiles.begin(), checkInFiles.end(),
							   [&editWidget](const DbFileInfo& fi)
							   {
									return fi.fileId() == editWidget->fileInfo().fileId();
							   });

		if (it != checkInFiles.end())
		{
			editWidget->saveWorkcopy();
		}
	}

	// Check in file
	//
	std::vector<DbFileInfo> updatedFiles;
	bool ok = CheckInDialog::checkIn(checkInFiles, false, &updatedFiles, db(), this);
	if (ok == false)
	{
		return;
	}

	m_filesView->refreshFiles();

	// Refresh fileInfo from the Db
	//
	std::vector<int> fileIds;
	fileIds.reserve(checkInFiles.size());

	for (const DbFileInfo& fi : checkInFiles)
	{
		fileIds.push_back(fi.fileId());
	}

	db()->getFileInfo(&fileIds, &checkInFiles, this);

	// Remove deleted files
	//
	checkInFiles.erase(std::remove_if(checkInFiles.begin(), checkInFiles.end(), [](const auto& file) { return file.deleted();}),
					   checkInFiles.end());

	// Set readonly to file if it is open
	//
	for (auto editWidget : m_openedFiles)
	{
		if (editWidget == nullptr)
		{
			Q_ASSERT(editWidget);
			continue;
		}

		for (const DbFileInfo& fi : checkInFiles)
		{
			if (editWidget->fileInfo().fileId() == fi.fileId() && editWidget->readOnly() == false)
			{
				editWidget->setReadOnly(true);
				editWidget->setFileInfo(fi);
				break;
			}
		}
	}

	m_filesView->selectionChanged({}, {});		// To update actions

	return;
}

void SchemaControlTabPageEx::undoChangesFiles()
{
	// 1 Ask user to confirm operation
	// 2 Undo changes to database
	// 3 Set frame to readonly mode
	//
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();
	std::vector<DbFileInfo> undoFiles;
	undoFiles.reserve(selectedFiles.size());

	for (const std::shared_ptr<DbFileInfo>& fi : selectedFiles)
	{
		if (fi->state() == VcsState::CheckedOut &&
			(fi->userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator() == true))
		{
			undoFiles.push_back(*fi);
		}
	}

	if (undoFiles.empty() == true)
	{
		// Nothing to undo
		//
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("This operation will undo all pending changes for the document and will revert it to the prior state!"));
	mb.setInformativeText(tr("Do you want to undo pending changes?"));
	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (mb.exec() != QMessageBox::Ok)
	{
		return;
	}

	// Undo changes in DB
	//
	db()->undoChanges(undoFiles, this);

	// Update open tab pages
	//
	for (auto editWidget : m_openedFiles)
	{
		Q_ASSERT(editWidget);

		for (const DbFileInfo& fi : undoFiles)
		{
			if (editWidget->fileInfo().fileId() == fi.fileId() &&
				editWidget->readOnly() == false)
			{
				editWidget->setReadOnly(true);
				editWidget->setFileInfo(fi);
				editWidget->setPageTitle();
				break;
			}
		}
	}

	m_filesView->refreshFiles();
	return;
}

void SchemaControlTabPageEx::showFileHistory()
{
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.size() != 1)
	{
		return;
	}

	// Get file history
	//
	const DbFileInfo& file = *(selectedFiles.front());
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistory(file, &fileHistory, this);
	if (ok == false)
	{
		return;
	}

	// Show history dialog
	//
	FileHistoryDialog::showHistory(db(), file.fileName(), fileHistory, this);
	return;
}

void SchemaControlTabPageEx::showFileHistoryRecursive()
{
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.size() != 1)
	{
		return;
	}

	// Get file history
	//
	const DbFileInfo& file = *(selectedFiles.front());
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistoryRecursive(file, &fileHistory, this);
	if (ok == false)
	{
		return;
	}

	// Show history dialog
	//
	FileHistoryDialog::showHistory(db(), file.fileName(), fileHistory, this);
	return;
}

void SchemaControlTabPageEx::compareSelectedFile()
{
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.size() != 1)
	{
		return;
	}

	// --
	//
	const DbFileInfo& file = *(selectedFiles.front());

	CompareDialog::showCompare(db(), DbChangesetObject(file), -1, this);

	return;
}

void SchemaControlTabPageEx::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension,
	// can compare next files
	//
	if (object.name().endsWith("." + QString(Db::File::AlFileExtension)) == false &&
		object.name().endsWith("." + QString(Db::File::AlTemplExtension)) == false &&
		object.name().endsWith("." + QString(Db::File::UfbFileExtension)) == false &&
		object.name().endsWith("." + QString(Db::File::UfbTemplExtension)) == false &&
		object.name().endsWith("." + QString(Db::File::MvsFileExtension)) == false &&
		object.name().endsWith("." + QString(Db::File::MvsTemplExtension)) == false &&
		object.name().endsWith("." + QString(Db::File::DvsFileExtension)) == false &&
		object.name().endsWith("." + QString(Db::File::DvsTemplExtension)) == false)
	{
		return;
	}

	// Get versions from the project database
	//
	std::shared_ptr<VFrame30::Schema> source = nullptr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceChangeset, &outFile, this);
			if (ok == true)
			{
				source = VFrame30::Schema::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceDate, &outFile, this);
			if (ok == true)
			{
				source = VFrame30::Schema::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				source = VFrame30::Schema::Create(outFile->data());
			}
		}
		break;
	default:
		Q_ASSERT(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	std::shared_ptr<VFrame30::Schema> target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetChangeset, &outFile, this);
			if (ok == true)
			{
				target = VFrame30::Schema::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetDate, &outFile, this);
			if (ok == true)
			{
				target = VFrame30::Schema::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				target = VFrame30::Schema::Create(outFile->data());
			}
		}
		break;
	default:
		Q_ASSERT(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Make single schema
	//
	std::map<QUuid, CompareAction> itemsActions;

	for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : target->Layers)
	{
		for (std::shared_ptr<VFrame30::SchemaItem> targetItem : targetLayer->Items)
		{
			// Look for this item in source
			//
			std::shared_ptr<VFrame30::SchemaItem> sourceItem = source->getItemById(targetItem->guid());

			if (sourceItem != nullptr)
			{
				// Item is found, so it was modified
				//

				// Check if properties where modified
				//
				QString sourceStr = ComparePropertyObjectDialog::objedctToCompareString(sourceItem.get());
				QString targetStr = ComparePropertyObjectDialog::objedctToCompareString(targetItem.get());

				if (sourceStr == targetStr)
				{
					// Check if position was changed
					//
					std::vector<VFrame30::SchemaPoint> sourcePoints = sourceItem->getPointList();
					std::vector<VFrame30::SchemaPoint> targetPoints = targetItem->getPointList();

					if (sourcePoints == targetPoints)
					{
						itemsActions[targetItem->guid()] = CompareAction::Unmodified;
					}
					else
					{
						itemsActions[targetItem->guid()] = CompareAction::Modified;
					}
				}
				else
				{
					itemsActions[targetItem->guid()] = CompareAction::Modified;
				}

				continue;
			}

			if (sourceItem == nullptr)
			{
				// Item was added to targer
				//
				itemsActions[targetItem->guid()] = CompareAction::Added;
				continue;
			}
		}
	}

	// Look for deteled items (in target)
	//
	for (std::shared_ptr<VFrame30::SchemaLayer> sourceLayer : source->Layers)
	{
		for (std::shared_ptr<VFrame30::SchemaItem> sourceItem : sourceLayer->Items)
		{
			// Look for this item in source
			//
			std::shared_ptr<VFrame30::SchemaItem> targetItem = target->getItemById(sourceItem->guid());

			if (targetItem == nullptr)
			{
				// Item is found, so it was deleted in target
				//
				itemsActions[sourceItem->guid()] = CompareAction::Deleted;

				// Add item to target
				//
				bool layerFound = false;
				for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : target->Layers)
				{
					if (targetLayer->guid() == sourceLayer->guid())
					{
						targetLayer->Items.push_back(sourceItem);
						layerFound = true;
						break;
					}
				}

				Q_ASSERT(layerFound);
			}
		}
	}

	// Create tab page and add it to TabWidget
	//
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget != nullptr);
		return;
	}

	EditSchemaTabPageEx* compareTabPage = new EditSchemaTabPageEx(tabWidget, target, DbFileInfo(), db());

	connect(compareTabPage, &EditSchemaTabPageEx::aboutToClose, this, &SchemaControlTabPageEx::removeFromOpenedList);
	connect(compareTabPage, &EditSchemaTabPageEx::pleaseDetachOrAttachWindow, this, &SchemaControlTabPageEx::detachOrAttachWindow);

	compareTabPage->setReadOnly(true);
	compareTabPage->setCompareWidget(true, source, target);
	compareTabPage->setCompareItemActions(itemsActions);

	compareTabPage->setWindowTitle("Compare " + target->schemaId());

	tabWidget->addTab(compareTabPage, compareTabPage->windowTitle());
	tabWidget->setCurrentWidget(compareTabPage);

	m_openedFiles.push_back(compareTabPage);

	return;
}

void SchemaControlTabPageEx::exportWorkcopy()
{
	// Get files workcopies form the database
	//
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (auto file : selectedFiles)
	{
		if (file->state() == VcsState::CheckedOut &&
			file->userId() == db()->currentUser().userId())
		{
			files.push_back(*file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	// Select destination folder
	//
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty() == true)
	{
		return;
	}

	// Get files from the database
	//
	std::vector<std::shared_ptr<DbFile>> out;
	db()->getWorkcopy(files, &out, this);

	// Save files to disk
	//
	for (unsigned int i = 0; i < out.size(); i++)
	{
		bool writeResult = out[i]->writeToDisk(dir);

		if (writeResult == false)
		{
			QMessageBox msgBox;
			msgBox.setText(tr("Write file error."));
			msgBox.setInformativeText(tr("Cannot write file %1.").arg(out[i]->fileName()));
			msgBox.exec();
		}
	}

	return;
}

void SchemaControlTabPageEx::importWorkcopy()
{
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file->state() == VcsState::CheckedOut &&
			file->userId() == db()->currentUser().userId())
		{
			files.push_back(*file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	// --
	//
	if (files.size() != 1)
	{
		return;
	}

	auto fileInfo = files[0];

	if (fileInfo.state() != VcsState::CheckedOut || fileInfo.userId() != db()->currentUser().userId())
	{
		return;
	}

	// Select file
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	static_cast<DbFileInfo*>(file.get())->operator=(fileInfo);

	bool readResult = file->readFromDisk(fileName);
	if (readResult == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Can't read file %1.").arg(fileName));
		mb.exec();
		return;
	}

	// Set file id for DbStore setWorkcopy
	//
	file->setFileId(fileInfo.fileId());

	std::vector<std::shared_ptr<DbFile>> workcopyFiles;
	workcopyFiles.push_back(file);

	db()->setWorkcopy(workcopyFiles, this);

	m_filesView->refreshFiles();
	return;
}

void SchemaControlTabPageEx::showFileProperties()
{
	std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();

	std::vector<DbFileInfo> requestFiles;
	requestFiles.reserve(selectedFiles.size());

	bool readOnly = true;

	for (const auto& file : selectedFiles)
	{
		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator() == true))
		{
			readOnly = false;
		}

		requestFiles.push_back(*file);
	}

	// If schema is opened, can't edit its' properties
	//
	for (const auto& file : selectedFiles)
	{
		auto foundTab = std::find_if(m_openedFiles.begin(), m_openedFiles.end(),
					[&file](const EditSchemaTabPageEx* tabPage)
					{
						Q_ASSERT(tabPage);
						return	tabPage->fileInfo().fileId() == file->fileId() &&
								tabPage->readOnly() == false;
					});

		if (foundTab != m_openedFiles.end())
		{
			EditSchemaTabPageEx* tab = *foundTab;
			QMessageBox::critical(this, qAppName(), tr("Can't edit %1 schema properties, as it is opened for edit. Close schema to edit it's properties.").arg(tab->schema()->schemaId()));
			return;
		}
	}

	// Load schemas
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool ok = db()->getLatestVersion(requestFiles, & out, this);
	if (ok == false)
	{
		return;
	}

	// Read schemas
	//
	std::vector<std::pair<std::shared_ptr<DbFile>, std::shared_ptr<VFrame30::Schema>>> schemas;
	schemas.reserve(out.size());

	QString initialSchemasId;

	for (std::shared_ptr<DbFile> file : out)
	{
		std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(file->data());
		if (schema == nullptr)
		{
			Q_ASSERT(schema != nullptr);
			return;
		}

		schemas.push_back({file, schema});

		initialSchemasId = schema->schemaId();		// Has sense if only one schema is selected
	}

	// Show schema properties dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("Schema(s) Properties"));
	d.setWindowFlags((d.windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

	ExtWidgets::PropertyEditor* propertyEditor = new ExtWidgets::PropertyEditor(this);
	propertyEditor->setReadOnly(readOnly);

	std::vector<std::shared_ptr<PropertyObject>> propertyObjects;
	propertyObjects.reserve(schemas.size());

	for (auto[schemaFile, schema] : schemas)
	{
		Q_ASSERT(schema != nullptr);

		propertyObjects.push_back(schema);

		// Now allow to edit SchemaID, only if one file is selected
		//
		std::shared_ptr<Property> schemaIdProp = schema->propertyByCaption("SchemaID");
		if (schemaIdProp == nullptr)
		{
			Q_ASSERT(schemaIdProp != nullptr);
			continue;
		}

		if (schemas.size() == 1)
		{
			schemaIdProp->setReadOnly(false);
		}
		else
		{
			schemaIdProp->setReadOnly(true);
		}
	}

	propertyEditor->setObjects(propertyObjects);
	propertyEditor->resizeColumnToContents(0);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(propertyEditor);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	d.resize(d.sizeHint() * 1.5);

	// Show proprties dialog
	// and save result on accept
	//
	if (int result = d.exec();
		result == QDialog::Accepted)
	{
		std::vector<std::shared_ptr<DbFile>> filesToSave;
		filesToSave.reserve(schemas.size());

		for (auto [file, schema]: schemas)
		{
			if (file->state() != VcsState::CheckedOut ||
				(file->userId() != db()->currentUser().userId() && db()->currentUser().isAdminstrator() == false))
			{
				continue;
			}

			QByteArray data;
			schema->saveToByteArray(&data);

			if (data.isEmpty() == true)
			{
				Q_ASSERT(data.isEmpty() == false);
				return;
			}

			// --
			//
			file->swapData(data);
			file->setDetails(schema->details());

			filesToSave.push_back(file);
		}

		// Check if SchemaID was changed and we need to rename file
		//
		if (schemas.size() == 1)
		{
			auto file = schemas.front().first;
			auto schema = schemas.front().second;

			if (schema->schemaId() != initialSchemasId)
			{
				// File must be renamed to new name
				//
				QString newFileName = schema->schemaId() + "." + file->extension();

				if (ok = db()->renameFile(*file, newFileName, file.get(), this);
					ok == false)
				{
					// Don't save file if it was not renamed, as it will lead that filename differs from SchemaID
					// Just return
					//
					return;
				}

				// variable file has spoiled 'details' while db()->renameFile (it returns new DbFileInfo into file)
				// so we need to update details again!!!
				// and it will be written to DB later (db()->setWorkcopy(filesToSave, this);)
				//
				file->setDetails(schema->details());
			}
		}

		if (filesToSave.empty() == false)
		{
			db()->setWorkcopy(filesToSave, this);
			m_filesView->refreshFiles();
		}
	}

	return;
}

void SchemaControlTabPageEx::ctrlF()
{
	Q_ASSERT(m_searchEdit);

	m_searchEdit->setFocus();
	m_searchEdit->selectAll();

	return;
}

void SchemaControlTabPageEx::search()
{
	// Search for text in schemas
	//
	Q_ASSERT(m_filesView);
	Q_ASSERT(m_searchEdit);

	QString searchText = m_searchEdit->text().trimmed();

	if (searchText.isEmpty() == true)
	{
		m_filesView->clearSelection();
		return;
	}

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("SchemaControlTabPageEx/SearchCompleter").toStringList();

	if (completerStringList.contains(searchText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(searchText);
		QSettings{}.setValue("SchemaControlTabPageEx/SearchCompleter", completerStringList);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_searchCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(completerStringList);
		}
	}

	// Search for text and select schemas with it
	//
	m_filesView->searchAndSelect(searchText);

	m_filesView->setFocus();

	return;
}

void SchemaControlTabPageEx::searchSchemaForLm(QString equipmentId)
{
	// Set focus to LogicSchemaTabPage and to ControlTabPage
	//
	QTabWidget* parentTabWidget = dynamic_cast<QTabWidget*>(this->parentWidget()->parentWidget());
	if (parentTabWidget == nullptr)
	{
		Q_ASSERT(parentTabWidget);
	}
	else
	{
		parentTabWidget->setCurrentWidget(this);
	}

	GlobalMessanger::instance().fireChangeCurrentTab(this->parentWidget()->parentWidget()->parentWidget());

	m_filesView->setFocus();

	// Set Search string and perform search
	//
	m_searchEdit->setText(equipmentId.trimmed());
	search();

	return;
}

void SchemaControlTabPageEx::filter()
{
	// Search for text in schemas
	//
	Q_ASSERT(m_filesView);
	Q_ASSERT(m_filterEdit);

	QString filterText = m_filterEdit->text().trimmed();

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("SchemaControlTabPageEx/SearchCompleter").toStringList();

	if (completerStringList.contains(filterText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(filterText);
		QSettings{}.setValue("SchemaControlTabPageEx/SearchCompleter", completerStringList);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_searchCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(completerStringList);
		}
	}

	// Search for text and select schemas with it
	//
	m_filesView->setFilter(filterText);
	m_filesView->setFocus();

	int schemaFiletrCount = m_filesView->filesModel().schemaFilterCount();

	if (filterText.trimmed().isEmpty() == false)
	{
		m_filterButton->setText(tr("Filter: %1 found").arg(schemaFiletrCount));

		QFont font = m_filterButton->font();
		font.setBold(true);
		m_filterButton->setFont(font);
	}
	else
	{
		m_filterButton->setText(tr("Filter"));

		QFont font = m_filterButton->font();
		font.setBold(false);
		m_filterButton->setFont(font);
	}

	m_resetFilterButton->setDisabled(filterText.trimmed().isEmpty());

	return;
}

void SchemaControlTabPageEx::resetFilter()
{
	Q_ASSERT(m_filesView);

	m_filterEdit->clear();

	m_filesView->setFilter("");
	m_filesView->setFocus();

	m_filterButton->setText(tr("Filter"));
	QFont font = m_filterButton->font();
	font.setBold(false);
	m_filterButton->setFont(font);

	m_resetFilterButton->setDisabled(true);

	return;
}

void SchemaControlTabPageEx::schemaTagsChanged()
{
	const std::set<QString>& tags = m_filesView->filesModel().tags();
	m_tagSelector->setTags(tags);

	// Selected tags could be removed so tag filter could be changed
	//
	if (m_tagSelector->selectedTags() != m_filesView->filesModel().tagFilter())
	{
		tagSelectorHasChanges();
	}

	return;
}

void SchemaControlTabPageEx::tagSelectorHasChanges()
{
	// Filter schemas by tags
	//
	QStringList selectedTags = m_tagSelector->selectedTags();

	m_filesView->setTagFilter(selectedTags);
	m_filesView->setFocus();

	return;
}

const DbFileInfo& SchemaControlTabPageEx::parentFile() const
{
	return m_filesView->parentFile();
}


//
//
// EditSchemaTabPage
//
//
EditSchemaTabPageEx::EditSchemaTabPageEx(QTabWidget* tabWidget,
										 std::shared_ptr<VFrame30::Schema> schema,
										 const DbFileInfo& fileInfo,
										 DbController* dbcontroller) :
	QMainWindow(nullptr, Qt::WindowType::Widget),	// Always created as widget as from start it's attached to TabWidget, later can be switcher to Qt::Window
	HasDbController(dbcontroller),
	m_schemaWidget(nullptr),
	m_tabWidget(tabWidget)
{
	Q_ASSERT(m_tabWidget);
	Q_ASSERT(schema.get() != nullptr);

	setWindowTitle(schema->schemaId());

	// Create controls
	//
	schema->setChangeset(fileInfo.changeset());

	m_schemaWidget = new EditSchemaWidget(schema, fileInfo, dbcontroller);

	connect(m_schemaWidget, &EditSchemaWidget::detachOrAttachWindow, this, &EditSchemaTabPageEx::detachOrAttachWindow);
	connect(m_schemaWidget, &EditSchemaWidget::closeTab, this, &EditSchemaTabPageEx::closeTab);
	connect(m_schemaWidget, &EditSchemaWidget::modifiedChanged, this, &EditSchemaTabPageEx::modifiedChanged);
	connect(m_schemaWidget, &EditSchemaWidget::saveWorkcopy, this, &EditSchemaTabPageEx::saveWorkcopy);
	connect(m_schemaWidget, &EditSchemaWidget::checkInFile, this, &EditSchemaTabPageEx::checkInFile);
	connect(m_schemaWidget, &EditSchemaWidget::checkOutFile, this, &EditSchemaTabPageEx::checkOutFile);
	connect(m_schemaWidget, &EditSchemaWidget::undoChangesFile, this, &EditSchemaTabPageEx::undoChangesFile);
	connect(m_schemaWidget, &EditSchemaWidget::getCurrentWorkcopy, this, &EditSchemaTabPageEx::getCurrentWorkcopy);
	connect(m_schemaWidget, &EditSchemaWidget::setCurrentWorkcopy, this, &EditSchemaTabPageEx::setCurrentWorkcopy);

	// ToolBar
	//
	m_toolBar = new QToolBar(tr("Toolbar"), this);
	m_toolBar->setOrientation(Qt::Vertical);
	m_toolBar->setFloatable(false);
	m_toolBar->setMovable(false);
	m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

	m_toolBar->addAction(m_schemaWidget->m_fileAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_schemaWidget->m_addLineAction);
	m_toolBar->addAction(m_schemaWidget->m_addRectAction);
	m_toolBar->addAction(m_schemaWidget->m_addPathAction);
	m_toolBar->addAction(m_schemaWidget->m_addTextAction);
	m_toolBar->addAction(m_schemaWidget->m_addImageAction);
	//m_toolBar->addAction(m_schemaWidget->m_addFrameAction);

	if (schema->isLogicSchema() == true)
	{
		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaWidget->m_addLinkAction);
		m_toolBar->addAction(m_schemaWidget->m_addInputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addInOutSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addOutputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addConstantAction);
		m_toolBar->addAction(m_schemaWidget->m_addTerminatorAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorAfb);
		m_toolBar->addAction(m_schemaWidget->m_addAfbAction);
		m_toolBar->addAction(m_schemaWidget->m_addUfbAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorConn);
		m_toolBar->addAction(m_schemaWidget->m_addTransmitter);
		m_toolBar->addAction(m_schemaWidget->m_addReceiver);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorLoop);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackSource);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackTarget);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorBus);
		m_toolBar->addAction(m_schemaWidget->m_addBusComposer);
		m_toolBar->addAction(m_schemaWidget->m_addBusExtractor);
	}

	if (schema->isUfbSchema())
	{
		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaWidget->m_addLinkAction);
		m_toolBar->addAction(m_schemaWidget->m_addInputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addOutputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addConstantAction);
		m_toolBar->addAction(m_schemaWidget->m_addTerminatorAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorAfb);
		m_toolBar->addAction(m_schemaWidget->m_addAfbAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorLoop);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackSource);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackTarget);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorBus);
		m_toolBar->addAction(m_schemaWidget->m_addBusComposer);
		m_toolBar->addAction(m_schemaWidget->m_addBusExtractor);
	}

	if (schema->isMonitorSchema())
	{
		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaWidget->m_addValueAction);
		m_toolBar->addAction(m_schemaWidget->m_addImageValueAction);
		m_toolBar->addAction(m_schemaWidget->m_addPushButtonAction);
		m_toolBar->addAction(m_schemaWidget->m_addLineEditAction);
		m_toolBar->addAction(m_schemaWidget->m_addIndicatorAction);
	}

	if (schema->isTuningSchema())
	{
		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaWidget->m_addValueAction);
		m_toolBar->addAction(m_schemaWidget->m_addImageValueAction);
		m_toolBar->addAction(m_schemaWidget->m_addPushButtonAction);
		m_toolBar->addAction(m_schemaWidget->m_addLineEditAction);
	}

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_schemaWidget->m_orderAction);
	m_toolBar->addAction(m_schemaWidget->m_sizeAndPosAction);

	m_toolBar->addAction(m_schemaWidget->m_infoModeAction);

	// --
	//
	setCentralWidget(m_schemaWidget);
	addToolBar(Qt::ToolBarArea::LeftToolBarArea, m_toolBar);

	// --
	//
	connect(m_schemaWidget->m_fileAction, &QAction::triggered, this, &EditSchemaTabPageEx::fileMenuTriggered);
	connect(m_schemaWidget->m_orderAction, &QAction::triggered, this, &EditSchemaTabPageEx::itemsOrderTriggered);
	connect(m_schemaWidget->m_sizeAndPosAction, &QAction::triggered, this, &EditSchemaTabPageEx::sizeAndPosMenuTriggered);

	connect(m_tabWidget, &QTabWidget::currentChanged, m_schemaWidget, &EditSchemaWidget::hideWorkDialogs);

	connect(dbc(), &DbController::projectClosed, this, &EditSchemaTabPageEx::projectClosed);

	setPageTitle();

	return;
}

EditSchemaTabPageEx::~EditSchemaTabPageEx()
{
	qDebug() << Q_FUNC_INFO;
}

void EditSchemaTabPageEx::closeEvent(QCloseEvent* event)
{
	if (windowFlags() == Qt::WindowType::Widget)
	{
		// If windowFlags() == Qt::WindowType::Widget then it is attachet to TabWidget, and close is
		// processed in slot closeTab
		//
		event->accept();
		return;
	}

	// Else (windowFlags() == Qt::WindowType::Window)
	// This is free floating window, ask for saving result
	//
	if (m_schemaWidget->modified() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("The document has been modified."));
		mb.setInformativeText(tr("Do you want to save chages to %1?").arg(fileInfo().fileName()));
		mb.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Save);

		int result = mb.exec();

		switch (result)
		{
		case QMessageBox::Save:
			saveWorkcopy();
			break;
		case QMessageBox::Discard:
			break;
		case QMessageBox::Cancel:
			event->ignore();
			return;
		default:
			Q_ASSERT(false);
		}
	}

	// Find current tab and close it
	//
	emit aboutToClose(this);
	this->deleteLater();

	event->accept();
	return;
}

void EditSchemaTabPageEx::ensureVisible()
{
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect  = QApplication::desktop()->availableGeometry(this);
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMinimized() == true)
	{
		showNormal();
	}

	if (isMaximized() == false &&
		(intersectRect.width() < size().width() ||
		 intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false &&
		(frameGeometry().width() > screenRect.width() ||
		 frameGeometry().height() > screenRect.height()))
	{
		resize(static_cast<int>(screenRect.width() * 0.7),
			   static_cast<int>(screenRect.height() * 0.7));
	}

	return;
}

void EditSchemaTabPageEx::setPageTitle()
{
	QString newTitle;

	if (readOnly() == true || fileInfo().userId() != db()->currentUser().userId())
	{
		if (fileInfo().changeset() == -1 || fileInfo().changeset() == 0)
		{
			newTitle = QString("%1: ReadOnly").arg(m_schemaWidget->schema()->schemaId());
		}
		else
		{
			newTitle = QString("%1: %2 ReadOnly").arg(m_schemaWidget->schema()->schemaId()).arg(fileInfo().changeset());
		}

		if (fileInfo().deleted() == true)
		{
			newTitle += QString(", deleted");
		}
	}
	else
	{
		newTitle = m_schemaWidget->schema()->schemaId();
		if (modified() == true)
		{
			 newTitle += "*";
		}
	}

	setWindowTitle(newTitle);

	if (parentWidget() != nullptr)
	{
		if (QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
			tabWidget != nullptr)
		{
			for (int i = 0; i < tabWidget->count(); i++)
			{
				if (tabWidget->widget(i) == this)
				{
					tabWidget->setTabText(i, newTitle);
					return;
				}
			}
		}
	}

	return;
}

void EditSchemaTabPageEx::updateZoomAndScrolls(bool repaint)
{
	m_schemaWidget->setZoom(m_schemaWidget->zoom(), repaint);
	return;
}

void EditSchemaTabPageEx::updateAfbSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		Q_ASSERT(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateAfbsForSchema();

	return;
}

void EditSchemaTabPageEx::updateUfbSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		Q_ASSERT(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateUfbsForSchema();

	return;
}

void EditSchemaTabPageEx::updateBussesSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		Q_ASSERT(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateBussesForSchema();

	return;
}

void EditSchemaTabPageEx::detachOrAttachWindow()
{
	emit pleaseDetachOrAttachWindow(this);
}

void EditSchemaTabPageEx::projectClosed()
{
	// Find current tab and close it
	//
	emit aboutToClose(this);

	this->deleteLater();

	return;
}

void EditSchemaTabPageEx::closeTab()
{
	if (m_schemaWidget->modified() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("The document has been modified."));
		mb.setInformativeText(tr("Do you want to save chages to %1?").arg(fileInfo().fileName()));
		mb.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Save);

		int result = mb.exec();

		switch (result)
		{
		case QMessageBox::Save:
			saveWorkcopy();
			break;
		case QMessageBox::Discard:
			break;
		case QMessageBox::Cancel:
			return;
		default:
			Q_ASSERT(false);
		}
	}

	// Find current tab and close it
	//
	emit aboutToClose(this);

	this->deleteLater();
	return;
}

void EditSchemaTabPageEx::modifiedChanged(bool /*modified*/)
{
	setPageTitle();
}

void EditSchemaTabPageEx::checkInFile()
{
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		(fileInfo().userId() != db()->currentUser().userId() && db()->currentUser().isAdminstrator() == false))
	{
		return;
	}

	// Save workcopy and checkin
	//
	if (modified() == true)
	{
		bool saveResult = saveWorkcopy();

		if (saveResult == false)
		{
			return;
		}
	}

	std::vector<DbFileInfo> files;
	files.push_back(fileInfo());

	std::vector<DbFileInfo> updatedFiles;

	bool checkInResult = CheckInDialog::checkIn(files, false, &updatedFiles, db(), this);
	if (checkInResult == false)
	{
		return;
	}

	emit vcsFileStateChanged();

	DbFileInfo fi;
	db()->getFileInfo(fileInfo().fileId(), &fi, this);

	setFileInfo(fi);

	setReadOnly(true);

	setPageTitle();

	return;
}

void EditSchemaTabPageEx::checkOutFile()
{
	if (readOnly() == false ||
		fileInfo().state() != VcsState::CheckedIn)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.push_back(fileInfo());

	bool result = db()->checkOut(files, this);
	if (result == false)
	{
		return;
	}

	// Read the workcopy and load it to the current document
	//
	std::vector<std::shared_ptr<DbFile>> out;

	result = db()->getWorkcopy(files, &out, this);
	if (result == false || out.size() != files.size())
	{
		return;
	}

	m_schemaWidget->schema()->Load(out[0].get()->data());

	setFileInfo(*(out.front().get()));

	setReadOnly(false);
	setPageTitle();

	m_schemaWidget->resetAction();
	m_schemaWidget->clearSelection();

	m_schemaWidget->update();

	emit vcsFileStateChanged();
	return;
}

void EditSchemaTabPageEx::undoChangesFile()
{
	// 1 Ask user to confirm operation
	// 2 Undo changes to database
	// 3 Set frame to readonly mode
	//
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().userId() != db()->currentUser().userId())
	{
		Q_ASSERT(fileInfo().userId() == db()->currentUser().userId());
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("This operation will undo all pending changes for the document and will revert it to the prior state!"));
	mb.setInformativeText(tr("Do you want to undo pending changes?"));
	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (mb.exec() == QMessageBox::Ok)
	{
		DbFileInfo fi = fileInfo();

		bool result = db()->undoChanges(fi, this);

		if (result == true)
		{
			setFileInfo(fi);

			setReadOnly(true);
			setPageTitle();

			m_schemaWidget->resetAction();
			m_schemaWidget->clearSelection();

			m_schemaWidget->update();
		}
	}

	emit vcsFileStateChanged();
	return;
}

void EditSchemaTabPageEx::fileMenuTriggered()
{
	if (m_toolBar == nullptr)
	{
		Q_ASSERT(m_toolBar);
		return;
	}

	m_schemaWidget->updateFileActions();
	QWidget* w = m_toolBar->widgetForAction(m_schemaWidget->m_fileAction);

	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_fileMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EditSchemaTabPageEx::sizeAndPosMenuTriggered()
{
	if (m_toolBar == nullptr)
	{
		Q_ASSERT(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_schemaWidget->m_sizeAndPosAction);

	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_sizeAndPosMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EditSchemaTabPageEx::itemsOrderTriggered()
{
	if (m_toolBar == nullptr)
	{
		Q_ASSERT(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_schemaWidget->m_orderAction);

	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_orderMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

bool EditSchemaTabPageEx::saveWorkcopy()
{
	if (readOnly() == true ||
		modified() == false ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().userId() != db()->currentUser().userId())
	{
		Q_ASSERT(fileInfo().userId() == db()->currentUser().userId());
		return false;
	}

	QByteArray data;
	schema()->saveToByteArray(&data);

	if (data.isEmpty() == true)
	{
		Q_ASSERT(data.isEmpty() == false);
		return false;
	}

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	static_cast<DbFileInfo*>(file.get())->operator=(fileInfo());
	file->swapData(data);

	// Check if schemaId was changed, rename file if so
	//
	bool fileWasRenamed = false;

	if (schema()->schemaId() != m_schemaWidget->m_initialSchemaId)
	{
		QString newFileName = schema()->schemaId() + "." + file->extension();

		if (bool ok = db()->renameFile(*file, newFileName, file.get(), this);
			ok == false)
		{
			// Don't save file if it was not renamed, as it will lead that filename differs from SchemaID
			// Just return
			//
			return false;
		}

		fileWasRenamed = true;
	}

	file->setDetails(schema()->details());	// Details must be set here, as file rename will spoils them

	// Save workcopy
	//
	if (bool result = db()->setWorkcopy(file, this);
		result == false)
	{
		return false;
	}

	resetModified();

	if (fileWasRenamed == true)
	{
		setPageTitle();
		emit vcsFileStateChanged();
	}

	return true;
}

void EditSchemaTabPageEx::getCurrentWorkcopy()
{
	// Select destination folder
	//
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty() == true)
	{
		return;
	}

	if (dir[dir.length() - 1] != '/')
	{
		dir.append("/");
	}

	// Save files to disk
	//
	QString fileName = dir + fileInfo().fileName();

	bool writeResult = m_schemaWidget->schema()->saveToFile(fileName);

	if (writeResult == false)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Write file error."));
		msgBox.setInformativeText(tr("Cannot write file %1.").arg(fileInfo().fileName()));
		msgBox.exec();
	}

	return;
}

void EditSchemaTabPageEx::setCurrentWorkcopy()
{
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		(fileInfo().userId() != db()->currentUser().userId() && db()->currentUser().isAdminstrator() == false))
	{
		Q_ASSERT(fileInfo().userId() == db()->currentUser().userId());
		return;
	}

	// Select file
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	// Load file
	//
	bool readResult = m_schemaWidget->schema()->Load(fileName);
	if (readResult == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Can't read file %1.").arg(fileName));
		mb.exec();
		return;
	}

	// --
	setPageTitle();

	m_schemaWidget->resetAction();
	m_schemaWidget->clearSelection();

	m_schemaWidget->resetEditEngine();
	m_schemaWidget->setModified();

	m_schemaWidget->update();

	return;
}

std::shared_ptr<VFrame30::Schema> EditSchemaTabPageEx::schema()
{
	Q_ASSERT(m_schemaWidget);
	std::shared_ptr<VFrame30::Schema> s = m_schemaWidget->schema();
	return s;
}

const DbFileInfo& EditSchemaTabPageEx::fileInfo() const
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->fileInfo();
}

void EditSchemaTabPageEx::setFileInfo(const DbFileInfo& fi)
{
	Q_ASSERT(m_schemaWidget);
	m_schemaWidget->setFileInfo(fi);

	m_schemaWidget->schema()->setChangeset(fi.changeset());

	setPageTitle();
}

bool EditSchemaTabPageEx::readOnly() const
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->readOnly();
}

void EditSchemaTabPageEx::setReadOnly(bool value)
{
	Q_ASSERT(m_schemaWidget);
	m_schemaWidget->setReadOnly(value);

	setPageTitle();
}

bool EditSchemaTabPageEx::modified() const
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->modified();
}

void EditSchemaTabPageEx::resetModified()
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->resetModified();
}

bool EditSchemaTabPageEx::compareWidget() const
{
	return m_schemaWidget->compareWidget();
}

bool EditSchemaTabPageEx::isCompareWidget() const
{
	return m_schemaWidget->compareWidget();
}

void EditSchemaTabPageEx::setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target)
{
	return m_schemaWidget->setCompareWidget(value, source, target);
}

void EditSchemaTabPageEx::setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions)
{
	m_schemaWidget->setCompareItemActions(itemsActions);
}

