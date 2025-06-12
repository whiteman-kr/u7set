#pragma once

#include <QWidget>

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
};
