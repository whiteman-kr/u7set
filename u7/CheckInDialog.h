#pragma once

namespace Ui
{
	class CheckInDialog;
}

class CheckInDialog : public QDialog,
					  protected HasDbController
{
	Q_OBJECT

private:
	CheckInDialog(const std::vector<DbFileInfo>& files, bool treeCheckIn, QString objectIdJsonName, DbController* dbc, QWidget* parent);
	~CheckInDialog();

public:
	static bool checkIn(std::vector<DbFileInfo>& files,
						std::vector<DbFileInfo>* checkedInFiles,
						bool treeCheckIn,
						QString objectIdJsonName,
						DbController* dbc,
						QWidget* parent,
						bool* checkInTree = nullptr);

protected:
	virtual void showEvent(QShowEvent* event);
	virtual void closeEvent(QCloseEvent* event);

private:
	enum class FileListColumn
	{
		Object,
		Other,
		FileName,
		FileID,
		Count
	};

private slots:
	void on_checkInButton_clicked();
	void on_cancelButton_clicked();

	void updateCheckInFiles(bool includeChildren);

private:
	std::vector<DbFileInfo> m_files;
	std::vector<DbFileInfo> m_checkInFiles;

	QString m_objectIdJsonName; // The JSON property name for ObjectID (e.g., "EquipmentID", "SchemaID")

	Ui::CheckInDialog* ui = nullptr;
	bool m_resizeDone = false;
	inline static QSize s_lastSize{};
};
