#include "../include/DbStruct.h"

const char* const rootFileName = "$root$";		// root file name
const char* const AfblFileName = "AFBL";		// Application Functional Block Library
const char* const AlFileName = "AL";			// Application Logic Schemes
const char* const HcFileName = "HC";			// Hardware Configuratiun
const char* const HpFileName = "HP";			// Hardware Presets
const char* const WvsFileName = "WVS";			// Workflow Video Schemes
const char* const DvsFileName = "DVS";			// Diagnostics Video Schemes
const char* const McFileName = "MC";			// Module Configuration


//
//
//	VcsState
//
//

VcsState::VcsState() :
	m_state(CheckedIn)
{
}

VcsState::VcsState(VcsStateType s) :
	m_state(s)
{
}

QString VcsState::text() const
{
	switch (m_state)
	{
	case CheckedIn:
		return QObject::tr("Checked In");
	case CheckedOut:
		return QObject::tr("Checked Out");
	default:
		assert(false);
	}

	return QString();
}

bool operator== (const VcsState& s1, const VcsState& s2)
{
	return s1.m_state == s2.m_state;
}

bool operator!= (const VcsState& s1, const VcsState& s2)
{
	return s1.m_state != s2.m_state;
}

//
//
// VcsItemAction
//
//
VcsItemAction::VcsItemAction() :
	m_action(Added)
{
}

VcsItemAction::VcsItemAction(VcsItemActionType s) :
	m_action(s)
{
}

QString VcsItemAction::text() const
{
	switch (m_action)
	{
	case Added:
		return QObject::tr("Added");
	case Modified:
		return QObject::tr("Modified");
	case Deleted:
		return QObject::tr("Deleted");
	default:
		assert(false);
	}

	return QString();
}

int VcsItemAction::toInt() const
{
	return static_cast<int>(m_action);
}

bool operator== (const VcsItemAction& s1, const VcsItemAction& s2)
{
	return s1.m_action == s2.m_action;
}

bool operator!= (const VcsItemAction& s1, const VcsItemAction& s2)
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
}

DbUser::DbUser(int userId) :
	m_userId(userId)
{

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
//	DbFileInfo
//
//
DbFileInfo::DbFileInfo() :
	m_state(VcsState::CheckedIn),
	m_action(VcsItemAction::Added)
{
}

DbFileInfo::DbFileInfo(const DbFile& file)
{
	*this = file;
	m_size = file.size();
}

DbFileInfo::~DbFileInfo()
{
}

QString DbFileInfo::fileName() const
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

int DbFileInfo::fileId() const
{
	return m_fileId;
}

void DbFileInfo::setFileId(int value)
{
	m_fileId = value;
}

void DbFileInfo::resetFileId()
{
	m_fileId = -1;
}

bool DbFileInfo::hasFileId() const
{
	return m_fileId == -1;
}

int DbFileInfo::parentId() const
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

int DbFileInfo::changeset() const
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

const VcsState& DbFileInfo::state() const
{
	return m_state;
}

void DbFileInfo::setState(const VcsState& state)
{
	m_state = state;
}

const VcsItemAction& DbFileInfo::action() const
{
	return m_action;
}

void DbFileInfo::setAction(const VcsItemAction& action)
{
	m_action = action;
}

int DbFileInfo::userId() const
{
	return m_userId;
}

void DbFileInfo::setUserId(int value)
{
	m_userId = value;
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
// DbChangesetInfo
//
//
DbChangesetInfo::DbChangesetInfo()
{
}

int DbChangesetInfo::changeset() const
{
	return m_changesetId;
}

void DbChangesetInfo::setChangeset(int value)
{
	m_changesetId = value;
}

QDateTime DbChangesetInfo::date() const
{
	return m_date;
}

void DbChangesetInfo::setDate(const QDateTime& value)
{
	m_date = value;
}

void DbChangesetInfo::setDate(const QString& value)
{
	m_date = QDateTime::fromString(value, "yyyy-MM-ddTHH:mm:ss");
}

const VcsItemAction& DbChangesetInfo::action() const
{
	return m_action;
}

void DbChangesetInfo::setAction(const VcsItemAction& value)
{
	m_action = value;
}

int DbChangesetInfo::userId() const
{
	return m_userId;
}

void DbChangesetInfo::setUserId(int value)
{
	m_userId = value;
}

const QString& DbChangesetInfo::comment() const
{
	return m_comment;
}

void DbChangesetInfo::setComment(const QString& value)
{
	m_comment = value;
}
