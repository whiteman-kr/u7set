#include "../lib/DbStruct.h"

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "Signal.h"

//
//
//	VcsState
//
//
VcsState::VcsState()  noexcept:
	m_state(CheckedIn)
{
}

VcsState::VcsState(VcsStateType s) noexcept :
	m_state(s)
{
}

QString VcsState::text() const noexcept
{
	switch (m_state)
	{
	case CheckedIn:		return QStringLiteral("Checked In");
	case CheckedOut:	return QStringLiteral("Checked Out");
	default:
		assert(false);
	}

	return {};
}

VcsState::VcsStateType VcsState::value() const noexcept
{
	return m_state;
}

bool operator== (const VcsState& s1, const VcsState& s2) noexcept
{
	return s1.m_state == s2.m_state;
}

bool operator!= (const VcsState& s1, const VcsState& s2) noexcept
{
	return s1.m_state != s2.m_state;
}

bool operator< (const VcsState& s1, const VcsState& s2) noexcept
{
	return s1.m_state < s2.m_state;
}

//
//
// VcsItemAction
//
//
VcsItemAction::VcsItemAction() noexcept :
	m_action(Added)
{
}

VcsItemAction::VcsItemAction(VcsItemActionType s) noexcept :
	m_action(s)
{
}

QString VcsItemAction::text() const noexcept
{
	switch (m_action)
	{
	case Unknown:		return QStringLiteral("Unknown");
	case Added:			return QStringLiteral("Added");
	case Modified:		return QStringLiteral("Modified");
	case Deleted:		return QStringLiteral("Deleted");
	default:
		qDebug() << static_cast<int>(m_action);
		assert(false);
	}

	return {};
}

int VcsItemAction::toInt() const noexcept
{
	return static_cast<int>(m_action);
}

VcsItemAction::VcsItemActionType VcsItemAction::value() const noexcept
{
	return m_action;
}

bool operator== (const VcsItemAction& s1, const VcsItemAction& s2) noexcept
{
	return s1.m_action == s2.m_action;
}

bool operator!= (const VcsItemAction& s1, const VcsItemAction& s2) noexcept
{
	return s1.m_action != s2.m_action;
}

//
//
//	DbProject
//
//
DbProject::DbProject() :
	m_version(0)
{
}

DbProject::~DbProject()
{
}

QString DbProject::databaseName() const
{
	return m_databaseName;
}

void DbProject::setDatabaseName(const QString& databaseName)
{
	m_databaseName = databaseName;
}

QString DbProject::projectName() const
{
	return m_projectName;
}

void DbProject::setProjectName(const QString& projectName)
{
	m_projectName = projectName;
}

QString DbProject::description() const
{
	return m_description;
}

void DbProject::setDescription(const QString& description)
{
	m_description = description;
}

bool DbProject::safetyProject() const
{
	return m_safetyProject;
}

void DbProject::setSafetyProject(bool value)
{
	m_safetyProject = value;
}

int DbProject::version() const
{
	return m_version;
}

void DbProject::setVersion(int value)
{
	m_version = value;
}

bool DbProject::uppercaseAppSignalId() const
{
	return m_uppercaseAppSignalId;
}

void DbProject::setUppercaseAppSignalId(bool value)
{
	m_uppercaseAppSignalId = value;
}

//
// DbProjectProperties
//
DbProjectProperties::DbProjectProperties()
{
	Property* p = nullptr;

	p = ADD_PROPERTY_GETTER_SETTER(QString, Db::ProjectProperty::Description, true, DbProjectProperties::description, DbProjectProperties::setDescription);
	p->setCategory("Project");
	p->setDescription("Project description");

	p = ADD_PROPERTY_GETTER_SETTER(bool, Db::ProjectProperty::SafetyProject, true, DbProjectProperties::safetyProject, DbProjectProperties::setSafetyProject);
	p->setCategory("Project");
	p->setDescription("Safety Propject");

	p = ADD_PROPERTY_GETTER_SETTER(QString, Db::ProjectProperty::SuppressWarnings, true, DbProjectProperties::suppressWarningsAsString, DbProjectProperties::setSuppressWarnings);
	p->setCategory("Build");
	p->setDescription("Comma separated suppress warning list. Example: 4004, 4005, 2000");

	p = ADD_PROPERTY_GETTER_SETTER(bool, Db::ProjectProperty::UppercaseAppSignalId, true, DbProjectProperties::uppercaseAppSignalId, DbProjectProperties::setUppercaseAppSignalId);
	p->setCategory("Editor");
	p->setDescription("Uppercase AppSignalIDs, to apply option reopen project is required");

	p = ADD_PROPERTY_GETTER_SETTER(bool, Db::ProjectProperty::GenerateAppSignalsXml, true, DbProjectProperties::generateAppSignalsXml, DbProjectProperties::setGenerateAppSignalsXml);
	p->setCategory("Build");
	p->setDescription("Generate file AppSignals.xml on build");

	p = ADD_PROPERTY_GETTER_SETTER(bool, Db::ProjectProperty::GenerateExtraDebugInfo, true, DbProjectProperties::generateExtraDebugInfo, DbProjectProperties::setGenerateExtraDebugInfo);
	p->setCategory("Build");
	p->setDescription("Generate extra debug information on build");

	return;
}

