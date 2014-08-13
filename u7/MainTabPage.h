#ifndef MAINTABPAGE_H
#define MAINTABPAGE_H

class DbStore;

class MainTabPage : public QWidget
{
	Q_OBJECT

private:
	MainTabPage();
public:
	MainTabPage(DbStore* dbstore, QWidget* parent);
	
signals:
	
public slots:
	
	// Properties
	//
protected:
	DbStore* dbStore();

	// Data
	//
private:
	DbStore* m_pDbStore;
};

#endif // MAINTABPAGE_H
