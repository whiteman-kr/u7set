#include "TagSelectorWidget.h"
#include <set>

TagSelectorWidget::TagSelectorWidget(QWidget* parent)
	: QWidget(parent)
{
	if (parent != nullptr)
	{
		setBackgroundRole(parent->backgroundRole());
	}
	else
	{
		setBackgroundRole(QPalette::Light);
	}

	// --
	//
	m_flowLayout = new TagSelector::FlowLayout;

	m_flowLayout->setMargin(0);
	m_flowLayout->setSpacing(0);

	setLayout(m_flowLayout);

	// Context menu
	//
	setContextMenuPolicy(Qt::DefaultContextMenu);

	m_resetAllTagsAction = new QAction{tr("Reset All"), this};
	m_copyTagAction = new QAction{tr("Copy"), this};

	connect(m_resetAllTagsAction, &QAction::triggered, this, &TagSelectorWidget::resetAllTags);
	connect(m_copyTagAction, &QAction::triggered, this, &TagSelectorWidget::copyTag);

	addAction(m_resetAllTagsAction);

	return;
}

TagSelectorWidget::~TagSelectorWidget()
{
	delete m_resetAllTagsAction;
}

void TagSelectorWidget::clear()
{
	setTags(std::set<QString>{});
	return;
}

void TagSelectorWidget::setTags(const QStringList& tags)
{
	std::set<QString> tagSet;

	for (const QString& tag : tags)
	{
		tagSet.insert(tag);
	}

	return setTags(tagSet);
}

void TagSelectorWidget::setTags(const std::vector<QString>& tags)
{
	std::set<QString> tagSet;

	for (const QString& tag : tags)
	{
		tagSet.insert(tag);
	}

	return setTags(tagSet);
}

void TagSelectorWidget::setTags(const std::set<QString>& tags)
{
	blockSignals(true);

	// Get all tags, remove them from layout
	//
	std::map<QString, QLayoutItem*> layoutItems;

	while (m_flowLayout->count() != 0)
	{
		QLayoutItem* li = m_flowLayout->takeAt(0);	// It removes item from  layout

		if (li->widget() != nullptr)
		{
			TagSelector::TagSelectorButton* b = dynamic_cast<TagSelector::TagSelectorButton*>(li->widget());

			if (b == nullptr)
			{
				assert(b);
				delete li;
				continue;
			}

			layoutItems[b->tag()] = li;
		}
		else
		{
			delete li;
			continue;
		}
	}

	// Add tags to
	//
	for (const QString& tag : tags)
	{
		if (auto it = layoutItems.find(tag);
			it != layoutItems.end())
		{
			// Tag was create earlier, insert ion into layout
			//
			QLayoutItem* li = it->second;
			m_flowLayout->addItem(li);

			layoutItems.erase(it);
		}
		else
		{
			// The new tag, cretae tag button and insert it into the layout
			//
			TagSelector::TagSelectorButton* tagButton = new TagSelector::TagSelectorButton(tag);
			//tagButton->setChecked(true);	// selected by default

			connect(tagButton, &TagSelector::TagSelectorButton::toggled, this, &TagSelectorWidget::changed);

			m_flowLayout->addWidget(tagButton);
		}
	}

	// Delete all what left in layoutItems
	//
	for (auto [t, li] : layoutItems)
	{
		if (li->widget() != nullptr)
		{
			li->widget()->deleteLater();
		}

		delete li;
	}

	blockSignals(false);

	return;
}

std::map<QString, bool> TagSelectorWidget::tags() const
{
	std::map<QString, bool> result;

	for (int i = 0; i < m_flowLayout->count(); i++)
	{
		QLayoutItem* li = m_flowLayout->itemAt(i);

		if (li->widget() != nullptr)
		{
			TagSelector::TagSelectorButton* b = dynamic_cast<TagSelector::TagSelectorButton*>(li->widget());
			assert(b);

			if (b != nullptr)
			{
				result[b->tag()] = b->selected();
			}
		}
	}

	return result;
}

