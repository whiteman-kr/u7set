#pragma once

class DbController;
class GlobalMessanger;

class MainTabPage : public QWidget
{
	Q_OBJECT

private:
	MainTabPage();
public:
	MainTabPage(DbController* dbcontroller, QWidget* parent);
	
signals:
	
public slots:
	
	// Properties
	//
protected:
	DbController* dbController();
	DbController* db();

	GlobalMessanger* globalMesssanger();

	// Data
	//
private:
	DbController* m_dbController;
};

