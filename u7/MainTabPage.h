#pragma once

class GlobalMessanger;

class MainTabPage : public QWidget
{
	Q_OBJECT

public:
	MainTabPage() = delete;
	MainTabPage(DbController* dbcontroller, QWidget* parent);
	
	// Properties
	//
protected:
	DbController* dbController();
	const DbController* dbController() const;

	DbController* db();
	const DbController* db() const;

	// Data
	//
private:
	DbController* m_dbController;
};

