#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <map>
#include <functional>
#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QtSql/QSqlRecord>
#include "PropertyObject.h"

class Signal;

namespace Db
{
	class ProjectProperty
	{
		ProjectProperty() = delete;

	public:
		constexpr static const char* Description = "Description";
		constexpr static const char* SafetyProject = "Safety Project";
		constexpr static const char* SuppressWarnings = "SuppressWarnings";					// A list of suppressed warnings on build
		constexpr static const char* UppercaseAppSignalId = "UppercaseAppSignalID";
		constexpr static const char* GenerateAppSignalsXml = "Generate AppSignals.xml";		// Generate file AppSignals.xml on build
		constexpr static const char* GenerateExtraDebugInfo = "Generate Extra Debug Info";	// Generate extra debug information on build
	};

	class File
	{
		File() = delete;

	public:
		constexpr static const char* RootFileName = "$root$";						// root file name
		constexpr static const char* AfblFileName = "$root$/AFBL";					// Application Functional Block Library

		constexpr static const char* SchemasFileName = "$root$/Schemas";			// Schemas root fie
		constexpr static const char* UfblFileName = "$root$/Schemas/UFBL";			// User Functional Block Library
		constexpr static const char* AlFileName = "$root$/Schemas/ApplicationLogic";// Application Logic Schemas
		constexpr static const char* MvsFileName = "$root$/Schemas/Monitor";		// Monitor Video Schemas
		constexpr static const char* TvsFileName = "$root$/Schemas/Tuning";			// TuningClient Video Schemas;			// Tuning Video Schemas
		constexpr static const char* DvsFileName = "$root$/Diagnostics";			// Diagnostics Video Schemas -> will be moved to $root$/Schemas,  see update 235

		constexpr static const char* HcFileName = "$root$/HC";						// Hardware Configuratiun
		constexpr static const char* HpFileName = "$root$/HP";						// Hardware Presets
		constexpr static const char* McFileName = "$root$/MC";						// Module Configuration
		constexpr static const char* ConnectionsFileName = "$root$/CONNECTIONS";	// Connections
		constexpr static const char* BusTypesFileName = "$root$/BUSTYPES";			// BustTypes
		constexpr static const char* EtcFileName = "$root$/ETC";					// Etc file name

		constexpr static const char* TestsFileName = "$root$/Tests";				// Tests folder
		constexpr static const char* SimTestsFileName = "$root$/Tests/SimTests";	// SimTests folder

		constexpr static const char* SignalPropertyBehaviorFileName = "SignalPropertyBehavior.csv";

		constexpr static const char* AlFileExtension = "als";						// Application Logic schema file extension
		constexpr static const char* AlTemplExtension = "templ_als";				// Application Logic schema template file extnesion

		constexpr static const char* UfbFileExtension = "ufb";						// User Functional Block schema file extnesion;		// User Functional Block schema file extnesion
		constexpr static const char* UfbTemplExtension = "templ_ufb";				// User Functional Block template file extnesion

		constexpr static const char* MvsFileExtension = "mvs";						// Monitor schema file extnesion
		constexpr static const char* MvsTemplExtension = "templ_mvs";				// Monitor schema template file extnesion

		constexpr static const char* TvsFileExtension = "tvs";						// TuningClient schema file extnesion
		constexpr static const char* TvsTemplExtension = "templ_tvs";				// TuningClient schema template file extnesion

		constexpr static const char* DvsFileExtension = "dvs";						// Diagnostics schema file extnesion
		constexpr static const char* DvsTemplExtension = "templ_dvs";				// Diagnostics schema template file extnesion

		constexpr static const char* OclFileExtension = "ocl";						// (Optical) Connection Link
		constexpr static const char* BusFileExtension = "bus_type";					// Bus type

		constexpr static const char* AppSignalFileExtension = "asg";				// Application signal file extention (::Proto::AppSignal message)
		constexpr static const char* AppSignalSetFileExtension = "asgs";			// Application signals set file extention (::Proto::AppSignalSet message)
	};
}


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
	VcsStateType value() const noexcept;

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

	bool safetyProject() const;
	void setSafetyProject(bool value);

	int version() const;
	void setVersion(int value);

	bool uppercaseAppSignalId() const;
	void setUppercaseAppSignalId(bool value);

protected:
	QString m_databaseName;
	QString m_projectName;
	QString m_description;
	bool m_safetyProject = true;
	int m_version = 0;
	bool m_uppercaseAppSignalId = true;
};


struct UpgradeItem
{
    QString upgradeFileName;
	QString text;
};

