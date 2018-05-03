#pragma once

#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QtSql/QSqlRecord>
#include <memory>
#include <assert.h>

class Signal;

// System files names
//
extern const char* const rootFileName;			// root file name
extern const char* const AfblFileName;			// Application Functional Block Library
extern const char* const UfblFileName;			// User Functional Block Library
extern const char* const AlFileName;			// Application Logic Schemas
extern const char* const HcFileName;			// Hardware Configuratiun
extern const char* const HpFileName;			// Hardware Presets
extern const char* const MvsFileName;			// Monitor Video Schemas
extern const char* const DvsFileName;			// Diagnostics Video Schemas
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

	VcsState();
	VcsState(VcsStateType s);

	QString text() const;

private:
	VcsStateType m_state;

	friend bool operator== (const VcsState& s1, const VcsState& s2);
	friend bool operator!= (const VcsState& s1, const VcsState& s2);
	friend bool operator< (const VcsState& s1, const VcsState& s2);
};

bool operator== (const VcsState& s1, const VcsState& s2);
bool operator!= (const VcsState& s1, const VcsState& s2);
bool operator<  (const VcsState& s1, const VcsState& s2);

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

	VcsItemAction();
	VcsItemAction(VcsItemActionType s);

	QString text() const;
	int toInt() const;

	VcsItemActionType value() const;

private:
	VcsItemActionType m_action;

	friend bool operator== (const VcsItemAction& s1, const VcsItemAction& s2);
	friend bool operator!= (const VcsItemAction& s1, const VcsItemAction& s2);
};


// signal management error codes
// returns in ObjectState.errCode field
//
const int	ERR_SIGNAL_OK = 0,
			ERR_SIGNAL_IS_NOT_CHECKED_OUT = 1,
			ERR_SIGNAL_ALREADY_CHECKED_OUT = 2,
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
//
// DbFileInfo
//
//
class DbFile;

class DbFileInfo
{
public:
	DbFileInfo();
	DbFileInfo(const DbFileInfo& fileInfo) = default;
	DbFileInfo(const DbFile& file);
	virtual ~DbFileInfo();

	// Methods
	//
public:

	// Properties
	//
public:
	QString fileName() const;
	void setFileName(const QString& value);

	int fileId() const;
	void setFileId(int value);
	void resetFileId();
	bool hasFileId() const;

	bool isNull() const;

	int parentId() const;
	void setParentId(int value);

	virtual int size() const;
	void setSize(int size);

	bool deleted() const;
	void setDeleted(bool value);

	int changeset() const;
	void setChangeset(int value);

	QDateTime created() const;
	void setCreated(const QDateTime& value);
	void setCreated(const QString& value);

	QDateTime lastCheckIn() const;
	void setLastCheckIn(const QDateTime& value);
	void setLastCheckIn(const QString& value);

	const VcsState& state() const;
	void setState(const VcsState& state);

	const VcsItemAction& action() const;
	void setAction(const VcsItemAction& action);

	int userId() const;
	void setUserId(int value);

	QString details() const;
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

