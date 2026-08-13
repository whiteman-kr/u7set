#pragma once

#include <QTableWidget>

class SignalsView : public QWidget
{
	Q_OBJECT
public:
	static SignalsView* create(QWidget* parent = nullptr);

private:
	explicit SignalsView(QWidget* parent = nullptr);
	~SignalsView();

private:
	void setupUi();

	void fillSignalsList();
	void updateSignalValues();

private slots:
	void onTimer();
	void onSignalClicked(unsigned int index);

private:
	QStringList m_tableHeaders;

	QTableWidget* m_tableWidget = nullptr;
	static inline SignalsView* m_instance = nullptr;

};