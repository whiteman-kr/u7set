#include "SchemaControlTabPage.h"

#include <HardwareLib/PropertyNames.h>
#include <UiLib/StandardColors.h>
#include <UiLib/TagSelectorWidget.h>
#include <VFrame30/ActuatorHeader.h>
#include <VFrame30/ActuatorSchema.h>
#include <VFrame30/DiagSchema.h>
#include <VFrame30/LogicSchema.h>
#include <VFrame30/MonitorSchema.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItem.h>
#include <VFrame30/TuningSchema.h>
#include <VFrame30/UfbSchema.h>
#include <VFrame30/VduSchema.h>

#include "AppSettings.h"
#include "CheckInDialog.h"
#include "CreateActuatorDialog.h"
#include "CreateSchemaDialog.h"
#include "DialogClientBehavior.h"
#include "EditSchemaTabPage.h"
#include "IdePropertyEditor.h"
#include "SchemasTabPage.h"

#include "Forms/CompareDialog.h"
#include "Forms/ComparePropertyObjectDialog.h"
#include "Forms/FileHistoryDialog.h"
#include "Forms/SelectChangesetDialog.h"
#include "Reports/DialogSchemasExport.h"
#include "Reports/SchemasReport.h"


#ifdef _DEBUG
	#include <QAbstractItemModelTester>
#endif

namespace
{
	bool isFileActuatorHeader(const DbFileInfo& file)
	{
		return file.isNull() == false && file.isFolder() == false &&
			   file.ext().compare(File::ActuatorHeaderFileExtension, Qt::CaseInsensitive) == 0;
	}
} // namespace

//
//
// SchemaListModel
//
//
SchemaListModel::SchemaListModel(DbController* dbc, QWidget* parentWidget) :
	QAbstractItemModel(parentWidget),
	HasDbController(dbc)
{
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaListModel::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaListModel::projectClosed);
}

QModelIndex SchemaListModel::index(int row, int column, const QModelIndex& parent /* = QModelIndex()*/) const
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

QModelIndex SchemaListModel::parent(const QModelIndex& index) const
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

int SchemaListModel::rowCount(const QModelIndex& parentIndex /* = QModelIndex()*/) const
{
	if (m_files.empty() == true || parentIndex.column() > 0)
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

int SchemaListModel::columnCount(const QModelIndex& /*parent*/ /* = QModelIndex()*/) const
{
	return static_cast<int>(Columns::ColumnCount);
}

QVariant SchemaListModel::data(const QModelIndex& index, int role /* = Qt::DisplayRole*/) const
{
	if (index.isValid() == false)
	{
		return {};
	}

	// int row = index.row();
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
			if (file->state() == E::VcsState::CheckedIn)
			{
				return {};
			}
			else
			{
				return E::valueToString<E::VcsState>(file->state());
			}

		case Columns::FileActionColumn:
			return E::valueToString<E::VcsItemAction>(file->action());

		case Columns::ChangesetColumn:
			return (file->state() == E::VcsState::CheckedIn) ? QVariant{file->changeset()} : QVariant{};

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
					excludedCount--; // match includes file itself
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

				// -- Issues
				//
				if (issues.errors == 0 && issues.warnings == 0) {}

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
		if (file->state() == E::VcsState::CheckedOut)
		{
			QBrush b{StandardColors::VcsCheckedIn};

			switch (file->action())
			{
			case E::VcsItemAction::Added:
				b.setColor(StandardColors::VcsAdded);
				break;
			case E::VcsItemAction::Modified:
				b.setColor(StandardColors::VcsModified);
				break;
			case E::VcsItemAction::Deleted:
				b.setColor(StandardColors::VcsDeleted);
				break;
			default:
				Q_ASSERT(false);
			}

			return QVariant(b);
		}
	}

	if (role == Qt::ForegroundRole)
	{
		if (column == Columns::IssuesColumn && excludedFromBuild(fileId) == false)
		{
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
				Q_ASSERT(fn.isEmpty() == false); // Empty file name?
				return {};
			}
		}
		else
		{
			return excludedFromBuild(fileId) ? QBrush{Qt::darkGray} : QVariant{};
		}
	}

	if (role == Qt::DecorationRole)
	{
		if (index.column() == 0 && file->isFolder() == true)
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
		if (auto it = m_details.find(fileId); it == m_details.end())
		{
			return false;
		}
		else
		{
			const VFrame30::SchemaDetails& details = it->second;
			return details.searchForString(m_searchText);
		}
	}

	if (role == SearchByFileIds)
	{
		return m_searchByFileIds.contains(file->fileId());
	}

	if (role == ExcludedSchemaRole)
	{
		bool excluded = excludedFromBuild(file->fileId());
		return excluded;
	}

	return QVariant{};
}

QVariant SchemaListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			// clang-format off
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
			// clang-format on
		}

		return {};
	}

	return {};
}

std::pair<QModelIndex, bool> SchemaListModel::addFile(QModelIndex parentIndex, std::shared_ptr<DbFileInfo> file)
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
		return {{}, false}; // At least parent must be present
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

