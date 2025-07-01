#pragma once

#include <QWidget>

class RWMultiToolBox;
class QRadioButton;

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
	void saveValueTypeSelection();
	void loadValueTypeSelection();
	void onEditCsvActionTriggered(RWMultiToolBox* rwMultiToolBox);

private:
	QRadioButton* m_analogBtn = nullptr;
	QRadioButton* m_boolBtn = nullptr;
	QRadioButton* m_discreteBtn = nullptr;
};