QStringList TagSelectorWidget::selectedTags() const
{
	QStringList result;
	result.reserve(m_flowLayout->count());

	for (int i = 0; i < m_flowLayout->count(); i++)
	{
		QLayoutItem* li = m_flowLayout->itemAt(i);

		if (li->widget() != nullptr)
		{
			TagSelector::TagSelectorButton* b = dynamic_cast<TagSelector::TagSelectorButton*>(li->widget());
			assert(b);

			if (b != nullptr && b->selected() == true)
			{
				result.push_back(b->tag());
			}
		}
	}

	return result;
}

void TagSelectorWidget::setSelectedTags(const QStringList& tags, bool emitNotify)
{
	blockSignals(true);

	for (int i = 0; i < m_flowLayout->count(); i++)
	{
		QLayoutItem* li = m_flowLayout->itemAt(i);

		if (li->widget() != nullptr)
		{
			TagSelector::TagSelectorButton* b = dynamic_cast<TagSelector::TagSelectorButton*>(li->widget());
			assert(b);

			if (b != nullptr)
			{
				if (tags.contains(b->tag()) == true)
				{
					b->setSelected(true);
				}
			}
		}
	}

	blockSignals(false);

	if (emitNotify == true)
	{
		emit changed();
	}

	return;
}

void TagSelectorWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	if (parentWidget() != nullptr)
	{
		setBackgroundRole(parentWidget()->backgroundRole());
	}
	else
	{
		setBackgroundRole(QPalette::Light);
	}

	return;
}

void TagSelectorWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);

	m_resetAllTagsAction->setEnabled(m_flowLayout->selectedCount() > 0);

	m_copyTag = m_flowLayout->tagAtPos(event->pos());
	m_copyTagAction->setEnabled(m_copyTag.isEmpty() == false);

	menu.addAction(m_resetAllTagsAction);
	menu.addAction(m_copyTagAction);

	// Show menu
	//
	menu.exec(event->globalPos());

	m_copyTag.clear();

	return;
}

void TagSelectorWidget::resetAllTags()
{
	bool wasReset = false;

	for (int i = 0; i < m_flowLayout->count(); i++)
	{
		QLayoutItem* li = m_flowLayout->itemAt(i);

		if (li->widget() != nullptr)
		{
			TagSelector::TagSelectorButton* b = dynamic_cast<TagSelector::TagSelectorButton*>(li->widget());
			assert(b);

			if (b != nullptr && b->selected() == true)
			{
				wasReset = true;

				b->blockSignals(true);
				b->setChecked(false);
				b->blockSignals(false);
			}
		}
	}

	if (wasReset == true)
	{
		emit changed();
	}
}

void TagSelectorWidget::copyTag()
{
	QClipboard* clipboard = QGuiApplication::clipboard();
	clipboard->setText(m_copyTag);

	return;
}

namespace TagSelector
{
	TagSelectorButton::TagSelectorButton(QString tag, QWidget* parent /*= nullptr*/) :
		QPushButton(tag, parent)
	{
		static const QString buttonStyle(R"_(
								QPushButton {
										border: none;
										color: #202020;
										border-radius: 8px;
										background-color: #e0e0e0;
										padding: 3px 8px 3px 8px;
								}
								QPushButton:checked {
									color: #f0f0f0;
									background-color: #606060;
								}
								QPushButton:disabled {
										 color: #808080;
										 background-color: #e0e0e0;
								}
								)_");

		setStyleSheet(buttonStyle);
		setFocusPolicy(Qt::NoFocus);
		setCheckable(true);

		return;
	}

	TagSelectorButton::~TagSelectorButton()
	{
	}

	QString TagSelectorButton::tag() const
	{
		return text();
	}

	bool TagSelectorButton::selected() const
	{
		return isChecked();
	}

	void TagSelectorButton::setSelected(bool value)
	{
		setChecked(value);
		return;
	}


	// Flow layout is copied form examples of Qt
	// https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html
	// No code guide was applied to this code
	//
	FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
		: QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
	{
		setContentsMargins(margin, margin, margin, margin);
	}

	FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
		: m_hSpace(hSpacing), m_vSpace(vSpacing)
	{
		setContentsMargins(margin, margin, margin, margin);
	}