QString DbProjectProperties::description() const
{
	return m_description;
}

void DbProjectProperties::setDescription(const QString& value)
{
	m_description = value;
}

bool DbProjectProperties::safetyProject() const
{
	return m_safetyProject;
}

void DbProjectProperties::setSafetyProject(bool value)
{
	m_safetyProject = value;
}

std::vector<int> DbProjectProperties::suppressWarnings() const
{
	return m_suppressWarnings;
}

QString DbProjectProperties::suppressWarningsAsString() const
{
	QString result;
	for (int w : m_suppressWarnings)
	{
		if (result.isEmpty() == false)
		{
			result += QStringLiteral(", ");
		}

		result += QString::number(w);
	}

	return result;
}

void DbProjectProperties::setSuppressWarnings(const QString& value)
{
	QStringList sl = value.split(QRegExp("\\W+"), Qt::SkipEmptyParts);

	m_suppressWarnings.clear();
	m_suppressWarnings.reserve(sl.size());

	for (QString& sw : sl)
	{
		bool ok = false;
		int warning = sw.toInt(&ok);

		if (ok == true)
		{
			m_suppressWarnings.push_back(warning);
		}
	}

	return;
}

bool DbProjectProperties::uppercaseAppSignalId() const
{
	return m_uppercaseAppSignalId;
}

void DbProjectProperties::setUppercaseAppSignalId(bool value)
{
	m_uppercaseAppSignalId = value;
}

bool DbProjectProperties::generateAppSignalsXml() const
{
	return m_generateAppSignalsXml;
}

void DbProjectProperties::setGenerateAppSignalsXml(bool value)
{
	m_generateAppSignalsXml = value;
}

bool DbProjectProperties::generateExtraDebugInfo() const
{
	return m_generateExtraDebugInfo;
}

void DbProjectProperties::setGenerateExtraDebugInfo(bool value)
{
	m_generateExtraDebugInfo = value;
}


//
//
//	DbUser
//
//
DbUser::DbUser()
{
	m_username = "";		// it makes qstring not null, null is bad for database
	m_firstName = "";
	m_lastName = "";
	m_password = "";
	m_newPassword = "";
}

DbUser::DbUser(int userId) :
	m_userId(userId)
{
	m_username = "";		// it makes qstring not null, null is bad for database
	m_firstName = "";
	m_lastName = "";
	m_password = "";
	m_newPassword = "";
}

bool DbUser::operator== (const DbUser& u) const
{
	return this->m_userId == u.m_userId;
}

bool DbUser::operator!= (const DbUser& u) const
{
	return this->m_userId != u.m_userId;
}

int DbUser::userId() const
{
	return m_userId;
}

void DbUser::setUserId(int value)
{
	m_userId = value;
}

const QDateTime& DbUser::date() const
{
	return m_date;
}

void DbUser::setDate(const QDateTime& value)
{
	m_date = value;
}

void DbUser::setDate(const QString& value)
{
	m_date = QDateTime::fromString(value);
}

const QString& DbUser::username() const
{
	return m_username;
}

void DbUser::setUsername(const QString& value)
{
	m_username = value.isNull() ? "" : value;
}

const QString& DbUser::firstName() const
{
	return m_firstName;
}

void DbUser::setFirstName(const QString& value)
{
	m_firstName = value.isNull() ? "" : value;
}

const QString& DbUser::lastName() const
{
	return m_lastName;
}

void DbUser::setLastName(const QString& value)
{
	m_lastName = value.isNull() ? "" : value;
}

const QString& DbUser::password() const
{
	return m_password;
}

void DbUser::setPassword(const QString& value)
{
	m_password = value.isNull() ? "" : value;
}

const QString& DbUser::newPassword() const
{
	return m_newPassword;
}

void DbUser::setNewPassword(const QString& value)
{
	m_newPassword = value.isNull() ? "" : value;
}


bool DbUser::isAdminstrator() const
{
	return m_administrator;
}

void DbUser::setAdministrator(bool value)
{
	m_administrator = value;
}

