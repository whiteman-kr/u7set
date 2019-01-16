#pragma once

class DbController;
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
	DbController* db();

	// Data
	//
private:
	DbController* m_dbController;
};

