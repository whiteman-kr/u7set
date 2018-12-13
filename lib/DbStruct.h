#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <map>
#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QtSql/QSqlRecord>

class Signal;

// System files names
//
extern const char* const RootFileName;			// root file name
extern const char* const AfblFileName;			// Application Functional Block Library

extern const char* const SchemasFileName;		// Schemas root fie
extern const char* const UfblFileName;			// User Functional Block Library
extern const char* const AlFileName;			// Application Logic Schemas
extern const char* const MvsFileName;			// Monitor Video Schemas
extern const char* const TvsFileName;			// Tuning Video Schemas
extern const char* const DvsFileName;			// Diagnostics Video Schemas

extern const char* const HcFileName;			// Hardware Configuratiun
extern const char* const HpFileName;			// Hardware Presets
extern const char* const McFileName;			// Modules Configurations
extern const char* const ConnectionsFileName;	// Connections
extern const char* const BusTypesFileName;		// Bus types
extern const char* const EtcFileName;			// Etc file name

extern const char* const SignalPropertyBehaviorFileName;			// SignalPropertyBehavior.csv file name

extern const char* const AlFileExtension;		// Application Logic schema file extnesion
extern const char* const AlTemplExtension;		// Application Logic schema template file extnesion

extern const char* const UfbFileExtension;		// User Functional Block schema file extnesion
extern const char* const UfbTemplExtension;		// User Functional Block template file extnesion

extern const char* const MvsFileExtension;		// Monitor schema file extnesion
extern const char* const MvsTemplExtension;		// Monitor schema template file extnesion

extern const char* const DvsFileExtension;		// Diagnostics schema file extnesion
extern const char* const DvsTemplExtension;		// Diagnostics schema template file extnesion

extern const char* const OclFileExtension;		// (Optical) Connection Link
extern const char* const BusFileExtension;		// Bus types

extern const char* const AppSignalFileExtension;	// Application signal file extention (::Proto::AppSignal message)
extern const char* const AppSignalSetFileExtension;	// Application signals set file extention (::Proto::AppSignalSet message)

//
//
// VcsState
//
//
class VcsState
{
public:
	enum VcsStateType
	{
		CheckedIn,					// File has no any action, it's normal state
		CheckedOut
	};

	VcsState() noexcept;
	VcsState(VcsStateType s) noexcept;

	QString text() const noexcept;

private:
	VcsStateType m_state;

	friend bool operator== (const VcsState& s1, const VcsState& s2) noexcept;
	friend bool operator!= (const VcsState& s1, const VcsState& s2) noexcept;
	friend bool operator< (const VcsState& s1, const VcsState& s2) noexcept;
};

bool operator== (const VcsState& s1, const VcsState& s2) noexcept;
bool operator!= (const VcsState& s1, const VcsState& s2) noexcept;
bool operator<  (const VcsState& s1, const VcsState& s2) noexcept;

//
//
// VcsItemAction
//
//
class VcsItemAction
{
public:
	enum VcsItemActionType
	{
		Unknown = 0,		// Don't change values, they are stored in DB
		Added = 1,			// Don't change values, they are stored in DB
		Modified = 2,		// Don't change values, they are stored in DB
		Deleted = 3			// Don't change values, they are stored in DB
	};

	VcsItemAction() noexcept;
	VcsItemAction(VcsItemActionType s) noexcept;

	QString text() const noexcept;
	int toInt() const noexcept;

	VcsItemActionType value() const noexcept;

private:
	VcsItemActionType m_action;

	friend bool operator== (const VcsItemAction& s1, const VcsItemAction& s2) noexcept;
	friend bool operator!= (const VcsItemAction& s1, const VcsItemAction& s2) noexcept;
};


// signal management error codes
// returns in ObjectState.errCode field
//
const int	ERR_SIGNAL_OK = 0,
			ERR_SIGNAL_IS_NOT_CHECKED_OUT = 1,
			ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER = 2,
			ERR_SIGNAL_DELETED = 3,
			ERR_SIGNAL_NOT_FOUND = 4,

			ERR_SIGNAL_EXISTS = 100;

struct ObjectState
{
	int id;
	bool deleted;
	bool checkedOut;
	int action;
	int userId;
	int errCode;
};

//
//
// DbProject
//
//
class DbProject
{
public:
	DbProject();
	virtual ~DbProject();

public:
	QString databaseName() const;
	void setDatabaseName(const QString& databaseName);

	QString projectName() const;
	void setProjectName(const QString& projectName);

	QString description() const;
	void setDescription(const QString& description);

	int version() const;
	void setVersion(int value);

protected:
	QString m_databaseName;
	QString m_projectName;
	QString m_description;
	int m_version;
};

struct UpgradeItem
{
    QString upgradeFileName;
	QString text;
};

//
//
// DbUser
//
//
class DbUser
{
public:
	DbUser();
	explicit DbUser(int userId);

public:
	bool operator== (const DbUser& u) const;
	bool operator!= (const DbUser& u) const;

public:
	int userId() const;
	void setUserId(int value);

	const QDateTime& date() const;
	void setDate(const QDateTime& value);
	void setDate(const QString& value);