bool DbUser::isReadonly() const
{
	return m_readonly;
}

void DbUser::setReadonly(bool value)
{
	m_readonly = value;
}

bool DbUser::isDisabled() const
{
	return m_disabled;
}

void DbUser::setDisabled(bool value)
{
	m_disabled = value;
}

//
//
//	DbFileTree
//
//
DbFileTree::DbFileTree(const std::vector<std::shared_ptr<DbFileInfo>>& files, int rootFileId)
{
	for (const std::shared_ptr<DbFileInfo>& file : files)
	{
		addFile(file);
	}

	setRoot(rootFileId);

	return;
}

DbFileTree::DbFileTree(const std::map<int, std::shared_ptr<DbFileInfo>>& files, int rootFileId)
{
	for (const auto&[fileId, file] : files)
	{
		Q_UNUSED(fileId);
		addFile(file);
	}

	setRoot(rootFileId);

	return;
}

DbFileTree::DbFileTree(DbFileTree&& src)
{
	operator=(std::move(src));
}

DbFileTree& DbFileTree::operator=(DbFileTree&& src)
{
	m_fileIdToChildren = std::move(src.m_fileIdToChildren);
	m_files = std::move(src.m_files);

	m_rootFileId = src.m_rootFileId;
	src.m_rootFileId = -1;

	return *this;
}


void DbFileTree::clear()
{
	m_fileIdToChildren.clear();
	m_files.clear();
	m_rootFileId = -1;
}

int DbFileTree::size() const
{
	return static_cast<int>(m_files.size());
}

bool DbFileTree::empty() const
{
	return m_files.empty();
}

bool DbFileTree::isDbFile() const
{
	if (m_files.empty() == true)
	{
		return false;
	}

	bool result = std::dynamic_pointer_cast<DbFile>(m_files.cbegin()->second) != nullptr;
	return result;
}

bool DbFileTree::isDbFileInfo() const
{
	if (m_files.empty() == true)
	{
		return false;
	}

	bool result = std::dynamic_pointer_cast<DbFileInfo>(m_files.cbegin()->second) != nullptr;
	return result;
}

bool DbFileTree::isRoot(int fileId) const
{
	return fileId == m_rootFileId;
}

bool DbFileTree::isRoot(const DbFileInfo& fileInfo) const
{
	return fileInfo.fileId() == m_rootFileId;
}

bool DbFileTree::hasFile(int fileId) const
{
	return m_files.find(fileId) != std::cend(m_files);
}

std::shared_ptr<DbFileInfo> DbFileTree::rootFile()
{
	auto it = m_files.find(m_rootFileId);
	if (it == std::end(m_files))
	{
		return {};
	}
	else
	{
		return it->second;
	}
}

std::shared_ptr<DbFileInfo> DbFileTree::rootFile() const
{
	auto it = m_files.find(m_rootFileId);
	if (it == std::end(m_files))
	{
		return {};
	}
	else
	{
		return it->second;
	}
}

std::shared_ptr<DbFileInfo> DbFileTree::file(int fileId)
{
	auto it = m_files.find(fileId);
	if (it == std::end(m_files))
	{
		return {};
	}
	else
	{
		return it->second;
	}
}

std::shared_ptr<DbFileInfo> DbFileTree::file(int fileId) const
{
	auto it = m_files.find(fileId);
	if (it == std::end(m_files))
	{
		return {};
	}
	else
	{
		assert(it->second->fileId() == fileId);
		return it->second;
	}
}

QString DbFileTree::filePath(int fileId) const
{
	QStringList pathList;
	std::shared_ptr<DbFileInfo> f = file(fileId);

	while (f != nullptr && f->parentId() != m_rootFileId)
	{
		f = file(f->parentId());

		if (f != nullptr)
		{
			pathList.push_front(f->fileName());
		}
	}

	return QChar('/') + pathList.join(QChar('/'));
}

const std::map<int, std::shared_ptr<DbFileInfo>>& DbFileTree::files() const
{
	return m_files;
}

std::vector<DbFileInfo> DbFileTree::toVector(bool excludeRoot) const
{
	std::vector<DbFileInfo> fileList;
	fileList.reserve(m_files.size());

	for (auto&[fileId, fileInfo] : m_files)
	{
		if (excludeRoot == true && fileId == rootFileId())
		{
			continue;
		}

		fileList.push_back(*fileInfo);
	}

	return fileList;
}

std::vector<DbFileInfo> DbFileTree::toVectorIf(std::function<bool(const DbFileInfo&)> pred) const
{
	std::vector<DbFileInfo> fileList;
	fileList.reserve(m_files.size());

	for (auto&[fileId, fileInfo] : m_files)
	{
		if (pred(*fileInfo) == false)
		{
			continue;
		}

		fileList.push_back(*fileInfo);
	}

	return fileList;
}

