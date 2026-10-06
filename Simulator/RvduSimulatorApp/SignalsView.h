#pragma once

#include <QAbstractTableModel>
class QTableView;

class SignalsModel : public QAbstractTableModel
{
public:
	SignalsModel();

	void setSignalsCount(int count);

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	int m_signalsCount = 0;
	QStringList m_tableHeaders;
};

class SignalsView : public QWidget
{
	Q_OBJECT
public:
	static SignalsView* create(bool autonomousMode, QWidget* parent);

private:
	explicit SignalsView(bool autonomousMode, QWidget* parent);
	~SignalsView();

private:
	void setupUi();

	void fillSignalsList();
	void updateSignalValues();

private slots:
	void onTimer();
	void onSignalClicked(unsigned int index);

private:
	SignalsModel m_model;
	bool m_autonomousMode = true;

	QTableView* m_tableView = nullptr;
	static inline SignalsView* m_instance = nullptr;

};