bool SchemaListModel::deleteFilesUpdate(const QModelIndexList& selectedIndexes,
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

bool SchemaListModel::moveFilesUpdate(const QModelIndexList& selectedIndexes,
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
	QModelIndexList matched =
		match(index(0, 0), Qt::UserRole, QVariant::fromValue(movedToParnetId), 1, Qt::MatchExactly | Qt::MatchRecursive);

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
		auto [mi, ok] = addFile(movedToParentIndex, std::make_shared<DbFileInfo>(f));

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

bool SchemaListModel::updateFiles(const QModelIndexList& selectedIndexes, const std::vector<DbFileInfo>& files)
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

	std::sort(
		sortedRowList.begin(),
		sortedRowList
			.end(), // Actually, this sort is not required anymore, as rows to remove are stored in map removeRows, which is sorted itslef
		[](QModelIndex& m1, QModelIndex m2)
		{
			return m1.internalId() >= m2.internalId();
		});

	// Update model
	//
	struct RemoveRows
	{
		QModelIndex parentModelIndex;
		std::map<int, int> childrenRows;     // map where key is child row to delete and value if FileId to delete
	};

	std::map<int, RemoveRows> removeRowsMap; // key - parent.fileid for deleting row. Used fileid as it must be dleted children first

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
		// int parentFileId = rit->first;
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

bool SchemaListModel::updateShemaDetails(VFrame30::SchemaDetails details)
{
	if (details.isNull() == true)
	{
		return false;
	}

	auto it = std::find_if(m_details.begin(),
						   m_details.end(),
						   [&details](const std::pair<int, VFrame30::SchemaDetails>& p) -> bool
						   {
							   if (p.second.m_schemaId == details.m_schemaId)
							   {
								   return true;
							   }

							   return false;
						   });

	if (it != m_details.end())
	{
		it->second = std::move(details);
	}

	return it != m_details.end();
}

DbFileInfo SchemaListModel::file(int fileId) const
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

DbFileInfo SchemaListModel::file(const QModelIndex& modelIndex) const
{
	if (modelIndex.isValid() == false)
	{
		return m_parentFile;
	}

	int fileId = static_cast<int>(modelIndex.internalId());
	Q_ASSERT(fileId != -1);

	return file(fileId);
}

std::shared_ptr<DbFileInfo> SchemaListModel::fileSharedPtr(const QModelIndex& modelIndex) const
{
	if (modelIndex.isValid() == false)
	{
		return std::make_shared<DbFileInfo>(m_parentFile);
	}

	int fileId = static_cast<int>(modelIndex.internalId());
	Q_ASSERT(fileId != -1);

	return m_files.file(fileId);
}

bool SchemaListModel::isFolder(const QModelIndex& modelIndex) const
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

QModelIndexList SchemaListModel::searchFor(const QString searchText)
{
	m_searchText = searchText;
	return match(index(0, 0), SearchSchemaRole, QVariant::fromValue(true), -1, Qt::MatchExactly | Qt::MatchRecursive);
}

QModelIndexList SchemaListModel::searchByFileIds(std::set<int> fileIds)
{
	m_searchByFileIds = std::move(fileIds);
	return match(index(0, 0), SearchByFileIds, QVariant::fromValue(true), -1, Qt::MatchExactly | Qt::MatchRecursive);
}

void SchemaListModel::setFilter(QString filter)
{
	m_filterText = filter;
	refresh();
	return;
}

void SchemaListModel::setTagFilter(const QStringList& tags)
{
	m_tagFilter = tags;
	refresh();
	return;
}

const QStringList& SchemaListModel::tagFilter() const
{
	return m_tagFilter;
}

void SchemaListModel::applyFilter(DbFileTree* filesTree, const std::map<int, VFrame30::SchemaDetails>& detailsMap)
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

	for (const auto& [fileId, file] : files)
	{
		if (isSystemFile(fileId) || fileId == rootFileId)
		{
			filteredFiles[fileId] = file;
			continue;
		}

		// Filter by text
		//
		if (file->fileName().contains(m_filterText, Qt::CaseInsensitive) == true ||
			m_users[file->userId()].contains(m_filterText, Qt::CaseInsensitive) == true)
		{
			filteredFiles[fileId] = file;
			schemaFilterCount++;

			continue;
		}

		if (auto dit = detailsMap.find(fileId); dit != detailsMap.end())
		{
			const VFrame30::SchemaDetails& details = dit->second;

			if (bool searchResult = details.searchForString(m_filterText); searchResult == true)
			{
				filteredFiles[fileId] = file;
				schemaFilterCount++;

				continue;
			}
		}
		else
		{
			// There is no info in detailsMap for this file
			// It can be folder
			//
			continue;
		}
	}

	// Add parents
	//
	std::map<int, std::shared_ptr<DbFileInfo>> parentFiles;

	for (auto& [fileId, file] : filteredFiles)
	{
		if (isSystemFile(fileId) || fileId == rootFileId)
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

		while (parentFile != nullptr && parentFile->isNull() == false && isSystemFile(parentFile->fileId()) == false)
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

	for (auto& [fileId, file] : parentFiles)
	{
		filteredFiles[fileId] = file;
	}

	// --
	//
	*filesTree = DbFileTree{filteredFiles, rootFileId};

	m_schemaFilterCount = schemaFilterCount;

	return;
}

void SchemaListModel::applyTagFilter(DbFileTree* filesTree, const std::map<int, VFrame30::SchemaDetails>& detailsMap)
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

	for (const auto& [fileId, file] : files)
	{
		if (isSystemFile(fileId) || fileId == rootFileId)
		{
			filteredFiles[fileId] = file;
			continue;
		}

		if (file->isFolder() == true)
		{
			continue; // If this folder contains any child it will be added later in "Add parent" part
		}

		if (auto dit = detailsMap.find(fileId); dit != detailsMap.end())
		{
			const VFrame30::SchemaDetails& details = dit->second;

			if (bool searchResult = details.hasSchemaTag(m_tagFilter); searchResult == true)
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

	for (auto& [fileId, file] : filteredFiles)
	{
		if (isSystemFile(fileId) || fileId == rootFileId)
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

		while (parentFile != nullptr && parentFile->isNull() == false && isSystemFile(parentFile->fileId()) == false)
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

	for (auto& [fileId, file] : parentFiles)
	{
		filteredFiles[fileId] = file;
	}

	// --
	//
	*filesTree = DbFileTree{filteredFiles, rootFileId};

	return;
}


bool SchemaListModel::isSystemFile(int fileId) const
{
	return m_systemFiles.find(fileId) != m_systemFiles.end();
}

void SchemaListModel::updateTagsFromDetails()
{
	m_tags.clear();

	for (auto& [fileId, details] : m_details)
	{
		for (const QString& tag : details.schemaTags())
		{
			m_tags.insert(tag);
		}
	}

	emit tagsChanged();

	return;
}

void SchemaListModel::refresh()
{
	if (db()->isProjectOpened() == false)
	{
		projectClosed();
		return;
	}

	// Get file tree
	//
	DbFileTree files;
	bool ok = dbc()->getFileListTree(&files, m_parentFile.fileId(), true, nullptr);
	if (ok == false)
	{
		return; // do not reset model, just leave it as is
	}

	files.removeIf(
		[](auto&& f)
		{
			return File::isSchemaTemplateFileExtension(f.extension());
		});

	// Parse file details, before applying filter, as we want to keep tags for all schemas
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
			// qDebug() << "void SchemaListModel::refresh(): File not parsed " << fileId << ", " << fileInfo->fileName();
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

	ok = dbc()->getUserList(&users, nullptr);
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

void SchemaListModel::projectOpened(DbProject /*project*/)
{
	m_parentFile = db()->systemFileInfo(DbDir::SchemasDir);
	Q_ASSERT(m_parentFile.fileId() != -1);

	std::vector<DbFileInfo> systemFiles = db()->systemFiles();
	for (const DbFileInfo& sf : systemFiles)
	{
		m_systemFiles.insert(sf.fileId());
	}

	refresh();

	return;
}

void SchemaListModel::projectClosed()
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

QString SchemaListModel::usernameById(int userId) const noexcept
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

QString SchemaListModel::tagsColumnText(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return {};
	}

	QString result;
	result.reserve(256);

	const VFrame30::SchemaDetails& d = it->second;
	for (QString tag : d.m_schemaTags)
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

QString SchemaListModel::detailsColumnText(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return {};
	}

	const VFrame30::SchemaDetails& d = it->second;
	return d.m_equipmentId;
}

QString SchemaListModel::fileCaption(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return {};
	}

	const VFrame30::SchemaDetails& d = it->second;
	return d.m_caption.trimmed();
}

bool SchemaListModel::excludedFromBuild(int fileId) const
{
	auto it = m_details.find(fileId);
	if (it == m_details.end())
	{
		return false;
	}

	const VFrame30::SchemaDetails& d = it->second;
	return d.m_excludedFromBuild;
}

const DbFileInfo& SchemaListModel::parentFile() const
{
	return m_parentFile;
}

const DbFileTree& SchemaListModel::files() const
{
	// m_files is filetred!
	//
	return m_files;
}

int SchemaListModel::schemaFilterCount() const
{
	return m_schemaFilterCount;
}

const std::set<QString>& SchemaListModel::tags() const
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


SchemaProxyListModel::~SchemaProxyListModel() {}

void SchemaProxyListModel::setSourceModel(QAbstractItemModel* sourceModel)
{
	QSortFilterProxyModel::setSourceModel(sourceModel);

	m_sourceModel = dynamic_cast<SchemaListModel*>(sourceModel);
	Q_ASSERT(m_sourceModel != nullptr);

	return;
}

bool SchemaProxyListModel::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
{
	// All folders always at top
	//
	bool leftIsFolder = m_sourceModel->isFolder(sourceLeft);
	bool rightIsFolder = m_sourceModel->isFolder(sourceRight);

	bool result = false;

	if ((leftIsFolder == true && rightIsFolder == true) || (leftIsFolder == false && rightIsFolder == false))
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

		if (treeView->isExpanded(mi) == true && fileId != DbFileInfo::Null)
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
SchemaFileView::SchemaFileView(DbController* dbc, QWidget* parent) :
	QTreeView(parent),
	HasDbController(dbc),
	m_filesModel(dbc, this)
{
	Q_ASSERT(dbc != nullptr);

	setUniformRowHeights(false);    // Helps to show multiline schema cations
	setWordWrap(false);
	setExpandsOnDoubleClick(false); // DoubleClick signal is used

	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	// --
	//
	createActions();
	createContextMenu();

	// Adjust view
	//
	// m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);
	m_proxyModel.setSourceModel(&m_filesModel);

	setModel(&m_proxyModel);

#ifdef _DEBUG
	#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
	[[maybe_unused]] QAbstractItemModelTester* modelTester =
		new QAbstractItemModelTester(&m_filesModel, QAbstractItemModelTester::FailureReportingMode::Fatal, this);
	#endif
#endif

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaFileView::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaFileView::projectClosed);

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &SchemaFileView::selectionChanged);

	connect(this, &QTreeView::doubleClicked, this, &SchemaFileView::slot_doubleClicked);

	// Timer for updates of WRN/ERR count
	//
	startTimer(50);

	// --
	//
	QByteArray lastState = QSettings{}.value("SchemeEditor/SchemaFileView/State").toByteArray();
	header()->restoreState(lastState);

	return;
}

SchemaFileView::~SchemaFileView()
{
	QSettings{}.setValue("SchemeEditor/SchemaFileView/State", header()->saveState());
}

void SchemaFileView::createActions()
{
	// clang-format off
	m_newSchemaAction = new QAction(tr("New Schema..."), parent());
	m_newSchemaAction->setIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));
	m_newSchemaAction->setStatusTip(tr("Add new file to version control..."));
	m_newSchemaAction->setEnabled(false);
	m_newSchemaAction->setShortcut(QKeySequence::StandardKey::New);

	m_newActuatorAction = new QAction(tr("New Actuator..."), parent());
	m_newActuatorAction->setIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));
	m_newActuatorAction->setStatusTip(tr("Add new file to version control..."));
	m_newActuatorAction->setEnabled(false);
	m_newActuatorAction->setShortcut(QKeySequence::StandardKey::New);

	m_newFolderAction = new QAction(tr("New Folder..."), parent());
	m_newFolderAction->setIcon(QIcon(":/Images/Images/SchemaAddFolder2.svg"));
	m_newFolderAction->setStatusTip(tr("Add new folder to version control..."));
	m_newFolderAction->setEnabled(false);

	m_cloneFileAction = new QAction(tr("Clone"), parent());
	m_cloneFileAction->setIcon(QIcon(":/Images/Images/SchemaClone.svg"));
	m_cloneFileAction->setStatusTip(tr("Clone file..."));
	m_cloneFileAction->setEnabled(false);

	m_openAction = new QAction(tr("Open"), parent());
	m_openAction->setIcon(QIcon(":/Images/Images/SchemaOpen.svg"));
	m_openAction->setStatusTip(tr("Open file to edit"));
	m_openAction->setEnabled(false);

	m_viewAction = new QAction(tr("View..."), parent());
	m_viewAction->setIcon(QIcon(":/Images/Images/SchemaView.svg"));
	m_viewAction->setStatusTip(tr("Open file to view"));
	m_viewAction->setEnabled(false);

	m_deleteAction = new QAction(tr("Delete"), parent());
	m_deleteAction->setIcon(QIcon(":/Images/Images/SchemaDelete.svg"));
	m_deleteAction->setStatusTip(tr("Mark file as deleted..."));
	m_deleteAction->setEnabled(false);
	m_deleteAction->setShortcut(QKeySequence::Delete);

	m_moveFileAction = new QAction(tr("Move"), parent());
	m_moveFileAction->setStatusTip(tr("Move file(s) to another folder..."));
	m_moveFileAction->setEnabled(false);

	// --
	//
	m_checkOutAction = new QAction(tr("Check Out"), parent());
	m_checkOutAction->setStatusTip(tr("Check Out for edit..."));
	m_checkOutAction->setIcon(QIcon(":/Images/Images/SchemaCheckOut.svg"));
	m_checkOutAction->setEnabled(false);

	m_checkInAction = new QAction(tr("Check In"), parent());
	m_checkInAction->setStatusTip(tr("Check In pending changes..."));
	m_checkInAction->setIcon(QIcon(":/Images/Images/SchemaCheckIn.svg"));
	m_checkInAction->setEnabled(false);

	m_undoChangesAction = new QAction(tr("Undo Changes"), parent());
	m_undoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_undoChangesAction->setIcon(QIcon{":/Images/Images/SchemaUndo.svg"});
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
	m_exportWorkingcopyAction->setIcon(QIcon(":/Images/Images/SchemaUpload.svg"));
	m_exportWorkingcopyAction->setStatusTip(tr("Export workingcopy file to disk..."));
	m_exportWorkingcopyAction->setEnabled(false);

	m_importWorkingcopyAction = new QAction(tr("Import Working Copy..."), parent());
	m_importWorkingcopyAction->setIcon(QIcon(":/Images/Images/SchemaDownload.svg"));
	m_importWorkingcopyAction->setStatusTip(tr("Import workingcopy file from disk to project file..."));
	m_importWorkingcopyAction->setEnabled(false);

	m_exportToPdfAction = new QAction(tr("Export to PDF..."), parent());
	m_exportToPdfAction->setStatusTip(tr("Export selected schemas to PDF files"));
	m_exportToPdfAction->setIcon(QIcon(":/Images/Images/SchemaToPdf.svg"));
	m_exportToPdfAction->setEnabled(false);

	m_exportToAlbumAction = new QAction(tr("Create Schemas Albums..."), parent());
	m_exportToAlbumAction->setStatusTip(tr("Create Schemas Albums in PDF format"));
	m_exportToAlbumAction->setIcon(QIcon(":/Images/Images/SchemaToAlbum.svg"));

	// --
	//
	m_refreshFileAction = new QAction(tr("Refresh"), parent());
	m_refreshFileAction->setIcon(QIcon(":/Images/Images/SchemaRefresh.svg"));
	m_refreshFileAction->setStatusTip(tr("Refresh file list..."));
	m_refreshFileAction->setEnabled(false);
	m_refreshFileAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshFileAction, &QAction::triggered, this, &SchemaFileView::slot_refreshFiles);

	m_behaviorAction = new QAction(tr("Behavior..."), parent());
	m_behaviorAction->setIcon(QIcon(":/Images/Images/SchemaBehavior.svg"));
	m_behaviorAction->setStatusTip(tr("Edit Behavior..."));
	m_behaviorAction->setEnabled(true);

	m_propertiesAction = new QAction(tr("Properties..."), parent());
	m_propertiesAction->setIcon(QIcon(":/Images/Images/SchemaProperties.svg"));
	m_propertiesAction->setStatusTip(tr("Edit schema properties..."));
	m_propertiesAction->setEnabled(false);


	// clang-format on
	return;
}

void SchemaFileView::createContextMenu()
{
	setContextMenuPolicy(Qt::ActionsContextMenu);

	addAction(m_openAction);
	addAction(m_viewAction);

	// --
	//
	QAction* separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	addAction(m_newSchemaAction);
	addAction(m_newActuatorAction);

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

	addAction(m_exportToPdfAction);

	// --
	//
	separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	addAction(m_refreshFileAction);
	addAction(m_behaviorAction);
	addAction(m_propertiesAction);

	// separator = new QAction(this);
	// separator->setSeparator(true);
	// addAction(separator);

	return;
}

void SchemaFileView::timerEvent(QTimerEvent* event)
{
	QTreeView::timerEvent(event);

	if (int buildIuuseCount = GlobalMessanger::instance().buildIssues().count(); buildIuuseCount != m_lastBuildIssueCount)
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

void SchemaFileView::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		QModelIndex currentIndex = QTreeView::currentIndex();
		if (currentIndex.isValid())
		{
			slot_doubleClicked(currentIndex);
		}
	}

	QTreeView::keyPressEvent(event);
}