std::vector<std::shared_ptr<DbFileInfo> > DbFileTree::toVectorOfSharedPointers(bool excludeRoot) const
{
	std::vector<std::shared_ptr<DbFileInfo>> fileList;
	fileList.reserve(m_files.size());

	for (auto&[fileId, fileInfo] : m_files)
	{
		if (excludeRoot == true && fileId == rootFileId())
		{
			continue;
		}

		fileList.push_back(fileInfo);
	}

	return fileList;
}

bool DbFileTree::hasChildren(int fileId) const
{
	auto it = m_fileIdToChildren.find(fileId);
	if (it == m_fileIdToChildren.end())
	{
		return false;
	}

	const auto& ch = it->second;
	assert(ch.m_fileId == fileId);

	return ch.m_children.empty() == false;
}

const std::vector<std::shared_ptr<DbFileInfo>>& DbFileTree::children(int parentId) const
{
	auto it = m_fileIdToChildren.find(parentId);
	if (it == m_fileIdToChildren.end())
	{
		static const std::vector<std::shared_ptr<DbFileInfo>> dummy;
		return dummy;
	}

	const auto& ch = it->second;
	assert(ch.m_fileId == parentId);

	return ch.m_children;
}

const std::vector<std::shared_ptr<DbFileInfo>>& DbFileTree::children(const DbFileInfo& fileInfo) const
{
	return children(fileInfo.fileId());
}

const std::vector<std::shared_ptr<DbFileInfo>>& DbFileTree::children(const std::shared_ptr<DbFileInfo>& fileInfo) const
{
	if (fileInfo == nullptr)
	{
		assert(fileInfo != nullptr);

		static const std::vector<std::shared_ptr<DbFileInfo>> dummy;
		return dummy;
	}

	return children(fileInfo->fileId());
}

std::shared_ptr<DbFileInfo> DbFileTree::child(int parentId, int index) const
{
	auto it = m_fileIdToChildren.find(parentId);
	if (it == m_fileIdToChildren.end())
	{
		return {};
	}

	const auto& ch = it->second;
	assert(ch.m_fileId == parentId);

	if (index >= ch.m_children.size())
	{
		assert(index < ch.m_children.size());
		return {};
	}

	return ch.m_children.at(index);
}

std::shared_ptr<DbFileInfo> DbFileTree::child(const DbFileInfo& parentFileInfo, int index) const
{
	return child(parentFileInfo.fileId(), index);
}

std::shared_ptr<DbFileInfo> DbFileTree::child(const std::shared_ptr<DbFileInfo>& parentFileInfo, int index) const
{
	if (parentFileInfo == nullptr)
	{
		assert(parentFileInfo);
		return {};
	}

	return child(parentFileInfo->fileId(), index);
}

int DbFileTree::rootChildrenCount() const
{
	assert(m_rootFileId != -1);
	return childrenCount(m_rootFileId);
}

int DbFileTree::childrenCount(int parentFileId) const
{
	auto it = m_fileIdToChildren.find(parentFileId);
	if (it == m_fileIdToChildren.end())
	{
		return 0;
	}

	const auto& ch = it->second;
	assert(ch.m_fileId == parentFileId);

	return static_cast<int>(ch.m_children.size());
}

int DbFileTree::indexInParent(int fileId) const
{

	// Get the file itself
	//
	auto fit = m_files.find(fileId);
	if (fit == std::end(m_files))
	{
		assert(fit != std::end(m_files));
		return -1;
	}

	auto file = fit->second;
	if (file == nullptr)
	{
		assert(file);
		return -1;
	}

	// Find file's position in it's parent
	//
	auto it = m_fileIdToChildren.find(file->parentId());
	if (it == m_fileIdToChildren.end())
	{
		assert(it != m_fileIdToChildren.end());	// Why do we asking index in parent for this fileId, it really can be an error
		return -1;
	}

	const auto& ch = it->second;
	assert(ch.m_fileId == file->parentId());

	int index = 0;
	for (const auto& fileChild : ch.m_children)
	{
		if (fileChild->fileId() == fileId)
		{
			return index;
		}

		index ++;
	}

	return -1;
}

int DbFileTree::indexInParent(const DbFileInfo& fileInfo) const
{
	return indexInParent(fileInfo.fileId());
}

