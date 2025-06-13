#pragma once

#include <QWidget>
#include <QRadioButton>

class MainWindow : public QWidget
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();

signals:
	void serverState(bool visible);

private:
	void setupUi();
	void setupMenu();

	QWidget* serverStateVisible = nullptr;
	QRadioButton* analogBtn = nullptr;
	QRadioButton* boolBtn = nullptr;
	QRadioButton* discreteBtn = nullptr;
	void saveValueTypeSelection();
	void loadValueTypeSelection();
	
};