	const QString& username() const;
	void setUsername(const QString& value);

	const QString& firstName() const;
	void setFirstName(const QString& value);

	const QString& lastName() const;
	void setLastName(const QString& value);

	const QString& password() const;
	void setPassword(const QString& value);

	const QString& newPassword() const;
	void setNewPassword(const QString& value);

	bool isAdminstrator() const;
	void setAdministrator(bool value);

	bool isReadonly() const;
	void setReadonly(bool value);

	bool isDisabled() const;
	void setDisabled(bool value);

private:
	int m_userId = 0;
	QDateTime m_date;

	QString m_username;
	QString m_firstName;
	QString m_lastName;

	QString m_password;
	QString m_newPassword;				// Required for setting new password

	bool m_administrator = false;
	bool m_readonly = false;
	bool m_disabled = false;
};

//
// DbFileTree
//
class DbFileInfo;

class DbFileTree
{
public:
	DbFileTree() = default;
	DbFileTree(const DbFileTree&) = default;
	DbFileTree& operator=(const DbFileTree&) = default;

	DbFileTree(DbFileTree&& src);
	DbFileTree& operator=(DbFileTree&& src);

public:
//	enum class SortBy
//	{
//		FileId,
//		FileName,
//		DetailsCaption
//		FileState,
//		FileUser,
//		FileAction,
//	};

public:
	void clear();

	int size() const;
	bool empty() const;

	bool isDbFile() const;		// true if contains DbFile whith data or tree is empty
	bool isDbFileInfo() const;	// true if contains DbFileInfo or tree is empty

	bool isRoot(int fileId) const;
	bool isRoot(const DbFileInfo& fileInfo) const;

	bool hasFile(int fileId) const;

	std::shared_ptr<DbFileInfo> rootFile();
	std::shared_ptr<DbFileInfo> rootFile() const;

	std::shared_ptr<DbFileInfo> file(int fileId);
	std::shared_ptr<DbFileInfo> file(int fileId) const;

	const std::map<int, std::shared_ptr<DbFileInfo>>& files() const;

	std::vector<std::shared_ptr<DbFileInfo>> children(int parentId) const;
	std::vector<std::shared_ptr<DbFileInfo>> children(const DbFileInfo& fileInfo) const;
	std::vector<std::shared_ptr<DbFileInfo>> children(const std::shared_ptr<DbFileInfo>& fileInfo) const;

	std::shared_ptr<DbFileInfo> child(int parentId, int index) const;
	std::shared_ptr<DbFileInfo> child(const DbFileInfo& parentFileInfo, int index) const;
	std::shared_ptr<DbFileInfo> child(const std::shared_ptr<DbFileInfo>& parentFileInfo, int index) const;

	int rootChildrenCount() const;
	int childrenCount(int parentFileId) const;

	int indexInParent(int fileId) const;
	int indexInParent(const DbFileInfo& fileId) const;

	// Modifying structure
	//
	void setRoot(int rootFileId);
	int rootFileId() const;

	void addFile(const DbFileInfo& fileInfo);
	void addFile(std::shared_ptr<DbFileInfo> fileInfo);

	bool removeFile(int fileId);
	bool removeFile(const DbFileInfo& fileInfo);
	bool removeFile(std::shared_ptr<DbFileInfo> fileInfo);

	bool removeFilesWithExtension(QString ext);

private:
	// WARNING, assigment move is present, adding new member, modify operator=(DbFileTree&&)!!!
	//
	std::multimap<int, std::shared_ptr<DbFileInfo>> m_parentIdToChildren;	// Key is parent, values are its' parent children
	std::map<int, std::shared_ptr<DbFileInfo>> m_files;						// Key if fileId, value is DbFile(Info) object
	int m_rootFileId = -1;
	// WARNING, assigment move is present, adding new member, modify operator=(DbFileTree&&)!!!
	//
};

//
//
// DbFileInfo
//
//
class DbFile;

class DbFileInfo
{
public:
	DbFileInfo() noexcept;
	DbFileInfo(const DbFileInfo& fileInfo) noexcept = default;
	DbFileInfo(const DbFile& file) noexcept;
	virtual ~DbFileInfo();

	// Methods
	//
public:
	void trace() const;

	// Properties
	//
public:
	const QString& fileName() const noexcept;
	void setFileName(const QString& value);

	QString extension() const noexcept;

	int fileId() const noexcept;
	void setFileId(int value);
	void resetFileId();
	bool hasFileId() const;

	bool isNull() const noexcept;

	int parentId() const noexcept;
	void setParentId(int value);

	virtual int size() const;
	void setSize(int size);

	bool deleted() const;
	void setDeleted(bool value);

	int changeset() const noexcept;
	void setChangeset(int value);

	QDateTime created() const;
	void setCreated(const QDateTime& value);
	void setCreated(const QString& value);

	QDateTime lastCheckIn() const;
	void setLastCheckIn(const QDateTime& value);
	void setLastCheckIn(const QString& value);

	const VcsState& state() const noexcept;
	void setState(const VcsState& state);