int DbFileTree::calcIf(int startFromFileId, std::function<int(const DbFileInfo&)> pred) const
{
	auto it = m_files.find(startFromFileId);
	if (it == m_files.end())
	{
		qDebug() << "DbFileTree::calcIf: Cant find file" << startFromFileId;
		assert(it != m_files.end());
		return 0;
	}

	const DbFileInfo& file = *it->second;

	int result = pred(file);

	auto fileChildren = children(file);
	for (auto& child : fileChildren)
	{
		result += calcIf(child->fileId(), pred);
	}

	return result;
}

void DbFileTree::setRoot(int rootFileId)
{
	Q_ASSERT(m_files.count(rootFileId) == 1);
	m_rootFileId = rootFileId;
}

int DbFileTree::rootFileId() const
{
	return m_rootFileId;
}

void DbFileTree::addFile(const DbFileInfo& fileInfo)
{
	std::shared_ptr<DbFileInfo> sp = std::make_shared<DbFileInfo>(fileInfo);
	return addFile(sp);
}

void DbFileTree::addFile(std::shared_ptr<DbFileInfo> fileInfo)
{
	// Add File
	//
	m_files[fileInfo->fileId()] = fileInfo;

	// Add children record to parent
	//
	int parentId = fileInfo->parentId();

	FileChildren& fileChildren = m_fileIdToChildren[parentId];

	if (fileChildren.m_children.empty() == true)
	{
		fileChildren.m_fileId = parentId;							// Just created
	}
	else
	{
		assert(fileChildren.m_fileId == parentId);
	}

	fileChildren.m_children.push_back(fileInfo);

	return;
}

bool DbFileTree::removeFile(int fileId)
{
	auto fit = m_files.find(fileId);
	if (fit == std::end(m_files))
	{
		assert(fit != std::end(m_files));
		return false;
	}

	std::shared_ptr<DbFileInfo>& fileInfo = fit->second;
	assert(fileId == fileInfo->fileId());

	// Recursively delete children
	//
	if (auto it = m_fileIdToChildren.find(fileInfo->fileId());
		it == m_fileIdToChildren.end())
	{
		// It is possible that file has not any children
		//
	}
	else
	{
		FileChildren& ch = it->second;
		assert(ch.m_fileId == fileInfo->fileId());

		for (std::shared_ptr<DbFileInfo>& fc : ch.m_children)
		{
			removeFile(fc->fileId());
		}
	}

	// Remove from children record
	//
	if (auto it = m_fileIdToChildren.find(fileInfo->parentId());
		it == m_fileIdToChildren.end())
	{
		assert(it != m_fileIdToChildren.end());
	}
	else
	{
		FileChildren& ch = it->second;
		assert(ch.m_fileId == fileInfo->parentId());

		// Delete file itself from its' parent
		//
		ch.m_children.erase(std::remove_if(ch.m_children.begin(),
										   ch.m_children.end(),
										   [fileId](const auto& f)
										   {
												return f->fileId() == fileId;
										   }),
							ch.m_children.end());
	}

	// Remove from m_files
	//
	m_files.erase(fit);

	return true;
}

bool DbFileTree::removeFile(const DbFileInfo& fileInfo)
{
	return removeFile(fileInfo.fileId());
}

bool DbFileTree::removeFile(std::shared_ptr<DbFileInfo> fileInfo)
{
	return removeFile(fileInfo->fileId());
}

bool DbFileTree::removeFilesWithExtension(QString ext)
{
	// Remove all file with extension
	//
	if (ext.isEmpty() == true)
	{
		return false;
	}

	// Find all files with extension
	//
	std::vector<std::shared_ptr<DbFileInfo>> filesToRemove;
	filesToRemove.reserve(64);

	for (auto[fileId, fileInfo] : m_files)
	{
		if (fileId != fileInfo->fileId())
		{
			assert(fileId != fileInfo->fileId());
			continue;
		}

		if (fileInfo->extension().compare(ext, Qt::CaseInsensitive) == 0)
		{
			filesToRemove.push_back(fileInfo);
		}
	}

	// Remove all files with extension
	//
	bool ok = true;
	for (std::shared_ptr<DbFileInfo> file : filesToRemove)
	{
		ok &= removeFile(file);
	}

	return ok;
}

bool DbFileTree::removeIf(std::function<bool(const DbFileInfo&)> pred)
{
	std::vector<std::shared_ptr<DbFileInfo>> filesToRemove;
	filesToRemove.reserve(64);

	for (auto[fileId, fileInfo] : m_files)
	{
		if (fileId != fileInfo->fileId())
		{
			assert(fileId != fileInfo->fileId());
			continue;
		}

		if (pred(*fileInfo) == true)
		{
			filesToRemove.push_back(fileInfo);
		}
	}

	// Remove files
	//
	bool ok = true;
	for (std::shared_ptr<DbFileInfo> file : filesToRemove)
	{
		ok &= removeFile(file);
	}

	return true;
}


