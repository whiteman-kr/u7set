#include "../lib/DbStruct.h"

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "Signal.h"

const char* const rootFileName = "$root$";				// root file name
const char* const AfblFileName = "AFBL";				// Application Functional Block Library
const char* const UfblFileName = "UFBL";				// User Functional Block Library
const char* const AlFileName = "AL";					// Application Logic Schemas
const char* const HcFileName = "HC";					// Hardware Configuratiun
const char* const HpFileName = "HP";					// Hardware Presets
const char* const MvsFileName = "MVS";					// Monitor Video Schemas
const char* const DvsFileName = "DVS";					// Diagnostics Video Schemas
const char* const McFileName = "MC";					// Module Configuration
const char* const ConnectionsFileName = "CONNECTIONS";	// Connections
const char* const BusTypesFileName = "BUSTYPES";		// BustTypes
const char* const EtcFileName = "ETC";					//

const char* const SignalPropertyBehaviorFileName = "SignalPropertyBehavior.csv";

const char* const AlFileExtension = "als";				// Application Logic schema file extension
const char* const AlTemplExtension = "templ_als";		// Application Logic schema template file extnesion

const char* const UfbFileExtension = "ufb";				// User Functional Block schema file extnesion
const char* const UfbTemplExtension = "templ_ufb";		// User Functional Block template file extnesion

const char* const MvsFileExtension = "mvs";				// Monitor schema file extnesion
const char* const MvsTemplExtension = "templ_mvs";		// Monitor schema template file extnesion

const char* const DvsFileExtension = "dvs";				// Diagnostics schema file extnesion
const char* const DvsTemplExtension = "templ_dvs";		// Diagnostics schema template file extnesion

const char* const OclFileExtension = "ocl";				// (Optical) Connection Link
const char* const BusFileExtension = "bus_type";		// Bus type

const char* const AppSignalFileExtension = "asg";		// Application signal file extention (::Proto::AppSignal message)
const char* const AppSignalSetFileExtension = "asgs";	// Application signals set file extention (::Proto::AppSignalSet message)
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
	case Deleted:		QStringLiteral("Deleted");
	default:
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

int DbProject::version() const
{
	return m_version;
}