	const VcsItemAction& action() const noexcept;
	void setAction(const VcsItemAction& action);

	int userId() const noexcept;
	void setUserId(int value);

	const QString& details() const noexcept;
	void setDetails(const QString& value);		// Value must be valid JSON, Example: "{}"

	// Data
	//
protected:
	QString m_fileName;
	int m_fileId = DbFileInfo::Null;
	int m_parentId = 0;
	int m_size = 0;
	bool m_deleted = false;				// File was deleted from database, from all tables, such FileInfo does not exist anymore

	int m_changeset = 0;
	QDateTime m_created;
	QDateTime m_lastCheckIn;

	VcsState m_state;
	VcsItemAction m_action;
	int m_userId = -1;

	QString m_details;

public:
	static const int Null = -1;

	static QString fullPathToFileName(const QString& fullPathName);		// $root$/Schemas/Monitor -> Monitor
};

//
//
// DbFile
//
//
class DbFile : public DbFileInfo
{
public:
	DbFile();
	DbFile(const DbFile& dbFile) = default;
	DbFile(const DbFileInfo& fileInfo);

	// Methods
	//
public:
	bool readFromDisk(const QString& fileName);
	bool writeToDisk(const QString& directory) const;

	void convertToDatabaseString(QString* str);										// returns E'\\x00010203......'
	static void convertToDatabaseString(const QByteArray& data, QString* result);	// returns E'\\x00010203......'

	DbFile& operator= (const DbFileInfo& fileInfo);

	// Properties
	//
public:
	const QByteArray& data() const;
	QByteArray& data();
	void swapData(QByteArray& data);
	void clearData();

	virtual int size() const override;

	// Data
	//
private:
	QByteArray m_data;
};

//
//
// DbChangeset
//
//
class DbChangeset
{
public:
	DbChangeset();
	virtual ~DbChangeset();

	// Properties
	//
public:
	int changeset() const;
	void setChangeset(int value);

	QDateTime date() const;
	void setDate(const QDateTime& value);
	void setDate(const QString& value);

	int userId() const;
	void setUserId(int value);

	const QString& username() const;
	void setUsername(const QString& value);

	const QString& comment() const;
	void setComment(const QString& value);

	const VcsItemAction& action() const;
	void setAction(const VcsItemAction& value);

	// Data
	//
private:
	int m_changesetId = 0;
	QDateTime m_date;
	int m_userId = -1;
	QString m_username;
	QString m_comment;
	VcsItemAction m_action;
};

//
//
// DbChangesetDetails
//
//
class DbChangesetObject;

class DbChangesetDetails : public DbChangeset
{
public:
	DbChangesetDetails();
	virtual ~DbChangesetDetails();

public:
	const std::vector<DbChangesetObject>& objects() const;
	void addObject(const DbChangesetObject& object);

private:
	std::vector<DbChangesetObject> m_objects;
};


class DbChangesetObject
{
public:
	DbChangesetObject();
	explicit DbChangesetObject(const DbFileInfo& file);
	explicit DbChangesetObject(const Signal& signal);
	virtual ~DbChangesetObject();

public:
	enum class Type
	{
		File,					//	Values are hardcoded in DB
		Signal
	};

public:
	DbChangesetObject::Type type() const;
	void setType(DbChangesetObject::Type value);

	bool isFile() const;
	bool isSignal() const;

	int id() const;
	void setId(int value);

	QString name() const;
	void setName(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	VcsItemAction action() const;
	void setAction(VcsItemAction value);

	QString parent() const;
	void setParent(const QString& value);

private:
	Type m_type = Type::File;
	int m_id = -1;				// File.FileID or Signal.SignalsID
	QString m_name;				// FileName or AppSignalID
	QString m_caption;
	VcsItemAction m_action = VcsItemAction::Added;
	QString m_parent;
};



// WIN_64 PLATFORM C4267 WARNING ISSUE, IT IS NOT ENOUGH TO DISBALE THIS WARNING
// To remove annoing warning c4267 under windows x64, go to qmetatype.h, line 897 (Qt 5.3.1) and set static_cast to int for the
// returning value.
// Was:
// { static int size(const std::vector<T> *t) { return t->size(); } };
// Now:
// { static int size(const std::vector<T> *t) { return static_cast<int>(t->size()); } };
//

Q_DECLARE_METATYPE(DbUser)
Q_DECLARE_METATYPE(DbFileTree)
Q_DECLARE_METATYPE(DbFileInfo)
Q_DECLARE_METATYPE(DbFile)
Q_DECLARE_METATYPE(DbChangeset)
Q_DECLARE_METATYPE(DbChangesetDetails)
Q_DECLARE_METATYPE(DbProject)
Q_DECLARE_METATYPE(std::vector<DbProject>)
Q_DECLARE_METATYPE(std::vector<DbFileInfo>)
Q_DECLARE_METATYPE(std::vector<std::shared_ptr<DbFile>>)
Q_DECLARE_METATYPE(std::vector<int>)
Q_DECLARE_METATYPE(std::vector<DbChangeset>)
Q_DECLARE_METATYPE(std::vector<DbChangesetDetails>)