std::vector<std::shared_ptr<DbFileInfo>> SchemaFileView::selectedFiles() const
{
	std::vector<std::shared_ptr<DbFileInfo>> result;

	QItemSelectionModel* selModel = selectionModel();
	if (selModel->hasSelection() == false)
	{
		return result;
	}

	QModelIndexList sel = selModel->selectedRows();
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

bool SchemaFileView::isActuatorFolder(const QModelIndex& index) const
{
	return isActuatorFolder(filesModel().file(index));
}

bool SchemaFileView::isActuatorFolder(DbFileInfo fileInfo) const
{
	auto actuatorsDirId = db()->systemFileId(DbDir::ActuatorsDir);
	bool isActuatorFolder = false;

	while (fileInfo.isNull() == false)
	{
		if (fileInfo.fileId() == actuatorsDirId)
		{
			isActuatorFolder = true;
			break;
		}

		fileInfo = filesModel().file(fileInfo.parentId());
	}

	return isActuatorFolder;
}

void SchemaFileView::refreshFiles()
{
	// Save old selection and expansion
	//
	const QItemSelection proxySelection = selectionModel()->selection();
	const QItemSelection mappedSelection = m_proxyModel.mapSelectionToSource(proxySelection);

	std::vector<int> selectedFilesIds;
	selectedFilesIds.reserve(mappedSelection.size());

	for (const QModelIndex& mi : mappedSelection.indexes())
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

	selectionChanged({}, {}); // To update actions
	return;
}

void SchemaFileView::searchAndSelect(QString searchText)
{
	clearSelection();

	QModelIndexList matched = m_filesModel.searchFor(searchText);
	if (matched.isEmpty() == true)
	{
		QMessageBox::information(this, qAppName(), tr("No schema found."));
		return;
	}

	selectFoundItems(matched);

	QMessageBox::information(this, qAppName(), tr("Found %1 schema(s)").arg(matched.size()));

	return;
}

void SchemaFileView::fullSearchAndSelect(QString searchText)
{
	clearSelection();

	if (searchText.trimmed().isEmpty() == true)
	{
		return;
	}

	QString limitedSearchText = searchText;
	if (limitedSearchText.size() > 32)
	{
		limitedSearchText.truncate(32);
		limitedSearchText += "...";
	}

	// Make shallow search in all schemas and filter all found schemas.
	//
	std::set<int> alreadyFoundIds; // The result of the shallow search.

	{
		QModelIndexList shallowSearchMatched = m_filesModel.searchFor(searchText);

		for (const auto& mi : shallowSearchMatched)
		{
			alreadyFoundIds.insert(m_filesModel.file(mi).fileId());
		}
	}

	std::set<int> matchedFileIds{
		alreadyFoundIds}; // The result of the search. matchedFileIds is a separate copy as it is accessed from parsing thread.

	// --
	//
	DbFileTree fileTree = m_filesModel.files();

	auto files = fileTree.toVectorIf(
		[](const DbFileInfo& f)
		{
			return f.isFolder() == false && f.size() > 0;
		});
	fileTree.clear();


	// --
	//
	QProgressDialog progress(tr("Searching text %1...").arg(limitedSearchText), tr("Cancel"), 0, std::ssize(files), this);
	progress.setMinimumDuration(200);
	progress.setWindowModality(Qt::WindowModal);

	std::atomic<int> processedCount =
		std::ssize(matchedFileIds); // These files will not be requested from the database and will not be parsed.
	std::atomic<int> foundCount = std::ssize(matchedFileIds);

	QElapsedTimer timer;
	timer.start();

	// 1. loads file from the database
	//
	std::condition_variable m_filesQueueCondition;
	std::mutex filesQueueMutex;
	std::queue<std::shared_ptr<DbFile>> filesQueue;

	std::atomic<bool> cancelSearch = false;
	std::atomic<bool> allFilesLoaded = false;

	auto loadFilesFromDb =
		[&files, &filesQueueMutex, &filesQueue, &m_filesQueueCondition, &cancelSearch, &allFilesLoaded, &alreadyFoundIds](DbController* db)
	{
		for (const DbFileInfo& file : files)
		{
			if (cancelSearch.load() == true)
			{
				return;
			}

			if (alreadyFoundIds.contains(file.fileId()) == true)
			{
				// This file already was matched.
				//
				continue;
			}

			std::shared_ptr<DbFile> outFile;
			bool ok = db->getLatestVersion(file, &outFile, nullptr);
			if (ok == true && outFile != nullptr && outFile->size() > 0)
			{
				filesQueueMutex.lock();
				filesQueue.push(outFile);
				filesQueueMutex.unlock();

				m_filesQueueCondition.notify_one();
			}
		}

		allFilesLoaded.store(true);

		return;
	};

	QFuture<void> loadFuture = QtConcurrent::run(loadFilesFromDb, db());

	// 2. parses schema
	//
	auto parseSchemaAndSearch = [&filesQueue,
								 &filesQueueMutex,
								 &m_filesQueueCondition,
								 &cancelSearch,
								 &processedCount,
								 &foundCount,
								 &allFilesLoaded,
								 &matchedFileIds](QString searchText)
	{
		do
		{
			std::shared_ptr<DbFile> file;

			// Wait schema to be loaded from teh database.
			//
			do
			{
				std::unique_lock<std::mutex> lock(filesQueueMutex);
				m_filesQueueCondition.wait_for(lock,
											   std::chrono::milliseconds{10},
											   [&filesQueue, &cancelSearch]()
											   {
												   return filesQueue.empty() == false || cancelSearch.load() == true;
											   });

				if (cancelSearch.load() == true)
				{
					// Check that cancel was requested.
					//
					return;
				}

				if (filesQueue.empty() == true)
				{
					if (allFilesLoaded.load() == true)
					{
						// All files are loaded and processed.
						//
						return;
					}
				}
				else
				{
					file = filesQueue.front();
					filesQueue.pop();
				}

			} while (file == nullptr); // it happens if timeout occurs in m_filesQueueCondition.wait_for().

			// Parse schema.
			//
			auto schema = VFrame30::Schema::Create(file->data());
			if (schema == nullptr)
			{
				continue;
			}

			// Search schema.
			//
			bool found = false;
			for (const auto& layer : schema->layers())
			{
				for (const auto& item : layer->items())
				{
					auto searchResult = item->searchTextByProps(searchText, Qt::CaseSensitive);

					if (searchResult.empty() == false)
					{
						// Hit!
						//
						found = true; // file.fileId();

						matchedFileIds.insert(file->fileId());

						foundCount++;
						break;
					}
				}

				if (found == true)
				{
					break;
				}
			}

			processedCount++;

		} while (cancelSearch.load() == false);
	};

	QFuture<void> parseFuture = QtConcurrent::run(parseSchemaAndSearch, searchText);

	// Wait all for finished.
	//
	int localProcessedCount = -1;
	int localFoundCount = -1;

	while (loadFuture.isRunning() == true || parseFuture.isRunning() == true)
	{
		if (progress.wasCanceled() == true) // Prevents flicking progress dialog - when progress.setValue it makes dialog visible again even
											// if the user canceled application.
		{
			cancelSearch.store(true);
			break;
		}

		if (localProcessedCount != processedCount || localFoundCount != foundCount)
		{
			localProcessedCount = processedCount;
			localFoundCount = foundCount;

			progress.setValue(processedCount);

			QString newProgressText = tr("Searching text \"%1\"\n Processed %2 of %3 file(s)...\n Found %4 schema(s)")
										  .arg(limitedSearchText)
										  .arg(processedCount)
										  .arg(files.size())
										  .arg(foundCount);
			progress.setLabelText(newProgressText);
		}

		QCoreApplication::processEvents();
		QThread::yieldCurrentThread();
	}

	loadFuture.waitForFinished();
	parseFuture.waitForFinished();

	qDebug() << "Search time: " << timer.elapsed() << "ms";

	if (progress.wasCanceled() == true && foundCount == 0)
	{
		// Do not show message box with text "No schema found."
		//
		return;
	}

	// Get model indexes for the found schemas
	//
	if (foundCount == 0)
	{
		QMessageBox::information(this, qAppName(), tr("No schema found."));
	}
	else
	{
		Q_ASSERT(matchedFileIds.empty() == false);

		QModelIndexList matched = m_filesModel.searchByFileIds(std::move(matchedFileIds));
		selectFoundItems(matched);

		QMessageBox::information(this, qAppName(), tr("Found %1 schema(s).").arg(foundCount));
	}

	return;
}

void SchemaFileView::selectFoundItems(QModelIndexList& indexes)
{
	QItemSelection selection;

	for (QModelIndex& fileModelIndex : indexes)
	{
		QModelIndex mappedModelIndex = m_proxyModel.mapFromSource(fileModelIndex);

		selection.select(mappedModelIndex, mappedModelIndex);

		QModelIndex expandParent = mappedModelIndex.parent();
		while (expandParent.isValid() == true)
		{
			expand(expandParent);
			expandParent = expandParent.parent();
		}
	}

	selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	// Scroll to somewhere, unfortunately selectedIndexes does not provide sorted list, so it's just scroll somewhere
	//
	if (selection.indexes().empty() == false)
	{
		scrollTo(selection.indexes().front());
	}
}

void SchemaFileView::setFilter(QString filter)
{
	m_filesModel.setFilter(filter);

	if (filter.trimmed().isEmpty() == false)
	{
		expandAll();
	}

	return;
}

void SchemaFileView::setTagFilter(const QStringList& tags)
{
	m_filesModel.setTagFilter(tags);

	expandAll();

	return;
}

void SchemaFileView::projectOpened()
{
	m_refreshFileAction->setEnabled(true);

	selectionChanged({}, {});

	return;
}

void SchemaFileView::projectClosed()
{
	m_newSchemaAction->setEnabled(false);
	m_newActuatorAction->setEnabled(false);
	m_newFolderAction->setEnabled(false);
	m_cloneFileAction->setEnabled(false);
	m_refreshFileAction->setEnabled(false);

	return;
}

void SchemaFileView::slot_refreshFiles()
{
	refreshFiles();
	return;
}

void SchemaFileView::slot_doubleClicked(const QModelIndex& index)
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
		if (qApp->keyboardModifiers().testFlag(Qt::AltModifier) == true)
		{
			// Show file properties
			//
			emit showFileProperties(file);
		}
		else
		{
			// Open file
			//
			if (file.state() == E::VcsState::CheckedOut)
			{
				emit openFileSignal(file);
			}
			else
			{
				emit viewFileSignal(file);
			}
		}
	}

	return;
}

