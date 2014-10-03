#pragma once

//
//
// DbProgress
//
//
class DbProgress : public QObject
{
public:
	DbProgress();
	virtual ~DbProgress();

	bool init();
	bool run(QWidget* parentWidget, const QString& description);

	bool completed() const;
	void setCompleted(bool value);

	bool wasCanceled() const;
	void setCancel(bool value);

	QString currentOperation() const;
	void setCurrentOperation(const QString& value);

	int value() const;
	void setValue(int value);

	QString errorMessage() const;
	void setErrorMessage(const QString& value);

	bool hasError() const;

	QString completeMessage() const;
	void setCompleteMessage(const QString& value);

	void enableProgress();
	void disableProgress();
	bool isProgressEnabled();

private:
	mutable QMutex m_mutex;

	bool m_completed;				// Set true if operation is completed
	bool m_cancel;					// Cancel operation

	QString m_currentOperation;

	int m_value;					// 0 - 100%

	QString m_errorMessage;			// In case of error, this variable will contain error description
	QString m_completeMessage;		// If this field is not empty, show message box with the text

	bool m_progressEnabled;			// QProgressDialog is enabled
};

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
};

bool operator== (const VcsState& s1, const VcsState& s2);
bool operator!= (const VcsState& s1, const VcsState& s2);

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
		Added = 1,			// Don't change values, it is stored in DB
		Modified = 2,
		Deleted = 3
	};

	VcsItemAction();
	VcsItemAction(VcsItemActionType s);

	QString text() const;
	int toInt() const;

private:
	VcsItemActionType m_action;

	friend bool operator== (const VcsItemAction& s1, const VcsItemAction& s2);
	friend bool operator!= (const VcsItemAction& s1, const VcsItemAction& s2);
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
	QString text;
    QString upgradeFileName;
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

	// Data
	//
protected:
	QString m_fileName;
	int m_fileId = -1;
	int m_parentId = 0;
	int m_size = 0;
	bool m_deleted = false;		// File was deleted from database, from all tables, such FileInfo does not exist anymore

	int m_changeset = -1;
	QDateTime m_created;
	QDateTime m_lastCheckIn;

	VcsState m_state;
	VcsItemAction m_action;
	int m_userId = -1;
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
// DbChangesetInfo
//
//
class DbChangesetInfo
{
public:
	DbChangesetInfo();

	// Properties
	//
public:
	int changeset() const;
	void setChangeset(int value);

	QDateTime date() const;
	void setDate(const QDateTime& value);
	void setDate(const QString& value);

	const DbUser& user() const;
	void setUser(const DbUser& value);

	const QString& comment() const;
	void setComment(const QString& value);

	// Data
	//
private:
	int m_changesetId;
	QDateTime m_date;
	DbUser m_user;
	QString m_comment;
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
Q_DECLARE_METATYPE(DbChangesetInfo)
Q_DECLARE_METATYPE(DbProject)
Q_DECLARE_METATYPE(std::vector<DbProject>)
Q_DECLARE_METATYPE(std::vector<DbFileInfo>)
Q_DECLARE_METATYPE(std::vector<std::shared_ptr<DbFile>>)
Q_DECLARE_METATYPE(std::vector<int>)
Q_DECLARE_METATYPE(std::vector<DbChangesetInfo>)

