#pragma once

class MainTabPage;

class CentralWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit CentralWidget(QWidget *parent = 0);

	// public methods
	//
public:
	int addTabPage(MainTabPage* tabPage, const QString& label);
	
signals:
	
public slots:
	void switchToTabPage(QWidget* w);
};
