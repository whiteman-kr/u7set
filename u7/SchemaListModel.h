#pragma once

#include <QTableView>
#include "../lib/DbStruct.h"
#include "../VFrame30/Schema.h"

class DbController;

class SchemaListModel : public QAbstractTableModel
{
public:
	SchemaListModel(QObject* parent = nullptr);
	virtual ~SchemaListModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	void addFile(std::shared_ptr<DbFileInfo> file);

	void setFiles(const std::vector<DbFileInfo>& files, const std::vector<DbUser>& users);
	void clear();

	std::shared_ptr<DbFileInfo> fileByRow(int row);
	std::shared_ptr<DbFileInfo> fileByFileId(int fileId);

	int getFileRow(int fileId) const;

	// Properties
	//
public:
	QString filter() const;
	void setFilter(const QString& value);		// "" -- no filter, "cdd" -- just cdd files

	const std::vector<std::shared_ptr<DbFileInfo>>& files() const;

	QString usernameById(int userId) const;

	QString detailsColumnText(int fileId) const;
	QString fileCaption(int fileId) const;

	// Data
	//
public:
	enum Columns
	{
		FileNameColumn,
		FileCaptionColumn,
		FileStateColumn,
		FileUserColumn,
		FileActionColumn,
		//FileLastCheckInColumn,
		//FileIdColumn,
		FileIssuesColumn,
		FileDetailsColumn,

		// Add other column befor this line
		//
		ColumnCount
	};

private:
	std::vector<std::shared_ptr<DbFileInfo>> m_files;
	QString m_filter;

	std::map<int, QString> m_users;

	std::map<int, VFrame30::SchemaDetails> m_details;		// Key is FileID
};
