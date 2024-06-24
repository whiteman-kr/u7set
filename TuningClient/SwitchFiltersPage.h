#ifndef SWITCHPRESETSPAGE_H
#define SWITCHPRESETSPAGE_H

#include "../AppSignalLib/TuningSignalState.h"

#include <ClientLib/TuningSignalManager.h>
#include <ClientLib/TuningUserManager.h>

#include "TuningConfigController.h"

namespace ClientLib
{
	class TuningConnection;
}

namespace TuningLib
{
	class TuningUiItem;
	class TuningUiStorage;
}

class TuningCountersManager;

//
// FilterPushButton
//

class FilterPushButton : public QPushButton
{
	Q_OBJECT
public:
	FilterPushButton(const QString& filterId, const QString& caption, QWidget* parent);

	QString filterId() const;

signals:
	void clicked(const QString& filterId);

private:
	virtual void mousePressEvent(QMouseEvent *event) override;

private:
	QString m_filterId;
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
	SwitchFiltersPage(TuningConfigController& configController,
					  ClientLib::TuningSignalManager& tuningSignalManager,
					  AppSignalLists::AppSignalListSet& appSignalLists,
					  ClientLib::TuningUserManager& userManager,
					  ClientLib::TuningConnection& tuningConnection,
					  const TuningLib::TuningUiItem& uiItem,
					  const TuningCountersManager& tuningCounters,
					  QWidget* parent = nullptr);

	void createControls();

private:

	void createFiltersList();
	void createButtons();
	void createListItems();

	bool changeFilterSignals(const QString& filterId);
	void apply();

	void countDiscretes(const AppSignalLists::AppSignalList& list, int& total, int& writingEnabled, int& alerted);

protected:
	  void showEvent(QShowEvent *ev) override;

private slots:
	void onOptions();
	void onPrev();
	void onNext();
	void onApply();
	void onTimer();
	void onFilterButtonClicked(const QString& filterId);
	void onFilterTablePressed();

private:

	enum class Columns
	{
		State = 0,
		Caption,
		Counter,
		ColumnCount
	};

	std::vector<AppSignalLists::AppSignalList*> m_buttonFilters;
	std::vector<AppSignalLists::AppSignalList*> m_listFilters;

	TuningConfigController& m_configController;
	ClientLib::TuningSignalManager& m_tuningSignalManager;
	AppSignalLists::AppSignalListSet& m_appSignalLists;
	ClientLib::TuningUserManager& m_userManager;
	ClientLib::TuningConnection& m_tuningConnection;
	const TuningLib::TuningUiItem& m_workspaceUi;
	const TuningCountersManager& m_tuningCounters;

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