//
//
//	DbFileInfo
//
//
DbFileInfo::DbFileInfo() noexcept :
	m_state(VcsState::CheckedIn),
	m_action(VcsItemAction::Added),
	m_details("{}")
{
}

DbFileInfo::DbFileInfo(const DbFile& file) noexcept
{
	*this = file;
	m_size = file.size();
}

DbFileInfo::DbFileInfo(int fileId) noexcept :
	DbFileInfo{}
{
	m_fileId = fileId;
}


DbFileInfo::~DbFileInfo()
{
}

void DbFileInfo::trace() const
{
	qDebug() << "File:";
	qDebug() << "	Name: " << m_fileName;
	qDebug() << "	ID: " << m_fileId;
	qDebug() << "	ParentID: " << m_parentId;
	qDebug() << "	Changeset: " << m_changeset;
	qDebug() << "	State: " << m_state.text();
	qDebug() << "	Attributes: " << m_attributes;

	return;
}

const QString& DbFileInfo::fileName() const noexcept
{
	return m_fileName;
}

void DbFileInfo::setFileName(const QString& value)
{
	m_fileName = value;
}

QString DbFileInfo::extension() const noexcept
{
	int pointPos = m_fileName.lastIndexOf('.');
	if (pointPos == -1)
	{
		return {};
	}

	return m_fileName.right(m_fileName.size() - pointPos - 1);
}

int DbFileInfo::size() const
{
	// If we have any date, return th size of the data, else we retrurn fake size
	return m_size;
}

void DbFileInfo::setSize(int size)
{
	m_size = size;
}

int DbFileInfo::fileId() const noexcept
{
	return m_fileId;
}

void DbFileInfo::setFileId(int value)
{
	m_fileId = value;
}

void DbFileInfo::resetFileId()
{
	m_fileId = DbFileInfo::Null;
}

bool DbFileInfo::hasFileId() const
{
	return m_fileId != DbFileInfo::Null;
}

bool DbFileInfo::isNull() const noexcept
{
	return m_fileId == DbFileInfo::Null;
}

int DbFileInfo::parentId() const noexcept
{
	return m_parentId;
}

void DbFileInfo::setParentId(int value)
{
	m_parentId = value;
}

bool DbFileInfo::deleted() const
{
	return m_deleted;
}

void DbFileInfo::setDeleted(bool value)
{
	m_deleted = value;
}

int DbFileInfo::changeset() const noexcept
{
	return m_changeset;
}

void DbFileInfo::setChangeset(int value)
{
	m_changeset = value;
}

QDateTime DbFileInfo::created() const
{
	return m_created;
}

void DbFileInfo::setCreated(const QDateTime& value)
{
	m_created = value;
}

void DbFileInfo::setCreated(const QString& value)
{
	QDateTime dt = QDateTime::fromString(value, "yyyy-MM-ddThh:mm:ss");
	//QDateTime dt = QDateTime::fromString(value, Qt::RFC2822Date);
	setCreated(dt);
}

QDateTime DbFileInfo::lastCheckIn() const
{
	return m_lastCheckIn;
}

void DbFileInfo::setLastCheckIn(const QDateTime& value)
{
	m_lastCheckIn = value;
}

void DbFileInfo::setLastCheckIn(const QString& value)
{
	QDateTime dt = QDateTime::fromString(value, "yyyy-MM-ddTHH:mm:ss");
	setLastCheckIn(dt);
}

const VcsState& DbFileInfo::state() const noexcept
{
	return m_state;
}

void DbFileInfo::setState(const VcsState& state)
{
	m_state = state;
}

const VcsItemAction& DbFileInfo::action() const noexcept
{
	return m_action;
}

void DbFileInfo::setAction(const VcsItemAction& action)
{
	m_action = action;
}

int DbFileInfo::userId() const noexcept
{
	return m_userId;
}

void DbFileInfo::setUserId(int value)
{
	m_userId = value;
}

const QString& DbFileInfo::details() const noexcept
{
	return m_details;
}

void DbFileInfo::setDetails(const QString& value)
{
	if (value.isEmpty() == true)
	{
		m_details = QStringLiteral("{}");
	}
	else
	{
		m_details = value;
	}

	return;
}

qint32 DbFileInfo::attributes() const
{
	return m_attributes;
}

void DbFileInfo::setAttributes(qint32 value)
{
	m_attributes = value;
}

bool DbFileInfo::isFolder() const
{
	return m_attrDirectory;
}

bool DbFileInfo::directoryAttribute() const
{
	return m_attrDirectory;
}