// DbProjectProperties
//
class DbProjectProperties : public PropertyObject
{
public:
	DbProjectProperties();

public:
	QString description() const;
	void setDescription(const QString& value);

	bool safetyProject() const;
	void setSafetyProject(bool value);

	std::vector<int> suppressWarnings() const;
	QString suppressWarningsAsString() const;
	void setSuppressWarnings(const QString& value);

	bool uppercaseAppSignalId() const;
	void setUppercaseAppSignalId(bool value);

	bool generateAppSignalsXml() const;
	void setGenerateAppSignalsXml(bool value);

	bool generateExtraDebugInfo() const;
	void setGenerateExtraDebugInfo(bool value);

private:
	QString m_description;
	bool m_safetyProject = true;
	std::vector<int> m_suppressWarnings;
	bool m_uppercaseAppSignalId = true;
	bool m_generateAppSignalsXml = false;
	bool m_generateExtraDebugInfo = false;
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
	DbFileTree(const std::vector<std::shared_ptr<DbFileInfo>>& files, int rootFileId);
	DbFileTree(const std::map<int, std::shared_ptr<DbFileInfo>>& files, int rootFileId);

	DbFileTree(DbFileTree&& src);
	DbFileTree& operator=(DbFileTree&& src);

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

	QString filePath(int fileId) const;		// Return file path "/ABC/DEF/", "/"

	const std::map<int, std::shared_ptr<DbFileInfo>>& files() const;
	std::vector<DbFileInfo> toVector(bool excludeRoot) const;
	std::vector<DbFileInfo> toVectorIf(std::function<bool(const DbFileInfo&)> pred) const;
	std::vector<std::shared_ptr<DbFileInfo>> toVectorOfSharedPointers(bool excludeRoot) const;

	bool hasChildren(int fileId) const;

	const std::vector<std::shared_ptr<DbFileInfo>>& children(int parentId) const;
	const std::vector<std::shared_ptr<DbFileInfo>>& children(const DbFileInfo& fileInfo) const;
	const std::vector<std::shared_ptr<DbFileInfo>>& children(const std::shared_ptr<DbFileInfo>& fileInfo) const;

	std::shared_ptr<DbFileInfo> child(int parentId, int index) const;
	std::shared_ptr<DbFileInfo> child(const DbFileInfo& parentFileInfo, int index) const;
	std::shared_ptr<DbFileInfo> child(const std::shared_ptr<DbFileInfo>& parentFileInfo, int index) const;

	int rootChildrenCount() const;
	int childrenCount(int parentFileId) const;

	int indexInParent(int fileId) const;
	int indexInParent(const DbFileInfo& fileId) const;

	int calcIf(int startFromFileId, std::function<int(const DbFileInfo&)> pred) const;

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
	bool removeIf(std::function<bool(const DbFileInfo&)> pred);

private:
	struct FileChildren
	{
		int m_fileId;
		std::vector<std::shared_ptr<DbFileInfo>> m_children;
	};

private:
	// WARNING, assigment move is present, adding new member, modify operator=(DbFileTree&&)!!!
	//
	std::map<int, FileChildren> m_fileIdToChildren;				// Key is fileid, values are its' children
	std::map<int, std::shared_ptr<DbFileInfo>> m_files;			// Key if fileId, value is DbFile(Info) object
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
	explicit DbFileInfo(const DbFile& file) noexcept;
	explicit DbFileInfo(int fileId) noexcept;
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

	// File Attributes
	//
	qint32 attributes() const;
	void setAttributes(qint32 value);

	bool isFolder() const;
	bool directoryAttribute() const;
	void setDirectoryAttribute(bool value);

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

	union
	{
		struct
		{
			qint32 m_attrDirectory: 1;
		};
		qint32 m_attributes = 0;
	};

public:
	static const int Null = -1;
	static const int ATTRIBUTE_DIRECTORY = 0x00000001;

	static QString fullPathToFileName(const QString& fullPathName);		// $root$/Schemas/Monitor -> Monitor

	friend bool operator< (const DbFileInfo& a, const DbFileInfo& b);
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
	//QByteArray& data();				// Commented, as this function cannot set size after changing data
	void setData(const QByteArray& data);
	void setData(QByteArray&& data);
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

	QString fileMoveText() const;
	void setFileMoveText(const QString& value);

	QString fileRenameText() const;
	void setFileRenameText(const QString& value);

private:
	Type m_type = Type::File;
	int m_id = -1;				// File.FileID or Signal.SignalsID
	QString m_name;				// FileName or AppSignalID
	QString m_caption;
	VcsItemAction m_action = VcsItemAction::Added;
	QString m_parent;
	QString m_fileMoveText;
	QString m_fileRenameText;
};


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

