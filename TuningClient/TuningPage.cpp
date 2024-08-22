#include "TuningPage.h"
#include "DialogChooseFilter.h"
#include "Main.h"
#include "MainWindow.h"
#include "Settings.h"
#include "TuningSignalInfo.h"
#include "TuningSourcesHelper.h"

#include <ClientLib/TuningConnection.h>
#include <ClientLib/TuningUserManager.h>
#include <SchemaClientLib/DialogWriteTuningValues.h>
#include <VFrame30/DrawParam.h>


TuningPageHelper::TuningPageHelper(const ClientLib::TuningUserManager& userManager):
	m_userManager(userManager)
{
}

bool TuningPageHelper::writingIsEnabled(const AppSignalParam& asp, const TuningSignalState& state) const
{
	bool controlEnabled = state.valid() == true && state.controlIsEnabled() == true && state.writingIsEnabled() == true;

	if (m_userManager.enabled() == true && m_userManager.loginPerOperation() == false)
	{
		// User is logged in and is allowed to tune this signal
		//
		controlEnabled &= (m_userManager.isLoggedIn() == true);

		if (controlEnabled == true)
		{
			bool tagFound = false;

			std::set<QString> signalTags = asp.tags();
			QStringList userTags = m_userManager.userTags();
			for (const QString& t : userTags)
			{
				if (signalTags.contains(t) == true)
				{
					tagFound = true;
					break;
				}
			}

			controlEnabled &= tagFound;
		}
	}
	return controlEnabled;
}
	
class SelectionControlDelegate : public QStyledItemDelegate
{
public:
	SelectionControlDelegate(QObject* parent, TuningModelClient* model);
	void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

private:
	TuningModelClient* m_model = nullptr;
};

//
// SelectionControlDelegate
//
SelectionControlDelegate::SelectionControlDelegate(QObject* parent, TuningModelClient* model) :
	QStyledItemDelegate(parent),
	m_model(model)
{
}

void SelectionControlDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
	QStyledItemDelegate::initStyleOption(option, index);

	bool selected = option->state & QStyle::State_Selected;

	// Set background color for selected item (by default it is displayed by white)
	//
	if (selected == true)
	{
		QBrush br = m_model->backColor(index);
		QBrush fr = m_model->foregroundColor(index);

		if (br.style() != Qt::NoBrush)
		{
			if (br.color() != QPalette().color(QPalette::Base))
			{

				option->palette.setColor(QPalette::Highlight, br.color());
				option->palette.setColor(QPalette::HighlightedText, fr.color());
			}
		}
	}
}

//
// TuningItemModelMain
//
TuningModelClient::TuningModelClient(ClientLib::TuningSignalManager& tuningSignalManager, const ClientLib::TuningUserManager& userManager, const std::vector<QString>& valueColumnsAppSignalIdSuffixes, QWidget* parent):
	TuningModel(tuningSignalManager, valueColumnsAppSignalIdSuffixes, parent),
	m_userManager(userManager),
	m_helper(m_userManager),
	m_parentWidget(parent)
{
}

void TuningModelClient::blink()
{
	m_blink = !m_blink;
}

bool TuningModelClient::hasPendingChanges()
{
	for (int i = 0; i < static_cast<int>(m_allHashes.size()); i++)
	{
		Hash hash = m_allHashes[i];

		if (m_tuningSignalManager.isUnapplied(hash) == true)
		{
			return true;
		}
	}

	return false;
}

QBrush TuningModelClient::backColor(const QModelIndex& index) const
{
	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return QBrush();
	}

	bool ok = false;

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= TuningLib::MaxValuesColumnCount)
		{
			assert(false);
			return QBrush();
		}

		Hash hash = hashByIndex(row, valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return QBrush();
		}

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);
		TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

		if (m_helper.writingIsEnabled(asp, state) == false)
		{
			QColor color = TuningClientAppSettings::instance().user().m_columnDisabledBackColor;
			return QBrush(color);
		}

		if (state.valid() == false)
		{
			QColor color = TuningClientAppSettings::instance().user().m_columnErrorBackColor;
			return QBrush(color);
		}

		if (m_blink == true && m_tuningSignalManager.isUnapplied(hash) == true)
		{
			QColor color = TuningClientAppSettings::instance().user().m_columnUnappliedBackColor;
			return QBrush(color);
		}

		if (state.isTuningDefault() == false)
		{
			QColor color = TuningClientAppSettings::instance().user().m_columnDefaultMismatchBackColor;
			return QBrush(color);
		}
	}

	QColor color;

	const TuningModelHashSet& hashes = hashSetByIndex(row);

	for (int c = 0; c < TuningLib::MaxValuesColumnCount; c++)
	{
		const Hash hash = hashes.hash[c];

		if (hash == UNDEFINED_HASH)
		{
			continue;
		}

		if (columnType == static_cast<int>(TuningModelColumns::Valid))
		{
			TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

			if (state.valid() == false)
			{
				color = TuningClientAppSettings::instance().user().m_columnErrorBackColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::LowLimit) || columnType == static_cast<int>(TuningModelColumns::HighLimit))
		{
			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

			if (state.limitsUnbalance(asp) == true)
			{
				color = TuningClientAppSettings::instance().user().m_columnErrorBackColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::OutOfRange))
		{
			TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

			if (state.outOfRange() == true)
			{
				color = TuningClientAppSettings::instance().user().m_columnErrorBackColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::Default))
		{
			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningValue defaultVal = defaultValue(asp);

			if (defaultVal < asp.tuningLowBound() || defaultVal > asp.tuningHighBound())
			{
				color = TuningClientAppSettings::instance().user().m_columnErrorBackColor;
				break;
			}
		}
	}

	if (color.isValid() == true)
	{
		return QBrush(color);
	}

	return QBrush();
}

QBrush TuningModelClient::foregroundColor(const QModelIndex& index) const
{
	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return QBrush();
	}

	bool ok = false;

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= TuningLib::MaxValuesColumnCount)
		{
			assert(false);
			return QBrush();
		}

		Hash hash = hashByIndex(row, valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return QBrush();
		}

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);
		TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

		if (m_helper.writingIsEnabled(asp, state) == false)
		{
			QColor color = TuningClientAppSettings::instance().user().m_columnDisabledTextColor;
			return QBrush(color);
		}

		if (state.valid() == false)
		{
			QColor color = TuningClientAppSettings::instance().user().m_columnErrorTextColor;
			return QBrush(color);
		}

		if (m_blink == true && m_tuningSignalManager.isUnapplied(hash) == true)
		{
			QColor color = TuningClientAppSettings::instance().user().m_columnUnappliedTextColor;
			return QBrush(color);
		}
	}

	QColor color;

	const TuningModelHashSet& hashes = hashSetByIndex(row);

	for (int c = 0; c < TuningLib::MaxValuesColumnCount; c++)
	{
		const Hash hash = hashes.hash[c];

		if (hash == UNDEFINED_HASH)
		{
			continue;
		}

		if (columnType == static_cast<int>(TuningModelColumns::Valid))
		{
			TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

			if (state.valid() == false)
			{
				color = TuningClientAppSettings::instance().user().m_columnErrorTextColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::LowLimit) || columnType == static_cast<int>(TuningModelColumns::HighLimit))
		{
			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

			if (state.limitsUnbalance(asp) == true)
			{
				color = TuningClientAppSettings::instance().user().m_columnErrorTextColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::OutOfRange))
		{
			TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

			if (state.outOfRange() == true)
			{
				color = TuningClientAppSettings::instance().user().m_columnErrorTextColor;
				break;
			}
		}
	}

	if (color.isValid() == true)
	{
		return QBrush(color);
	}

	return QBrush();
}

Qt::ItemFlags TuningModelClient::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = TuningModel::flags(index);

	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return f;
	}

	int valueColumn = 0;

	if (valueColumnsCount() > 1)
	{
		if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
		{
			valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		}
	}

	if (valueColumn < 0 || valueColumn >= TuningLib::MaxValuesColumnCount)
	{
		assert(false);
		return f;
	}


	Hash hash = hashByIndex(row, valueColumn);
	if (hash == UNDEFINED_HASH)
	{
		return f;
	}

	bool ok = false;
	AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);
	TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

	if (state.valid() == true)
	{
		if (m_helper.writingIsEnabled(asp, state) == true)
		{
			if (asp.isAnalog() == true)
			{
				f |= Qt::ItemIsEditable;
			}
		}
	}

	return f;
}

