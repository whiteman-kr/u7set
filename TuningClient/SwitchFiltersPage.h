#ifndef SWITCHPRESETSPAGE_H
#define SWITCHPRESETSPAGE_H

#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilter.h"
#include "TuningClientTcpClient.h"


#include <QTableWidget>
#include <QTableWidgetItem>
#include <QCheckBox>

//
// FilterPushButton
//

class FilterPushButton : public QPushButton
{
	Q_OBJECT
public:
	FilterPushButton(const QString& caption, std::shared_ptr<TuningFilter> filter, QWidget* parent);

	std::shared_ptr<TuningFilter> filter();

signals:
	void clicked(std::shared_ptr<TuningFilter> filter);

private slots:
	void slot_clicked();

private:
	std::shared_ptr<TuningFilter> m_filter;

};

//
// FilterTableWidget
//

class FilterTableWidget:public QTableWidget
{
	Q_OBJECT
public:
	FilterTableWidget(QWidget* parent): QTableWidget(parent){}
protected:
	void keyPressEvent(QKeyEvent *e)
	{
		if(e->key()==Qt::Key_Space)
		{
			emit spacePressed();
		}
		else
		{
			QTableWidget::keyPressEvent(e);
		}
	}
signals:
	void spacePressed();
};

//
// FilterCheckBox
//

class FilterCheckBox:public QCheckBox
{
	Q_OBJECT
public:
	FilterCheckBox(const QString& text, QWidget* parent = nullptr) : QCheckBox(text, parent){}

protected:
	void mousePressEvent(QMouseEvent* e) override
	{
		Q_UNUSED(e);

		if (hitButton(e->pos()) == true)
		{
			emit pressed();
		}
	}

	void keyPressEvent(QKeyEvent *e)
	{
		if(e->key()==Qt::Key_Space)
		{
			emit pressed();
		}
		else
		{
			QCheckBox::keyPressEvent(e);
		}
	}
signals:
	void pressed();
};

//
// SwitchFiltersPage
//

class SwitchFiltersPage : public QWidget
{
	Q_OBJECT
public:
	explicit SwitchFiltersPage(std::shared_ptr<TuningFilter> workspaceFilter,
								TuningSignalManager* tuningSignalManager,
								TuningClientTcpClient* tuningTcpClient,
								TuningFilterStorage* tuningFilterStorage,
								QWidget* parent = 0);
	~SwitchFiltersPage();

	void updateFilters(std::shared_ptr<TuningFilter> root);

private:
	void createFiltersList(std::shared_ptr<TuningFilter> filter);
	void createButtons();
	void createListItems();

	void changeFilterSignals(std::shared_ptr<TuningFilter> filter);
	void apply();

	int countDiscretes(TuningFilter* filter);

protected:
	  void showEvent(QShowEvent *ev) override;

private slots:
	void onOptions();
	void onPrev();
	void onNext();
	void onApply();
	void slot_timerTick500();

	void slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter);

	void onFilterTablePressed();




private:

	enum class Columns
	{
		State = 0,
		Caption,
		Counter,
		ColumnCount
	};

	std::vector<std::shared_ptr<TuningFilter>> m_buttonFilters;
	std::vector<std::shared_ptr<TuningFilter>> m_listFilters;

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningClientTcpClient* m_tuningTcpClient = nullptr;

	TuningFilterStorage* m_tuningFilterStorage = nullptr;

	std::shared_ptr<TuningFilter> m_workspaceFilter;

	//

	QVBoxLayout* m_mainLayout = nullptr;

	// Buttons part
	//

	int m_buttonStartIndex = 0;

	QWidget* m_filterButtonsWidget = nullptr;

	QGridLayout* m_buttonsLayout = nullptr;

	std::vector<FilterPushButton*> m_filterButtons;

	// Table part
	//
	QWidget* m_filterTableWidget = nullptr;

	QSplitter* m_vSplitter = nullptr;

	FilterTableWidget* m_filterTable = nullptr;

	QPushButton* m_applyButton = nullptr;

	QLabel* m_promptLabel = nullptr;

	//

	static QString tag_FilterButton;
	static QString tag_FilterSwitch;

	//QColor m_alertBackColor = QColor(255, 140, 0);
	QColor m_alertBackColor = QColor(Qt::darkRed);
	QColor m_alertTextColor = QColor(255, 255, 255);

	QColor m_partialBackColor = QColor(Qt::yellow);
	QColor m_partialTextColor = QColor(0, 0, 0);

};

#endif // SWITCHPRESETSPAGE_H