	FlowLayout::~FlowLayout()
	{
		//QLayoutItem *item;
		//while ((item = takeAt(0)))
		//	delete item;
		// Prev three lines ate the same as the next cycle
		//
		for (auto item : itemList)
		{
			delete item;
		}

		itemList.clear();
	}

	void FlowLayout::addItem(QLayoutItem *item)
	{
		itemList.append(item);
	}

	int FlowLayout::horizontalSpacing() const
	{
		if (m_hSpace >= 0) {
			return m_hSpace;
		} else {
			return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
		}
	}

	int FlowLayout::verticalSpacing() const
	{
		if (m_vSpace >= 0) {
			return m_vSpace;
		} else {
			return smartSpacing(QStyle::PM_LayoutVerticalSpacing) / 2;
		}
	}

	int FlowLayout::count() const
	{
		return itemList.size();
	}

	QLayoutItem *FlowLayout::itemAt(int index) const
	{
		return itemList.value(index);
	}

	QLayoutItem *FlowLayout::takeAt(int index)
	{
		if (index >= 0 && index < itemList.size())
			return itemList.takeAt(index);
		else
			return 0;
	}

	Qt::Orientations FlowLayout::expandingDirections() const
	{
		return 0;
	}

	bool FlowLayout::hasHeightForWidth() const
	{
		return true;
	}

	int FlowLayout::heightForWidth(int width) const
	{
		int height = doLayout(QRect(0, 0, width, 0), true);
		return height;
	}

	void FlowLayout::setGeometry(const QRect &rect)
	{
		QLayout::setGeometry(rect);
		doLayout(rect, false);
	}

	QSize FlowLayout::sizeHint() const
	{
		return minimumSize();
	}

	QSize FlowLayout::minimumSize() const
	{
		QSize size;
		QLayoutItem *item;
		foreach (item, itemList)
			size = size.expandedTo(item->minimumSize());

		size += QSize(2*margin(), 2*margin());
		return size;
	}

	int FlowLayout::selectedCount() const
	{
		int result = 0;

		for (QLayoutItem* li : itemList)
		{
			TagSelectorButton* tsb = dynamic_cast<TagSelectorButton*>(li->widget());

			if (tsb != nullptr && tsb->isChecked() == true)
			{
				result ++;
			}
		}

		return result;
	}

	QString FlowLayout::tagAtPos(QPoint pos) const
	{
		for (QLayoutItem* li : itemList)
		{
			TagSelectorButton* tsb = dynamic_cast<TagSelectorButton*>(li->widget());

			if (tsb != nullptr &&
				tsb->geometry().contains(pos) == true)
			{
				return tsb->tag();
			}
		}

		return {};
	}

	int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
	{
		int left, top, right, bottom;
		getContentsMargins(&left, &top, &right, &bottom);
		QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
		int x = effectiveRect.x();
		int y = effectiveRect.y();
		int lineHeight = 0;

		QLayoutItem *item;
		foreach (item, itemList) {
			QWidget *wid = item->widget();
			int spaceX = horizontalSpacing();
			if (spaceX == -1)
				spaceX = wid->style()->layoutSpacing(
							 QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
			int spaceY = verticalSpacing();
			if (spaceY == -1)
				spaceY = wid->style()->layoutSpacing(
							 QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
			int nextX = x + item->sizeHint().width() + spaceX;
			if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
				x = effectiveRect.x();
				y = y + lineHeight + spaceY;
				nextX = x + item->sizeHint().width() + spaceX;
				lineHeight = 0;
			}

			if (!testOnly)
				item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

			x = nextX;
			lineHeight = qMax(lineHeight, item->sizeHint().height());
		}
		return y + lineHeight - rect.y() + bottom;
	}

	int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
	{
		QObject *parent = this->parent();
		if (!parent) {
			return -1;
		} else if (parent->isWidgetType()) {
			QWidget *pw = static_cast<QWidget *>(parent);
			return pw->style()->pixelMetric(pm, 0, pw);
		} else {
			return static_cast<QLayout *>(parent)->spacing();
		}
	}

}