void SchemaFileView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	auto selectedFiles = this->selectedFiles();
	auto selectedFolders = selectedFiles | std::views::filter(
											   [](const std::shared_ptr<DbFileInfo>& file)
											   {
												   return file->isFolder() == true;
											   });
	auto selectedActuatorHeaders = selectedFiles | std::views::filter(
													   [this](const std::shared_ptr<DbFileInfo>& file)
													   {
														   return isFileActuatorHeader(*file);
													   });
	auto selectedSchemas = selectedFiles | std::views::filter(
											   [](const std::shared_ptr<DbFileInfo>& file)
											   {
												   return file->isFolder() == false && isFileActuatorHeader(*file) == false;
											   });

	bool selectedOneNonSystemFile = selectedFiles.size() == 1 && db()->systemFileInfo(selectedFiles.front()->fileId()).isNull() == true &&
									selectedFiles.front()->directoryAttribute() == false;

	if (selectedFiles.size() == 1)
	{
		// Can create some file, depending on the type of the selection.

		bool isActuator = false;

		if (isActuatorFolder(*selectedFiles.front()) == true)
		{
			// If selected file is Actuator or has Actuator as one of the parents, then add Schema.
			//
			DbFileInfo selectedFile = *selectedFiles.front();

			while (selectedFile.isNull() == false && selectedFile.fileId() != dbc()->rootFileId())
			{
				if (isFileActuatorHeader(selectedFile) == true)
				{
					isActuator = true;
					break;
				}

				selectedFile = filesModel().file(selectedFile.parentId());
			}
		}

		bool actuatorCanBeCreated = isActuatorFolder(*selectedFiles.front()) == true && isActuator == false;

		m_newActuatorAction->setVisible(actuatorCanBeCreated);
		m_newActuatorAction->setEnabled(actuatorCanBeCreated);

		m_newSchemaAction->setVisible(!actuatorCanBeCreated);
		m_newSchemaAction->setEnabled(!actuatorCanBeCreated);
	}

	m_newFolderAction->setEnabled(selectedFiles.size() == 1);
	m_cloneFileAction->setEnabled(selectedOneNonSystemFile);

	// --
	//
	int currentUserId = dbc()->currentUser().userId();
	bool currentUserIsAdmin = dbc()->currentUser().isAdministrator();

	bool hasDeletePossibility = false;
	bool hasMovePossibility = false;
	bool hasCheckOutPossibility = false;
	bool hasCheckInPossibility = false;
	bool hasUndoPossibility = false;
	bool hasAbilityToOpen = false;
	bool hasViewPossibility = false;

	int canExportToPdf = 0;

	bool canGetWorkcopy = false;
	int canSetWorkcopy = 0;

	// hasAbilityToOpen
	//
	if (selectedFiles.size() == 1 && selectedFiles.front()->state() == E::VcsState::CheckedOut &&
		selectedFiles.front()->directoryAttribute() == false &&
		(selectedFiles.front()->userId() == currentUserId || currentUserIsAdmin == true))
	{
		hasAbilityToOpen = true;
	}

	// hasViewPossibility
	//
	if (selectedFiles.size() == 1 && selectedFiles.front()->directoryAttribute() == false)
	{
		hasViewPossibility = true;
	}

	for (const std::shared_ptr<DbFileInfo>& file : selectedFiles)
	{
		bool fileIsSystem = dbc()->systemFileInfo(file->fileId()).isNull() == false;

		if (fileIsSystem == true) // No any possibilty on system files
		{
			continue;
		}

		// hasDeletePossibility
		//
		if ((file->state() == E::VcsState::CheckedOut && file->userId() == currentUserId) || file->state() == E::VcsState::CheckedIn)
		{
			hasDeletePossibility = true;
		}

		// hasMovePossibility
		//
		if (file->state() == E::VcsState::CheckedOut && file->userId() == currentUserId)
		{
			hasMovePossibility = true;
		}

		// hasCheckOutPossibility
		//
		if (file->state() == E::VcsState::CheckedIn)
		{
			hasCheckOutPossibility = true;
		}

		// hasCheckInPossibility
		//
		if (file->state() == E::VcsState::CheckedOut && (file->userId() == currentUserId || currentUserIsAdmin == true))
		{
			hasCheckInPossibility = true;
		}

		// hasUndoPossibility
		//
		if (file->state() == E::VcsState::CheckedOut && (file->userId() == currentUserId || currentUserIsAdmin == true))
		{
			hasUndoPossibility = true;
		}

		// canGetWorkcopy, canSetWorkcopy
		//
		if (file->state() == E::VcsState::CheckedOut && file->directoryAttribute() == false && file->userId() == currentUserId)
		{
			canGetWorkcopy = true;
			canSetWorkcopy++;
		}

		// hasExportToAlbumPossibility
		//
		if (file->directoryAttribute() == false)
		{
			canExportToPdf++;
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
	m_importWorkingcopyAction->setEnabled(canSetWorkcopy == 1);                        // can set work copy just for one file

	m_exportToPdfAction->setEnabled(canExportToPdf > 0);

	bool selectedOnlySchemasOrOnlyActuatorHeaders = selectedFolders.empty() == true && //
													((selectedSchemas.empty() == false && selectedActuatorHeaders.empty() == true) || //
													 (selectedSchemas.empty() == true && selectedActuatorHeaders.empty() == false));
	m_propertiesAction->setEnabled(selectedOnlySchemasOrOnlyActuatorHeaders);

	return;
}

SchemaListModel& SchemaFileView::filesModel()
{
	return m_filesModel;
}

const SchemaListModel& SchemaFileView::filesModel() const
{
	return m_filesModel;
}

SchemaProxyListModel& SchemaFileView::proxyModel()
{
	return m_proxyModel;
}

// const std::vector<std::shared_ptr<DbFileInfo>>& SchemaFileView::files() const
//{
//	return m_filesModel.files();
// }

const DbFileInfo& SchemaFileView::parentFile() const
{
	return m_filesModel.parentFile();
}

int SchemaFileView::parentFileId() const
{
	return m_filesModel.parentFile().fileId();
}

//
//
// SchemaControlTabPage
//
//
SchemaControlTabPage::SchemaControlTabPage(DbController* db, AppSignalSetProvider* signalSetProvider) :
	HasDbController(db),
	m_signalSetProvider(signalSetProvider)
{
	Q_ASSERT(signalSetProvider);

	// Create controls
	//
	m_filesView = new SchemaFileView(db, this);

	// --
	//
	m_toolBar = new QToolBar{};

	createToolBar();

	m_toolBar->setStyleSheet("QToolButton { padding-top: 3px; padding-bottom: 3px; padding-left: 3px; padding-right: 3px;}");
	m_toolBar->setIconSize(m_toolBar->iconSize() * 0.9);

	connect(m_filesView->m_openAction, &QAction::triggered, this, &SchemaControlTabPage::openSelectedFile);
	connect(m_filesView->m_viewAction, &QAction::triggered, this, &SchemaControlTabPage::viewSelectedFile);

	connect(m_filesView->m_newSchemaAction, &QAction::triggered, this, &SchemaControlTabPage::onAddSchemaFile);
	connect(m_filesView->m_newActuatorAction, &QAction::triggered, this, &SchemaControlTabPage::onAddActuatorFile);

	connect(m_filesView->m_newFolderAction, &QAction::triggered, this, &SchemaControlTabPage::addFolder);
	connect(m_filesView->m_cloneFileAction, &QAction::triggered, this, &SchemaControlTabPage::cloneFile);
	connect(m_filesView->m_deleteAction, &QAction::triggered, this, &SchemaControlTabPage::deleteFiles);
	connect(m_filesView->m_moveFileAction, &QAction::triggered, this, &SchemaControlTabPage::moveFiles);

	connect(m_filesView->m_checkOutAction, &QAction::triggered, this, &SchemaControlTabPage::checkOutFiles);
	connect(m_filesView->m_checkInAction, &QAction::triggered, this, &SchemaControlTabPage::checkInFiles);
	connect(m_filesView->m_undoChangesAction, &QAction::triggered, this, &SchemaControlTabPage::undoChangesFiles);

	connect(m_filesView->m_historyAction, &QAction::triggered, this, &SchemaControlTabPage::showFileHistory);
	connect(m_filesView->m_recursiveHistoryAction, &QAction::triggered, this, &SchemaControlTabPage::showFileHistoryRecursive);
	connect(m_filesView->m_compareAction, &QAction::triggered, this, &SchemaControlTabPage::compareSelectedFile);

	connect(m_filesView->m_exportWorkingcopyAction, &QAction::triggered, this, &SchemaControlTabPage::exportWorkcopy);
	connect(m_filesView->m_importWorkingcopyAction, &QAction::triggered, this, &SchemaControlTabPage::importWorkcopy);

	connect(m_filesView->m_exportToPdfAction, &QAction::triggered, this, &SchemaControlTabPage::exportToPdf);
	connect(m_filesView->m_exportToAlbumAction, &QAction::triggered, this, &SchemaControlTabPage::exportToAlbum);

	connect(m_filesView->m_propertiesAction, &QAction::triggered, this, &SchemaControlTabPage::showFileProperties);

	connect(m_filesView->m_behaviorAction, &QAction::triggered, this, &SchemaControlTabPage::showBehaviorEditor);

	connect(&m_filesView->filesModel(), &SchemaListModel::tagsChanged, this, &SchemaControlTabPage::schemaTagsChanged);

	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::saveUnsavedSchemas,
			this,
			&SchemaControlTabPage::saveUnsavedSchemas,
			Qt::ConnectionType::DirectConnection);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::refreshSchemas, this, &SchemaControlTabPage::refresh);

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

	QStringList completerStringList = QSettings{}.value("SchemaControlTabPage/SearchCompleter").toStringList();
	m_searchCompleter = new QCompleter(completerStringList, this);
	m_searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_searchEdit->setCompleter(m_searchCompleter);
	m_filterEdit->setCompleter(m_searchCompleter);

	m_searchButton = new QPushButton(tr("Search"));
	m_searchButton->setToolTip(tr("Limited and fast search for the text in the schemas"));

	m_fullSearchButton = new QPushButton(tr("Full Search"));
	m_fullSearchButton->setToolTip(tr("Full search for the text in the schemas.\nFull search loads schemas from the project database and "
									  "tries to match all elements and properties."));

	m_filterButton = new QPushButton(tr("Filter"));

	m_resetFilterButton = new QPushButton(tr("Reset Filter"));
	m_resetFilterButton->setDisabled(true);

	m_tagSelector = new UiLib::TagSelectorWidget(this);
	m_tagSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(m_tagSelector, &UiLib::TagSelectorWidget::changed, this, &SchemaControlTabPage::tagSelectorHasChanges);

	// --
	//
	QGridLayout* layout = new QGridLayout(this);

	auto margins = layout->contentsMargins();
	margins.setTop(0);
	layout->setContentsMargins(margins);

	layout->setMenuBar(m_toolBar); // Set ToolBar here as menu, so no gaps and margins
	layout->addWidget(m_filesView, 0, 0, 1, 6);

	layout->addWidget(m_searchEdit, 1, 0, 1, 2);
	layout->addWidget(m_searchButton, 1, 2, 1, 1);
	layout->addWidget(m_fullSearchButton, 1, 3, 1, 1);

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
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaControlTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaControlTabPage::projectClosed);

	connect(m_filesView, &SchemaFileView::showFileProperties, this, &SchemaControlTabPage::showFileProperties);
	connect(m_filesView, &SchemaFileView::openFileSignal, this, &SchemaControlTabPage::openFile);
	connect(m_filesView, &SchemaFileView::viewFileSignal, this, qOverload<const DbFileInfo&>(&SchemaControlTabPage::viewFile));

	connect(m_searchAction, &QAction::triggered, this, &SchemaControlTabPage::ctrlF);
	connect(m_searchEdit,
			&QLineEdit::returnPressed,
			this,
			[this]()
			{
				search(false);
			});
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &SchemaControlTabPage::filter);
	connect(m_searchButton,
			&QPushButton::clicked,
			this,
			[this]()
			{
				search(false);
			});
	connect(m_fullSearchButton,
			&QPushButton::clicked,
			this,
			[this]()
			{
				search(true);
			});
	connect(m_filterButton, &QPushButton::clicked, this, &SchemaControlTabPage::filter);
	connect(m_resetFilterButton, &QPushButton::clicked, this, &SchemaControlTabPage::resetFilter);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::addLogicSchema, this, &SchemaControlTabPage::addLogicSchema);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::searchSchemaForLm, this, &SchemaControlTabPage::searchSchemaForLm);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::openSchema, this, &SchemaControlTabPage::openFile);
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::viewSchema,
			[this](const DbFileInfo& file)
			{
				viewFile(file);
			});

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &SchemaControlTabPage::compareObject);

	return;
}


SchemaControlTabPage::~SchemaControlTabPage() {}

namespace
{
	// This needed to return the focus to the last active widget in SchemaControlTabPage when it was hidden.
	// Mainly it is return to the schema list.
	//
	struct HideEventFocusWidget : QObject
	{
		QWidget* widget = nullptr;
	};

	HideEventFocusWidget s_hideEventFocusWidget;
} // namespace

void SchemaControlTabPage::showEvent(QShowEvent* /*event*/)
{
	if (s_hideEventFocusWidget.widget != nullptr)
	{
		s_hideEventFocusWidget.widget->setFocus();
	}

	return;
}

void SchemaControlTabPage::hideEvent(QHideEvent* /*event*/)
{
	if (s_hideEventFocusWidget.widget != nullptr)
	{
		disconnect(s_hideEventFocusWidget.widget, nullptr, &s_hideEventFocusWidget, nullptr);
	}

	s_hideEventFocusWidget.widget = focusWidget();

	if (s_hideEventFocusWidget.widget != nullptr)
	{
		connect(s_hideEventFocusWidget.widget,
				&QObject::destroyed,
				&s_hideEventFocusWidget,
				[]()
				{
					s_hideEventFocusWidget.widget = nullptr;
				});
	}

	return;
}

bool SchemaControlTabPage::hasUnsavedSchemas() const
{
	bool result = false;
	for (const EditSchemaTabPage* editWidget : m_openedFiles)
	{
		result |= editWidget->modified();
	}

	return result;
}

bool SchemaControlTabPage::saveUnsavedSchemas()
{
	bool ok = true;

	for (EditSchemaTabPage* editWidget : m_openedFiles)
	{
		if (editWidget->modified() == true)
		{
			ok &= editWidget->saveWorkcopy();
		}
	}

	return ok;
}

bool SchemaControlTabPage::resetModified()
{
	bool ok = true;

	for (EditSchemaTabPage* editWidget : m_openedFiles)
	{
		editWidget->resetModified();
	}

	return ok;
}

void SchemaControlTabPage::refresh()
{
	Q_ASSERT(m_filesView);

	if (m_filesView != nullptr)
	{
		m_filesView->refreshFiles();
	}

	std::vector<int> openedFileIds;
	openedFileIds.reserve(m_openedFiles.size());
	std::transform(m_openedFiles.begin(),
				   m_openedFiles.end(),
				   std::back_inserter(openedFileIds),
				   [](EditSchemaTabPage* e)
				   {
					   return e->fileInfo().fileId();
				   });

	std::vector<DbFileInfo> openedFileInfos;
	db()->getFileInfo(&openedFileIds, &openedFileInfos, this);

	// Set read-only to file if it is open
	//
	for (auto editWidget : m_openedFiles)
	{
		if (editWidget->readOnly() == true)
		{
			// If schema is already read-only then it has some reason, do not change it.
			//
			continue;
		}

		auto it = std::find_if(openedFileInfos.begin(),
							   openedFileInfos.end(),
							   [editWidget](const DbFileInfo& fi)
							   {
								   return fi.fileId() == editWidget->fileInfo().fileId();
							   });

		if (it != openedFileInfos.end())
		{
			const DbFileInfo& fi = *it;

			bool checkedOut = (fi.state() == E::VcsState::CheckedOut &&
							   (fi.userId() == dbc()->currentUser().userId() || dbc()->currentUser().isAdministrator() == true));

			if (checkedOut == false) // We know that tab page in state read-write (see 'if' in the top of the loop).
			{
				editWidget->setReadOnly(true);
				editWidget->setFileInfo(fi);
			}
		}
		else
		{
			// File was deleted?
			//
			editWidget->setReadOnly(true);
		}
	}

	m_filesView->selectionChanged({}, {}); // To update actions

	return;
}

void SchemaControlTabPage::createToolBar()
{
	// Actions created in SchemaVileViewEx
	//
	m_toolBar->addAction(m_filesView->m_openAction);
	m_toolBar->addAction(m_filesView->m_viewAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_filesView->m_newSchemaAction);
	m_toolBar->addAction(m_filesView->m_newActuatorAction);
	m_toolBar->addAction(m_filesView->m_newFolderAction);
	m_toolBar->addAction(m_filesView->m_cloneFileAction);
	m_toolBar->addAction(m_filesView->m_deleteAction);
	// m_toolBar->addAction(m_filesView->m_moveFileAction);

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
	m_toolBar->addAction(m_filesView->m_behaviorAction);
	m_toolBar->addAction(m_filesView->m_propertiesAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_filesView->m_exportToPdfAction);
	m_toolBar->addAction(m_filesView->m_exportToAlbumAction);

	// m_toolBar->addSeparator();

	return;
}