QVariant TuningModelClient::data(const QModelIndex& index, int role) const
{
	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return QVariant();
	}

	if (role == Qt::DecorationRole && 
		columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= TuningLib::MaxValuesColumnCount)
		{
			assert(false);
			return QVariant();
		}

		Hash hash = hashByIndex(index.row(), valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return QVariant();
		}

		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

		if (asp.isDiscrete() == true && state.valid() == true)
		{
			quint32 discreteValue = 0;

			if (m_tuningSignalManager.isUnapplied(hash) == true)
			{
				discreteValue = m_tuningSignalManager.unappliedValue(hash).discreteValue();
			}
			else
			{
				discreteValue = state.value().discreteValue();
			}

			return drawCheckBox(discreteValue == 0 ? Qt::Unchecked : Qt::Checked, m_helper.writingIsEnabled(asp, state) == true);
		}
	}

	if (role == Qt::ToolTipRole &&
		columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= TuningLib::MaxValuesColumnCount)
		{
			assert(false);
			return QVariant();
		}

		Hash hash = hashByIndex(index.row(), valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return QVariant();
		}

		bool ok = false;

		TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

		if (state.valid() == true)
		{
			if (state.controlIsEnabled() == false)
			{
				return tr("Control is disabled - tuning source is not active.");
			}
			if (state.writingIsEnabled() == false)
			{
				return tr("Writing is disabled - no access key is set.");
			}
			if (state.writingIsEnabled() == false)
			{
				return tr("Writing is disabled - no access key is set.");
			}

			if (m_userManager.enabled() == true && m_userManager.loginPerOperation() == false)
			{
				if (m_userManager.isLoggedIn() == false)
				{
					return tr("Writing is disabled - user is not logged in.");
				}
				
				AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

				if (m_helper.writingIsEnabled(asp, state) == false)
				{
					return tr("Writing is disabled - current user is not allowed to tune this signal.");
				}
			}
		}		
	}

	return TuningModel::data(index, role);
}

bool TuningModelClient::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
	{
		return false;
	}

	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return false;
	}

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= TuningLib::MaxValuesColumnCount)
		{
			assert(false);
			return false;
		}

		Hash hash = hashByIndex(row, valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			assert(false);
			return false;
		}

		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

		if (role == Qt::EditRole &&
				state.valid() == true &&
				state.controlIsEnabled() == true &&
				m_helper.writingIsEnabled(asp, state) == true)
		{
			ok = false;
			double v = value.toDouble(&ok);
			if (ok == false)
			{
				QMessageBox::critical(m_parentWidget, qAppName(), tr("Value has invalid format!"));
				return false;
			}

			if (v < asp.tuningLowBound().toDouble() || v > asp.tuningHighBound().toDouble())
			{
				QMessageBox::critical(m_parentWidget, qAppName(), tr("Value is out of range!"));
				return false;
			}

			m_tuningSignalManager.setUnappliedValue(asp.hash(), TuningValue(asp.tuningType(), v));
			return true;
		}
	}

	return false;
}

QIcon TuningModelClient::drawCheckBox(int state, bool enabled) const
{
	QStyleOptionButton opt;
	switch (state)
	{
	case Qt::Checked:
		opt.state |= QStyle::State_On;
		break;
	case Qt::Unchecked:
		opt.state |= QStyle::State_Off;
		break;
	case Qt::PartiallyChecked:
		opt.state |= QStyle::State_NoChange;
		break;
	default:
		Q_ASSERT(false);
	}

	if (enabled == false)
	{
		opt.state |= QStyle::State_ReadOnly;
	}
	else
	{
		opt.state |= QStyle::State_Enabled;
	}

	const QStyle* style = QApplication::style();

	const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
	const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);

	opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
	QPixmap pixmap = QPixmap(indicatorWidth, indicatorHeight);
	pixmap.fill(Qt::transparent);
	{
		QPainter painter(&pixmap);
		style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
	}
	return QIcon(pixmap);
}


//
// TuningTableView
//

TuningTableView::TuningTableView(const ClientLib::TuningUserManager& userManager):
	QTableView(),
	m_helper(userManager)
{

}

bool TuningTableView::editorActive()
{
	return m_editorActive;
}

bool TuningTableView::edit(const QModelIndex&  index, EditTrigger trigger, QEvent* event)
{
	if ((trigger & QAbstractItemView::EditKeyPressed) == 0)
	{
		return false;
	}
	
	TuningModel* m_model = dynamic_cast<TuningModel*>(model());
	if (m_model == nullptr)
	{
		assert(m_model);
		return false;
	}

	int row = index.row();
	if (row < 0 || row >= m_model->rowCount())
	{
		assert(false);
		return false;
	}

	int columnIndex = 0;

	if (m_model->valueColumnsCount() == 1)
	{
		// For single-column, find value column index and edit it
		//
		for (int i = 0; i < m_model->columnCount(); i++)
		{
			if (m_model->columnType(i) == TuningModelColumns::ValueFirst)
			{
				break;
			}
			columnIndex++;
		}
	}
	else
	{
		// For multiple-columns, edit exactly selected column
		//
		columnIndex = index.column();
	}

	int	columnType = static_cast<int>(m_model->columnType(columnIndex));

	int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
	if (valueColumn < 0 || valueColumn >= TuningLib::MaxValuesColumnCount)
	{
		return false;
	}

	Hash hash = m_model->hashByIndex(row, valueColumn);

	bool ok = false;
	AppSignalParam asp = m_model->tuningSignalManager().signalParam(hash, &ok);
	if (ok == false || asp.isAnalog() == false)
	{
		return false;
	}

	TuningSignalState state = m_model->tuningSignalManager().queuedState(hash, &ok);
	if (ok == false || m_helper.writingIsEnabled(asp, state) == false)
	{
		return false;
	}

	m_editorActive = true;

	return QTableView::edit(m_model->index(index.row(), columnIndex), trigger, event);
}

void TuningTableView::mousePressEvent(QMouseEvent* event)
{
	QTableView::mousePressEvent(event);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		QPoint pos = event->pos();

		QModelIndex mi = indexAt(pos);

		QRect vr = visualRect(mi);

		QStyleOption option;
		option.initFrom(this);
		QRect iconRect = QApplication::style()->subElementRect(QStyle::SubElement::SE_CheckBoxIndicator, &option, this);

		const int iconOffset = (vr.height() - iconRect.height()) / 2;

		QRect iconRectangle;
		iconRectangle.setTopLeft(vr.topLeft() + QPoint(2, iconOffset));
		iconRectangle.setWidth(iconRect.width());
		iconRectangle.setHeight(iconRect.height());

		if (iconRectangle.contains(pos))
		{
			emit checkBoxClicked(mi);
		}
	}
}

void TuningTableView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{

	//qDebug() << "closeEditor";

	m_editorActive = false;

	QTableView::closeEditor(editor, hint);
}

//
// TuningPageColumnsWidth
//

TuningPageColumnsWidth::TuningPageColumnsWidth()
{
	m_defaultWidthMap[TuningModelColumns::CustomAppSignalID] = 180;
	m_defaultWidthMap[TuningModelColumns::EquipmentID] = 180;
	m_defaultWidthMap[TuningModelColumns::AppSignalID] = 180;
	m_defaultWidthMap[TuningModelColumns::Caption] = 180;
	m_defaultWidthMap[TuningModelColumns::Units] = 70;
	m_defaultWidthMap[TuningModelColumns::Type] = 70;

	for (int i = 0; i < TuningLib::MaxValuesColumnCount; i++)
	{
		int valueColumn = static_cast<int>(TuningModelColumns::ValueFirst) + i;
		m_defaultWidthMap[static_cast<TuningModelColumns>(valueColumn)] = 70;
	}

	m_defaultWidthMap[TuningModelColumns::LowLimit] = 70;
	m_defaultWidthMap[TuningModelColumns::HighLimit] = 70;
	m_defaultWidthMap[TuningModelColumns::Default] = 70;
	m_defaultWidthMap[TuningModelColumns::Valid] = 70;
	m_defaultWidthMap[TuningModelColumns::OutOfRange] = 70;
}