void DbProject::setVersion(int value)
{
	m_version = value;
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
DbFileTree::DbFileTree(DbFileTree&& src)
{
	operator=(std::move(src));
}

DbFileTree& DbFileTree::operator=(DbFileTree&& src)
{
	m_parentIdToChildren = std::move(src.m_parentIdToChildren);
	m_files = std::move(src.m_files);

	assert(m_files.size() == m_parentIdToChildren.size());

	m_rootFileId = src.m_rootFileId;
	src.m_rootFileId = -1;

	return *this;
}


void DbFileTree::clear()
{
	m_parentIdToChildren.clear();
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

const std::map<int, std::shared_ptr<DbFileInfo>>& DbFileTree::files() const
{
	return m_files;
}

std::vector<std::shared_ptr<DbFileInfo>> DbFileTree::children(int parentId) const
{
	std::vector<std::shared_ptr<DbFileInfo>> result;
	result.reserve(16);

	auto[pitBegin, pitEnd] = m_parentIdToChildren.equal_range(parentId);

	for (auto pit = pitBegin; pit != pitEnd; ++pit)
	{
		std::shared_ptr<DbFileInfo> sp = pit->second;

		if (sp == nullptr)
		{
			assert(sp != nullptr);
			return result;
		}

		if (sp->parentId() != parentId)
		{
			assert(sp->parentId() != parentId);
			continue;
		}

		result.push_back(sp);
	}

	return result;
}

std::vector<std::shared_ptr<DbFileInfo>> DbFileTree::children(const DbFileInfo& fileInfo) const
{
	return children(fileInfo.fileId());
}

std::vector<std::shared_ptr<DbFileInfo>> DbFileTree::children(const std::shared_ptr<DbFileInfo>& fileInfo) const
{
	if (fileInfo == nullptr)
	{
		assert(fileInfo != nullptr);
		return {};
	}

	return children(fileInfo->fileId());
}

std::shared_ptr<DbFileInfo> DbFileTree::child(int parentId, int index) const
{
	auto[pitBegin, pitEnd] = m_parentIdToChildren.equal_range(parentId);

	for (auto pit = pitBegin; pit != pitEnd && index >= 0; ++pit, --index)
	{
		if (index == 0)
		{
			const std::shared_ptr<DbFileInfo>& sp = pit->second;

			if (sp == nullptr)
			{
				assert(sp != nullptr);
				return {};
			}

			if (sp->parentId() != parentId)
			{
				assert(sp->parentId() != parentId);
				return {};
			}

			return sp;
		}
	}

	assert(false);	// wrong index
	return {};
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
	size_t cc = m_parentIdToChildren.count(parentFileId);
	return static_cast<int>(cc);
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
	auto[pitBegin, pitEnd] = m_parentIdToChildren.equal_range(file->parentId());

	int index = 0;
	for (auto pit = pitBegin; pit != pitEnd; ++pit, ++index)
	{
		std::shared_ptr<DbFileInfo> sp = pit->second;

		if (sp->fileId() == fileId)
		{
			return index;
		}
	}

	return -1;
}

int DbFileTree::indexInParent(const DbFileInfo& fileInfo) const
{
	return indexInParent(fileInfo.fileId());
}

void DbFileTree::setRoot(int rootFileId)
{
	assert(m_files.count(rootFileId) == 1);
	m_rootFileId = rootFileId;
}

int DbFileTree::rootFileId() const
{
	return m_rootFileId;
}

void DbFileTree::addFile(const DbFileInfo& fileInfo)
{
	std::shared_ptr<DbFileInfo> sp = std::make_shared<DbFileInfo>(fileInfo);

	m_files[fileInfo.fileId()] = sp;
	m_parentIdToChildren.insert({fileInfo.parentId(), sp});

	assert(m_files.size() == m_parentIdToChildren.size());

	return;
}

void DbFileTree::addFile(std::shared_ptr<DbFileInfo> fileInfo)
{
	m_files[fileInfo->fileId()] = fileInfo;
	m_parentIdToChildren.insert({fileInfo->parentId(), fileInfo});

	assert(m_files.size() == m_parentIdToChildren.size());

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

	// m_parentIdToChildren is multimap,.
	// so find the reange with key (parentId),
	// iterate range, and delete the right one
	//
	auto[pitBegin, pitEnd] = m_parentIdToChildren.equal_range(fileInfo->parentId());
	if (pitBegin == std::end(m_parentIdToChildren))
	{
		assert(false);
		return false;
	}

	bool deleted = false;
	for (auto pit = pitBegin; pit != pitEnd; ++pit)
	{
		if (pit->second == fileInfo)
		{
			m_parentIdToChildren.erase(pit);
			deleted = true;
			break;
		}
	}

	if (deleted == false)
	{
		// Not found
		//
		assert(deleted);
		return false;
	}

	// Remove from m_files
	//
	m_files.erase(fit);

	assert(m_files.size() == m_parentIdToChildren.size());

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

DbFileInfo::~DbFileInfo()
{
}

void DbFileInfo::trace() const
{
	qDebug() << "File:";
	qDebug() << "	Name: " << m_fileName;
	qDebug() << "	ID: " << m_fileId;
	qDebug() << "	ParentID: " << m_parentId;
}

const QString& DbFileInfo::fileName() const noexcept
{
	return m_fileName;
}

void DbFileInfo::setFileName(const QString& value)
{
	m_fileName = value;
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
	m_details = value;

	if (m_details.isEmpty())
	{
		m_details = "{}";
	}

	return;
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

QByteArray& DbFile::data()
{
	return m_data;
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