std::shared_ptr<VFrame30::Schema> SchemaControlTabPage::createSchema(const DbFileInfo& parentFile) const
{
	if (parentFile.isNull() == true)
	{
		Q_ASSERT(parentFile.isNull() == false);
		return {};
	}

	// If parent  or it's parent... is $root$/Schemas/ApplicationLogic
	// the create als
	// clang-format off
	static const std::map<int, std::function<std::shared_ptr<VFrame30::Schema>()>> createSchemaMap = {
		{db()->systemFileId(DbDir::AppLogicDir), []() { return std::make_shared<VFrame30::LogicSchema>(); }},
		{db()->systemFileId(DbDir::MonitorSchemasDir), []() { return std::make_shared<VFrame30::MonitorSchema>(); }},
		{db()->systemFileId(DbDir::TuningSchemasDir), []() { return std::make_shared<VFrame30::TuningSchema>(); }},
		{db()->systemFileId(DbDir::UfblDir), []() { return std::make_shared<VFrame30::UfbSchema>(); }},
		{db()->systemFileId(DbDir::DiagSchemasDir), []() { return std::make_shared<VFrame30::DiagSchema>(); }},
		{db()->systemFileId(DbDir::VduSchemasDir), []() { return std::make_shared<VFrame30::VduSchema>(); }},
		{db()->systemFileId(DbDir::ActuatorsDir), []() { return std::make_shared<VFrame30::ActuatorSchema>(); }},
	};
	// clang-format on

	DbFileInfo lookForSystemParent = parentFile;

	do
	{
		if (auto it = createSchemaMap.find(lookForSystemParent.fileId()); it != createSchemaMap.end())
		{
			return it->second();
		}

		lookForSystemParent = m_filesView->filesModel().file(lookForSystemParent.parentId());
	} while (lookForSystemParent.isNull() == false);

	// What kind of schema suppose to be created?
	//
	Q_ASSERT(false);

	return {};
}

EditSchemaTabPage* SchemaControlTabPage::findOpenedFile(const DbFileInfo& file, bool readOnly)
{
	for (auto editSchema : m_openedFiles)
	{
		if (readOnly == false)
		{
			if (editSchema->fileInfo().fileId() == file.fileId() && editSchema->readOnly() == false)
			{
				return editSchema;
			}
		}
		else
		{
			if (editSchema->fileInfo().fileId() == file.fileId() && editSchema->readOnly() == true &&
				editSchema->fileInfo().changeset() == file.changeset())
			{
				return editSchema;
			}
		}
	}

	return nullptr;
}

void SchemaControlTabPage::removeFromOpenedList(EditSchemaTabPage* editTabPage)
{
	if (editTabPage == nullptr)
	{
		Q_ASSERT(editTabPage);
		return;
	}

	m_openedFiles.remove(editTabPage);
	return;
}

void SchemaControlTabPage::detachOrAttachWindow(EditSchemaTabPage* editTabPage)
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

	editTabPage->updateZoomAndScrolls(false, false);
	editTabPage->setVisible(true);
	editTabPage->activateWindow();

	return;
}

void SchemaControlTabPage::openFile(const DbFileInfo& file)
{
	if (file.isNull() == true)
	{
		Q_ASSERT(file.isNull() == false);
		return;
	}

	if (file.state() != E::VcsState::CheckedOut)
	{
		QMessageBox mb(this);
		mb.setText(tr("Check Out file for edit first."));
		mb.exec();
		return;
	}

	if (file.state() == E::VcsState::CheckedOut && file.userId() != db()->currentUser().userId())
	{
		QMessageBox mb(this);
		QString username = db()->username(file.userId());
		mb.setText(tr("File %1 is already checked out by user <b>%2</b>.").arg(file.fileName()).arg(username));
		mb.exec();
		return;
	}

	Q_ASSERT(file.state() == E::VcsState::CheckedOut && file.userId() == db()->currentUser().userId());

	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget != nullptr);
		return;
	}

	GlobalMessanger::instance().fireChangeCurrentTab(this->parentWidget()->parentWidget()->parentWidget());

	// Check if file already open, and activate it if it's so
	//
	if (auto editTabPage = findOpenedFile(file, false); editTabPage != nullptr)
	{
		// File already opened, check if it is opened for edit then activate this tab
		//
		if (editTabPage->readOnly() == false && editTabPage->fileInfo().fileId() == file.fileId())
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

	// Chose between schema file and actuator header file by extension.
	//
	if (isFileActuatorHeader(*out[0]))
	{
		auto actuatorHeader = VFrame30::ActuatorHeader::Create(out[0].get()->data());
		if (actuatorHeader == nullptr)
		{
			QMessageBox::critical(this, tr("Error"), tr("File %1 cannot be read or is corrupted.").arg(out[0]->fileName()));
			return;
		}

		// Show properties dialog for actuator header file
		//
		showActuatorHeaderProperties({out[0]}, false);
	}
	else
	{
		// Load schema file
		//
		std::shared_ptr<VFrame30::Schema> vf(VFrame30::Schema::Create(out[0].get()->data()));
		if (vf == nullptr)
		{
			QMessageBox::critical(this, tr("Error"), tr("File %1 cannot be read or is corrupted.").arg(out[0]->fileName()));
			return;
		}

		// Create TabPage and add it to the TabControl
		//
		DbFileInfo fi(*(out.front().get()));

		EditSchemaTabPage* editTabPage = new EditSchemaTabPage{tabWidget, vf, fi, db(), m_signalSetProvider};

		connect(editTabPage, &EditSchemaTabPage::vcsFileStateChanged, m_filesView, &SchemaFileView::slot_refreshFiles);
		connect(editTabPage, &EditSchemaTabPage::aboutToClose, this, &SchemaControlTabPage::removeFromOpenedList);
		connect(editTabPage, &EditSchemaTabPage::pleaseDetachOrAttachWindow, this, &SchemaControlTabPage::detachOrAttachWindow);
		connect(editTabPage, &EditSchemaTabPage::fileWasSaved, this, &SchemaControlTabPage::schemaWasSaved);

		Q_ASSERT(tabWidget->parent());

		SchemasTabPage* schemasTabPage = dynamic_cast<SchemasTabPage*>(tabWidget->parent());
		if (schemasTabPage == nullptr)
		{
			Q_ASSERT(dynamic_cast<SchemasTabPage*>(tabWidget->parent()));
			return;
		}

		connect(&GlobalMessanger::instance(), &GlobalMessanger::buildStarted, editTabPage, &EditSchemaTabPage::saveWorkcopy);

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

		editTabPage->updateZoomAndScrolls(true, false);

		m_openedFiles.push_back(editTabPage);
	}

	return;
}

void SchemaControlTabPage::viewFile(const DbFileInfo& file)
{
	if (file.isNull() == true)
	{
		Q_ASSERT(file.isNull() == false);
		return;
	}

	// Show chageset dialog
	//
	int changesetId = SelectChangesetDialog::getFileChangeset(db(), file, this);
	if (changesetId == -1)
	{
		return;
	}

	viewFile(file, changesetId);

	return;
}

void SchemaControlTabPage::viewFile(const DbFileInfo& file, int changesetId)
{
	GlobalMessanger::instance().fireChangeCurrentTab(this->parentWidget()->parentWidget()->parentWidget());

	if (changesetId == -1)
	{
		Q_ASSERT(changesetId != -1);
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

	if (fi.isFolder() == false && //
		(File::isSchemaFileExtension(fi.fileName()) == true || File::isSchemaTemplateFileExtension(fi.fileName()) == true))
	{
		viewSchemaFile(*out);
	}

	if (fi.isFolder() == false && isFileActuatorHeader(fi) == true)
	{
		viewActuatorHeaderFile(*out);
	}

	return;
}

void SchemaControlTabPage::viewSchemaFile(const DbFile& file)
{
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget != nullptr);
		return;
	}

	std::shared_ptr<VFrame30::Schema> vf(VFrame30::Schema::Create(file.data()));

	// Find the opened read only file with the same changeset
	//
	if (auto editTabPage = findOpenedFile(file, true); editTabPage != nullptr)
	{
		// File already opened, check if it is opened for edit then activate this tab
		//
		if (editTabPage->readOnly() == true && editTabPage->fileInfo().fileId() == file.fileId() &&
			editTabPage->fileInfo().changeset() == file.changeset())
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
	EditSchemaTabPage* editTabPage = new EditSchemaTabPage{tabWidget, vf, file, db(), m_signalSetProvider};

	connect(editTabPage, &EditSchemaTabPage::aboutToClose, this, &SchemaControlTabPage::removeFromOpenedList);
	connect(editTabPage, &EditSchemaTabPage::pleaseDetachOrAttachWindow, this, &SchemaControlTabPage::detachOrAttachWindow);

	editTabPage->setReadOnly(true);

	tabWidget->addTab(editTabPage, editTabPage->windowTitle());
	tabWidget->setCurrentWidget(editTabPage);

	editTabPage->updateZoomAndScrolls(true, false);

	m_openedFiles.push_back(editTabPage);
	return;
}

void SchemaControlTabPage::viewActuatorHeaderFile(const DbFile& file)
{
	showActuatorHeaderProperties({std::make_shared<DbFile>(file)}, true);
}

void SchemaControlTabPage::projectOpened()
{
	m_lastSelectedNewSchemaForLmFileId = db()->systemFileId(DbDir::AppLogicDir);
	setEnabled(true);
}

void SchemaControlTabPage::projectClosed()
{
	m_lastSelectedNewSchemaForLmFileId = -1;
	m_tagSelector->clear();
	setEnabled(false);
}