bool TuningPageColumnsWidth::load(const QString& pageId)
{
	m_widthMap.clear();

	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
	QString value = settings.value(QString("PageColumnsWidth/%1").arg(pageId)).toString();

	QStringList l = value.split(';', Qt::SkipEmptyParts);

	for (const QString& s : l)
	{
		QStringList pairList = s.split('=');
		if (pairList.size() != 2)
		{
			assert(false);
			return false;
		}

		TuningModelColumns column = static_cast<TuningModelColumns>(pairList[0].toInt());
		int width = pairList[1].toInt();

		setWidth(column, width);
	}


	return true;
}

bool TuningPageColumnsWidth::save(const QString& pageId) const
{
	if (pageId.isEmpty() == true)
	{
		assert(false);
		return false;
	}

	QString value;

	for (const auto& it : m_widthMap)
	{
		TuningModelColumns column = it.first;
		int width = it.second;

		value += QString("%1=%2;").arg(static_cast<int>(column)).arg(width);
	}

	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
	settings.setValue(QString("PageColumnsWidth/%1").arg(pageId), value);

	return true;
}

int TuningPageColumnsWidth::width(TuningModelColumns column) const
{
	auto it = m_widthMap.find(column);
	if (it != m_widthMap.end())
	{
		return it->second;
	}

	auto dit = m_defaultWidthMap.find(column);
	if (dit != m_defaultWidthMap.end())
	{
		return dit->second;
	}

	assert(false);
	return 100;

}

void TuningPageColumnsWidth::setWidth(TuningModelColumns column, int width)
{
	m_widthMap[column] = width;
}

//
// TuningPage
//

int TuningPage::m_instanceCounter = 0;

TuningPage::TuningPage(TuningConfigController& configController,
					   ClientLib::TuningSignalManager& tuningSignalManager,
					   TuningLib::TuningUiStorage& tuningUi,
					   TuningSignalListSet& appSignalLists,
					   ClientLib::TuningUserManager& userManager,
					   ClientLib::TuningConnection& tuningConnection,
					   const QUuid& treeListUuid,                // List selected in list tree
					   const TuningLib::TuningUiItem& pageUi, // Ui item specifies this page
					   const TuningCountersManager& tuningCounters,
					   QWidget* parent) :
	QWidget(parent),
	m_configController(configController),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningUi(tuningUi),
	m_appSignalLists(appSignalLists),
	m_userManager(userManager),
	m_tuningConnection(tuningConnection),
	m_treeListUuid(treeListUuid),
	m_pageUi(&pageUi),
	m_tuningCounters(tuningCounters),
	m_helper(userManager)
{
	Q_ASSERT(m_pageUi);

	// Get the tab filter (if page filter is button, take its parent, if no tab exists - root will be taken

	static TuningLib::TuningUiItem emptyTab;

	const TuningLib::TuningUiItem* tabUi = m_pageUi;
	
	while (tabUi->isTab() == false && tabUi->isRoot() == false) 
	{
		tabUi = tabUi->parentItem();
	}
	if (tabUi == nullptr || tabUi->isTab() == false) 
	{
		tabUi = &emptyTab;
	}

	//qDebug() << "TuningPage::TuningPage m_instanceCounter = " << m_instanceCounter;

	m_instanceNo = m_instanceCounter;
	m_instanceCounter++;

	// Object List
	//
	m_objectList = new TuningTableView(m_userManager);
	m_objectList->setWordWrap(false);
	m_objectList->horizontalHeader()->setHighlightSections(false);

	// Models and data
	//

	std::vector<QString> valueColumnsAppSignalIdSuffixes = tabUi->valueColumnsAppSignalIdSuffixes();

	m_model = new TuningModelClient(m_tuningSignalManager, m_userManager, valueColumnsAppSignalIdSuffixes, this);
	m_objectList->setItemDelegate(new SelectionControlDelegate(this, m_model));

	QFont f = m_objectList->font();

	f.setBold(false);
	m_model->setFont(f);

	f.setBold(true);
	m_model->setImportantFont(f);

	if (tabUi->columnCustomAppId() == true)
	{
		m_model->addColumn(TuningModelColumns::CustomAppSignalID);
	}
	if (tabUi->columnAppId() == true)
	{
		m_model->addColumn(TuningModelColumns::AppSignalID);
	}
	if (tabUi->columnEquipmentId() == true)
	{
		m_model->addColumn(TuningModelColumns::EquipmentID);
	}
	if (tabUi->columnCaption() == true)
	{
		m_model->addColumn(TuningModelColumns::Caption);
	}
	if (tabUi->columnUnits() == true)
	{
		m_model->addColumn(TuningModelColumns::Units);
	}
	if (tabUi->columnType() == true)
	{
		m_model->addColumn(TuningModelColumns::Type);
	}

	int valuesColumnsCount = m_model->valueColumnsCount();
	for (int c = 0; c < valuesColumnsCount; c++)
	{
		int valueColumn = static_cast<int>(TuningModelColumns::ValueFirst) + c;
		m_model->addColumn(static_cast<TuningModelColumns>(valueColumn));
	}

	if (tabUi->columnLimits() == true)
	{
		m_model->addColumn(TuningModelColumns::LowLimit);
		m_model->addColumn(TuningModelColumns::HighLimit);
	}
	if (tabUi->columnDefault() == true)
	{
		m_model->addColumn(TuningModelColumns::Default);
	}
	if (tabUi->columnValid() == true)
	{
		m_model->addColumn(TuningModelColumns::Valid);
	}
	if (tabUi->columnOutOfRange() == true)
	{
		m_model->addColumn(TuningModelColumns::OutOfRange);
	}

	m_bottomLayout = new QHBoxLayout();

	// Filter controls
	//
	m_filterTypeCombo = new QComboBox();
	m_filterTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterIDType::All));
	m_filterTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterIDType::AppSignalID));
	m_filterTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterIDType::CustomAppSignalID));
	m_filterTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterIDType::EquipmentID));
	m_filterTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterIDType::Caption));
	m_bottomLayout->addWidget(m_filterTypeCombo);

	connect(m_filterTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_FilterTypeIndexChanged(int)));

	m_filterTypeCombo->setCurrentIndex(0);

	// Masks combo

	m_filterTextCombo = new QComboBox();
	m_filterTextCombo->setEditable(true);
	m_filterTextCombo->setMinimumWidth(150);
	m_filterTextCombo->setInsertPolicy(QComboBox::NoInsert);
	m_bottomLayout->addWidget(m_filterTextCombo);

	// Load masks
	//
	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
	QStringList masks = settings.value(QString("Masks/%1").arg(m_pageUi->uuidString())).toStringList();
	m_filterTextCombo->addItems(masks);
	m_filterTextCombo->setEditText(QString());

	QLineEdit* filterLineEdit = m_filterTextCombo->lineEdit();
	if (filterLineEdit == nullptr)
	{
		Q_ASSERT(filterLineEdit);
	}
	else
	{
		connect(filterLineEdit, &QLineEdit::returnPressed, this, &TuningPage::slot_ApplyFilter);

		filterLineEdit->setClearButtonEnabled(true);
		connect(filterLineEdit, &QLineEdit::textChanged, this, [this](const QString& str) {
			if (str.isEmpty() == true)
			{
				// Process mask if text was cleared
				slot_ApplyFilter();
			}
			});
	}

	// Filter button

	m_filterButton = new QPushButton(tr("Filter"));
	m_bottomLayout->addWidget(m_filterButton);
	connect(m_filterButton, &QPushButton::clicked, this, &TuningPage::slot_ApplyFilter);

	m_bottomLayout->addSpacing(20);

	// Value filter controls
	//

	QLabel* l = new QLabel(tr("Value:"));
	m_bottomLayout->addWidget(l);

	m_filterValueCombo = new QComboBox();
	m_filterValueCombo->addItem(tr("Any Value"), static_cast<int>(FilterValueType::All));
	m_filterValueCombo->addItem(tr("Discrete 0"), static_cast<int>(FilterValueType::Zero));
	m_filterValueCombo->addItem(tr("Discrete 1"), static_cast<int>(FilterValueType::One));
	m_filterValueCombo->addItem(tr("Not Default"), static_cast<int>(FilterValueType::DefaultNotSet));
	m_bottomLayout->addWidget(m_filterValueCombo);

	connect(m_filterValueCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_FilterValueIndexChanged(int)));

	m_filterValueCombo->setCurrentIndex(0);

	m_bottomLayout->addStretch();


	// Button controls
	//

	m_setValueButton = new QPushButton(tr("Set Value"));
	m_bottomLayout->addWidget(m_setValueButton);
	connect(m_setValueButton, &QPushButton::clicked, this, &TuningPage::slot_setValue);

	m_setAllButton = new QPushButton(tr("Set All"));
	m_bottomLayout->addWidget(m_setAllButton);
	connect(m_setAllButton, &QPushButton::clicked, this, &TuningPage::slot_setAll);

	m_bottomLayout->addStretch();

	m_writeButton = new QPushButton(tr("Write"));
	m_bottomLayout->addWidget(m_writeButton);
	connect(m_writeButton, &QPushButton::clicked, this, &TuningPage::slot_Write);

	m_undoButton = new QPushButton(tr("Undo"));
	m_bottomLayout->addWidget(m_undoButton);
	connect(m_undoButton, &QPushButton::clicked, this, &TuningPage::slot_undo);

	if (m_configController.configuration().clientSettings.applyMode == TuningClientSettings::ApplyMode::Manual)
	{
		m_applyButton = new QPushButton(tr("Apply"));
		connect(m_applyButton, &QPushButton::clicked, this, &TuningPage::slot_Apply);
		m_bottomLayout->addWidget(m_applyButton);
	}

	m_mainLayout = new QVBoxLayout(this);

	m_mainLayout->addWidget(m_objectList);
	m_mainLayout->addLayout(m_bottomLayout);


	m_objectList->setModel(m_model);
	m_objectList->verticalHeader()->hide();
	m_objectList->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_objectList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_objectList->setSortingEnabled(true);
	m_objectList->setEditTriggers(QAbstractItemView::EditKeyPressed);

	m_objectList->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_objectList, &QWidget::customContextMenuRequested, this, &TuningPage::slot_listContextMenuRequested);

	connect(m_objectList->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &TuningPage::sortIndicatorChanged);

	connect(m_objectList, &QTableView::doubleClicked, this, &TuningPage::slot_tableDoubleClicked);

	connect(m_objectList, &TuningTableView::checkBoxClicked, this, &TuningPage::slot_tableCheckboxClicked);

	m_objectList->installEventFilter(this);

	fillObjectsList();

	// load column width

	m_columnWidthStorage.load(m_pageUi->uuidString());

	for (int c = 0; c < m_model->columnCount(); c++)
	{
		int width = m_columnWidthStorage.width(m_model->columnType(c));

		m_objectList->setColumnWidth(c, width);
	}

	// Color

	if (tabUi->useColors() == true)
	{
		QPalette Pal(palette());

		Pal.setColor(QPalette::Window, tabUi->backColor());
		setAutoFillBackground(true);
		setPalette(Pal);
	}
	else
	{
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
	}

	//

	connect(theApp.mainWindow(), &MainWindow::timerTick500, this, &TuningPage::onTimer);
	connect(&m_userManager, &ClientLib::TuningUserManager::loggedOut, this, &TuningPage::slot_undo);

}

