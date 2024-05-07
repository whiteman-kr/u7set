#pragma once

#include <UiLib/TabWidgetEx.h>

class MainTabPage;


class CentralWidget : public UiLib::TabWidgetEx
{
	Q_OBJECT

public:
	explicit CentralWidget(QWidget* parent = nullptr);

	// public methods
	//
public:
	int addTabPage(MainTabPage* tabPage, const QString& label);
	
signals:

private slots:
	void currentChanged(int index);
	
public slots:
	void switchToTabPage(QWidget* w);
};