int SchemaControlTabPage::showSelectFolderDialog(int parentFileId, int currentSelectionFileId, bool showRootFile)
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

	if (bool ok = dbc()->getFileListTree(&files, parentFileId, true, this); ok == false)
	{
		return -1;
	}

	files.removeIf(
		[](const DbFileInfo& fi)
		{
			return fi.directoryAttribute() == false;
		});

	std::shared_ptr<DbFileInfo> schemaFile = files.rootFile(); // SchemaFile
	Q_ASSERT(schemaFile->directoryAttribute() == true);

	static QIcon staticFolderIcon(":/Images/Images/SchemaFolder.svg");
	const QIcon* const ptrToIcon = &staticFolderIcon;
	QTreeWidgetItem* treeItemToSelect = nullptr;

	std::function<void(std::shared_ptr<DbFileInfo>, QTreeWidgetItem*)> addChilderenFilesFunc =
		[&addChilderenFilesFunc, &files, treeWidget, currentSelectionFileId, &treeItemToSelect, ptrToIcon](
			std::shared_ptr<DbFileInfo> parent,
			QTreeWidgetItem* parentTreeItem)
	{
		Q_ASSERT(parent->isNull() == false);

		const auto& childeren = files.children(parent->fileId());

		for (auto file : childeren)
		{
			if (file->isNull() == true || file->directoryAttribute() == false)
			{
				Q_ASSERT(file->isNull() == false);
				Q_ASSERT(file->directoryAttribute() == true);
				return;
			}

			QTreeWidgetItem* treeItem = nullptr;

			if (parentTreeItem == nullptr)
			{
				treeItem = new QTreeWidgetItem(treeWidget, {file->fileName()}, file->fileId());
				treeWidget->addTopLevelItem(treeItem);
			}
			else
			{
				treeItem = new QTreeWidgetItem(parentTreeItem, {file->fileName()}, file->fileId());
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
		rootTreeItem = new QTreeWidgetItem(treeWidget, {schemaFile->fileName()}, schemaFile->fileId());
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

void SchemaControlTabPage::openSelectedFile()
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

void SchemaControlTabPage::viewSelectedFile()
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

void SchemaControlTabPage::schemaWasSaved(QString schemaDetails)
{
	if (m_filesView == nullptr)
	{
		Q_ASSERT(m_filesView != nullptr);
		return;
	}

	m_filesView->filesModel().updateShemaDetails(VFrame30::SchemaDetails{schemaDetails});
	return;
}

void SchemaControlTabPage::addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile)
{
	int parentFileId = showSelectFolderDialog(dbc()->systemFileId(DbDir::AppLogicDir), m_lastSelectedNewSchemaForLmFileId, true);
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
		QMessageBox::critical(
			this,
			qAppName(),
			tr("Can add Logic Schema only to '%1' or it's descendands.").arg(Db::File::systemDirToName(DbDir::AppLogicDir)));
		return;
	}

	// Set New Guid
	//
	schema->setGuid(QUuid::createUuid());
	int sequenceNo = db()->nextCounterValue();

	// Set default properties
	//
	schema->setSchemaId("APPSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0'));
	schema->setCaption("Caption " + QString::number(sequenceNo).rightJustified(6, '0'));

	schema->setDocWidth(420.0 / 25.4);
	schema->setDocHeight(297.0 / 25.4);

	if (VFrame30::LogicSchema* logicSchema = dynamic_cast<VFrame30::LogicSchema*>(schema.get()); logicSchema != nullptr)
	{
		logicSchema->setEquipmentIdList(deviceStrIds);
		logicSchema->setPropertyValue(Hardware::PropertyNames::lmDescriptionFile, QVariant(lmDescriptionFile));
	}

	// Show Schema Properties
	//
	CreateSchemaDialog propertiesDialog(schema, db(), this);

	if (propertiesDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	// --
	//
	addSchemaFile(schema, File::AlFileExtension, parentFile.fileId());

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

void SchemaControlTabPage::onAddSchemaFile()
{
	QModelIndexList selectedRows = m_filesView->selectionModel()->selectedRows();
	if (selectedRows.size() != 1)
	{
		Q_ASSERT(selectedRows.size() == 1);
		return;
	}

	QModelIndex selectedModelIndex = m_filesView->proxyModel().mapToSource(selectedRows.front());
	DbFileInfo selectedFile = m_filesView->filesModel().file(selectedModelIndex);

	DbFileInfo parentFile;

	// If folder selected, then create new file in this folder.
	// If ActuatorHeader is selected, then we can add schema directly to it, like to folder.
	//
	if (selectedFile.directoryAttribute() == true || isFileActuatorHeader(selectedFile) == true)
	{
		parentFile = selectedFile;
	}
	else
	{
		// If File selected, the create new file in the same folder as selected one
		//
		parentFile = m_filesView->filesModel().file(selectedFile.parentId());
	}

	if (parentFile.isNull() == true || (parentFile.directoryAttribute() == false && isFileActuatorHeader(parentFile) == false))
	{
		Q_ASSERT(parentFile.isNull() == false);
		Q_ASSERT(parentFile.directoryAttribute() || isFileActuatorHeader(parentFile));
		return;
	}

	return addSchema(parentFile);
}

void SchemaControlTabPage::addSchema(const DbFileInfo& parentFile)
{
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
		extension = File::AlFileExtension;
	}

	if (schema->isUfbSchema() == true)
	{
		defaultId = "UFBID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = File::UfbFileExtension;
	}

	if (schema->isMonitorSchema() == true)
	{
		defaultId = "MONITORSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = File::MvsFileExtension;
	}

	if (schema->isTuningSchema() == true)
	{
		defaultId = "TUNINGSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = File::TvsFileExtension;
	}

	if (schema->isDiagSchema() == true)
	{
		defaultId = "DIAGSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = File::DvsFileExtension;
	}

	if (schema->isVduSchema() == true)
	{
		defaultId = "VDUSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = File::VduFileExtension;
	}

	if (schema->isActuatorSchema() == true)
	{
		defaultId = "ACTUATORSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
		extension = File::ActuatorFileExtension;
	}

	Q_ASSERT(extension.isEmpty() == false);

	schema->setSchemaId(defaultId);

	// Set Caption
	//
	schema->setCaption("Caption " + QString::number(sequenceNo).rightJustified(6, '0'));

	// Set default EqupmnetIDs for LogicSchema
	//
	if (schema->isLogicSchema() == true)
	{
		auto logicSchema = schema->toLogicSchema();

		QString defaultEquipmentIds;
		dbc()->getUserProperty("SchemaEditor/NewLogicSchemaLastParam/EquipmentIds", &defaultEquipmentIds, this);
		if (defaultEquipmentIds.isEmpty() == true)
		{
			defaultEquipmentIds = "SYSTEMID_RACKID_CH01_MD00";
		}

		QString defaultLmDescriptionFile;
		dbc()->getUserProperty("SchemaEditor/NewLogicSchemaLastParam/LmDescriptionFile", &defaultLmDescriptionFile, this);
		if (defaultLmDescriptionFile.isEmpty() == true)
		{
			defaultLmDescriptionFile = "LM_SF41.xml";
		}

		logicSchema->setEquipmentIds(defaultEquipmentIds);
		logicSchema->setLmDescriptionFile(defaultLmDescriptionFile);
	}

	if (schema->isActuatorSchema() == true)
	{
		auto actuatorSchema = schema->toActuatorSchema();

		// Get ActuatorHeader for this schema.
		//
		{
			DbFileInfo actuatorHeaderFile = parentFile;

			while (actuatorHeaderFile.isNull() == false && actuatorHeaderFile.fileId() != dbc()->rootFileId())
			{
				if (isFileActuatorHeader(actuatorHeaderFile) == true)
				{
					break;
				}

				actuatorHeaderFile = m_filesView->filesModel().file(actuatorHeaderFile.parentId());
			}

			if (isFileActuatorHeader(actuatorHeaderFile) == true)
			{
				std::shared_ptr<DbFile> file;
				bool readOk = dbc()->getLatestVersion(actuatorHeaderFile, &file, this);
				if (readOk == true)
				{
					auto actuatorHeader = VFrame30::ActuatorHeader::Create(file->data());
					if (actuatorHeader != nullptr)
					{
						actuatorSchema->setActuatorTypeId(actuatorHeader->actuatorTypeId());
						actuatorSchema->setLmDescriptionFile(actuatorHeader->descriptionFile());
					}
				}
			}
			else
			{
				assert(isFileActuatorHeader(actuatorHeaderFile) == true);
			}
		}
	}

	// Set Width and Height
	//
	if (schema->unit() == SchemaUnit::Display)
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

	// --
	//
	CreateSchemaDialog propertiesDialog(schema, db(), this);

	if (propertiesDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	// Save default params for next time
	//
	if (schema->isLogicSchema() == true)
	{
		auto logicSchema = schema->toLogicSchema();
		dbc()->setUserProperty("SchemaEditor/NewLogicSchemaLastParam/EquipmentIds", logicSchema->equipmentIds(), this);
		dbc()->setUserProperty("SchemaEditor/NewLogicSchemaLastParam/LmDescriptionFile", logicSchema->lmDescriptionFile(), this);
	}

	addSchemaFile(schema, extension, parentFile.fileId());

	return;
}

void SchemaControlTabPage::onAddActuatorFile()
{
	QModelIndexList selectedRows = m_filesView->selectionModel()->selectedRows();
	if (selectedRows.size() != 1)
	{
		Q_ASSERT(selectedRows.size() == 1);
		return;
	}

	QModelIndex selectedModelIndex = m_filesView->proxyModel().mapToSource(selectedRows.front());
	DbFileInfo selectedFile = m_filesView->filesModel().file(selectedModelIndex);

	DbFileInfo parentFile;

	// If folder selected, then create new file in this folder
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

	if (parentFile.isNull() == true || parentFile.directoryAttribute() == false)
	{
		Q_ASSERT(parentFile.isNull() == false);
		Q_ASSERT(parentFile.directoryAttribute());
		return;
	}

	return addActuator(parentFile);
}

void SchemaControlTabPage::addActuator(const DbFileInfo& parentFile)
{
	auto actuatorHeader = std::make_shared<VFrame30::ActuatorHeader>();

	actuatorHeader->setActuatorTypeId("ACTUATORID_" + QString::number(db()->nextCounterValue()).rightJustified(6, '0'));
	actuatorHeader->setCaption("Valve");
	actuatorHeader->setAcmPresetName("PRESET_ACM1");

	{
		bool ok = false;
		QString value;

		ok = db()->getUserProperty("SchemaEditor/NewActuatorLastParam/PresetName", &value, this);
		if (ok == true && value.isEmpty() == false)
		{
			actuatorHeader->setAcmPresetName(value); // To update description file based on preset name
		}

		ok = db()->getUserProperty("SchemaEditor/NewActuatorLastParam/DescriptionFile", &value, this);
		if (ok == true && value.isEmpty() == false)
		{
			actuatorHeader->setDescriptionFile(value);
		}

		ok = db()->getUserProperty("SchemaEditor/NewActuatorLastParam/SubsystemID", &value, this);
		if (ok == true && value.isEmpty() == false)
		{
			actuatorHeader->setSubsystemId(value);
		}

		ok = db()->getUserProperty("SchemaEditor/NewActuatorLastParam/LmNumber", &value, this);
		if (ok == true && value.isEmpty() == false)
		{
			bool lmNumberOk = false;
			int lmNumber = value.toInt(&lmNumberOk);
			if (lmNumberOk == true)
			{
				actuatorHeader->setLmNumber(lmNumber + 1);
			}
		}
	}

	CreateActuatorDialog dialog{actuatorHeader, db(), this};
	auto result = dialog.exec();

	if (result == QDialog::Accepted)
	{
		addActuatorHeaderFile(actuatorHeader, File::ActuatorHeaderFileExtension, parentFile.fileId());

		// Save last used params.
		//
		db()->setUserProperty("SchemaEditor/NewActuatorLastParam/PresetName", actuatorHeader->acmPresetName(), this);
		db()->setUserProperty("SchemaEditor/NewActuatorLastParam/DescriptionFile", actuatorHeader->descriptionFile(), this);

		db()->setUserProperty("SchemaEditor/NewActuatorLastParam/SubsystemID", actuatorHeader->subsystemId(), this);
		db()->setUserProperty("SchemaEditor/NewActuatorLastParam/LmNumber", QString::number(actuatorHeader->lmNumber()), this);
	}

	return;
}


// Find the QModelIndex for FileID, and call addSchemaFileToDb
//
void SchemaControlTabPage::addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, int parentFileId)
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
void SchemaControlTabPage::addSchemaFileToDb(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, QModelIndex parentIndex)
{
	if (schema == nullptr)
	{
		Q_ASSERT(schema);
		return;
	}

	//  Save file in DB
	//
	if (fileExtension.isEmpty() == false && fileExtension.startsWith('.') == false)
	{
		fileExtension = '.' + fileExtension;
	}

	QByteArray data;
	schema->saveToByteArray(&data);

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

	file->setFileName(schema->schemaId() + fileExtension);
	file->setDetails(schema->details(QString{})); // Ignore path here
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

	if (bool ok = db()->addUniqueFile(file, parentFileId, db()->systemFileId(DbDir::SchemasDir), this); ok == false)
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
			m_filesView->selectionModel()->setCurrentIndex(addedProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows); //
		}
	}

	return;
}

// Find the QModelIndex for FileID, and call addSchemaFileToDb
//
void SchemaControlTabPage::addActuatorHeaderFile(std::shared_ptr<VFrame30::ActuatorHeader> actuatorHeader,
												 QString fileExtension,
												 int parentFileId)
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

	addActuatorHeaderFileToDb(actuatorHeader, fileExtension, parentIndex);

	return;
}

// Add file to DB
//
void SchemaControlTabPage::addActuatorHeaderFileToDb(std::shared_ptr<VFrame30::ActuatorHeader> actuatorHeader,
													 QString fileExtension,
													 QModelIndex parentIndex)
{
	if (actuatorHeader == nullptr)
	{
		Q_ASSERT(actuatorHeader);
		return;
	}

	//  Save file in DB
	//
	if (fileExtension.isEmpty() == false && fileExtension.startsWith('.') == false)
	{
		fileExtension = '.' + fileExtension;
	}

	QByteArray data;
	actuatorHeader->saveToByteArray(&data);

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

	file->setFileName(actuatorHeader->actuatorTypeId() + fileExtension);
	// file->setDetails(actuatorHeader->details(QString{})); // Ignore path here
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

	if (bool ok = db()->addUniqueFile(file, parentFileId, db()->systemFileId(DbDir::ActuatorsDir), this); //
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
			m_filesView->selectionModel()->setCurrentIndex(addedProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows); //
		}
	}

	return;
}

void SchemaControlTabPage::addFolder()
{
	// Folder can be created only for another folder or Actuator
	//
	QModelIndexList selectedRows = m_filesView->selectionModel()->selectedRows();
	if (selectedRows.size() != 1)
	{
		Q_ASSERT(selectedRows.size() == 1);
		return;
	}

	QModelIndex selectedModelIndex = m_filesView->proxyModel().mapToSource(selectedRows.front());
	DbFileInfo selectedFile = m_filesView->filesModel().file(selectedModelIndex);

	DbFileInfo parentFile;

	// If folder selected, then create new folder in the selected one
	//
	if (selectedFile.directoryAttribute() == true || isFileActuatorHeader(selectedFile) == true)
	{
		parentFile = selectedFile;
	}
	else
	{
		// If file is selected, the create new folder in the same folder as selected file
		//
		parentFile = m_filesView->filesModel().file(selectedFile.parentId());
	}

	if (parentFile.isNull() == true || parentFile.directoryAttribute() == false)
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

	if (bool ok = db()->addFile(file, parentFile.fileId(), this); // File may not be unique
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
			m_filesView->selectionModel()->setCurrentIndex(addedProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows); //
		}
	}

	return;
}