void DbFileInfo::setDirectoryAttribute(bool value)
{
	m_attrDirectory = value ? 1 : 0;
}

QString DbFileInfo::fullPathToFileName(const QString& fullPathName)
{
	int pos = fullPathName.lastIndexOf('/');
	if (pos == -1)
	{
		return fullPathName;
	}

	QString result = fullPathName.right(fullPathName.size() - pos - 1);
	return result;
}

bool operator< (const DbFileInfo& a, const DbFileInfo& b)
{
	// Used in SchemaTabPage for map
	// DO NOT add to this m_state, as it will break finding open windows in SchemaTab
	//
	quint64 ax = (static_cast<quint64>(a.m_fileId) << 32) | static_cast<quint64>(a.m_changeset);
	quint64 bx = (static_cast<quint64>(b.m_fileId) << 32) | static_cast<quint64>(b.m_changeset);
	return ax < bx;
}

//
//
//	DbFile
//
//
DbFile::DbFile() :
	DbFileInfo()
{
}


DbFile::DbFile(const DbFileInfo& fileInfo) :
	DbFileInfo(fileInfo)
{
}

bool DbFile::readFromDisk(const QString& fileName)
{
	QFile file(fileName);

	bool ok = file.open(QIODevice::ReadOnly);
	if (ok == false)
	{
		return false;
	}

	m_data = file.readAll();

	//m_fileId =	Leave the same
	m_size = m_data.size();
	m_fileName = QFileInfo(file).fileName();
	m_lastCheckIn = QDateTime();

	return true;
}

bool DbFile::writeToDisk(const QString& directory) const
{
	QString fileName = directory + "/" + m_fileName;

	QFile file(fileName);
	bool openResult = file.open(QIODevice::WriteOnly);

	if (openResult == false)
	{
		qDebug() << Q_FUNC_INFO << " Can't open file " + fileName + " for writing.";
		return false;
	}

	qint64 result = file.write(m_data);

	return result != -1;
}

void DbFile::convertToDatabaseString(QString* str)
{
	assert(str);

	str->clear();
	str->reserve(m_data.size() * 2 + 256);
	str->append("E'\\\\x");

const static char* rawhex = {"000102030405060708090a0b0c0d0e0f"
							"101112131415161718191a1b1c1d1e1f"
							"202122232425262728292a2b2c2d2e2f"
							"303132333435363738393a3b3c3d3e3f"
							"404142434445464748494a4b4c4d4e4f"
							"505152535455565758595a5b5c5d5e5f"
							"606162636465666768696a6b6c6d6e6f"
							"707172737475767778797a7b7c7d7e7f"
							"808182838485868788898a8b8c8d8e8f"
							"909192939495969798999a9b9c9d9e9f"
							"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
							"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
							"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
							"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
							"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
							"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"};

    QString hex(rawhex);
    const QChar* hexptr = hex.data();

	int fileSize = m_data.size();
	const char* dataptr = m_data.constData();

	for (int i = 0; i < fileSize; i++)
	{
		unsigned int asbyte = static_cast<uint8_t>(*dataptr) & 0xFF;
		str->append(hexptr + asbyte*2, 2);

		dataptr ++;
	}

	str->append("'");
	return;
}

void DbFile::convertToDatabaseString(const QByteArray& data, QString* result)
{
	assert(result);

	result->clear();
	result->reserve(data.size() * 2 + 256);
	result->append("E'\\\\x");

const static char* rawhex = {"000102030405060708090a0b0c0d0e0f"
							"101112131415161718191a1b1c1d1e1f"
							"202122232425262728292a2b2c2d2e2f"
							"303132333435363738393a3b3c3d3e3f"
							"404142434445464748494a4b4c4d4e4f"
							"505152535455565758595a5b5c5d5e5f"
							"606162636465666768696a6b6c6d6e6f"
							"707172737475767778797a7b7c7d7e7f"
							"808182838485868788898a8b8c8d8e8f"
							"909192939495969798999a9b9c9d9e9f"
							"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
							"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
							"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
							"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
							"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
							"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"};

	QString hex(rawhex);
	const QChar* hexptr = hex.data();

	int fileSize = data.size();
	const char* dataptr = data.constData();

	for (int i = 0; i < fileSize; i++)
	{
		unsigned int asbyte = static_cast<uint8_t>(*dataptr) & 0xFF;
		result->append(hexptr + asbyte*2, 2);

		dataptr ++;
	}

	result->append("'");
	return;
}

DbFile& DbFile::operator= (const DbFileInfo& fileInfo)
{
	*(static_cast<DbFileInfo*>(this)) = fileInfo;			// Downcast to DbFileInfo to call just DbFileInfo::operator=();
	return *this;
}

