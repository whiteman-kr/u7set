#pragma once
#include <QLayout>
#include <QPushButton>

namespace TagSelector
{
	class FlowLayout;
	class TagSelectorButton;
}


class TagSelectorWidget : public QWidget
{
	Q_OBJECT

public:
	explicit TagSelectorWidget(QWidget* parent = nullptr);
	virtual ~TagSelectorWidget();

public:
	void clear();

	void setTags(const QStringList& tags);
	void setTags(const std::vector<QString>& tags);
	void setTags(const std::set<QString>& tags);

	std::map<QString, bool> tags() const;

	QStringList selectedTags() const;
	void setSelectedTags(const QStringList& tags, bool emitNotify);

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;

signals:
	void changed();

private slots:
	void resetAllTags();
	void copyTag();

private:
	TagSelector::FlowLayout* m_flowLayout;

	QAction* m_resetAllTagsAction = nullptr;
	QAction* m_copyTagAction = nullptr;

	QString m_copyTag;
};


namespace TagSelector
{

	class TagSelectorButton : public QPushButton
	{
		Q_OBJECT

	public:
		explicit TagSelectorButton(QString tag, QWidget* parent = nullptr);
		virtual ~TagSelectorButton();

	public:
		QString tag() const;

		bool selected() const;
		void setSelected(bool value);
	};

	// Flow layout is copied form examples of Qt
	// https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html
	// No code guide was applied to this code
	// The only my change here is in verticalSpacing (/ 2 for value)
	//
	class FlowLayout : public QLayout
	{
	public:
		explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
		explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
		~FlowLayout();

		void addItem(QLayoutItem *item) override;
		int horizontalSpacing() const;
		int verticalSpacing() const;
		Qt::Orientations expandingDirections() const override;
		bool hasHeightForWidth() const override;
		int heightForWidth(int) const override;
		int count() const override;
		QLayoutItem *itemAt(int index) const override;
		QSize minimumSize() const override;
		void setGeometry(const QRect &rect) override;
		QSize sizeHint() const override;
		QLayoutItem *takeAt(int index) override;

	public:
		int selectedCount() const;
		QString tagAtPos(QPoint pos) const;

	private:
		int doLayout(const QRect &rect, bool testOnly) const;
		int smartSpacing(QStyle::PixelMetric pm) const;

		QList<QLayoutItem*> itemList;
		int m_hSpace;
		int m_vSpace;
	};
}




