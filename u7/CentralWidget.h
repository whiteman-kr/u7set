#pragma once

class MainTabPage;
#include <QSize>

class TabBar:public QTabBar
{
public:
	TabBar() : QTabBar()
	{
		setUsesScrollButtons(false);
	}

	void setMinimumTabSizeHint(const QSize& value)
	{
		m_tabMinimumSize = value;
	}

protected:

	virtual QSize minimumTabSizeHint(int index) const override
	{
		Q_UNUSED(index);
		return m_tabMinimumSize;
	}

	virtual QSize sizeHint() const override
	{
		int tabCount = count();
		if (tabCount == 0)
		{
			tabCount = 1;
		}

		int minimumWidth = m_tabMinimumSize.width() * tabCount;

		QSize s(minimumWidth, m_tabMinimumSize.height());
		return s;
	}

private:
	QSize m_tabMinimumSize = QSize(100, 50);

};

class CentralWidget : public QTabWidget
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