const QByteArray& DbFile::data() const
{
	return m_data;
}

//QByteArray& DbFile::data()
//{
//	return m_data;
//}

void DbFile::setData(const QByteArray& data)
{
	m_data = data;
	m_size = m_data.size();
}

void DbFile::setData(QByteArray&& data)
{
	m_data = std::move(data);
	m_size = m_data.size();
}

void DbFile::swapData(QByteArray& data)
{
	m_data.swap(data);
	m_size = m_data.size();
}

void DbFile::clearData()
{
	m_data.clear();
	m_size = 0;
}

int DbFile::size() const
{
	// If we have any date, return th size of the data, else we retrurn fake size
	return m_data.size();
}

//
//
// DbChangeset
//
//
DbChangeset::DbChangeset()
{
}

DbChangeset::~DbChangeset()
{
}

int DbChangeset::changeset() const
{
	return m_changesetId;
}

void DbChangeset::setChangeset(int value)
{
	m_changesetId = value;
}

QDateTime DbChangeset::date() const
{
	return m_date;
}

void DbChangeset::setDate(const QDateTime& value)
{
	m_date = value;
}

void DbChangeset::setDate(const QString& value)
{
	m_date = QDateTime::fromString(value, "yyyy-MM-ddTHH:mm:ss");
}

const VcsItemAction& DbChangeset::action() const
{
	return m_action;
}

void DbChangeset::setAction(const VcsItemAction& value)
{
	m_action = value;
}

int DbChangeset::userId() const
{
	return m_userId;
}

void DbChangeset::setUserId(int value)
{
	m_userId = value;
}

const QString& DbChangeset::username() const
{
	return m_username;
}

void DbChangeset::setUsername(const QString& value)
{
	m_username = value;
}

const QString& DbChangeset::comment() const
{
	return m_comment;
}

void DbChangeset::setComment(const QString& value)
{
	m_comment = value;
}

//
//
// DbChangesetDetails
//
//
DbChangesetDetails::DbChangesetDetails() :
	DbChangeset()
{
	m_objects.reserve(64);
}

DbChangesetDetails::~DbChangesetDetails()
{
}

const std::vector<DbChangesetObject>& DbChangesetDetails::objects() const
{
	return m_objects;
}

void DbChangesetDetails::addObject(const DbChangesetObject& object)
{
	m_objects.push_back(object);
}


DbChangesetObject::DbChangesetObject()
{
}

DbChangesetObject::DbChangesetObject(const DbFileInfo& file) :
	m_type(Type::File),
	m_id(file.fileId()),
	m_name(file.fileName()),
	m_caption(file.fileName()),
	m_action(file.action()),
	m_parent(QString::number(file.parentId()))
{
}

DbChangesetObject::DbChangesetObject(const Signal& signal) :
	m_type(Type::Signal),
	m_id(signal.ID()),
	m_name(signal.appSignalID()),
	m_caption(signal.caption()),
	m_action(VcsItemAction::Modified)
{
}


DbChangesetObject::~DbChangesetObject()
{
}

DbChangesetObject::Type DbChangesetObject::type() const
{
	return m_type;
}

void DbChangesetObject::setType(DbChangesetObject::Type value)
{
	m_type = value;
}

bool DbChangesetObject::isFile() const
{
	return m_type == Type::File;
}

bool DbChangesetObject::isSignal() const
{
	return m_type == Type::Signal;
}

int DbChangesetObject::id() const
{
	return m_id;
}

void DbChangesetObject::setId(int value)
{
	m_id = value;
}

QString DbChangesetObject::name() const
{
	return m_name;
}

void DbChangesetObject::setName(const QString& value)
{
	m_name = value;
}

QString DbChangesetObject::caption() const
{
	return m_caption;
}

void DbChangesetObject::setCaption(const QString& value)
{
	m_caption = value;
}

VcsItemAction DbChangesetObject::action() const
{
	return m_action;
}

void DbChangesetObject::setAction(VcsItemAction value)
{
	m_action = value;
}

QString DbChangesetObject::parent() const
{
	return m_parent;
}

void DbChangesetObject::setParent(const QString& value)
{
	m_parent = value;
}

QString DbChangesetObject::fileMoveText() const
{
	return m_fileMoveText;
}

void DbChangesetObject::setFileMoveText(const QString& value)
{
	m_fileMoveText = value;
}

QString DbChangesetObject::fileRenameText() const
{
	return m_fileRenameText;
}

void DbChangesetObject::setFileRenameText(const QString& value)
{
	m_fileRenameText = value;
}