TuningPage::~TuningPage()
{
	Q_ASSERT(m_pageUi);

	for (int c = 0; c < m_model->columnCount(); c++)
	{
		m_columnWidthStorage.setWidth(m_model->columnType(c), m_objectList->columnWidth(c));
	}

	m_columnWidthStorage.save(m_pageUi->uuidString());

	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	// Save masks
	//

	QStringList masks;
	for (int i = 0; i < m_filterTextCombo->count(); i++)
	{
		masks.push_back(m_filterTextCombo->itemText(i));
	}
	settings.setValue(QString("Masks/%1").arg(m_pageUi->uuidString()), masks);

	m_instanceCounter--;
	//qDebug() << "TuningPage::TuningPage m_instanceCounter = " << m_instanceCounter;
}

void TuningPage::fillObjectsList()
{
	Q_ASSERT(m_pageUi);

	//qDebug() << "FillObjectsList";

	if (m_pageUi == nullptr) 
	{
		Q_ASSERT(m_pageUi);
		return;
	}

	std::set<Hash> hashes;
	
	if (m_pageUi->filtersList().isEmpty() == true)
	{
		auto allHashes = m_tuningSignalManager.signalHashes();
		hashes.insert(allHashes.begin(), allHashes.end());
	}
	else
	{
		static const auto re =
			QRegularExpression("[;\\s]"); // Separators are whitespace and semicolon, '+' is NOT a separator!  We need to keep unions.
		QStringList filtersList = m_pageUi->filters().split(re, Qt::SkipEmptyParts);
		std::set<QString> filtersSet;
		for (const QString& s : filtersList)
		{
			filtersSet.insert(s);
		}

		hashes = m_appSignalLists.filtersSetHashes(filtersSet);
	}

	// Tree Filter

	std::set<Hash> treeHashes;

	AppSignalLists::AppSignalList* treeList = m_treeListUuid.isNull() == false ? m_appSignalLists.get(m_treeListUuid).get() : nullptr;
	if (treeList != nullptr)
	{
		treeHashes = treeList->tuningListHashesCache();

		std::vector<Hash> v_intersection;
		std::set_intersection(hashes.begin(),
							  hashes.end(),
							  treeHashes.begin(),
							  treeHashes.end(),
							  std::back_inserter(v_intersection));
		hashes.clear();
		hashes.insert(v_intersection.begin(), v_intersection.end());
	}

	//

	std::vector<Hash> filteredHashes;
	filteredHashes.reserve(hashes.size());

	std::vector<std::pair<Hash, TuningValue>> defaultValues;

	QString mask = m_filterTextCombo->currentText();
	if (mask.isEmpty() == false)
	{
		if (m_filterTextCombo->findText(mask) == -1)
		{
			m_filterTextCombo->addItem(mask);
		}
		while (m_filterTextCombo->count() > 10)
		{
			m_filterTextCombo->removeItem(0);
		}
		m_filterTextCombo->setCurrentText(mask);
	}

	FilterIDType filterType = FilterIDType::All;
	QVariant data = m_filterTypeCombo->currentData();
	if (data.isValid() == true)
	{
		filterType = static_cast<FilterIDType>(data.toInt());
	}

	FilterValueType filterValue = FilterValueType::All;
	data = m_filterValueCombo->currentData();
	if (data.isValid() == true)
	{
		filterValue = static_cast<FilterValueType>(data.toInt());
	}

	bool ok = false;

	for (Hash hash : hashes)
	{
		const AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

		if (ok == false)
		{
			assert(ok);
			continue;
		}

		bool modifyDefaultValue = false;
		TuningValue modifiedDefaultValue;

		// Modify the default value from selected tree filter
		//
		if (treeList != nullptr)
		{
			if (treeList->itemExists(hash) == true)
			{
				AppSignalLists::AppSignalListItem tv = treeList->itemByHash(hash);
				if (tv.hasValue() == true)
				{
					modifyDefaultValue = true;
					modifiedDefaultValue = tv.value();
				}
			}
		}
		/*else
		{
			// Modify the default value from page filter
			//
			if (m_pageFilter == nullptr)
			{
				Q_ASSERT(m_pageFilter);
				return;
			}

			if (m_pageFilter->filterSignalExists(hash) == true)
			{
				TuningFilterSignal filterSignal;

				ok = m_pageFilter->filterSignal(hash, filterSignal);
				if (ok == false)
				{
					Q_ASSERT(false);
					return;
				}

				if (filterSignal.useValue() == true)
				{
					modifyDefaultValue = true;
					modifiedDefaultValue = filterSignal.value();
				}
			}
		}*/

		// Value filter
		//

		if (filterValue != FilterValueType::All)
		{
			if (filterValue == FilterValueType::DefaultNotSet)
			{
				ok = false;

				const TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

				if (ok == false || state.valid() == false)
				{
					continue;
				}

				if (state.isTuningDefault() == true)
				{
					// Value is set to default
					continue;
				}
			}
			else
			{
				if (asp.isAnalog() == true)
				{
					continue;
				}

				if (asp.isDiscrete() == true)
				{
					ok = false;

					const TuningSignalState state = m_tuningSignalManager.state(hash, &ok);

					if (ok == false || state.valid() == false)
					{
						continue;
					}

					if (filterValue == FilterValueType::Zero && state.value().discreteValue() != 0)
					{
						continue;
					}

					if (filterValue == FilterValueType::One && state.value().discreteValue() != 1)
					{
						continue;
					}
				}
			}
		}

		// Text filter
		//


		if (mask.isEmpty() == false)
		{
			bool filterMatch = false;

			switch (filterType)
			{
			case FilterIDType::All:
				if (asp.appSignalId().contains(mask, Qt::CaseInsensitive) == true
						|| asp.customSignalId().contains(mask, Qt::CaseInsensitive) == true
						|| asp.lmEquipmentId().contains(mask, Qt::CaseInsensitive) == true
						|| asp.caption().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::AppSignalID:
				if (asp.appSignalId().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::CustomAppSignalID:
				if (asp.customSignalId().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::EquipmentID:
				if (asp.lmEquipmentId().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::Caption:
				if (asp.caption().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			}

			if (filterMatch == false)
			{
				continue;
			}
		}


		filteredHashes.push_back(hash);

		if (modifyDefaultValue == true)
		{
			defaultValues.push_back(std::make_pair(hash, modifiedDefaultValue));
		}
	}

	m_model->setHashes(filteredHashes);
	m_model->setDefaultValues(defaultValues);

	// Sort list
	//
	const auto& it = m_sortData.find(m_pageUi->uuidString());
	if (it == m_sortData.end())
	{
		const std::pair<int, Qt::SortOrder> sortData = std::make_pair(0, Qt::AscendingOrder);
		m_sortData[m_pageUi->uuidString()] = sortData;
		m_objectList->sortByColumn(sortData.first, sortData.second);
	}
	else
	{
		const std::pair<int, Qt::SortOrder>& sortData = it->second;
		m_objectList->sortByColumn(sortData.first, sortData.second);
	}

	return;
}

bool TuningPage::hasPendingChanges()
{
	return m_model->hasPendingChanges();
}

bool TuningPage::askForSavePendingChanges()
{
	bool hasPendingChanges = m_model->hasPendingChanges();

	if (hasPendingChanges == false)
	{
		return true;
	}

	QMessageBox msgBox{this};
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setText(tr("Some values were modified but not written. Please select the following:"));
	QPushButton* saveButton = msgBox.addButton(QMessageBox::Save);
	QPushButton* undoButton = msgBox.addButton(tr("Undo"), QMessageBox::ActionRole);
	/*QPushButton* cancelButton = */msgBox.addButton(QMessageBox::Cancel);

	msgBox.exec();

	if (msgBox.clickedButton() == saveButton)
	{
		return write();
	}

	if (msgBox.clickedButton() == undoButton)
	{
		undo();
		return true;
	}

	//	if (msgBox.clickedButton() == cancelButton)
	//	{
	//		return false;
	//	}

	return false;
}

bool TuningPage::write()
{
	std::vector<Hash> allHashes = m_model->allHashes();

	std::vector<Hash> modifiedHashes;
	modifiedHashes.reserve(allHashes.size());

	std::map<Hash, QString> modifiedSourcesMap;	// Key is source hash, value is LM eqipment Id

	for (Hash hash : allHashes)
	{
		if (m_tuningSignalManager.isUnapplied(hash) == false)
		{
			continue;
		}

		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

		if (ok == false || state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
		{
			continue;
		}

		modifiedHashes.push_back(hash);

		// Create list of sources to take control
		//
		if (m_configController.singleLmControlMode() == true)
		{
			AppSignalParam param = m_tuningSignalManager.signalParam(hash, &ok);
			if (ok == false)
			{
				continue;
			}

			modifiedSourcesMap[::calcHash(param.lmEquipmentId())] = param.lmEquipmentId();
		}
	}

	if (modifiedHashes.empty() == true)
	{
		return false;
	}

	if (m_userManager.login(this) == false)
	{
		return false;
	}

	// Check if all sources connections are active
	//
	QStringList partiallyActivatedSources;

	for (const auto& it : modifiedSourcesMap)
	{
		int sourceStatesCount = m_tuningConnection.tuningSourceStatesCount(it.first);
		int activeStatesCount = m_tuningConnection.activatedTuningSourceStatesCount(it.first);

		if (activeStatesCount < sourceStatesCount)
		{
			partiallyActivatedSources.push_back(it.second);
		}
	}

	if (partiallyActivatedSources.empty() == false)
	{
		if (QMessageBox::warning(this, qAppName(), tr("To write changes, all connections to following tuning sources will be activated:\n\n%1\n\nContinue?")
								 .arg(partiallyActivatedSources.join('\n')),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return false;
		}
	}

	// Take control on required services
	//
	for (const auto& it : modifiedSourcesMap)
	{
		if (m_tuningConnection.activateTuningSource(it.first, true) == false)
		{
			QMessageBox::critical(this, qAppName(), QObject::tr("Activating control of tuning source '%1' failed!").arg(it.second));
			return false;
		}

		if (m_tuningConnection.takeClientControl(it.first) == false)
		{
			QMessageBox::critical(this, qAppName(), QObject::tr("Taking control of tuning source '%1' failed!").arg(it.second));
			return false;
		}
	}

	std::vector<AppSignalParam> writeSignalParams;
	std::vector<TuningValue> oldValues;
	std::vector<TuningValue> newValues;

	// Ask confirmation question
	//
	for (Hash hash : modifiedHashes)
	{
		bool ok = false;
		writeSignalParams.push_back(m_tuningSignalManager.signalParam(hash, &ok));
		oldValues.push_back(m_tuningSignalManager.state(hash, &ok).value());
		newValues.push_back(m_tuningSignalManager.unappliedValue(hash));
	}

	auto confirmation =
		SchemaClientLib::DialogWriteTuningValues::askConfirmation(writeSignalParams, oldValues, newValues, m_model->analogFormat(), this);

	if (confirmation != QDialog::Accepted)
	{
		return false;
	}

	// Write values on all clients
	//
	std::vector<ClientLib::TuningWriteCommand> commands;

	for (Hash hash : modifiedHashes)
	{
		commands.push_back({hash, m_tuningSignalManager.unappliedValue(hash)});
	}

	m_tuningConnection.writeTuningSignals(commands);

	return true;
}

void TuningPage::apply()
{
	if (m_userManager.login(this) == false)
	{
		return;
	}

	// Get SOR counters

	if (m_tuningCounters.totalCounters().sorCounter > 0)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!!!\n\nSOR Signal(s) are set in logic modules!\n\nIf you apply these changes, module can run into RUN SAFE STATE.\n\nAre you sure you STILL WANT TO APPLY the changes?"),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	std::vector<Hash> allHashes = m_model->allHashes();

	std::vector<Hash> hashesToApply;

	std::map<Hash, QString> modifiedSourcesMap;

	for (Hash hash : allHashes)
	{
		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);
		
		TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

		if (ok == false || state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
		{
			continue;
		}

		hashesToApply.push_back(hash);

		// Create list of sources to take control
		//
		if (m_configController.singleLmControlMode() == true)
		{
			AppSignalParam param = m_tuningSignalManager.signalParam(hash, &ok);
			if (ok == false)
			{
				Q_ASSERT(ok);
				continue;
			}

			modifiedSourcesMap[::calcHash(param.lmEquipmentId())] = param.lmEquipmentId();
		}
	}

	// Take control on required sources
	//
	if (modifiedSourcesMap.empty() == false)
	{
		for (const auto& it: modifiedSourcesMap)
		{
			// Take control on required sources
			//
			if (m_tuningConnection.takeClientControl(it.first) == false)
			{
				QMessageBox::critical(this, qAppName(), QObject::tr("Taking control of tuning source '%1' failed!").arg(it.second));
				return;
			}
		}
	}

	if (QMessageBox::warning(this, qAppName(),
							 tr("Are you sure you want apply the changes?"),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	m_tuningConnection.applyTuningSignals(hashesToApply);

	return;
}

void TuningPage::undo()
{
	slot_undo();
}

void TuningPage::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	Q_ASSERT(m_pageUi);
	m_sortData[m_pageUi->uuidString()] = std::make_pair(column, order);
	m_objectList->sortByColumn(column, order);

	return;
}

void TuningPage::slot_setValue()
{
	QModelIndexList selection = m_objectList->selectionModel()->selectedRows();

	std::vector<int> selectedRows;

	for (const QModelIndex& i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == true)
	{
		return;
	}

	bool first = true;
	TuningValue value;
	TuningValue defaultValue;
	bool sameValue = true;
	bool sameDefaultValue = true;
	int precision = 0;
	TuningValue lowLimit;
	TuningValue highLimit;

	std::vector<Hash> selectedHashes;

	for (int row : selectedRows)
	{
		const TuningModelHashSet& hashSet = m_model->hashSetByIndex(row);

		for (int c = 0; c < m_model->valueColumnsCount(); c++)
		{
			Hash hash = hashSet.hash[c];

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			bool ok = false;

			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
			{
				continue;
			}

			if (asp.isAnalog() == true)
			{
				if (state.limitsUnbalance(asp) == true)
				{
					QMessageBox::warning(this, tr("Set Value"), tr("There is limits mismatch in signal '%1'. Value setting is disabled.").arg(asp.customSignalId()));
					return;
				}
			}

			if (asp.isAnalog() == true)
			{
				if (asp.precision() > precision)
				{
					precision = asp.precision();
				}
			}

			if (first == true)
			{
				value = state.value();
				defaultValue = m_model->defaultValue(asp);
				lowLimit = asp.tuningLowBound();
				highLimit = asp.tuningHighBound();
				first = false;
			}
			else
			{
				if (asp.tuningType() != value.type())
				{
					QMessageBox::warning(this, tr("Set Value"), tr("Please select objects of the same type."));
					return;
				}

				if (asp.isAnalog() == true)
				{
					if (lowLimit != asp.tuningLowBound() || highLimit != asp.tuningHighBound())
					{
						QMessageBox::warning(this, tr("Set Value"), tr("Selected objects have different input range."));
						return;
					}
				}

				if (value != state.value())
				{
					sameValue = false;
				}

				if (defaultValue != m_model->defaultValue(asp))
				{
					sameDefaultValue = false;
				}
			}

			selectedHashes.push_back(hash);
		}	// c
	}	// row

	if (selectedHashes.empty() == true)
	{
		return;
	}

	DialogInputTuningValue d(value, defaultValue, sameValue, sameDefaultValue, lowLimit, highLimit, m_model->analogFormat(), precision, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	TuningValue newValue = d.value();

	for (Hash hash : selectedHashes)
	{
		m_tuningSignalManager.setUnappliedValue(hash, newValue);
	}
}

void TuningPage::slot_tableDoubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	slot_setValue();
}

void TuningPage::slot_FilterTypeIndexChanged(int index)
{
	Q_UNUSED(index);
	fillObjectsList();
}

void TuningPage::slot_FilterValueIndexChanged(int index)
{
	FilterValueType filterValue = FilterValueType::All;
	QVariant data = m_filterValueCombo->currentData();
	if (data.isValid() == true)
	{
		filterValue = static_cast<FilterValueType>(data.toInt());
	}

	if (filterValue != FilterValueType::All)
	{
		m_filterValueCombo->setStyleSheet("QComboBox { color: red }");
	}
	else
	{
		m_filterValueCombo->setStyleSheet(QString());
	}

	Q_UNUSED(index);
	fillObjectsList();
}

void TuningPage::slot_listContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	QModelIndexList mi = m_objectList->selectionModel()->selectedRows();

	std::vector<Hash> selectedHashes;
	for (const QModelIndex& index : mi)
	{
		if (index.isValid() == false)
		{
			Q_ASSERT(index.isValid());
			return;
		}
		const TuningModelHashSet& hashes = m_model->hashSetByIndex(index.row());
		for (int i = 0; i < hashes.hashCount(); i++)
		{
			selectedHashes.push_back(hashes.hash[i]);
		}
	}

	bool writeEnabled = true;
	for (Hash hash : selectedHashes)
	{
		bool found = false;
		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &found);
		TuningSignalState state = m_tuningSignalManager.state(hash, &found);

		if (m_helper.writingIsEnabled(asp, state) == false)
		{
			writeEnabled = false;
			break;
		}
	}

	QMenu menu(this);

	// Set Value
	//
	{
		QAction* a = new QAction(tr("Set Value..."), &menu);
		auto f = [this]() -> void
		{
			slot_setValue();
		};
		connect(a, &QAction::triggered, this, f);
		a->setEnabled(writeEnabled && selectedHashes.empty() == false);
		menu.addAction(a);
	}

	// Set defaults
	//
	{
		QAction* a = new QAction(tr("Set to Defaults"), &menu);
		auto f = [this, selectedHashes]() -> void
		{
			setToDefaults(selectedHashes);
		};
		connect(a, &QAction::triggered, this, f);
		a->setEnabled(writeEnabled && selectedHashes.empty() == false);
		menu.addAction(a);
	}

	menu.addSeparator();

	// Signal actions
	//
	{
		int menuSignalCount = 0;

		for (const QModelIndex& index : mi)
		{
			if (index.isValid() == false)
			{
				return;
			}

			const TuningModelHashSet& hashes = m_model->hashSetByIndex(index.row());

			for (int i = 0; i < m_model->valueColumnsCount(); i++)
			{
				Hash hash = hashes.hash[i];

				if (hash == UNDEFINED_HASH)
				{
					continue;
				}

				bool found = false;

				AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &found);

				if (found == false)
				{
					assert(false);
					return;
				}

				QAction* a = new QAction(tr("%1 - %2").arg(asp.customSignalId()).arg(asp.caption()), &menu);

				auto f = [this, hash]() -> void
				{
					TuningSignalInfo* d = new TuningSignalInfo(m_configController,
															   m_tuningSignalManager,
															   m_tuningConnection,
															   hash,
															   m_model->analogFormat(),
															   this);
					d->show();
				};

				connect(a, &QAction::triggered, this, f);

				menu.addAction(a);

				menuSignalCount++;

				if (menuSignalCount > 16)
				{
					a = new QAction(tr("..."), &menu);
					a->setEnabled(false);
					menu.addAction(a);
					break;
				}
			}

			if (menuSignalCount > 16)
			{
				break;
			}
		}

		if (menuSignalCount == 0)
		{
			return;
		}
	}

	// Add additional commands

	menu.addSeparator();

	// View
	{
		QMenu* submenuV = menu.addMenu(tr("Format"));

		QAction* a = new QAction(tr("Auto-select"), &menu);
		a->setCheckable(true);
		a->setChecked(m_model->analogFormat() == E::AnalogFormat::g_9_or_9e || m_model->analogFormat() == E::AnalogFormat::G_9_or_9E);
		connect(a, &QAction::triggered, this, [this]()
				{
					slot_setAnalogFormat(E::AnalogFormat::g_9_or_9e);
				});
		submenuV->addAction(a);

		a = new QAction(tr("Decimal (as [-]9.9)"), &menu);
		a->setCheckable(true);
		a->setChecked(m_model->analogFormat() == E::AnalogFormat::f_9);
		connect(a, &QAction::triggered, this, [this]()
				{
					slot_setAnalogFormat(E::AnalogFormat::f_9);
				});
		submenuV->addAction(a);

		a = new QAction(tr("Exponential (as [-]9.9e[+|-]999)"), &menu);
		a->setCheckable(true);
		a->setChecked(m_model->analogFormat() == E::AnalogFormat::e_9e || m_model->analogFormat() == E::AnalogFormat::E_9E);
		connect(a, &QAction::triggered, this, [this]()
				{
					slot_setAnalogFormat(E::AnalogFormat::e_9e);
				});
		submenuV->addAction(a);
	}

	// More
	{
		QMenu* submenuA = menu.addMenu(tr("More"));

		QAction* a = new QAction(tr("Add To New List..."), &menu);
		connect(a, &QAction::triggered, this, &TuningPage::slot_saveSignalsToNewFilter);
		submenuA->addAction(a);

		a = new QAction(tr("Add To Existing List..."), &menu);
		connect(a, &QAction::triggered, this, &TuningPage::slot_saveSignalsToExistingFilter);
		submenuA->addAction(a);

		// If AutoFilter filter exists, add Restore command
		//
		for (int i = 0; i < m_appSignalLists.count(); i++)
		{
			if (m_appSignalLists.get(i)->systemTagsList().contains(AppSignalLists::AppSignalList::tagTcAuto) == true)
			{
				submenuA->addSeparator();

				a = new QAction(tr("Restore Values From List..."), &menu);
				connect(a, &QAction::triggered, this, &TuningPage::slot_restoreValuesFromExistingFilter);
				submenuA->addAction(a);
			}
		}

		submenuA->addSeparator();

		a = new QAction(tr("Export Current View to CSV..."), &menu);
		connect(a, &QAction::triggered, this, &TuningPage::slot_exportContentsToCSV);
		submenuA->addAction(a);
	}

	menu.exec(QCursor::pos());

}

void TuningPage::slot_saveSignalsToNewFilter()
{
	if (m_userManager.login(this) == false)
	{
		return;
	}

	QString filterName;
	do
	{
		bool ok;
		filterName =
			QInputDialog::getText(this, tr("Add Signals To List"), tr("Enter the list name:"), QLineEdit::Normal, tr("Name"), &ok);

		if (ok == false)
		{
			return;
		}
		bool alreadyExists = false;
		for (int i = 0; i < m_appSignalLists.count(); i++)
		{
			if (m_appSignalLists.get(i)->caption() == filterName)
			{
				alreadyExists = true;
				break;
			}
		}
		if (alreadyExists == true) 
		{
			QMessageBox::critical(this, qAppName(), tr("List with such name already exists. Please enter another name."));
		}
		else 
		{
			break;
		}
	}while(true);

	std::shared_ptr<AppSignalLists::AppSignalList> autoCreatedList = std::make_shared<AppSignalLists::AppSignalList>();
	autoCreatedList->setId(autoCreatedList->uuid().toString());
	autoCreatedList->setCaption(filterName);
	autoCreatedList->systemTagsList().push_back(AppSignalLists::AppSignalList::tagTcAuto);
	m_appSignalLists.add(autoCreatedList);
	addSelectedSignalsToFilter(*autoCreatedList);
}

void TuningPage::slot_saveSignalsToExistingFilter()
{
	if (m_userManager.login(this) == false)
	{
		return;
	}

	DialogChooseFilter* d = new DialogChooseFilter(m_appSignalLists, {AppSignalLists::AppSignalList::tagTcAuto}, this);
	if (d->exec() != QDialog::Accepted)
	{
		return;
	}

	auto list = m_appSignalLists.get(d->chosenFilterUuid()).get();
	if (list == nullptr) 
	{
		Q_ASSERT(list);
		return;
	}
	addSelectedSignalsToFilter(*list);
}

void TuningPage::slot_exportContentsToCSV()
{
	int columnCount = m_model->columnCount();

	int rowCount = m_model->rowCount();

	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV"),
													path + QDir::separator(),
													tr("CSV (*.csv)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	QFile file(fileName);
	if (file.open(QFile::WriteOnly | QFile::Truncate) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Error writing file %1!").arg(fileName));
		return;
	}

	QTextStream out(&file);
	out.setEncoding(QStringConverter::Utf8);

	QString csvHeader;

	for (int c = 0; c < columnCount; c++)
	{
		QString str = m_model->columnText(c);
		csvHeader += str;
		csvHeader += ';';
	}

	out << csvHeader << "\r\n";

	for (int r = 0; r < rowCount; r++)
	{
		QString csvRow;

		for (int c = 0; c < columnCount; c++)
		{
			QString str = m_model->cellText(c, r);
			csvRow += str;
			csvRow += ';';
		}

		out << csvRow << "\r\n";
	}

	out.flush();

	QMessageBox::information(this, qAppName(), tr("Export complete."));

	return;
}

void TuningPage::slot_restoreValuesFromExistingFilter()
{

	DialogChooseFilter* d = new DialogChooseFilter(m_appSignalLists, {AppSignalLists::AppSignalList::tagTcAuto}, this);
	if (d->exec() != QDialog::Accepted)
	{
		return;
	}

	auto list = m_appSignalLists.get(d->chosenFilterUuid()).get();
	if (list == nullptr) 
	{
		Q_ASSERT(list);
		return;
	}

	restoreSignalsFromFilter(*list);
}

void TuningPage::slot_setAnalogFormat(E::AnalogFormat analogFormat)
{
	m_model->setAnalogFormat(analogFormat);

	m_objectList->update();
}

void TuningPage::slot_tableCheckboxClicked(const QModelIndex& index)
{
	int col = index.column();
	int columnType = static_cast<int>(m_model->columnType(col));

	int row = index.row();
	if (row >= m_model->rowCount())
	{
		assert(false);
		return;
	}

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		invertValue(columnType - static_cast<int>(TuningModelColumns::ValueFirst));
	}
}

void TuningPage::slot_ApplyFilter()
{
	if (m_filterTextCombo->currentText().isEmpty() == false)
	{
		m_filterTextCombo->setStyleSheet("QComboBox { color: red }");
		m_filterButton->setStyleSheet("QPushButton { color: red }");
	}
	else
	{
		m_filterTextCombo->setStyleSheet(QString());
		m_filterButton->setStyleSheet(QString());
	}

	fillObjectsList();
}

void TuningPage::slot_treeFilterChanged(const QUuid& filterUuid)
{
	m_treeListUuid = filterUuid;

	fillObjectsList();
}

void TuningPage::slot_pageFilterChanged(const QUuid& uiItemUuid)
{
	m_pageUi = m_tuningUi.get(uiItemUuid);
	Q_ASSERT(m_pageUi);

	fillObjectsList();
}

bool TuningPage::eventFilter(QObject* object, QEvent* event)
{
	if (object == m_objectList && event->type()==QEvent::KeyPress)
	{
		QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
		if(pKeyEvent->key() == Qt::Key_Return)
		{
			if (m_objectList->editorActive() == false)
			{
				slot_Write();
				return true;
			}
			return true;
		}

		if(pKeyEvent->key() == Qt::Key_Space)
		{
			invertValue(-1);
			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}

void TuningPage::invertValue(int channel)
{
	if (channel != -1 && (channel < 0 || channel >= m_model->valueColumnsCount()))
	{
		Q_ASSERT(false);
		return;
	}

	QModelIndexList selection = m_objectList->selectionModel()->selectedRows();

	std::vector<int> selectedRows;

	for (const QModelIndex& i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == true)
	{
		return;
	}

	bool ok = false;

	for (int row : selectedRows)
	{
		const TuningModelHashSet& hashSet = m_model->hashSetByIndex(row);

		for (int c = 0; c < m_model->valueColumnsCount(); c++)
		{
			if (channel != -1 && channel != c) 
			{
				continue;
			}

			Hash hash = hashSet.hash[c];

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
			{
				continue;
			}

			if (asp.isDiscrete() == true)
			{
				TuningValue tv;

				tv.setType(TuningValueType::Discrete);

				tv.setDiscreteValue(state.value().discreteValue());

				if (m_tuningSignalManager.isUnapplied(hash) == true)
				{
					tv = m_tuningSignalManager.unappliedValue(hash);
				}

				if (tv.discreteValue() == 0)
				{
					tv.setDiscreteValue(1);
				}
				else
				{
					tv.setDiscreteValue(0);
				}

				m_tuningSignalManager.setUnappliedValue(hash, tv);
			}
		}
	}
}

void TuningPage::addSelectedSignalsToFilter(AppSignalLists::AppSignalList& list)
{
	int addedCount = 0;

	QModelIndexList mi = m_objectList->selectionModel()->selectedRows();

	for (const QModelIndex& index : mi)
	{
		if (index.isValid() == false)
		{
			assert(false);
			return;
		}

		const TuningModelHashSet& hashes = m_model->hashSetByIndex(index.row());

		for (int i = 0; i < m_model->valueColumnsCount(); i++)
		{
			Hash hash = hashes.hash[i];

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			bool found = false;

			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &found);

			if (found == false)
			{
				assert(false);
				return;
			}

			TuningSignalState state = m_tuningSignalManager.queuedState(hash, &found);

			if (found == false)
			{
				continue;
			}

			
			AppSignalLists::AppSignalListItem tv;
			tv.setAppSignalId(asp.appSignalId());

			if (state.valid() == true)
			{
				tv.setValue(state.value());
			}

			list.add(tv);
			list.mutableAppListHashesCache().insert(asp.hash());
			list.mutableTuningListHashesCache().insert(asp.hash());

			addedCount++;
		}
	}

	if (addedCount == 0)
	{
		QMessageBox::warning(this, qAppName(), tr("No signals were added."));
		return;
	}

	// SaveFilters to file

    QString errorMsg;
    if (m_appSignalLists.save(&errorMsg) == false)
	{
		QMessageBox::critical(this, tr("Error"), errorMsg);
	}

	QMessageBox::information(this, qAppName(), tr("Adding signals complete."));

	QTimer::singleShot(500, theApp.mainWindow(), &MainWindow::slot_signalListsChanged);
}

void TuningPage::restoreSignalsFromFilter(const AppSignalLists::AppSignalList& list)
{
	int restoredCount = 0;

	;

	for (int r = 0; r < m_model->rowCount(); r++)
	{
		for (int c = 0; c < m_model->valueColumnsCount(); c++)
		{
			Hash hash = m_model->hashByIndex(r, c);

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			bool exists = list.itemExists(hash);
			if (exists == true)
			{
				AppSignalLists::AppSignalListItem tv = list.itemByHash(hash);

				bool found = false;

				TuningSignalState state = m_tuningSignalManager.queuedState(hash, &found);
				if (found == false)
				{
					continue;
				}

				AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &found);

				if (state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
				{
					continue;
				}

				if (state.value() != tv.value())
				{
					m_tuningSignalManager.setUnappliedValue(hash, tv.value());
					restoredCount++;
				}
			}
		}
	}

	if (restoredCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("No values restored from the list for current signals."));
	}
	else
	{
		QMessageBox::warning(this, qAppName(), tr("%1 values were restored from the list. Check them and apply the changes.").arg(restoredCount));
	}
}

void TuningPage::setToDefaults(const std::vector<Hash>& hashes)
{
	bool ok = false;

	for (Hash hash : hashes)
	{
		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

		if (state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
		{
			continue;
		}

		// If signal is not in default OR default value was overloaded then set it

		if ((state.isTuningDefault() == false || m_model->defaultValue(asp) != asp.tuningDefaultValue()) && ok == true)
		{
			TuningValue tvDefault = m_model->defaultValue(asp);

			if (tvDefault < asp.tuningLowBound() || tvDefault > asp.tuningHighBound())
			{
				QString message = tr("Invalid default value '%1' in signal %2 [%3]")
									  .arg(tvDefault.toString(m_model->analogFormat(), asp.precision()))
									  .arg(asp.appSignalId())
									  .arg(asp.caption());
				QMessageBox::critical(this, qAppName(), message);
			}
			else
			{
				m_tuningSignalManager.setUnappliedValue(hash, tvDefault);
			}
		}
	}
}

void TuningPage::setActionButtonsState()
{
	bool writeEnabled = false;
	bool setValueEnabled = false;
	bool setAllEnabled = false;

	bool applyButtonExists = m_configController.configuration().clientSettings.applyMode == TuningClientSettings::ApplyMode::Manual;
	bool applyButtonEnabled = false;

	std::vector<Hash> hashes = m_model->allHashes();

	std::set<Hash> sourceHashes;

	bool ok = false;

	for (Hash hash : hashes)
	{
		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

		if (applyButtonExists == true)
		{
			sourceHashes.insert(::calcHash(asp.lmEquipmentId()));
		}

		TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

		if (ok == false ||
				state.valid() == false ||
				state.controlIsEnabled() == false ||
				m_helper.writingIsEnabled(asp, state) == false)
		{
			continue;
		}

		setAllEnabled = true;

		if(m_tuningSignalManager.isUnapplied(hash))
		{
			writeEnabled = true;
		}

		if (setAllEnabled == true && writeEnabled == true)
		{
			break;
		}
	}

	if (setAllEnabled == true)
	{
		// Verify if Setting value is possible (selection is writable)

		QModelIndexList selection = m_objectList->selectionModel()->selectedRows();
		for (const QModelIndex& index : selection)
		{
			const TuningModelHashSet& hashSet = m_model->hashSetByIndex(index.row());
			for (int c = 0; c < m_model->valueColumnsCount(); c++)
			{
				Hash hash = hashSet.hash[c];
				if (hash == UNDEFINED_HASH)
				{
					continue;
				}
				
				AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

				TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);
				
				if (ok == true &&
						state.valid() == true &&
						state.controlIsEnabled() == true &&
						m_helper.writingIsEnabled(asp, state) == true)
				{
					setValueEnabled = true;
					break;
				}
			}
			if (setValueEnabled == true)
			{
				break;
			}
		}
	}

	if (m_setValueButton->isEnabled() != setValueEnabled)
	{
		m_setValueButton->setEnabled(setValueEnabled);
	}

	if (m_setAllButton->isEnabled() != setAllEnabled)
	{
		m_setAllButton->setEnabled(setAllEnabled);
	}

	if (m_writeButton->isEnabled() != writeEnabled)
	{
		m_writeButton->setEnabled(writeEnabled);
	}

	if (m_undoButton->isEnabled() != writeEnabled)
	{
		m_undoButton->setEnabled(writeEnabled);
	}

	// Enable or disable "Apply" button
	//
	if (applyButtonExists == true && m_applyButton != nullptr)
	{
		for (Hash sourceHash : sourceHashes)
		{
			std::vector<ClientLib::TuningSource> tss = m_tuningConnection.tuningSourceInfo(sourceHash);
			for (const ClientLib::TuningSource& ts : tss)
			{
				for (int i = 0; i < ts.statesCount(); i++)
				{
					if (ts.state(i).hasunappliedparams() == true)
					{
						applyButtonEnabled = true;
						break;
					}
				}
				if (applyButtonEnabled == true)
				{
					break;
				}
			}
			if (applyButtonEnabled == true)
			{
				break;
			}
		}

		if (m_applyButton->isEnabled() != applyButtonEnabled)
		{
			m_applyButton->setEnabled(applyButtonEnabled);
		}
	}
}

void TuningPage::updateVisibleItems()
{
	m_model->blink();

	// Update only visible dynamic items
	//
	int from = m_objectList->rowAt(0);
	int to = m_objectList->rowAt(m_objectList->height() - m_objectList->horizontalHeader()->height());

	if (from == -1)
	{
		from = 0;
	}

	if (to == -1)
	{
		to = m_model->rowCount() - 1;
	}

	// Redraw visible table items
	//
	for (int row = from; row <= to; row++)
	{
		for (int col = 0; col < m_model->columnCount(); col++)
		{
			int columnType = static_cast<int>(m_model->columnType(col));

			if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst))
			{
				m_objectList->update(m_model->index(row, col));
			}
		}
	}
}

void TuningPage::onTimer()
{
	if  (isVisible() == true && m_model->rowCount() > 0)
	{
		setActionButtonsState();
		updateVisibleItems();
	}
}

void TuningPage::slot_setAll()
{
	QMenu menu(this);


	// Check all signals to have correct limits
	{
		std::vector<Hash> hashes = m_model->allHashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
			{
				continue;
			}

			if (asp.isAnalog() == true)
			{
				if (state.limitsUnbalance(asp) == true)
				{
					QMessageBox::warning(this, tr("Set All"), tr("There is limits mismatch in signal '%1'. Operation is disabled.").arg(asp.customSignalId()));
					return;
				}
			}
		}
	}

	// Set All To On
	QAction* actionAllToOn = new QAction(tr("Set All Discretes To On"), &menu);

	auto fAllToOn = [this]() -> void
	{
		std::vector<Hash> hashes = m_model->allHashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
			{
				continue;
			}

			if (asp.isDiscrete() == true)
			{
				TuningValue tv;
				tv.setType(TuningValueType::Discrete);
				tv.setDiscreteValue(1);
				m_tuningSignalManager.setUnappliedValue(hash, tv);
			}
		}
	};
	connect(actionAllToOn, &QAction::triggered, this, fAllToOn);

	// Set All To Onff
	QAction* actionAllToOff = new QAction(tr("Set All Discretes To Off"), &menu);

	auto fAllToOff = [this]() -> void
	{
		std::vector<Hash> hashes = m_model->allHashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_helper.writingIsEnabled(asp, state) == false)
			{
				continue;
			}

			if (asp.isDiscrete() == true)
			{
				TuningValue tv;
				tv.setType(TuningValueType::Discrete);
				tv.setDiscreteValue(0);
				m_tuningSignalManager.setUnappliedValue(hash, tv);
			}
		}
	};

	connect(actionAllToOff, &QAction::triggered, this, fAllToOff);

	// Set All To Defaults
	QAction* actionAllToDefault = new QAction(tr("Set All To Defaults"), &menu);
	connect(actionAllToDefault, &QAction::triggered, this, [this]() {
				setToDefaults(m_model->allHashes());
		});

	// Run the menu

	menu.addAction(actionAllToOn);
	menu.addAction(actionAllToOff);
	menu.addSeparator();
	menu.addAction(actionAllToDefault);

	menu.exec(QCursor::pos());
}


void TuningPage::slot_undo()
{
	std::vector<Hash> hashes = m_model->allHashes();

	bool ok = false;

	for (Hash hash : hashes)
	{
		TuningSignalState state = m_tuningSignalManager.queuedState(hash, &ok);
		m_tuningSignalManager.setUnappliedValue(hash, state.value());
	}
}

void TuningPage::slot_Write()
{
	write();
}

void TuningPage::slot_Apply()
{
	apply();
}

