#ifndef SWITCHPRESETSPAGE_H
#define SWITCHPRESETSPAGE_H

#include "../AppSignalLib/TuningSignalState.h"

#include <ClientLib/TuningSignalManager.h>
#include <ClientLib/TuningUserManager.h>
#include "TuningClientUiStorage.h"
#include "TuningConfigController.h"
#include <TuningLib/TuningFilter.h>

namespace ClientLib
{
	class TuningConnection;
}

//
// FilterPushButton
//

class FilterPushButton : public QPushButton
{
	Q_OBJECT
public:
	FilterPushButton(const QString& caption, std::shared_ptr<TuningFilters::TuningFilter> filter, QWidget* parent);

	std::shared_ptr<TuningFilters::TuningFilter> filter();

signals:
	void clicked(std::shared_ptr<TuningFilters::TuningFilter> filter);

private:
	virtual void mousePressEvent(QMouseEvent *event) override;

private:
	std::shared_ptr<TuningFilters::TuningFilter> m_filter;

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

	void mousePressEvent(QMouseEvent* e) override
	{
		QTableWidgetItem* item = itemAt(e->pos());
		if (item == nullptr)
		{
			clearSelection();
		}

		QTableWidget::mousePressEvent(e);
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
	explicit SwitchFiltersPage(TuningConfigController& configController,
							   ClientLib::TuningSignalManager& tuningSignalManager,
							   TuningFilters::TuningFilterStorage& tuningFilterStorage,
							   ClientLib::TuningUserManager& userManager,
							   ClientLib::TuningConnection& tuningConnection,
							   std::shared_ptr<TuningFilters::TuningFilter> workspaceFilter,
							   QWidget* parent = 0);
	~SwitchFiltersPage();

	void createControls(std::shared_ptr<TuningFilters::TuningFilter> root);

private:

	void createFiltersList(std::shared_ptr<TuningFilters::TuningFilter> filter);
	void createButtons();
	void createListItems();

	bool changeFilterSignals(std::shared_ptr<TuningFilters::TuningFilter> filter);
	void apply();

	int countDiscretes(TuningFilters::TuningFilter* filter);
	int countWritingEnabled(TuningFilters::TuningFilter* filter);

protected:
	  void showEvent(QShowEvent *ev) override;

private slots:
	void onOptions();
	void onPrev();
	void onNext();
	void onApply();
	void onTimer();
	void onFilterButtonClicked(std::shared_ptr<TuningFilters::TuningFilter> filter);
	void onFilterTablePressed();

private:

	enum class Columns
	{
		State = 0,
		Caption,
		Counter,
		ColumnCount
	};

	std::vector<std::shared_ptr<TuningFilters::TuningFilter>> m_buttonFilters;
	std::vector<std::shared_ptr<TuningFilters::TuningFilter>> m_listFilters;

	TuningConfigController& m_configController;
	ClientLib::TuningSignalManager& m_tuningSignalManager;
	TuningFilters::TuningFilterStorage& m_tuningFilterStorage;
	ClientLib::TuningUserManager& m_userManager;

	ClientLib::TuningConnection& m_tuningConnection;

	std::shared_ptr<TuningFilters::TuningFilter> m_workspaceFilter;

	//

	QVBoxLayout* m_mainLayout = nullptr;

	// Buttons part
	//

	int m_buttonStartIndex = 0;

	QWidget* m_filterButtonsWidget = nullptr;

	QGridLayout* m_buttonsLayout = nullptr;

	QPushButton* m_prevButton = nullptr;

	QPushButton* m_nextButton = nullptr;

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

	QColor m_alertBackColor = QColor(192, 0, 0);
	QColor m_alertTextColor = QColor(255, 255, 255);

	QColor m_partialBackColor = QColor(Qt::yellow);
	QColor m_partialTextColor = QColor(0, 0, 0);

};

#endif // SWITCHPRESETSPAGE_H