void SchemaControlTabPage::cloneFile()
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

	if (isFileActuatorHeader(*out) == true)
	{
		cloneActuatorHeader(*out);
	}
	else
	{
		cloneSchema(*out);
	}

	return;
}

void SchemaControlTabPage::cloneSchema(const DbFile& file)
{
	std::shared_ptr<VFrame30::Schema> schema(VFrame30::Schema::Create(file.data()));
	if (schema == nullptr)
	{
		Q_ASSERT(schema != nullptr);
		return;
	}

	// Get new SchemaID
	//
	bool ok = false;
	int globalCounter = db()->nextCounterValue();
	QString newSchemaId = QInputDialog::getText(this,
												qAppName(),
												tr("New SchemaID:"),
												QLineEdit::Normal,
												schema->schemaId() + QString::number(globalCounter),
												&ok,
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

	for (const auto& layer : schema->layers())
	{
		layer->setGuid(QUuid::createUuid());

		for (const SchemaItemPtr& item : layer->items())
		{
			item->setNewGuid();

			globalCounter = db()->nextCounterValue();
			item->setLabel(schema->schemaId() + "_" + QString::number(globalCounter));
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
			QMessageBox::critical(this,
								  qAppName(),
								  tr("Cannot clone schema, not all GUIDs were updated. Please, inform developers about this problem."));
			return;
		}
	}

	// Get folder for clonned schema
	//
	int parentFileId = showSelectFolderDialog(dbc()->systemFileId(DbDir::SchemasDir), file.parentId(), false);
	if (parentFileId == -1)
	{
		return;
	}

	addSchemaFile(schema, file.extension(), parentFileId);
	return;
}

void SchemaControlTabPage::cloneActuatorHeader(const DbFile& file)
{
	auto actuatorHeader = VFrame30::ActuatorHeader::Create(file.data());
	if (actuatorHeader == nullptr)
	{
		Q_ASSERT(actuatorHeader != nullptr);
		return;
	}

	// Get new typeID
	//
	bool ok = false;
	int globalCounter = db()->nextCounterValue();
	QString newTypeId = QInputDialog::getText(this,
											  qAppName(),
											  tr("New ActuatorTypeID:"),
											  QLineEdit::Normal,
											  actuatorHeader->actuatorTypeId() + QString::number(globalCounter),
											  &ok,
											  Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	if (ok == false || newTypeId.isEmpty() == true)
	{
		return;
	}

	// Set new IDs, labels, etc.
	//
	actuatorHeader->setActuatorTypeId(newTypeId);

	// Get folder for clonned actuator header.
	//
	int parentFileId = showSelectFolderDialog(dbc()->systemFileId(DbDir::ActuatorsDir), file.parentId(), true);
	if (parentFileId == -1)
	{
		return;
	}

	addActuatorHeaderFile(actuatorHeader, file.extension(), parentFileId);
	return;
}

void SchemaControlTabPage::deleteFiles()
{
	QModelIndexList selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi : selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> files = m_filesView->selectedFiles();

	if (files.empty() == true)
	{
		Q_ASSERT(files.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == static_cast<int>(files.size()));

	// --
	//
	std::vector<std::shared_ptr<DbFileInfo>> deleteFiles;
	deleteFiles.reserve(files.size());

	for (const std::shared_ptr<DbFileInfo>& f : files)
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
	mb.setInformativeText(tr("If files have not been checked in before they will be deleted permanently.\nIf files were checked in at "
							 "least one time they will be marked as deleted, to confirm operation perform Check In."));

	QString detailedText;
	for (auto f : deleteFiles)
	{
		detailedText += f->fileName() + "\n";
	}
	mb.setDetailedText(detailedText.trimmed());

	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (int mbResult = mb.exec(); mbResult == QMessageBox::Cancel)
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

void SchemaControlTabPage::moveFiles()
{
	QModelIndexList selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi : selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> files = m_filesView->selectedFiles();

	if (files.empty() == true)
	{
		Q_ASSERT(files.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == static_cast<int>(files.size()));

	// If schema is opened, can't move it
	//
	for (const auto& file : files)
	{
		auto foundTab = std::find_if(m_openedFiles.begin(),
									 m_openedFiles.end(),
									 [&file](const EditSchemaTabPage* tabPage)
									 {
										 Q_ASSERT(tabPage);
										 return tabPage->fileInfo().fileId() == file->fileId() && tabPage->readOnly() == false;
									 });

		if (foundTab != m_openedFiles.end())
		{
			EditSchemaTabPage* tab = *foundTab;
			QMessageBox::critical(
				this,
				qAppName(),
				tr("Can't move schema %1, as it is opened for edit. Close schema and repeat operation.").arg(tab->schema()->schemaId()));
			return;
		}
	}

	// --
	//
	std::vector<DbFileInfo> filesToMove;
	filesToMove.reserve(files.size());

	for (const std::shared_ptr<DbFileInfo>& f : files)
	{
		if (dbc()->isSystemFile(f->fileId()) == true || f->state() != E::VcsState::CheckedOut)
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
	int moveToFileId = showSelectFolderDialog(dbc()->systemFileId(DbDir::SchemasDir), filesToMove.front().parentId(), false);
	if (moveToFileId == -1)
	{
		return;
	}

	// Move files in DB
	//
	std::vector<DbFileInfo> movedFiles;

	if (bool ok = db()->moveFiles(filesToMove, moveToFileId, &movedFiles, this); ok == false)
	{
		return;
	}

	// Update model/view
	//
	std::vector<QModelIndex> addedIndexes;
	addedIndexes.reserve(selectedIndexes.size());

	if (bool ok = m_filesView->filesModel().moveFilesUpdate(selectedIndexes, moveToFileId, movedFiles, &addedIndexes); ok == false)
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

void SchemaControlTabPage::checkOutFiles()
{
	QModelIndexList selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi : selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> files = m_filesView->selectedFiles();
	if (files.empty() == true)
	{
		Q_ASSERT(files.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == static_cast<int>(files.size()));

	// --
	//
	std::vector<DbFileInfo> checkOutFiles;
	checkOutFiles.reserve(files.size());

	for (const std::shared_ptr<DbFileInfo>& f : files)
	{
		if (dbc()->isSystemFile(f->fileId()) == true)
		{
			continue;
		}

		if (f->state() == E::VcsState::CheckedIn)
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

	m_filesView->selectionChanged({}, {}); // To update actions

	return;
}

void SchemaControlTabPage::checkInFiles()
{
	QModelIndexList selectedIndexes = m_filesView->selectionModel()->selectedRows();
	for (QModelIndex& mi : selectedIndexes)
	{
		mi = m_filesView->proxyModel().mapToSource(mi);
	}

	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();
	if (selectedFiles.empty() == true)
	{
		Q_ASSERT(selectedFiles.empty() == false);
		return;
	}

	Q_ASSERT(selectedIndexes.size() == static_cast<int>(selectedFiles.size()));

	// --
	//
	std::vector<DbFileInfo> checkInFiles;
	checkInFiles.reserve(selectedFiles.size());

	for (const std::shared_ptr<DbFileInfo>& file : selectedFiles)
	{
		if (dbc()->isSystemFile(file->fileId()) == true)
		{
			continue;
		}

		if (file->state() == E::VcsState::CheckedIn)
		{
			continue;
		}

		if (file->userId() == db()->currentUser().userId() || db()->currentUser().isAdministrator() == true)
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

		auto it = std::find_if(checkInFiles.begin(),
							   checkInFiles.end(),
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

	// Here I left the same value key as in EditSchemaTabPage, to have the same behavior
	//
	bool checkInTree = QSettings{}.value("EditSchemaTabPage::checkInFile/checkInTree", false).toBool();

	bool ok = CheckInDialog::checkIn(checkInFiles, &updatedFiles, checkInTree, "SchemaID", db(), this, &checkInTree);
	if (ok == false)
	{
		return;
	}

	QSettings{}.setValue("EditSchemaTabPage::checkInFile/checkInTree", checkInTree);

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
	checkInFiles.erase(std::remove_if(checkInFiles.begin(),
									  checkInFiles.end(),
									  [](const auto& file)
									  {
										  return file.deleted();
									  }),
					   checkInFiles.end());

	// Set read-only to file if it is open
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

	m_filesView->selectionChanged({}, {}); // To update actions

	return;
}

void SchemaControlTabPage::undoChangesFiles()
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
		if (fi->state() == E::VcsState::CheckedOut &&
			(fi->userId() == db()->currentUser().userId() || db()->currentUser().isAdministrator() == true))
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
			if (editWidget->fileInfo().fileId() == fi.fileId() && editWidget->readOnly() == false)
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

void SchemaControlTabPage::showFileHistory()
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

void SchemaControlTabPage::showFileHistoryRecursive()
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

void SchemaControlTabPage::compareSelectedFile()
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

void SchemaControlTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	if (isVisible() == false)
	{
		return;
	}

	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension,
	// can compare next files
	//
	if (QString ext = QFileInfo(object.name()).suffix(); //
		File::isSchemaFileExtension(ext) == false && File::isSchemaTemplateFileExtension(ext) == false)
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

	for (const auto& targetLayer : target->layers())
	{
		for (const SchemaItemPtr& targetItem : targetLayer->items())
		{
			// Look for this item in source
			//
			SchemaItemPtr sourceItem = source->getItemById(targetItem->guid());

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
	for (const auto& sourceLayer : source->layers())
	{
		for (const SchemaItemPtr& sourceItem : sourceLayer->items())
		{
			// Look for this item in source
			//
			SchemaItemPtr targetItem = target->getItemById(sourceItem->guid());

			if (targetItem == nullptr)
			{
				// Item is found, so it was deleted in target
				//
				itemsActions[sourceItem->guid()] = CompareAction::Deleted;

				// Add item to target
				//
				bool layerFound = false;
				for (const auto& targetLayer : target->layers())
				{
					if (targetLayer->guid() == sourceLayer->guid())
					{
						targetLayer->pushBackItem(sourceItem);
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

	EditSchemaTabPage* compareTabPage = new EditSchemaTabPage(tabWidget, target, DbFileInfo(), db(), m_signalSetProvider);

	connect(compareTabPage, &EditSchemaTabPage::aboutToClose, this, &SchemaControlTabPage::removeFromOpenedList);
	connect(compareTabPage, &EditSchemaTabPage::pleaseDetachOrAttachWindow, this, &SchemaControlTabPage::detachOrAttachWindow);

	compareTabPage->setReadOnly(true);
	compareTabPage->setCompareWidget(true, source, target);
	compareTabPage->setCompareItemActions(itemsActions);

	compareTabPage->setWindowTitle("Compare " + target->schemaId());

	tabWidget->addTab(compareTabPage, compareTabPage->windowTitle());
	tabWidget->setCurrentWidget(compareTabPage);

	compareTabPage->updateZoomAndScrolls(true, false);

	m_openedFiles.push_back(compareTabPage);

	return;
}

void SchemaControlTabPage::exportWorkcopy()
{
	// Get files workcopies form the database
	//
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (auto file : selectedFiles)
	{
		if (file->state() == E::VcsState::CheckedOut && file->userId() == db()->currentUser().userId())
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
	QString dir = QFileDialog::getExistingDirectory(this,
													tr("Select Directory"),
													QString(),
													QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

void SchemaControlTabPage::importWorkcopy()
{
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file->state() == E::VcsState::CheckedOut && file->userId() == db()->currentUser().userId())
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

	if (fileInfo.state() != E::VcsState::CheckedOut || fileInfo.userId() != db()->currentUser().userId())
	{
		return;
	}

	// Select file
	//
	static QString path{"."};
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), path);
	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

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

void SchemaControlTabPage::exportToPdf()
{
	// Get selected files list
	//
	const std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();

	std::vector<DbFileInfo> files;

	for (auto& f : selectedFiles)
	{
		if (f->directoryAttribute() == true)
		{
			continue;
		}

		files.push_back(*f);
	}

	if (files.empty() == true)
	{
		return;
	}

	// Get export params
	//
	bool singleFile = false;
	QString singleFileName;

	QString pathName;
	Builder::SchemasReportOptions options = Builder::SchemasReportOptions::optionsForSingleSchema();

	if (files.size() == 1)
	{
		static QString path{"."};

		QString schemaFileName = files[0].fileName();
		qsizetype ptPos = schemaFileName.lastIndexOf('.');
		if (ptPos != -1)
		{
			schemaFileName.remove(ptPos, schemaFileName.length() - ptPos);
		}
		singleFileName = QFileDialog::getSaveFileName(this,
													  tr("Export to PDF"),
													  path + QDir::separator() + schemaFileName + ".pdf",
													  tr("Portable Documnet Format (*.pdf)"));
		if (singleFileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(singleFileName).path(); // store path for next time

		singleFile = true;
	}
	else
	{
		Builder::SchemasReportOptions storedOptions = Builder::SchemasReportOptions::optionsForSchemasAlbum(db());

		DialogSchemasExport d(
			storedOptions,
			QSettings{}.value("SchemaEditor/Export/SchemaPdfPath", QDir().toNativeSeparators(QDir::currentPath())).toString(),
			QSettings{}.value("SchemaEditor/Export/SchemaPdfFile", "Schemas.pdf").toString(),
			this);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		storedOptions = options = d.options();
		storedOptions.save(db());

		if (d.isSingleFile() == true)
		{
			singleFile = true;
			singleFileName = d.fileName();
			QSettings{}.setValue("SchemaEditor/Export/SchemaPdfFile", singleFileName);
		}
		else
		{
			pathName = d.pathName();
			QSettings{}.setValue("SchemaEditor/Export/SchemaPdfPath", pathName);
		}
	}

	SchemasReportGeneratorThread r(theAppSettings.serverHost(),
								   theAppSettings.serverPort(),
								   theAppSettings.serverUsername(),
								   theAppSettings.serverPassword(),
								   db()->currentProject().projectName(),
								   db()->currentUser().username(),
								   db()->currentUser().password(),
								   &m_signalSetProvider->signalSet(),
								   this,
								   options,
								   {});

	if (singleFile == true)
	{
		r.exportSchemasToSinglePdf(singleFileName, files);
	}
	else
	{
		r.exportSchemasToMultiplePdf(pathName, files);
	}

	return;
}

void SchemaControlTabPage::exportToAlbum()
{
	SchemasAlbumGenerator::createSchemasAlbums(db(), &m_signalSetProvider->signalSet(), this);
	return;
}

void SchemaControlTabPage::showFileProperties()
{
	std::vector<std::shared_ptr<DbFileInfo>> selectedFiles = m_filesView->selectedFiles();

	std::vector<DbFileInfo> requestFiles;
	requestFiles.reserve(selectedFiles.size());

	bool readOnly = true;

	for (const auto& file : selectedFiles)
	{
		if (file->isFolder() == true)
		{
			continue;
		}

		if (file->state() == E::VcsState::CheckedOut &&
			(file->userId() == db()->currentUser().userId() || db()->currentUser().isAdministrator() == true))
		{
			readOnly = false;
		}

		requestFiles.push_back(*file);
	}

	if (requestFiles.empty() == true)
	{
		return;
	}

	// If schema is opened, can't edit its' properties
	//
	for (const auto& file : selectedFiles)
	{
		auto foundTab = std::find_if(m_openedFiles.begin(),
									 m_openedFiles.end(),
									 [&file](const EditSchemaTabPage* tabPage)
									 {
										 Q_ASSERT(tabPage);
										 return tabPage->fileInfo().fileId() == file->fileId() && tabPage->readOnly() == false;
									 });

		if (foundTab != m_openedFiles.end())
		{
			EditSchemaTabPage* tab = *foundTab;
			QMessageBox::critical(this,
								  qAppName(),
								  tr("Can't edit %1 schema properties, as it is opened for edit. Close schema to edit it's properties.")
									  .arg(tab->schema()->schemaId()));
			return;
		}
	}

	// Load schemas
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool ok = db()->getLatestVersion(requestFiles, &out, this);
	if (ok == false || out.empty() == true)
	{
		return;
	}

	// Expected: all files either schemas or actuator headers.
	//
	bool allAreActuatorHeaders = std::ranges::all_of(out,
													 [](const std::shared_ptr<DbFile>& f)
													 {
														 return f->isFolder() == false && isFileActuatorHeader(*f) == true;
													 });
	bool allAreSchemaFiles = std::ranges::all_of(out,
												 [](const std::shared_ptr<DbFile>& f)
												 {
													 return f->isFolder() == false && File::isSchemaFileExtension(f->extension());
												 });

	// Only one type of files can be processed, so if there are mixed types, just return
	//
	if (allAreSchemaFiles == true && allAreActuatorHeaders == false)
	{
		showSchemaProperties(out, readOnly);
	}
	else if (allAreActuatorHeaders == true && allAreSchemaFiles == false)
	{
		showActuatorHeaderProperties(out, readOnly);
	}
	else
	{
		assert(allAreActuatorHeaders == true && allAreSchemaFiles == false);
	}

	return;
}

void SchemaControlTabPage::showSchemaProperties(const std::vector<std::shared_ptr<DbFile>>& files, bool readOnly)
{
	// Read schemas
	//
	std::vector<std::pair<std::shared_ptr<DbFile>, std::shared_ptr<VFrame30::Schema>>> schemas;
	schemas.reserve(files.size());

	QString initialSchemasId;

	auto filterFunc = [](const std::shared_ptr<DbFile>& file)
	{
		return file->isFolder() == false && File::isSchemaFileExtension(file->extension()) == true;
	};

	for (std::shared_ptr<DbFile> file : files | std::views::filter(filterFunc))
	{
		std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(file->data());
		if (schema == nullptr)
		{
			Q_ASSERT(schema != nullptr);
			return;
		}

		schemas.push_back({file, schema});

		initialSchemasId = schema->schemaId(); // Has sense if only one schema is selected
	}

	// Show schema properties dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("Schema(s) Properties"));
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	IdePropertyEditor* propertyEditor = new IdePropertyEditor(this, dbc());
	propertyEditor->setReadOnly(readOnly);
	propertyEditor->setDefaultSpecificPropertyCategory(tr("Params"));

	std::vector<std::shared_ptr<PropertyObject>> propertyObjects;
	propertyObjects.reserve(schemas.size());

	for (auto [schemaFile, schema] : schemas)
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
	propertyEditor->autoAdjustSplitterPosition();

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(propertyEditor);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	if (QSize s = QSettings().value("SchemaFileProperties/size").toSize(); s.isValid() == true)
	{
		d.resize(s);
	}
	else
	{
		d.resize(d.sizeHint() * 2);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// Show proprties dialog
	// and save result on accept
	//
	int result = d.exec();

	QSettings().setValue("SchemaFileProperties/size", d.size());

	if (result == QDialog::Accepted)
	{
		std::vector<std::shared_ptr<DbFile>> filesToSave;
		filesToSave.reserve(schemas.size());

		for (auto [file, schema] : schemas)
		{
			if (file->state() != E::VcsState::CheckedOut ||
				(file->userId() != db()->currentUser().userId() && db()->currentUser().isAdministrator() == false))
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
			file->setDetails(schema->details(QString{})); // Ignore path here

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

				if (bool ok = db()->renameFile(*file, newFileName, file.get(), this); //
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
				file->setDetails(schema->details(QString{})); // Ignore path here
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

void SchemaControlTabPage::showActuatorHeaderProperties(const std::vector<std::shared_ptr<DbFile>>& files, bool readOnly)
{
	// Read schemas
	//
	std::vector<std::pair<std::shared_ptr<DbFile>, std::shared_ptr<VFrame30::ActuatorHeader>>> actuatorHeaders;
	actuatorHeaders.reserve(files.size());

	QString initialId;

	auto filterFunc = [](const std::shared_ptr<DbFile>& file)
	{
		return file->isFolder() == false && isFileActuatorHeader(*file) == true;
	};

	for (auto file : files | std::views::filter(filterFunc))
	{
		std::shared_ptr<VFrame30::ActuatorHeader> actuatorHeader = VFrame30::ActuatorHeader::Create(file->data());
		if (actuatorHeader == nullptr)
		{
			Q_ASSERT(actuatorHeader != nullptr);
			return;
		}

		actuatorHeaders.push_back({file, actuatorHeader});

		initialId = actuatorHeader->actuatorTypeId(); // Has sense if only one actuator header is selected
	}

	// Show actuator header properties dialog
	//
	QDialog d{this};

	d.setWindowTitle(tr("Actuator Header(s) Properties"));
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	IdePropertyEditor* propertyEditor = new IdePropertyEditor(this, dbc());
	propertyEditor->setReadOnly(readOnly);
	propertyEditor->setDefaultSpecificPropertyCategory(tr("Params"));

	std::vector<std::shared_ptr<PropertyObject>> propertyObjects;
	propertyObjects.reserve(actuatorHeaders.size());

	for (auto actuatorHeader : actuatorHeaders | std::views::values)
	{
		Q_ASSERT(actuatorHeader);
		propertyObjects.push_back(actuatorHeader);

		// Now allow to edit ActuatorTypeID, only if one file is selected
		//
		auto actuatorTypeIdProp = actuatorHeader->propertyByCaption(VFrame30::PropertyNames::ActuatorTypeId);
		if (actuatorTypeIdProp == nullptr)
		{
			Q_ASSERT(actuatorTypeIdProp);
			continue;
		}

		actuatorTypeIdProp->setReadOnly(actuatorHeaders.size() != 1);
	}

	propertyEditor->setObjects(propertyObjects);
	propertyEditor->autoAdjustSplitterPosition();

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(propertyEditor);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	if (QSize s = QSettings().value("ActuatorHeaderFileProperties/size").toSize(); s.isValid() == true)
	{
		d.resize(s);
	}
	else
	{
		d.resize(d.sizeHint() * 2);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// Show proprties dialog
	// and save result on accept
	//
	int result = d.exec();

	QSettings().setValue("ActuatorHeaderFileProperties/size", d.size());

	if (result == QDialog::Accepted)
	{
		std::vector<std::shared_ptr<DbFile>> filesToSave;
		filesToSave.reserve(actuatorHeaders.size());

		for (auto [file, actuatorHeader] : actuatorHeaders)
		{
			if (file->state() != E::VcsState::CheckedOut ||
				(file->userId() != db()->currentUser().userId() && db()->currentUser().isAdministrator() == false))
			{
				continue;
			}

			QByteArray data;
			actuatorHeader->saveToByteArray(&data);

			if (data.isEmpty() == true)
			{
				Q_ASSERT(data.isEmpty() == false);
				return;
			}

			// --
			//
			file->swapData(data);
			// file->setDetails(actuatorHeader->details(QString{})); // Ignore path here

			filesToSave.push_back(file);
		}

		// Check if ActuatorTypeID was changed and we need to rename file
		//
		if (actuatorHeaders.size() == 1)
		{
			auto file = actuatorHeaders.front().first;
			auto actuatorHeader = actuatorHeaders.front().second;

			if (actuatorHeader->actuatorTypeId() != initialId)
			{
				// File must be renamed to new name
				//
				QString newFileName = actuatorHeader->actuatorTypeId() + "." + file->extension();

				if (bool ok = db()->renameFile(*file, newFileName, file.get(), this); //
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
				// file->setDetails(actuatorHeader->details(QString{})); // Ignore path here
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

void SchemaControlTabPage::showBehaviorEditor()
{
	DialogClientBehavior d(db(), this);
	d.exec();
}

void SchemaControlTabPage::ctrlF()
{
	Q_ASSERT(m_searchEdit);

	m_searchEdit->setFocus();
	m_searchEdit->selectAll();

	return;
}

void SchemaControlTabPage::search(bool fullSearch)
{
	// Search for text in schemas
	//
	Q_ASSERT(m_filesView);
	Q_ASSERT(m_searchEdit);

	QString searchText = m_searchEdit->text();

	if (searchText.trimmed().isEmpty() == true)
	{
		m_filesView->clearSelection();
		return;
	}

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("SchemaControlTabPage/SearchCompleter").toStringList();

	if (completerStringList.contains(searchText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(searchText);
		QSettings{}.setValue("SchemaControlTabPage/SearchCompleter", completerStringList);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_searchCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(completerStringList);
		}
	}

	// Search for text and select schemas with it
	//
	if (fullSearch == true)
	{
		m_filesView->fullSearchAndSelect(searchText);
	}
	else
	{
		m_filesView->searchAndSelect(searchText);
	}

	m_filesView->setFocus();

	return;
}

void SchemaControlTabPage::searchSchemaForLm(QString equipmentId)
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
	search(false);

	return;
}

void SchemaControlTabPage::filter()
{
	// Search for text in schemas
	//
	Q_ASSERT(m_filesView);
	Q_ASSERT(m_filterEdit);

	QString filterText = m_filterEdit->text().trimmed();

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("SchemaControlTabPage/SearchCompleter").toStringList();

	if (completerStringList.contains(filterText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(filterText);
		QSettings{}.setValue("SchemaControlTabPage/SearchCompleter", completerStringList);

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

void SchemaControlTabPage::resetFilter()
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

void SchemaControlTabPage::schemaTagsChanged()
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

void SchemaControlTabPage::tagSelectorHasChanges()
{
	// Filter schemas by tags
	//
	QStringList selectedTags = m_tagSelector->selectedTags();

	m_filesView->setTagFilter(selectedTags);
	m_filesView->setFocus();

	return;
}

const DbFileInfo& SchemaControlTabPage::parentFile() const
{
	return m_filesView->parentFile();
}
