#include "SignalsTabPage.h"
#include "Settings.h"
#include "SignalPropertiesDialog.h"
#include "BusStorage.h"
#include "../lib/DbController.h"
#include "../lib/SignalProperties.h"
#include "../lib/WidgetUtils.h"
#include "../lib/WUtils.h"
#include "../lib/ConstStrings.h"
#include "../lib/StandardColors.h"
#include "../lib/SignalSetProvider.h"
#include "./Forms/ComparePropertyObjectDialog.h"
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QLabel>
#include <QCompleter>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QClipboard>
#include <QSplitter>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QAbstractItemModelTester>


const int DEFAULT_COLUMN_WIDTH = 50;


SignalsDelegate::SignalsDelegate(SignalSetProvider* signalSetProvider, SignalsModel* model, SignalsProxyModel* proxyModel, QObject *parent) :
	QStyledItemDelegate(parent),
	m_signalSetProvider(signalSetProvider),
	m_model(model),
	m_proxyModel(proxyModel)
{
	connect(this, &QAbstractItemDelegate::closeEditor, this, &SignalsDelegate::onCloseEditorEvent);
}

QWidget *SignalsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();

	m_signalSetProvider->loadSignal(m_signalSetProvider->key(row));	// get current checkedOut state

	const Signal& s = m_signalSetProvider->getLoadedSignal(row);

	SignalPropertyManager& manager = m_signalSetProvider->signalPropertyManager();
	manager.reloadPropertyBehaviour();

	bool isExpert = theSettings.isExpertMode();

	E::PropertyBehaviourType behaviour = manager.getBehaviour(s, col);
	if (manager.isHidden(behaviour, isExpert) || manager.isReadOnly(behaviour, isExpert))
	{
		return nullptr;
	}

	if (!s.checkedOut())
	{
		signalIdForUndoOnCancelEditing = s.ID();
	}
	else
	{
		signalIdForUndoOnCancelEditing = -1;
	}

	if (!m_signalSetProvider->checkoutSignal(row))
	{
		return nullptr;
	}

	m_signalSetProvider->loadSignal(m_signalSetProvider->key(row));	// update new checkedOut state on view

	auto values = manager.values(col);

	if (values.size() > 0)
	{
		QComboBox* cb = new QComboBox(parent);
		for (const auto& value : values)
		{
			cb->addItem(value.second, value.first);
		}
		return cb;
	}

	switch (manager.type(col))
	{
	case QVariant::String:
	{
		QLineEdit* le = new QLineEdit(parent);

		if (manager.name(col).right(2) == "ID")
		{
			QRegExp rx4ID(SignalProperties::cacheValidator);
			le->setValidator(new QRegExpValidator(rx4ID, le));
		}
		else
		{
			QRegExp rx4Name("^.+$");
			le->setValidator(new QRegExpValidator(rx4Name, le));
		}

		return le;
	}
	case QVariant::Double:
	{
		QLineEdit* le = new QLineEdit(parent);
		le->setValidator(new QDoubleValidator(le));
		return le;
	}
	case QVariant::Int:
	case QVariant::UInt:
	{
		QLineEdit* le = new QLineEdit(parent);
		le->setValidator(new QIntValidator(le));
		return le;
	}
	case QVariant::Bool:
	{
		QComboBox* cb = new QComboBox(parent);
		cb->addItems(QStringList() << tr("False") << tr("True"));
		return cb;
	}
	default:
		if (manager.type(col) == qMetaTypeId<TuningValue>())
		{
			QLineEdit* le = new QLineEdit(parent);
			if (s.isAnalog())
			{
				le->setValidator(new QDoubleValidator(le));
			}
			else
			{
				le->setValidator(new QIntValidator(le));
			}
			return le;
		}
		else
		{
			assert(false);
			return QStyledItemDelegate::createEditor(parent, option, index);
		}
	}
}

void SignalsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
	editor->setGeometry(option.rect);
	QComboBox* cb = dynamic_cast<QComboBox*>(editor);
	if (cb != nullptr)
	{
		cb->showPopup();
	}
}

void SignalsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();
	if (row >= m_signalSetProvider->signalCount())
	{
		return;
	}

	QComboBox* cb = dynamic_cast<QComboBox*>(editor);

	const Signal& s = m_signalSetProvider->getLoadedSignal(row);

	SignalPropertyManager& manager = m_signalSetProvider->signalPropertyManager();
	bool isExpert = theSettings.isExpertMode();

	const auto values = manager.values(col);

	if (values.size() > 0)
	{
		if (cb == nullptr)
		{
			assert(false);
			return;
		}

		cb->setCurrentIndex(cb->findData(manager.value(&s, col, isExpert)));
		return;
	}

	QVariant::Type type = manager.type(col);

	if (type == QVariant::Bool)
	{
		if (cb == nullptr)
		{
			assert(false);
			return;
		}

		cb->setCurrentIndex(manager.value(&s, col, isExpert).toBool());
		return;
	}

	QLineEdit* le = dynamic_cast<QLineEdit*>(editor);
	if (le == nullptr)
	{
		assert(false);
		return;
	}

	switch (type)
	{
	case QVariant::String:
	case QVariant::Double:
	case QVariant::Int:
	case QVariant::UInt:
		le->setText(manager.value(&s, col, isExpert).toString());
		break;
	default:
		if (type == qMetaTypeId<TuningValue>())
		{
			le->setText(manager.value(&s, col, isExpert).toString());
		}
		else
		{
			assert(false);
			return;
		}
	}
}

void SignalsDelegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const
{
	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();
	if (row >= m_signalSetProvider->signalCount())
	{
		return;
	}

	QComboBox* cb = dynamic_cast<QComboBox*>(editor);

	Signal s = m_signalSetProvider->getLoadedSignal(row);

	SignalPropertyManager& manager = m_signalSetProvider->signalPropertyManager();
	bool isExpert = theSettings.isExpertMode();

	const auto values = manager.values(col);

	if (values.size() > 0)
	{
		if (cb == nullptr)
		{
			assert(false);
			return;
		}

		QVariant data = cb->currentData();

		if (data.isValid())
		{
			manager.setValue(&s, col, data, isExpert);
			m_signalSetProvider->saveSignal(s);

			signalIdForUndoOnCancelEditing = -1;
		}

		return;
	}

	QVariant::Type type = manager.type(col);

	if (type == QVariant::Bool)
	{
		if (cb == nullptr)
		{
			assert(false);
			return;
		}

		manager.setValue(&s, col, cb->currentIndex(), isExpert);
		m_signalSetProvider->saveSignal(s);
		signalIdForUndoOnCancelEditing = -1;
		return;
	}

	QLineEdit* le = dynamic_cast<QLineEdit*>(editor);
	if (le == nullptr)
	{
		assert(false);
		return;
	}

	QString value = le->text();

	switch (type)
	{
	case QVariant::String:
	{
		QString name = manager.name(col);

		if (name == SignalProperties::appSignalIDCaption &&
				(value.isEmpty() || value[0] != '#'))
		{
			value = ('#' + value).trimmed();
		}
		if ((name == SignalProperties::customSignalIDCaption ||
			 name == SignalProperties::busTypeIDCaption ||
			 name == SignalProperties::equipmentIDCaption) &&
				(value.isEmpty() == false && value[0] == '#'))
		{
			value = value.mid(1).trimmed();
		}
		if (name == SignalProperties::captionCaption)
		{
			value = value.trimmed();
		}
		manager.setValue(&s, col, value, isExpert);
		break;
	}
	case QVariant::Double:
		manager.setValue(&s, col, value.toDouble(), isExpert);
		break;
	case QVariant::Int:
		manager.setValue(&s, col, value.toInt(), isExpert);
		break;
	case QVariant::UInt:
		manager.setValue(&s, col, value.toUInt(), isExpert);
		break;
	default:
		if (type == qMetaTypeId<TuningValue>())
		{
			manager.setValue(&s, col, value, isExpert);
		}
		else
		{
			assert(false);
			return;
		}
	}

	m_signalSetProvider->saveSignal(s);
	signalIdForUndoOnCancelEditing = -1;
}

void SignalsDelegate::onCloseEditorEvent(QWidget*, QAbstractItemDelegate::EndEditHint hint)
{
	if (hint == QAbstractItemDelegate::RevertModelCache && signalIdForUndoOnCancelEditing != -1)
	{
		m_signalSetProvider->undoSignal(signalIdForUndoOnCancelEditing);
		signalIdForUndoOnCancelEditing = -1;
	}
}

bool SignalsDelegate::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex &)
{
	if (event->type() == QEvent::MouseButtonDblClick)
	{
		emit itemDoubleClicked();
		return true;
	}
	return false;
}


SignalsModel::SignalsModel(SignalSetProvider* signalSetProvider, SignalsTabPage* parent) :
	QAbstractTableModel(parent),
	m_signalSetProvider(signalSetProvider),
	m_parentWindow(parent)
{
	connect(m_signalSetProvider, &SignalSetProvider::signalCountChanged, this, &SignalsModel::changeRowCount);
	connect(m_signalSetProvider, &SignalSetProvider::signalUpdated, this, &SignalsModel::updateSignal);
	connect(&m_signalSetProvider->signalPropertyManager(), &SignalPropertyManager::propertyCountIncreased, this, &SignalsModel::changeColumnCount);
}

SignalsModel::~SignalsModel()
{

}


int SignalsModel::rowCount(const QModelIndex& parentIndex) const
{
	if (parentIndex.isValid())
	{
		return 0;
	}
	return m_rowCount;
}

int SignalsModel::columnCount(const QModelIndex& parentIndex) const
{
	if (parentIndex.isValid())
	{
		return 0;
	}
	return m_columnCount + 1;	// Usual properties and "Last change user"
}

QString SignalsModel::getUserStr(int userId) const
{
	QString user = m_signalSetProvider->getUserStr(userId);
	return user == "" ? tr("Unknown user ID = %1").arg(userId) : user;
}


QVariant SignalsModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();

	const Signal& signal = m_signalSetProvider->getLoadedSignal(row);

	if (row == m_signalSetProvider->signalCount() || signal.isLoaded() == false)
	{
		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (signal.checkedOut())
		{
			QBrush b(StandardColors::VcsCheckedIn);

			switch (signal.instanceAction().value())
			{
			case VcsItemAction::Added:
				b.setColor(StandardColors::VcsAdded);
				break;
			case VcsItemAction::Modified:
				b.setColor(StandardColors::VcsModified);
				break;
			case VcsItemAction::Deleted:
				b.setColor(StandardColors::VcsDeleted);
				break;
			default:
				assert(false);
			}

			return {b};
		}
	}

	SignalPropertyManager& manager = m_signalSetProvider->signalPropertyManager();

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (col >= manager.count())
		{
			return signal.checkedOut() ? getUserStr(signal.userID()) : "";
		}

		QVariant value = manager.value(&signal, col, theSettings.isExpertMode());

		if (value.isValid() && signal.isAnalog() && manager.dependsOnPrecision(col))
		{
			switch (static_cast<QMetaType::Type>(value.type()))
			{
			case QMetaType::Double:
			case QMetaType::Float:
				return QString::number(value.toDouble(), 'f', signal.decimalPlaces());
			case QMetaType::Short:
			case QMetaType::UShort:
			case QMetaType::Int:
			case QMetaType::UInt:
			case QMetaType::Long:
			case QMetaType::ULong:
			case QMetaType::LongLong:
			case QMetaType::ULongLong:
				return value.toString();
			default:
				assert(false);
				return QVariant();
			}

		}
		return value;
	}

	return QVariant();
}

QVariant SignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	SignalPropertyManager& propertyManager = m_signalSetProvider->signalPropertyManager();
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (orientation == Qt::Horizontal)
		{
			if (section == propertyManager.count())
			{
				return "Last change user";
			}
			return propertyManager.caption(section);
		}
		if (orientation == Qt::Vertical)
		{
			if (section < m_signalSetProvider->signalCount())
			{
				return m_signalSetProvider->key(section);
			}
		}
	}
	return QVariant();
}

bool SignalsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	SignalPropertyManager& propertyManager = m_signalSetProvider->signalPropertyManager();
	if (role == Qt::EditRole)
	{
		int row = index.row();

		assert(row < m_signalSetProvider->signalCount());

		Signal s = m_signalSetProvider->getLoadedSignal(row);

		propertyManager.setValue(&s, index.column(), value, theSettings.isExpertMode());

		// This should be done by SignalsDelegate::setModelData
		m_signalSetProvider->saveSignal(s);

		m_signalSetProvider->loadSignal(s.ID());
	}
	else
	{
		return QAbstractTableModel::setData(index, value, role);
	}

	return true;
}

Qt::ItemFlags SignalsModel::flags(const QModelIndex &index) const
{
	SignalPropertyManager& propertyManager = m_signalSetProvider->signalPropertyManager();
	if (index.isValid() == false)
	{
		return QAbstractTableModel::flags(index);
	}
	int row = index.row();
	int column = index.column();

	if (column >= propertyManager.count())
	{
		return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
	}

	assert(row < m_signalSetProvider->signalCount());

	const Signal& s = m_signalSetProvider->getLoadedSignal(row);

	if (propertyManager.getBehaviour(s, index.column()) == E::PropertyBehaviourType::Write)
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	else
	{
		return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
	}
}

void SignalsModel::updateSignal(int signalIndex)
{
	assert(signalIndex < m_rowCount);
	emit dataChanged(index(signalIndex, 0), index(signalIndex, m_columnCount));
}

void SignalsModel::changeRowCount()
{
	if (m_rowCount != m_signalSetProvider->signalCount())
	{
		beginResetModel();
		m_rowCount = m_signalSetProvider->signalCount();
		m_columnCount = m_signalSetProvider->signalPropertyManager().count();
		endResetModel();
	}
}

void SignalsModel::changeColumnCount()
{
	int signalPropertyCount = m_signalSetProvider->signalPropertyManager().count();
	if (m_columnCount < signalPropertyCount)
	{
		beginInsertColumns(QModelIndex(), m_columnCount, signalPropertyCount - 1);
		m_columnCount = signalPropertyCount;
		endInsertColumns();
	}
	else
	{
		assert(false);
	}
}



//
//
// SignalsTabPage
//
//

SignalsTabPage* SignalsTabPage::m_instance = nullptr;


SignalsTabPage::SignalsTabPage(SignalSetProvider* signalSetProvider, DbController* dbController, QWidget* parent) :
	MainTabPage(dbController, parent),
	m_signalSetProvider(signalSetProvider)
{
	assert(signalSetProvider != nullptr);
	assert(m_instance == nullptr);

	m_instance = this;

	m_signalTypeFilterCombo = new QComboBox(this);
	m_signalTypeFilterCombo->addItem(tr("All signals"), ST_ANY);
	m_signalTypeFilterCombo->addItem(tr("Analog signals"), ST_ANALOG);
	m_signalTypeFilterCombo->addItem(tr("Discrete signals"), ST_DISCRETE);
	m_signalTypeFilterCombo->addItem(tr("Bus signals"), ST_BUS);

	m_signalIdFieldCombo = new QComboBox(this);
	m_signalIdFieldCombo->addItem(tr("Any"), FI_ANY);
	m_signalIdFieldCombo->addItem(tr("AppSignalID"), FI_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("CustomAppSignalID"), FI_CUSTOM_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("EquipmentID"), FI_EQUIPMENT_ID);
	m_signalIdFieldCombo->addItem(tr("Caption"), FI_CAPTION);

	QToolBar* toolBar = new QToolBar(this);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::showDeviceApplicationSignals, this, &SignalsTabPage::changeSignalIdFilter);

	QToolBar* filterToolBar = new QToolBar(this);

	m_filterEdit = new QLineEdit(this);
	filterToolBar->addWidget(new QLabel("Filter ", this));
	filterToolBar->addWidget(m_signalTypeFilterCombo);
	filterToolBar->addWidget(new QLabel(" by ", this));
	filterToolBar->addWidget(m_signalIdFieldCombo);
	filterToolBar->addWidget(new QLabel(" complies ", this));
	filterToolBar->addWidget(m_filterEdit);

	QSettings settings;
	m_filterHistory = settings.value("SignalsTabPage/filterHistory").toStringList();

	m_completer = new QCompleter(m_filterHistory, this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_filterEdit->setCompleter(m_completer);
	connect(m_filterEdit, &QLineEdit::textEdited, [=](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_filterEdit, &QLineEdit::setText);

	QPushButton* applyButton = new QPushButton("Apply", this);
	connect(applyButton, &QPushButton::clicked, this, &SignalsTabPage::applySignalIdFilter);
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &SignalsTabPage::applySignalIdFilter);
	filterToolBar->addWidget(applyButton);

	QPushButton* resetButton = new QPushButton("Reset", this);
	connect(resetButton, &QPushButton::clicked, this, &SignalsTabPage::resetSignalIdFilter);
	filterToolBar->addWidget(resetButton);

	// Property View
	//
	m_signalsModel = new SignalsModel(signalSetProvider, this);

	//For testing purposes
	//
	// new QAbstractItemModelTester(m_signalsModel, QAbstractItemModelTester::FailureReportingMode::Fatal, this);
	//

	m_signalsProxyModel = new SignalsProxyModel(m_signalsModel, this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsProxyModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_signalsView->verticalHeader()->setFixedWidth(DEFAULT_COLUMN_WIDTH);
	m_signalsView->verticalHeader()->hide();
	SignalsDelegate* delegate = m_signalsModel->createDelegate(m_signalsProxyModel);
	m_signalsView->setItemDelegate(delegate);

	QHeaderView* horizontalHeader = m_signalsView->horizontalHeader();
	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	horizontalHeader->setHighlightSections(false);

	horizontalHeader->setSortIndicator(-1, Qt::AscendingOrder);
	m_signalsView->setSortingEnabled(true);

	horizontalHeader->setDefaultSectionSize(150);

	auto& propertyManager = signalSetProvider->signalPropertyManager();
	int wideColumnWidth = 400;

	m_signalsView->setColumnWidth(propertyManager.index(SignalProperties::appSignalIDCaption), wideColumnWidth);
	m_signalsView->setColumnWidth(propertyManager.index(SignalProperties::customSignalIDCaption), wideColumnWidth);
	m_signalsView->setColumnWidth(propertyManager.index(SignalProperties::busTypeIDCaption), wideColumnWidth);
	m_signalsView->setColumnWidth(propertyManager.index(SignalProperties::captionCaption), wideColumnWidth);
	m_signalsView->setColumnWidth(propertyManager.index(SignalProperties::equipmentIDCaption), wideColumnWidth);

	QVector<int> defaultColumnVisibility;

	const QVector<QString> defaultSignalPropertyVisibility =
	{
		SignalProperties::appSignalIDCaption,
		SignalProperties::customSignalIDCaption,
		SignalProperties::captionCaption,
		SignalProperties::typeCaption,
		SignalProperties::inOutTypeCaption,
		SignalProperties::equipmentIDCaption,
		SignalProperties::lowEngineeringUnitsCaption,
		SignalProperties::highEngineeringUnitsCaption,
	};

	for (const QString& columnName : defaultSignalPropertyVisibility)
	{
		defaultColumnVisibility.push_back(propertyManager.index(columnName));
	}

	m_signalsColumnVisibilityController = new TableDataVisibilityController(m_signalsView, "SignalsTabPage", defaultColumnVisibility);
	connect(&signalSetProvider->signalPropertyManager(), &SignalPropertyManager::propertyCountIncreased, m_signalsColumnVisibilityController, &TableDataVisibilityController::checkNewColumns);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));
	m_signalsView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	connect(delegate, &SignalsDelegate::itemDoubleClicked, this, &SignalsTabPage::editSignal);
	connect(m_signalTypeFilterCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SignalsTabPage::changeSignalTypeFilter);

	connect(m_signalsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &SignalsTabPage::changeLazySignalLoadingSequence);

	connect(signalSetProvider, &SignalSetProvider::error, this, &SignalsTabPage::showError);

	// Create Actions
	//
	CreateActions(toolBar);

	toolBar->setStyleSheet("QToolButton { padding: 6px; }");
	toolBar->setIconSize(toolBar->iconSize() * 0.9);

	//
	// Layouts
	//

	QVBoxLayout* pMainLayout = new QVBoxLayout();

	pMainLayout->addWidget(toolBar);
	pMainLayout->addWidget(filterToolBar);
	pMainLayout->addWidget(m_signalsView);

	setLayout(pMainLayout);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SignalsTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SignalsTabPage::projectClosed);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &SignalsTabPage::compareObject);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SignalsTabPage::~SignalsTabPage()
{
	if (m_findSignalDialog != nullptr)
	{
		m_findSignalDialog->close();
		delete m_findSignalDialog;
	}
}

bool SignalsTabPage::updateSignalsSpecProps(DbController* dbc, const QVector<Hardware::DeviceSignal*>& deviceSignalsToUpdate, const QStringList& forceUpdateProperties)
{
	Q_UNUSED(forceUpdateProperties)

	TEST_PTR_RETURN_FALSE(dbc)

	QStringList equipmentIDs;

	for(const Hardware::DeviceSignal* deviceSignal: deviceSignalsToUpdate)
	{
		TEST_PTR_CONTINUE(deviceSignal)
		equipmentIDs.append(deviceSignal->equipmentId());
	}

	QMultiHash<QString, int> signalIDsMap;

	bool result = dbc->getMultipleSignalsIDsWithEquipmentID(equipmentIDs, &signalIDsMap, nullptr);

	if (result == false)
	{
		return false;
	}

	QVector<int> checkoutSignalIDs;
	QVector<Signal> newSignalWorkcopies;

	for(const Hardware::DeviceSignal* deviceSignal: deviceSignalsToUpdate)
	{
		TEST_PTR_CONTINUE(deviceSignal)

		QString deviceSignalSpecPropStruct = deviceSignal->signalSpecPropsStruct();

		if (	deviceSignalSpecPropStruct.contains(SignalProperties::MISPRINT_lowEngineeringUnitsCaption) ||
				deviceSignalSpecPropStruct.contains(SignalProperties::MISPRINT_highEngineeringUnitsCaption))
		{
			QMessageBox::critical(m_instance,
						  QApplication::applicationName(),
						  QString(tr("Misprinted signal specific properties HighEngEneeringUnits/LowEngEneeringUnits has detected in device signal %1. \n\n"
									 "Update module preset first. \n\nUpdating from preset is aborted!")).
											arg(deviceSignal->equipmentId()));
			return false;
		}

		QList<int> signalIDs = signalIDsMap.values(deviceSignal->equipmentId());

		if (signalIDs.count() == 0)
		{
			continue;
		}

		for(int signalID : signalIDs)
		{
			bool signalChanged = false;

			Signal s;

			result = dbc->getLatestSignal(signalID, &s, nullptr);

			if (result == false)
			{
				QMessageBox::critical(m_instance,
							  QApplication::applicationName(),
							  QString(tr("Cannot getLatestSignal with id = %1, update from preset is aborted.")).arg(signalID));
				return false;
			}

			if (s.specPropStruct() != deviceSignalSpecPropStruct)
			{
				signalChanged = true;
			}

			SignalSpecPropValues specPropValues;

			result = specPropValues.parseValuesFromArray(s.protoSpecPropValues());

			if (result == false)
			{
				QMessageBox::critical(m_instance,
							  QApplication::applicationName(),
							  QString(tr("Signal %1 specific properties values parsing error, \nupdate from preset is aborted.")).arg(s.appSignalID()));
				return false;
			}

			result = specPropValues.updateFromSpecPropStruct(deviceSignalSpecPropStruct);

			if (result == false)
			{
				QMessageBox::critical(m_instance,
							  QApplication::applicationName(),
							  QString(tr("Signal %1 specific properties values updating error, \nupdate from preset is aborted.")).arg(s.appSignalID()));
				return false;
			}

			QByteArray newValues;

			result = specPropValues.serializeValuesToArray(&newValues);

			if (newValues != s.protoSpecPropValues())		// compare proto-data arrays
			{
				signalChanged = true;
			}

			if (signalChanged == false)
			{
				continue;
			}

			// signal should be updated
			//
			s.setSpecPropStruct(deviceSignalSpecPropStruct);
			s.setProtoSpecPropValues(newValues);

			checkoutSignalIDs.append(signalID);
			newSignalWorkcopies.append(s);
		}
	}

	if (checkoutSignalIDs.count() == 0)
	{
		return true;
	}

	QVector<ObjectState> objStates;

	result = dbc->checkoutSignals(&checkoutSignalIDs, &objStates, nullptr);

	if (result == false)
	{
		QMessageBox::critical(m_instance,
							  QApplication::applicationName(),
							  tr("App signals check out error, update is not possible!"));
		return false;
	}

	if (objStates.size() != checkoutSignalIDs.size())
	{
		QMessageBox::critical(m_instance,
							  QApplication::applicationName(),
							  tr("Not all necessery app signals was checked out, update is not possible!"));
		return false;
	}

	bool allSignalsCheckedOut = true;

	for(const ObjectState& objState : objStates)
	{
		if (objState.checkedOut == false || objState.errCode != ERR_SIGNAL_OK)
		{
			allSignalsCheckedOut = false;
			break;
		}
	}

	if (allSignalsCheckedOut == false)
	{
		QMessageBox::critical(m_instance,
					  QApplication::applicationName(),
					  tr("Cannot check out one or more app signals, update from preset is not posible."));
		return false;
	}

	result = dbc->setSignalsWorkcopies(&newSignalWorkcopies, nullptr);

	if (result == false)
	{
		QMessageBox::critical(m_instance,
					  QApplication::applicationName(),
					  QString(tr("Error setting signals new workcopies, update from preset is aborted.")));
		return false;
	}


/*
	for(Signal& s : newSignalWorkcopies)
	{
		ObjectState objState;

		result = dbc->setSignalWorkcopy(&s, &objState, nullptr);

		if (result == false)
		{
			QMessageBox::critical(m_instance,
						  QApplication::applicationName(),
						  QString(tr("Cannot set workcopy of signal %1, update from preset is aborted.")).arg(s.appSignalID()));
			return false;
		}
	}*/

	return result;
}

int SignalsTabPage::getMiddleVisibleRow()
{
	QRect rect = m_signalsView->viewport()->rect();
	return m_signalsView->indexAt(rect.center()).row();
}

void SignalsTabPage::CreateActions(QToolBar *toolBar)
{
	QAction* action = nullptr;

	action = new QAction(QIcon(":/Images/Images/SchemaOpen.svg"), tr("Edit properties"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::editSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaAddFile.svg"), tr("New signal"), this);
	action->setShortcut(QKeySequence::StandardKey::New);
	connect(action, &QAction::triggered, this, &SignalsTabPage::addSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaClone.svg"), tr("Clone signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::cloneSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaDelete.svg"), tr("Delete signal"), this);
	action->setShortcut(Qt::Key_Delete);
	connect(action, &QAction::triggered, this, &SignalsTabPage::deleteSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	m_signalsView->addAction(toolBar->addSeparator());

	action = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("Check in signal(s)"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::checkIn);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo changes"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::undoSignalChanges);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaHistory.svg"), tr("History"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::viewSignalHistory);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	m_signalsView->addAction(toolBar->addSeparator());

	action = new QAction(QIcon(":/Images/Images/SchemaRefresh.svg"), tr("Refresh"), this);
	action->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(action, &QAction::triggered, this, &SignalsTabPage::loadSignals);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/Find.svg"), tr("Find"), this);
	action->setShortcuts(QList<QKeySequence>() << QKeySequence::Find << QKeySequence::Replace);
	connect(action, &QAction::triggered, this, &SignalsTabPage::findAndReplaceSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);
}

void SignalsTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void SignalsTabPage::keyPressEvent(QKeyEvent* e)
{
	if(e->type() == QKeyEvent::KeyPress && e->matches(QKeySequence::Copy))
	{
		QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
		if (selection.count() == 0)
		{
			QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
		}
		QString selectedSignalIds;
		for (int i = 0; i < selection.count(); i++)
		{
			int row = m_signalsProxyModel->mapToSource(selection[i]).row();
			selectedSignalIds.append(m_signalSetProvider->getLoadedSignal(row).appSignalID() + "\n");
		}

		QApplication::clipboard()->setText(selectedSignalIds);
	}
}

void SignalsTabPage::projectOpened()
{
	this->setEnabled(true);

	m_signalSetProvider->initLazyLoadSignals();

	changeLazySignalLoadingSequence();
}

void SignalsTabPage::projectClosed()
{
	m_signalSetProvider->stopLoadingSignals();

	m_signalsColumnVisibilityController->saveAllHeaderGeomery();

	this->setEnabled(false);

	m_signalsModel->prepareForReset();
	m_signalSetProvider->clearSignals();
	m_signalsModel->finishReset();

	resetSignalIdFilter();

	if (m_findSignalDialog != nullptr)
	{
		m_findSignalDialog->close();
		delete m_findSignalDialog;
		m_findSignalDialog = nullptr;
	}
}

void SignalsTabPage::onTabPageChanged()
{
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(sender());
	if (tabWidget == nullptr)
	{
		assert(false);
		return;
	}

	if (isEnabled() == false)
	{
		return;
	}

	if (m_findSignalDialog != nullptr)
	{
		if (tabWidget->currentWidget() == this && m_findSignalDialog->shouldReopen() == true)
		{
			findAndReplaceSignal();
		}
		else
		{
			m_findSignalDialog->hide();
		}
	}
}

void SignalsTabPage::loadSignals()
{
	saveSelection();
	m_signalSetProvider->loadSignals();
	restoreSelection();
}

void SignalsTabPage::addSignal()
{
	QDialog signalTypeDialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	QFormLayout* fl = new QFormLayout(&signalTypeDialog);

	QLineEdit* deviceIdEdit = new QLineEdit(&signalTypeDialog);
	deviceIdEdit->setText("");

	fl->addRow(tr("EquipmentID"), deviceIdEdit);

	QComboBox* signalTypeCombo = new QComboBox(&signalTypeDialog);
	signalTypeCombo->addItems(QStringList() << tr("Analog") << tr("Discrete") << tr("Bus"));
	signalTypeCombo->setCurrentIndex(1);

	fl->addRow(tr("Signal type"), signalTypeCombo);

	QLineEdit* signalChannelCountEdit = new QLineEdit(&signalTypeDialog);
	signalChannelCountEdit->setText("1");
	QRegExp channelRegExp("[1-6]");
	QValidator *validator = new QRegExpValidator(channelRegExp, &signalTypeDialog);
	signalChannelCountEdit->setValidator(validator);

	fl->addRow(tr("Signal channel count"), signalChannelCountEdit);

	QLineEdit* signalCountEdit = new QLineEdit(&signalTypeDialog);
	signalCountEdit->setText("1");
	QRegExp countRegExp("[1-9]\\d{0,3}");
	validator = new QRegExpValidator(countRegExp, &signalTypeDialog);
	signalCountEdit->setValidator(validator);

	fl->addRow(tr("Signal count"), signalCountEdit);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, &QDialogButtonBox::accepted, &signalTypeDialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &signalTypeDialog, &QDialog::reject);

	fl->addRow(buttonBox);

	signalTypeDialog.setLayout(fl);
	signalTypeDialog.setWindowTitle("Create signals");

	if (signalTypeDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	int channelCount = signalChannelCountEdit->text().toInt();
	int signalCount = signalCountEdit->text().toInt();

	Signal signal;

	signal.setSignalType(static_cast<E::SignalType>(signalTypeCombo->currentIndex()));

	if (signal.isAnalog())
	{
		// Temporary default value, should be removed later
		//
		signal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	}

	initNewSignal(signal);

	if (!deviceIdEdit->text().isEmpty())
	{
		signal.setEquipmentID(deviceIdEdit->text());
	}

	int signalCounter = dbController()->nextCounterValue();
	if (signalCounter >= 0)
	{
		QString newId = QString(E::valueToString<E::SignalType>(signal.signalType()).toUpper() + "_%1").arg(signalCounter, 3, 10, Latin1Char::ZERO);
		signal.setAppSignalID('#' + newId);
		signal.setCustomAppSignalID(newId);
		signal.setCaption(newId);
	}

	SignalPropertiesDialog dlg(dbController(), QVector<Signal*>() << &signal, false, false, this);

	SignalSetProvider::trimSignalTextFields(signal);

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<Signal> resultSignalVector;

		resultSignalVector.reserve(signalCount * channelCount);

		for (int s = 0; s < signalCount; s++)
		{
			QVector<Signal> signalVector;

			for (int i = 0; i < channelCount; i++)
			{
				signalVector << signal;
				QString suffix;

				if (signalCount > 1)
				{
					suffix = QString("_SIG%1").arg(s, 3, 10, QChar('0'));
				}

				if (channelCount > 1)
				{
					suffix += "_" + QString('A' + i);
				}

				signalVector[i].setAppSignalID((signalVector[i].appSignalID() + suffix).toUpper());
				signalVector[i].setCustomAppSignalID((signalVector[i].customAppSignalID() + suffix));
			}

			if (dbController()->addSignal(E::SignalType(signalTypeCombo->currentIndex()), &signalVector, this))
			{
				for (int i = 0; i < signalVector.count(); i++)
				{
					resultSignalVector.append(signalVector[i]);
				}
			}
		}

		if (!resultSignalVector.isEmpty())
		{
			int addedSignalId = -1;
			for (int i = 0; i < resultSignalVector.count(); i++)
			{
				m_signalSetProvider->addSignal(resultSignalVector[i]);
				addedSignalId = resultSignalVector[i].ID();
			}
			m_signalsModel->changeRowCount();
			restoreSelection(addedSignalId);
		}
	}
}

void SignalsTabPage::editSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);

	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
		return;
	}

	int currentRow = m_signalsProxyModel->mapToSource(m_signalsView->currentIndex()).row();
	int currentColumn = m_signalsView->currentIndex().column();
	int currentId = m_signalSetProvider->key(currentRow);

	QVector<int> selectedSignalId;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		selectedSignalId.append(m_signalSetProvider->key(row));
	}

	editSignals(selectedSignalId);

	m_signalsView->scrollTo(m_signalsProxyModel->mapFromSource(m_signalsModel->index(m_signalSetProvider->keyIndex(currentId), currentColumn)));
}

bool SignalsTabPage::editSignals(QVector<int> ids)
{
	m_signalSetProvider->loadSignalSet(ids);

	bool readOnly = false;
	QVector<Signal*> signalVector;

	for (int i = 0; i < ids.count(); i++)
	{
		int index = m_signalSetProvider->keyIndex(ids[i]);
		Signal* signal = new Signal(m_signalSetProvider->getLoadedSignal(index));

		if (!m_signalSetProvider->isEditableSignal(index))
		{
			readOnly = true;
		}

		signalVector.append(signal);
	}

	SignalPropertiesDialog dlg(dbController(), signalVector, readOnly, true, this);

	if (dlg.isValid() == false)
	{
		return false;
	}

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<ObjectState> states;
		for (int i = 0; i < ids.count(); i++)
		{
			if (!dlg.isEditedSignal(ids[i]))
			{
				delete signalVector[i];
				signalVector.remove(i);
			}
		}

		m_signalSetProvider->saveSignals(signalVector);
		for (int i = 0; i < signalVector.count(); i++)
		{
			delete signalVector[i];
		}

		m_signalSetProvider->loadSignalSet(ids);
		return true;
	}

	for (int i = 0; i < signalVector.count(); i++)
	{
		delete signalVector[i];
	}

	if (dlg.hasEditedSignals())
	{
		m_signalSetProvider->loadSignalSet(ids);	//Signal could be checked out but not changed
	}
	return false;
}

void SignalsTabPage::cloneSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
	}

	QSet<int> clonedSignalIDs;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		int id = m_signalSetProvider->key(row);
		clonedSignalIDs.insert(id);
	}

	m_selectedRowsSignalID = m_signalSetProvider->cloneSignals(clonedSignalIDs);
	if (!m_selectedRowsSignalID.isEmpty())
	{
		m_focusedCellSignalID = m_selectedRowsSignalID[0];
	}

	m_signalsView->clearSelection();

	restoreSelection();
}

void SignalsTabPage::deleteSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
	}
	QSet<int> deletedSignalIDs;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		int groupId = m_signalSetProvider->getLoadedSignal(row).signalGroupID();
		if (groupId != 0)
		{
			QVector<int> ids = m_signalSetProvider->getChannelSignalsID(groupId);
			deletedSignalIDs.unite(QSet<int>(ids.begin(), ids.end()));
		}
		else
		{
			deletedSignalIDs.insert(m_signalSetProvider->key(row));
		}
	}
	m_signalSetProvider->deleteSignals(deletedSignalIDs);
}

void SignalsTabPage::findAndReplaceSignal()
{
	if (m_findSignalDialog == nullptr)
	{
		const DbUser user = dbController()->currentUser();
		m_findSignalDialog = new FindSignalDialog(user.userId(), user.isAdminstrator(), m_signalsView);
		m_findSignalDialog->setModal(false);

		connect(m_findSignalDialog, &FindSignalDialog::signalSelected, this, &SignalsTabPage::restoreSelection);
	}
	m_findSignalDialog->allowReopen();
	m_findSignalDialog->show();
	m_findSignalDialog->activateWindow();
	m_findSignalDialog->raise();
	m_findSignalDialog->setFocus();
}

void SignalsTabPage::updateFindOrReplaceDialog()
{
	if (m_findSignalDialog != nullptr)
	{
		m_findSignalDialog->notifyThatSignalSetHasChanged();
	}
}

void SignalsTabPage::undoSignalChanges()
{
	m_signalSetProvider->finishLoadingSignals();

	UndoSignalsDialog dlg(m_signalsModel, m_signalsColumnVisibilityController, this);

	const QItemSelection& proxySelection = m_signalsView->selectionModel()->selection();
	const QItemSelection& sourceSelection = m_signalsProxyModel->mapSelectionToSource(proxySelection);
	dlg.setCheckStates(sourceSelection.indexes(), true);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalSetProvider->loadSignals();
}

void SignalsTabPage::checkIn()
{
	const QItemSelection& proxySelection = m_signalsView->selectionModel()->selection();
	const QItemSelection& sourceSelection = m_signalsProxyModel->mapSelectionToSource(proxySelection);

	m_signalSetProvider->finishLoadingSignals();

	CheckinSignalsDialog dlg(m_signalsModel, m_signalsColumnVisibilityController, sourceSelection.indexes(), this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalSetProvider->loadSignals();
}

void SignalsTabPage::viewSignalHistory()
{
	int row = m_signalsView->currentIndex().row();

	if (row < 0 || row >= m_signalsModel->rowCount())
	{
		return;
	}

	const Signal& signal = m_signalSetProvider->getLoadedSignal(row);
	SignalHistoryDialog dlg(dbController(), signal.appSignalID(), signal.ID(), this);

	dlg.exec();
}

void SignalsTabPage::changeLazySignalLoadingSequence()
{
	m_signalSetProvider->setMiddleVisibleSignalIndex(getMiddleVisibleRow());
}

void SignalsTabPage::setSelection(const QVector<int>& selectedRowsSignalID, int focusedCellSignalID)
{
	if (selectedRowsSignalID.isEmpty())
	{
		return;
	}
	if (focusedCellSignalID == -1)
	{
		focusedCellSignalID = selectedRowsSignalID.last();
	}
	m_selectedRowsSignalID = selectedRowsSignalID;

	int focusedRow = m_signalSetProvider->keyIndex(focusedCellSignalID);

	m_lastVerticalScrollPosition = m_signalsView->rowViewportPosition(focusedRow);
	m_lastHorizontalScrollPosition = 0;

	restoreSelection(focusedCellSignalID);
}

void SignalsTabPage::saveSelection()
{
	// Save signal id list of selected rows and signal id with column number of focused cell
	//
	m_selectedRowsSignalID.clear();
	QModelIndexList selectedList = m_signalsView->selectionModel()->selectedRows(0);
	m_selectedRowsSignalID.resize(selectedList.size());
	int currentIdIndex = 0;
	foreach (const QModelIndex& index, selectedList)
	{
		int row = m_signalsProxyModel->mapToSource(index).row();
		m_selectedRowsSignalID[currentIdIndex++] = m_signalSetProvider->key(row);
	}
	QModelIndex index = m_signalsView->currentIndex();
	if (index.isValid())
	{
		int row = m_signalsProxyModel->mapToSource(index).row();
		m_focusedCellSignalID = m_signalSetProvider->key(row);
		m_focusedCellColumn = index.column();
	}
	m_lastHorizontalScrollPosition = m_signalsView->horizontalScrollBar()->value();
	m_lastVerticalScrollPosition = m_signalsView->verticalScrollBar()->value();
}

void SignalsTabPage::restoreSelection(int focusedSignalId)
{
	if (focusedSignalId != -1)
	{
		m_focusedCellSignalID = focusedSignalId;
		m_focusedCellColumn = 0;
	}

	QModelIndex currentSourceIndex = m_signalsModel->index(m_signalSetProvider->keyIndex(m_focusedCellSignalID), m_focusedCellColumn);
	QModelIndex currentProxyIndex = m_signalsProxyModel->mapFromSource(currentSourceIndex);

	m_signalsView->selectionModel()->setCurrentIndex(currentProxyIndex, QItemSelectionModel::Select);
	m_signalsView->selectionModel()->select(currentProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	m_signalsView->horizontalScrollBar()->setValue(m_lastHorizontalScrollPosition);
	m_signalsView->verticalScrollBar()->setValue(m_lastVerticalScrollPosition);

	m_signalsView->scrollTo(currentProxyIndex);
}

void SignalsTabPage::changeSignalTypeFilter(int selectedType)
{
	saveSelection();
	int signalType = m_signalTypeFilterCombo->itemData(selectedType, Qt::UserRole).toInt();
	m_signalsProxyModel->setSignalTypeFilter(signalType);
	restoreSelection();

	for (int i = 0; i < m_signalsModel->columnCount(); i++)
	{
		if (signalType == ST_ANY ||
			m_signalSetProvider->signalPropertyManager().isHiddenFor(static_cast<E::SignalType>(signalType), i, theSettings.isExpertMode()) == false)
		{
			bool hidden = m_signalsColumnVisibilityController->getColumnVisibility(i) == false;
			m_signalsView->setColumnHidden(i, hidden);
		}
		else
		{
			m_signalsView->setColumnHidden(i, true);
		}
	}
}

void SignalsTabPage::changeSignalIdFilter(QStringList strIds, bool refreshSignalList)
{
	// Update signals
	//
	if (refreshSignalList == true)
	{
		m_signalSetProvider->loadSignals();
	}

	// Reset signal type filter
	//
	QObject* changeCustomer = sender();
	if (changeCustomer != nullptr && typeid(*changeCustomer) == typeid(GlobalMessanger))
	{
		for (int i = 0; i < m_signalTypeFilterCombo->count(); i++)
		{
			if (m_signalTypeFilterCombo->itemData(i) == ST_ANY)
			{
				m_signalTypeFilterCombo->setCurrentIndex(i);
			}
		}
		m_signalsProxyModel->setSignalTypeFilter(ST_ANY);
		m_signalIdFieldCombo->setCurrentIndex(FI_EQUIPMENT_ID);
		m_signalsProxyModel->setIdFilterField(FI_EQUIPMENT_ID);
	}

	// Set signal id filter
	//
	m_signalsProxyModel->setSignalIdFilter(strIds);

	// Set signal id filter editor text and save filter history
	//
	QString newFilter = strIds.join(" | ");
	while (newFilter.indexOf("  ") != -1)
	{
		newFilter.replace("  ", " ");
	}

	if (!newFilter.isEmpty() && !m_filterHistory.contains(newFilter))
	{
		m_filterHistory.append(newFilter);

		QStringListModel* model = dynamic_cast<QStringListModel*>(m_completer->model());
		assert(model != nullptr);
		if (model != nullptr)
		{
			model->setStringList(m_filterHistory);
		}

		QSettings settings;
		settings.setValue("SignalsTabPage/filterHistory", m_filterHistory);
	}

	m_filterEdit->setText(newFilter);

	GlobalMessanger::instance().fireChangeCurrentTab(this);
}

void SignalsTabPage::applySignalIdFilter()
{
	m_signalsProxyModel->setIdFilterField(m_signalIdFieldCombo->currentIndex());
	changeSignalIdFilter(m_filterEdit->text().trimmed().split("|", Qt::SkipEmptyParts), false);
}

void SignalsTabPage::resetSignalIdFilter()
{
	m_signalsProxyModel->setSignalIdFilter(QStringList());
	m_signalsProxyModel->setSignalTypeFilter(ST_ANY);
	m_filterEdit->setText("");
	m_signalTypeFilterCombo->setCurrentIndex(0);
}

void SignalsTabPage::showError(QString message)
{
	if (!message.isEmpty())
	{
		QMessageBox::warning(this, "Error", message);
	}
}

void SignalsTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isSignal() == false)
	{
		return;
	}

	// Get versions from the project database
	//
	std::shared_ptr<SignalProperties> source = nullptr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			std::vector<int> signalIds;
			signalIds.push_back(object.id());

			std::vector<Signal> outSignals;

			bool ok = db()->getSpecificSignals(&signalIds, compareData.sourceChangeset, &outSignals, this);
			if (ok == true && outSignals.size() == 1)
			{
				source = std::make_shared<SignalProperties>(outSignals.front());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			assert(false);
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			Signal outSignal;

			bool ok = db()->getLatestSignal(object.id(), &outSignal, this);
			if (ok == true)
			{
				source = std::make_shared<SignalProperties>(outSignal);
			}
		}
		break;
	default:
		assert(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	std::shared_ptr<SignalProperties> target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			std::vector<int> signalIds;
			signalIds.push_back(object.id());

			std::vector<Signal> outSignals;

			bool ok = db()->getSpecificSignals(&signalIds, compareData.targetChangeset, &outSignals, this);
			if (ok == true && outSignals.size() == 1)
			{
				target = std::make_shared<SignalProperties>(outSignals.front());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			assert(false);
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			Signal outSignal;

			bool ok = db()->getLatestSignal(object.id(), &outSignal, this);
			if (ok == true)
			{
				target = std::make_shared<SignalProperties>(outSignal);
			}
		}
		break;
	default:
		assert(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Compare
	//
	ComparePropertyObjectDialog::showDialog(object, compareData, source, target, this);

	return;
}


CheckedoutSignalsModel::CheckedoutSignalsModel(SignalsModel* sourceModel, QTableView* view, QObject* parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel),
	m_view(view)
{
	setSourceModel(sourceModel);
	states.resize(rowCount());
}

QVariant CheckedoutSignalsModel::data(const QModelIndex& index, int role) const
{
	if (index.column() == 0 && role == Qt::CheckStateRole)
	{
		return states[index.row()];
	}
	return QSortFilterProxyModel::data(index, role);
}

bool CheckedoutSignalsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.column() == 0 && role == Qt::CheckStateRole)
	{
		QModelIndexList list = m_view->selectionModel()->selectedRows(0);
		for (int i = 0; i < list.count(); i++)
		{
			setCheckState(list[i].row(), Qt::CheckState(value.toInt()));
		}
		return true;
	}
	return QSortFilterProxyModel::setData(index, value, role);
}

Qt::ItemFlags CheckedoutSignalsModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = QSortFilterProxyModel::flags(index);
	flags &= ~Qt::ItemIsEditable;
	if (index.column() == 0)
	{
		flags |= Qt::ItemIsUserCheckable;
	}
	return flags;
}

bool CheckedoutSignalsModel::filterAcceptsRow(int source_row, const QModelIndex&) const
{
	return SignalSetProvider::getInstance()->isCheckinableSignalForMe(source_row);
}

void CheckedoutSignalsModel::initCheckStates(const QModelIndexList& list, bool fromSourceModel)
{
	for (int i = 0; i < list.count(); i++)
	{
		QModelIndex proxyIndex = fromSourceModel ? mapFromSource(list[i]) : list[i];
		if (proxyIndex.isValid())
		{
			setCheckState(proxyIndex.row(), Qt::Checked);
		}
	}
}

void CheckedoutSignalsModel::setAllCheckStates(bool state)
{
	for (int i = 0; i < states.count(); i++)
	{
		states[i] = state ? Qt::Checked : Qt::Unchecked;
	}
	emit dataChanged(index(0, 0), index(states.count() - 1, 0), QVector<int>() << Qt::CheckStateRole);
}

void CheckedoutSignalsModel::setCheckState(int row, Qt::CheckState state)
{
	QVector<int> sourceRows = SignalSetProvider::getInstance()->getSameChannelSignals(mapToSource(index(row, 0)).row());
	foreach (const int sourceRow, sourceRows)
	{
		QModelIndex changedIndex = mapFromSource(m_sourceModel->index(sourceRow, 0));
		if (!changedIndex.isValid())
		{
			continue;
		}
		states[changedIndex.row()] = state;
		emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::CheckStateRole);
	}
}


CheckinSignalsDialog::CheckinSignalsDialog(SignalsModel *sourceModel, TableDataVisibilityController *columnManager, QModelIndexList selection, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle("Check In Signal(s)");

	m_splitter = new QSplitter(Qt::Vertical, this);

	QVBoxLayout* vl1 = new QVBoxLayout;
	QVBoxLayout* vl2 = new QVBoxLayout;
	vl2->setMargin(0);

	m_signalsView = new QTableView(this);
	m_proxyModel = new CheckedoutSignalsModel(sourceModel, m_signalsView, this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);

	if (selection.count() != 0)
	{
		m_proxyModel->initCheckStates(selection);
	}
	else
	{
		selectAll->setChecked(true);
	}

	m_commentEdit = new QPlainTextEdit(this);

	vl2->addWidget(selectAll);

	m_signalsView->setModel(m_proxyModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_signalsView->horizontalHeader()->setHighlightSections(false);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));

	int signalPropertyCount = SignalPropertyManager::getInstance()->count();

	QSettings settings;
	m_signalsView->setColumnWidth(0, columnManager->getColumnWidth(0) + 30);	// basic column width + checkbox size

	for (int i = 1; i < signalPropertyCount; i++)
	{
		bool visible = columnManager->getColumnVisibility(i);
		m_signalsView->setColumnHidden(i, !visible);

		if (visible)
		{
			m_signalsView->setColumnWidth(i, columnManager->getColumnWidth(i));
		}
	}

	vl2->addWidget(m_signalsView);

	QWidget* w = new QWidget(this);

	w->setLayout(vl2);

	m_splitter->addWidget(m_commentEdit);
	m_splitter->addWidget(w);

	vl1->addWidget(new QLabel(tr("Check In Comment:"), this));
	vl1->addWidget(m_splitter);

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addStretch();

	QPushButton* checkinSelectedButton = new QPushButton(tr("Check In"), this);
	connect(checkinSelectedButton, &QPushButton::clicked, this, &CheckinSignalsDialog::checkinSelected);
	hl->addWidget(checkinSelectedButton);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, &QPushButton::clicked, this, &CheckinSignalsDialog::cancel);
	hl->addWidget(cancelButton);

	vl1->addLayout(hl);

	setLayout(vl1);

	m_splitter->setChildrenCollapsible(false);

	setWindowPosition(this, "CheckinSignalsDialog");

	QList<int> list = m_splitter->sizes();
	list[0] = height();
	list[1] = m_commentEdit->height();
	m_splitter->setSizes(list);

	m_splitter->restoreState(settings.value("CheckinSignalsDialog/splitterPosition", m_splitter->saveState()).toByteArray());
}

void CheckinSignalsDialog::checkinSelected()
{
	saveDialogGeometry();

	QString commentText = m_commentEdit->toPlainText();
	if (commentText.isEmpty())
	{
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("Checkin comment is empty"));
		return;
	}
	QVector<int> IDs;
	SignalSetProvider* signalSetProvider = SignalSetProvider::getInstance();
	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, 0);
		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();
		IDs << signalSetProvider->key(sourceRow);
	}
	if (IDs.count() == 0)
	{
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	states.resize(IDs.size());
	signalSetProvider->dbController()->checkinSignals(&IDs, commentText, &states, this);
	signalSetProvider->showErrors(states);

	accept();
}


void CheckinSignalsDialog::cancel()
{
	saveDialogGeometry();

	reject();
}


void CheckinSignalsDialog::closeEvent(QCloseEvent* event)
{
	saveDialogGeometry();

	QDialog::closeEvent(event);
}

void CheckinSignalsDialog::saveDialogGeometry()
{
	saveWindowPosition(this, "CheckinSignalsDialog");

	QSettings settings;
	settings.setValue("CheckinSignalsDialog/splitterPosition", m_splitter->saveState());
}



UndoSignalsDialog::UndoSignalsDialog(SignalsModel* sourceModel, TableDataVisibilityController* columnManager, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle(tr("Undo signal changes"));

	setWindowPosition(this, "UndoSignalsDialog");

	QVBoxLayout* vl = new QVBoxLayout;

	QTableView* signalsView = new QTableView(this);
	m_proxyModel = new CheckedoutSignalsModel(sourceModel, signalsView, this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);
	vl->addWidget(selectAll);

	signalsView->setModel(m_proxyModel);
	signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(signalsView->fontMetrics().height() * 1.4));

	const auto& propertyManager = *SignalPropertyManager::getInstance();

	QSettings settings;
	signalsView->setColumnWidth(0, columnManager->getColumnWidth(0) + 30);	// basic column width + checkbox size

	for (int i = 1; i < propertyManager.count(); i++)
	{
		bool visible = columnManager->getColumnVisibility(i);
		signalsView->setColumnHidden(i, !visible);

		if (visible)
		{
			signalsView->setColumnWidth(i, columnManager->getColumnWidth(i));
		}
	}

	signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	signalsView->horizontalHeader()->setHighlightSections(false);

	vl->addWidget(signalsView);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &UndoSignalsDialog::undoSelected);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &UndoSignalsDialog::saveDialogGeometry);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	vl->addWidget(buttonBox);

	setLayout(vl);
}

void UndoSignalsDialog::setCheckStates(QModelIndexList selection, bool fromSourceModel)
{
	if (!selection.isEmpty())
	{
		m_proxyModel->initCheckStates(selection, fromSourceModel);
	}
}

void UndoSignalsDialog::saveDialogGeometry()
{
	saveWindowPosition(this, "UndoSignalsDialog");
}

void UndoSignalsDialog::undoSelected()
{
	saveDialogGeometry();

	SignalSetProvider* signalSetProvider = SignalSetProvider::getInstance();

	QVector<int> IDs;
	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, 0);
		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();
		IDs << signalSetProvider->key(sourceRow);
	}
	if (IDs.count() == 0)
	{
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	foreach (int ID, IDs)
	{
		ObjectState state;
		signalSetProvider->dbController()->undoSignalChanges(ID, &state, m_sourceModel->parentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}
	if (!states.isEmpty())
	{
		signalSetProvider->showErrors(states);
	}

	accept();
}

void UndoSignalsDialog::closeEvent(QCloseEvent* event)
{
	saveDialogGeometry();

	QDialog::closeEvent(event);
}


SignalsProxyModel::SignalsProxyModel(SignalsModel *sourceModel, QObject *parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel)
{
	m_signalSetProvider = SignalSetProvider::getInstance();
	connect(this, &SignalsProxyModel::aboutToSort, m_signalSetProvider, &SignalSetProvider::finishLoadingSignals, Qt::DirectConnection);
	connect(this, &SignalsProxyModel::aboutToFilter, m_signalSetProvider, &SignalSetProvider::finishLoadingSignals, Qt::DirectConnection);
	setSourceModel(sourceModel);
}

bool SignalsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
	const Signal& currentSignal = m_signalSetProvider->getLoadedSignal(source_row);
	if (!(m_signalType == ST_ANY || m_signalType == currentSignal.signalTypeInt()))
	{
		return false;
	}
	if (m_strIdMasks.isEmpty())
	{
		return true;
	}

	for (QString idMask : m_strIdMasks)
	{
		QRegExp rx(idMask.trimmed());
		rx.setPatternSyntax(QRegExp::Wildcard);
		bool result = false;

		switch (m_idFilterField)
		{
			case FI_ANY:
				result = rx.exactMatch(currentSignal.appSignalID().trimmed()) ||
						rx.exactMatch(currentSignal.customAppSignalID().trimmed()) ||
						rx.exactMatch(currentSignal.equipmentID().trimmed()) ||
						rx.exactMatch(currentSignal.caption().trimmed());
				break;
			case FI_APP_SIGNAL_ID:
				result = rx.exactMatch(currentSignal.appSignalID().trimmed());
				break;
			case FI_CUSTOM_APP_SIGNAL_ID:
				result = rx.exactMatch(currentSignal.customAppSignalID().trimmed());
				break;
			case FI_EQUIPMENT_ID:
				result = rx.exactMatch(currentSignal.equipmentID().trimmed());
				break;
			case FI_CAPTION:
				result = rx.exactMatch(currentSignal.caption().trimmed());
				break;
			default:
				assert(false);
				return false;
		}
		if (result == true)
		{
			return true;
		}
	}
	return false;
}

bool SignalsProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
	QVariant l = m_sourceModel->data(source_left);
	QVariant r = m_sourceModel->data(source_right);

	if (l == r)
	{
		const Signal& sl = m_signalSetProvider->getLoadedSignal(source_left.row());
		const Signal& sr = m_signalSetProvider->getLoadedSignal(source_right.row());

		return sl.appSignalID() < sr.appSignalID();
	}
	else
	{
		return QSortFilterProxyModel::lessThan(source_left, source_right);
	}
}

void SignalsProxyModel::setSignalTypeFilter(int signalType)
{
	if (m_signalType != signalType)
	{
		m_signalType = signalType;

		applyNewFilter();
	}
}

void SignalsProxyModel::setSignalIdFilter(QStringList strIds)
{
	bool equal = true;
	if (m_strIdMasks.count() != strIds.count())
	{
		equal = false;
	}
	else
	{
		for (int i = 0; i < m_strIdMasks.count(); i++)
		{
			if (m_strIdMasks[i] != strIds[i])
			{
				equal = false;
				break;
			}
		}
	}
	if (!equal)
	{
		m_strIdMasks = strIds;

		applyNewFilter();
	}
}

void SignalsProxyModel::setIdFilterField(int field)
{
	if (m_idFilterField != field)
	{
		m_idFilterField = field;

		applyNewFilter();
	}
}

void SignalsProxyModel::sort(int column, Qt::SortOrder order)
{
	emit aboutToSort();

	QSortFilterProxyModel::sort(column, order);
}

void SignalsProxyModel::applyNewFilter()
{
	emit aboutToFilter();

	invalidateFilter();
}

SignalHistoryDialog::SignalHistoryDialog(DbController* dbController, const QString& appSignalId, int signalId, QWidget* parent) :
	QDialog(parent),
	m_dbController(dbController),
	m_signalId(signalId)
{
	// Initial data
	//
	std::vector<DbChangeset> signalChanges;
	dbController->getSignalHistory(signalId, &signalChanges, this);

	QVector<std::pair<QString, std::function<QVariant (DbChangeset&)>>> changesetColumnDescription =
	{
		{"Changeset", [](DbChangeset& c) { return c.changeset(); }},
		{"User", [](DbChangeset& c) { return c.username(); }},
		{"Date", [](DbChangeset& c) { return c.date().toString("dd MMM yyyy HH:mm:ss"); }},
		{"Comment", [](DbChangeset& c) { return c.comment();}},
	};

	int changesetColumnCount = changesetColumnDescription.size();

	// Interface
	//
	setWindowTitle(tr("History - ") + appSignalId);

	setWindowPosition(this, "SignalHistoryDialog");

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

	QVBoxLayout* vl = new QVBoxLayout;

	setLayout(vl);

	m_historyModel = new QStandardItemModel(static_cast<int>(signalChanges.size()), changesetColumnCount, this);

	QTableView* historyView = new QTableView(this);
	historyView->setModel(m_historyModel);
	vl->addWidget(historyView);

	historyView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	historyView->setAlternatingRowColors(false);
	historyView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");
	historyView->setEditTriggers(QTableView::NoEditTriggers);

	historyView->verticalHeader()->setDefaultSectionSize(static_cast<int>(historyView->fontMetrics().height() * 1.4));
	historyView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	historyView->horizontalHeader()->setHighlightSections(false);
	historyView->horizontalHeader()->setDefaultSectionSize(150);
	historyView->horizontalHeader()->setStretchLastSection(true);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::clicked, this, &QDialog::accept);
	vl->addWidget(buttonBox);

	// Changeset details
	//
	QVector<int> defaultColumns;

	for (int i = 0; i < changesetColumnCount; i++)
	{
		m_historyModel->setHeaderData(i, Qt::Horizontal, changesetColumnDescription[i].first);
		defaultColumns.push_back(i);
	}

	QVector<Signal> signalInstances;
	signalInstances.reserve(static_cast<int>(signalChanges.size()));
	std::vector<int> signalIds = { signalId };
	std::vector<Signal> signalInstance;

	SignalPropertyManager* pSignalPropertyManager = SignalPropertyManager::getInstance();
	pSignalPropertyManager->reloadPropertyBehaviour();

	int row = 0;
	for (DbChangeset& changeset : signalChanges)
	{
		for (int i = 0; i < changesetColumnCount; i++)
		{
			m_historyModel->setData(m_historyModel->index(row, i), changesetColumnDescription[i].second(changeset));
		}

		dbController->getSpecificSignals(&signalIds, changeset.changeset(), &signalInstance, this);

		if (signalInstance.size() == 1)
		{
			signalInstances.push_back(signalInstance[0]);
			pSignalPropertyManager->detectNewProperties(signalInstance[0]);
			signalInstance.clear();
		}
		else
		{
			assert(false);
		}

		row++;
	}

	// Signal instances details
	//
	for (int propertyIndex = 0; propertyIndex < pSignalPropertyManager->count(); propertyIndex++)
	{
		if (signalInstances.count() == 0)
		{
			break;
		}

		bool isExpert = theSettings.isExpertMode();

		QVariant previousValue = pSignalPropertyManager->value(&signalInstances[0], propertyIndex, isExpert);

		QList<QStandardItem*> column;
		int columnIndex = m_historyModel->columnCount();

		for (int signalIndex = 0; signalIndex < signalInstances.count(); signalIndex++)
		{
			QVariant currentValue = pSignalPropertyManager->value(&signalInstances[signalIndex], propertyIndex, isExpert);
			QStandardItem* newItem = new QStandardItem(currentValue.toString());

			if (currentValue != previousValue)
			{
				column.last()->setData(QColor(Qt::yellow), Qt::BackgroundRole);
				previousValue = currentValue;
				if (defaultColumns.contains(columnIndex) == false)
				{
					defaultColumns.push_back(columnIndex);
				}
			}
			column.push_back(newItem);
		}

		m_historyModel->appendColumn(column);
		m_historyModel->setHeaderData(columnIndex, Qt::Horizontal, pSignalPropertyManager->caption(propertyIndex));
	}

	new TableDataVisibilityController(historyView, "SignalHistoryDialog", defaultColumns);
}

void SignalHistoryDialog::closeEvent(QCloseEvent* event)
{
	saveWindowPosition(this, "SignalHistoryDialog");

	QDialog::closeEvent(event);
}


const QString FindSignalDialog::notUniqueMessage("No - not unique");
const QString FindSignalDialog::notEditableMessage("No - checked out by another user");
const QString FindSignalDialog::notCorrectIdMessage("No - uncorrect ID");
const QString FindSignalDialog::cannotCheckoutMessage("No - can't checkout");
const QString FindSignalDialog::replaceableMessage("Yes");
const QString FindSignalDialog::replacedMessage("Yes - replaced");

FindSignalDialog::FindSignalDialog(int currentUserId, bool currentUserIsAdmin, QTableView* parent) :
	QDialog(parent, Qt::Dialog),
	m_signalTable(parent),
	m_signalSetProvider(SignalSetProvider::getInstance()),
	m_findString(new QLineEdit(this)),
	m_replaceString(new QLineEdit(this)),
	m_searchInPropertyList(new QComboBox(this)),
	m_caseSensitive(new QCheckBox("Case sensitive", this)),
	m_wholeWords(new QCheckBox("Whole words only", this)),
	m_searchInSelected(new QCheckBox("Search in selected only", this)),
	m_signalsQuantityLabel(new QLabel(this)),
	m_canBeReplacedQuantityLabel(new QLabel(this)),
	m_foundList(new QTableView(this)),
	m_foundListModel(new QStandardItemModel(0, 3, this)),
	m_replaceAllButton(new QPushButton("Replace All", this)),
	m_replaceAndFindNextButton(new QPushButton("Replace and Find", this)),
	m_findPreviousButton(new QPushButton("Find Previous", this)),
	m_findNextButton(new QPushButton("Find Next", this)),
	m_replaceableSignalQuantityBlinkTimer(new QTimer(this)),
	m_regExp4Id(SignalProperties::cacheValidator),
	m_currentUserId(currentUserId),
	m_currentUserIsAdmin(currentUserIsAdmin)
{
	setWindowTitle("Find and Replace");
	m_signalProxyModel = dynamic_cast<SignalsProxyModel*>(m_signalTable->model());
	if (m_signalProxyModel == nullptr)
	{
		assert(false);
		deleteLater();
	}

	m_signalModel = dynamic_cast<SignalsModel*>(m_signalProxyModel->sourceModel());
	if (m_signalModel == nullptr)
	{
		assert(false);
		deleteLater();
	}

	m_searchInPropertyList->addItems(QStringList() <<
									 SignalProperties::appSignalIDCaption <<
									 SignalProperties::customSignalIDCaption <<
									 SignalProperties::captionCaption <<
									 SignalProperties::equipmentIDCaption);

	m_foundList->setModel(m_foundListModel);

	m_foundList->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_foundList->setAlternatingRowColors(false);
	m_foundList->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");
	m_foundList->setEditTriggers(QTableView::NoEditTriggers);
	m_foundList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_foundList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	m_foundList->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_foundList->fontMetrics().height() * 1.4));
	m_foundList->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	m_foundList->horizontalHeader()->setHighlightSections(false);
	m_foundList->horizontalHeader()->setDefaultSectionSize(100);
	m_foundList->horizontalHeader()->setStretchLastSection(true);

	m_foundListModel->setHeaderData(0, Qt::Horizontal, "Found");
	m_foundListModel->setHeaderData(1, Qt::Horizontal, "Replace result");
	m_foundListModel->setHeaderData(2, Qt::Horizontal, "Can be replaced");

	m_replaceAllButton->setAutoDefault(false);
	m_replaceAndFindNextButton->setAutoDefault(false);
	m_findPreviousButton->setAutoDefault(false);
	m_findNextButton->setAutoDefault(false);

	connect(m_findString, &QLineEdit::returnPressed, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_replaceString, &QLineEdit::returnPressed, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_searchInPropertyList, &QComboBox::currentTextChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_caseSensitive, &QCheckBox::stateChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_wholeWords, &QCheckBox::stateChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);
	connect(m_searchInSelected, &QCheckBox::stateChanged, this, &FindSignalDialog::generateListIfNeededWithWarning);

	connect(m_replaceString, &QLineEdit::textEdited, this, &FindSignalDialog::updateAllReplacement);

	connect(m_foundList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FindSignalDialog::selectCurrentSignalOnAppSignalsTab);

	connect(m_replaceAllButton, &QPushButton::clicked, this, &FindSignalDialog::replaceAll);
	connect(m_replaceAndFindNextButton, &QPushButton::clicked, this, &FindSignalDialog::replaceAndFindNext);
	connect(m_findPreviousButton, &QPushButton::clicked, this, &FindSignalDialog::findPrevious);
	connect(m_findNextButton, &QPushButton::clicked, this, &FindSignalDialog::findNext);

	QFormLayout* form = new QFormLayout;

	form->addRow("Find:", m_findString);
	form->addRow("Replace with:", m_replaceString);

	QHBoxLayout* searchSettingsRow = new QHBoxLayout;
	searchSettingsRow->addWidget(m_searchInPropertyList);
	searchSettingsRow->addWidget(m_caseSensitive);
	searchSettingsRow->addWidget(m_wholeWords);
	searchSettingsRow->addWidget(m_searchInSelected);
	searchSettingsRow->addStretch();

	form->addRow("Search in:", searchSettingsRow);

	QHBoxLayout* resultRow = new QHBoxLayout;
	resultRow->addWidget(m_signalsQuantityLabel);
	resultRow->addWidget(m_canBeReplacedQuantityLabel);
	resultRow->addStretch();

	connect(m_replaceableSignalQuantityBlinkTimer, &QTimer::timeout, this, &FindSignalDialog::blinkReplaceableSignalQuantity);

	form->addRow(resultRow);

	QVBoxLayout* vLayout = new QVBoxLayout;
	vLayout->addLayout(form);

	vLayout->addWidget(m_foundList);

	QHBoxLayout* hLayout = new QHBoxLayout;

	hLayout->addWidget(m_replaceAllButton);
	hLayout->addWidget(m_replaceAndFindNextButton);
	hLayout->addWidget(m_findPreviousButton);
	hLayout->addWidget(m_findNextButton);

	vLayout->addLayout(hLayout);

	setLayout(vLayout);

	setWindowPosition(this, "FindSignalDialog");
	new TableDataVisibilityController(m_foundList, "FindSignalDialog", QVector<int>() << 0 << 1 << 2);

	QTimer::singleShot(0, [this](){ m_findString->setFocus(); });
}

void FindSignalDialog::notifyThatSignalSetHasChanged()
{
	m_isMatchToCurrentSignalSet = false;
	generateListIfNeeded(false);
}

void FindSignalDialog::closeEvent(QCloseEvent* event)
{
	saveDialogGeometry();
	m_shouldReopen = false;

	QDialog::closeEvent(event);
}

void FindSignalDialog::generateListIfNeeded(bool throwWarning)
{
	SearchOptions currentOptions = getCurrentSearchOptions();
	if (m_searchOptionsUsedLastTime == currentOptions && m_isMatchToCurrentSignalSet == true)
	{
		return;
	}

	if (currentOptions.findString.isEmpty())
	{
		m_findString->setFocus();
		return;
	}

	m_searchOptionsUsedLastTime = currentOptions;

	QString fieldName = m_searchInPropertyList->currentText();
	m_checkCorrectnessOfId = ((fieldName == SignalProperties::appSignalIDCaption) ||
							  (fieldName == SignalProperties::customSignalIDCaption));

	reloadCurrentIdsMap();

	int selectedSignalId = getSignalId(getSelectedRow());

	m_foundListModel->removeRows(0, m_foundListModel->rowCount());
	m_totalSignalQuantity = 0;
	m_replaceableSignalQuantity = 0;

	if (currentOptions.searchInSelected == false)
	{
		for (int i = 0; i < m_signalModel->rowCount(); i++)
		{
			addSignalIfNeeded(m_signalSetProvider->getLoadedSignal(i));
		}
	}
	else
	{
		QModelIndexList selection = m_signalTable->selectionModel()->selectedRows(0);
		if (selection.count() == 0 && throwWarning)
		{
			QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
		}
		for (int i = 0; i < selection.count(); i++)
		{
			int row = m_signalProxyModel->mapToSource(selection[i]).row();
			addSignalIfNeeded(m_signalSetProvider->getLoadedSignal(row));
		}
	}

	markFistInstancesIfItTheyNotUnique();

	for (int currentIndex = 0; currentIndex < m_foundListModel->rowCount(); currentIndex++)
	{
		if (selectedSignalId == getSignalId(currentIndex))
		{
			m_foundList->setCurrentIndex(m_foundListModel->index(currentIndex, 0));
		}
	}

	updateCounters();

	m_isMatchToCurrentSignalSet = true;
}

void FindSignalDialog::updateAllReplacement()
{
	m_replaceableSignalQuantity = 0;

	reloadCurrentIdsMap();

	for (int i = 0; i < m_foundListModel->rowCount(); i++)
	{
		updateReplacement(i);
	}

	markFistInstancesIfItTheyNotUnique();

	updateCounters();
}

void FindSignalDialog::updateReplacement(int row)
{
	int signalId = getSignalId(row);
	int signalIndex = m_signalSetProvider->keyIndex(signalId);
	const Signal& signal = m_signalSetProvider->getLoadedSignal(signalIndex);

	updateReplacement(signal, row);
}

void FindSignalDialog::updateReplacement(const Signal& signal, int row)
{
	QString propertyValue = getProperty(signal);

	int start = 0;
	int end = -1;

	QString replaced = propertyValue;
	while (match(replaced, start, end) == true)
	{
		replaced = replaced.left(start) + m_replaceString->text() + replaced.mid(end);
		start += m_replaceString->text().length();
	}

	m_foundListModel->setData(m_foundListModel->index(row, 1), replaced, Qt::DisplayRole);

	bool replaceable = true;

	if (replaceable == true && checkForEditableSignal(signal) == false)
	{
		replaceable = false;
		m_foundListModel->setData(m_foundListModel->index(row, 2), notEditableMessage, Qt::DisplayRole);
	}

	if (replaceable == true && m_checkCorrectnessOfId == true && checkForCorrectSignalId(replaced) == false)
	{
		replaceable = false;
		m_foundListModel->setData(m_foundListModel->index(row, 2), notCorrectIdMessage, Qt::DisplayRole);
	}

	if (replaceable == true && m_checkCorrectnessOfId == true && checkForUniqueSignalId(propertyValue, replaced) == false)
	{
		replaceable = false;
		m_foundListModel->setData(m_foundListModel->index(row, 2), notUniqueMessage, Qt::DisplayRole);
	}

	m_foundListModel->setData(m_foundListModel->index(row, 2), replaceable, Qt::UserRole);
	if (replaceable == true)
	{
		m_replaceableSignalQuantity++;
		m_foundListModel->setData(m_foundListModel->index(row, 2), replaceableMessage, Qt::DisplayRole);
	}
}

void FindSignalDialog::addSignalIfNeeded(const Signal& signal)
{
	QString propertyValue = getProperty(signal);

	int start = 0;
	int end = -1;
	if (match(propertyValue, start, end) == true)
	{
		int currentIndex = m_foundListModel->rowCount();
		m_foundListModel->insertRows(currentIndex, 1);

		m_foundListModel->setData(m_foundListModel->index(currentIndex, 0), propertyValue, Qt::DisplayRole);
		m_foundListModel->setData(m_foundListModel->index(currentIndex, 0), signal.ID(), Qt::UserRole);

		m_totalSignalQuantity++;

		updateReplacement(signal, currentIndex);
	}
}

bool FindSignalDialog::match(QString signalProperty, int& start, int& end)
{
	if (m_findString == nullptr)
	{
		assert(false);
		close();
		return false;
	}
	QString findString = m_searchOptionsUsedLastTime.findString;
	if (findString.isEmpty())
	{
		return false;
	}

	if (m_searchOptionsUsedLastTime.caseSensitive == false)
	{
		signalProperty = signalProperty.toUpper();
	}

	start = signalProperty.indexOf(findString, start);
	if (start != -1)
	{
		end = start + findString.size();

		if (m_searchOptionsUsedLastTime.wholeWords == true)
		{
			bool isPartOfWord = false;

			if (start > 0)
			{
				QChar previousChar = signalProperty[start - 1];
				if (previousChar.isLetterOrNumber() || previousChar == '_')
				{
					isPartOfWord = true;
				}
			}

			if (end < signalProperty.size())
			{
				QChar nextChar = signalProperty[end];
				if (nextChar.isLetterOrNumber() || nextChar == '_')
				{
					isPartOfWord = true;
				}
			}

			if (isPartOfWord == true)
			{
				start = -1;
				end = -1;
				return false;
			}
		}
		return true;
	}
	return false;
}

bool FindSignalDialog::checkForEditableSignal(const Signal& signal)
{
	return (signal.checkedOut() == false || signal.userID() == m_currentUserId || m_currentUserIsAdmin == true);
}

bool FindSignalDialog::checkForUniqueSignalId(const QString& original, const QString& replaced)
{
	bool result = true;

	if (m_repeatedSignalIds.contains(original))
	{
		result = false;
	}

	if (m_signalIds.contains(replaced) == true)
	{
		result = false;
		m_repeatedSignalIds.insert(replaced);
	}
	else
	{
		m_signalIds.insert(replaced);
	}

	return result;
}

bool FindSignalDialog::checkForCorrectSignalId(const QString& replaced)
{
	return m_regExp4Id.exactMatch(replaced);
}

FindSignalDialog::SearchOptions FindSignalDialog::getCurrentSearchOptions()
{
	SearchOptions options;
	if (m_findString == nullptr ||
			m_searchInPropertyList == nullptr ||
			m_searchInSelected == nullptr ||
			m_caseSensitive == nullptr ||
			m_wholeWords == nullptr)
	{
		assert(false);
		return options;
	}

	int propertyIndex = SignalPropertyManager::getInstance()->index(m_searchInPropertyList->currentText());

	if (propertyIndex == -1)
	{
		assert(false);
		return options;
	}

	options.searchedPropertyIndex = propertyIndex;
	options.searchInSelected = m_searchInSelected->isChecked();
	options.caseSensitive = m_caseSensitive->isChecked();
	options.wholeWords = m_wholeWords->isChecked();

	options.findString = m_findString->text();

	if (options.caseSensitive == false)
	{
		options.findString = options.findString.toUpper();
	}

	return options;
}

QString FindSignalDialog::getProperty(const Signal& signal)
{
	if (m_searchOptionsUsedLastTime.searchedPropertyIndex == -1)
	{
		return QString();
	}

	return SignalPropertyManager::getInstance()->value(&signal, m_searchOptionsUsedLastTime.searchedPropertyIndex, theSettings.isExpertMode()).toString();
}

void FindSignalDialog::setProperty(Signal& signal, const QString& value)
{
	if (m_searchOptionsUsedLastTime.searchedPropertyIndex == -1)
	{
		assert(false);
		return;
	}

	SignalPropertyManager::getInstance()->setValue(&signal, m_searchOptionsUsedLastTime.searchedPropertyIndex, value, theSettings.isExpertMode());
}

int FindSignalDialog::getSignalId(int row)
{
	return m_foundListModel->data(m_foundListModel->index(row, 0), Qt::UserRole).toInt();
}

int FindSignalDialog::getSelectedRow()
{
	return m_foundList->currentIndex().row();
}

void FindSignalDialog::selectRow(int row)
{
	m_foundList->setCurrentIndex(m_foundListModel->index(row, 0));
}

bool FindSignalDialog::isReplaceable(int row)
{
	bool replaceable = m_foundListModel->data(m_foundListModel->index(row, 2), Qt::UserRole).toBool();
	if (replaceable == false)
	{
		return false;
	}
	int signalId = getSignalId(row);

	m_signalSetProvider->loadSignal(signalId);

	int signalIndex = m_signalSetProvider->keyIndex(signalId);
	if (signalIndex == -1)	// Doesn't exist???
	{
		assert(false);
		return false;
	}

	return m_signalSetProvider->isEditableSignal(signalIndex);
}

void FindSignalDialog::replace(int row)
{
	int signalId = getSignalId(row);
	int signalIndex = m_signalSetProvider->keyIndex(signalId);
	if (signalIndex == -1)	// Doesn't exist???
	{
		assert(false);
		return;
	}

	QString errorMessage;
	bool checkedout = m_signalSetProvider->checkoutSignal(signalIndex, errorMessage);
	if (checkedout == false)
	{
		m_foundListModel->setData(m_foundListModel->index(row, 2), cannotCheckoutMessage + ':' + errorMessage, Qt::DisplayRole);
		m_foundListModel->setData(m_foundListModel->index(row, 2), false, Qt::UserRole);
		return;
	}

	Signal signal = m_signalSetProvider->getSignalByID(signalId);
	QString newValue = m_foundListModel->data(m_foundListModel->index(row, 1), Qt::DisplayRole).toString();

	setProperty(signal, newValue);

	m_signalSetProvider->saveSignal(signal);
	m_foundListModel->setData(m_foundListModel->index(row, 2), replacedMessage, Qt::DisplayRole);
}

void FindSignalDialog::reloadCurrentIdsMap()
{
	if (m_checkCorrectnessOfId == false)
	{
		return;
	}

	m_signalIds.clear();
	m_repeatedSignalIds.clear();

	for (int i = 0; i < m_signalModel->rowCount(); i++)
	{
		QString id = getProperty(m_signalSetProvider->getLoadedSignal(i));
		if (m_signalIds.contains(id))
		{
			m_repeatedSignalIds.insert(id);
		}
		else
		{
			m_signalIds.insert(id);
		}
	}
}

void FindSignalDialog::markFistInstancesIfItTheyNotUnique()
{
	if (m_checkCorrectnessOfId == false)
	{
		return;
	}

	for (int currentIndex = 0; currentIndex < m_foundListModel->rowCount(); currentIndex++)
	{
		bool replaceable = m_foundListModel->data(m_foundListModel->index(currentIndex, 2), Qt::UserRole).toBool();
		QString replacement = m_foundListModel->data(m_foundListModel->index(currentIndex, 1), Qt::DisplayRole).toString();
		if (replaceable == true && m_repeatedSignalIds.contains(replacement) == true)
		{
			m_foundListModel->setData(m_foundListModel->index(currentIndex, 2), false, Qt::UserRole);
			m_foundListModel->setData(m_foundListModel->index(currentIndex, 2), notUniqueMessage, Qt::DisplayRole);
			m_replaceableSignalQuantity--;
		}
	}
}

void FindSignalDialog::updateCounters()
{
	m_signalsQuantityLabel->setText(QString("Found signals (%1) / ").arg(m_totalSignalQuantity, 3, 10, Latin1Char::ZERO));
	m_canBeReplacedQuantityLabel->setText(QString("Can be replaced:(%1)").arg(m_replaceableSignalQuantity, 3, 10, Latin1Char::ZERO));

	if (m_totalSignalQuantity != m_replaceableSignalQuantity)
	{
		blinkReplaceableSignalQuantity();
		m_replaceableSignalQuantityBlinkTimer->start(500);
	}
	else
	{
		m_replaceableSignalQuantityBlinkTimer->stop();
		m_canBeReplacedQuantityLabel->setStyleSheet("");
	}
}

void FindSignalDialog::saveDialogGeometry()
{
	saveWindowPosition(this, "FindSignalDialog");
}

void FindSignalDialog::generateListIfNeededWithWarning()
{
	generateListIfNeeded(true);
}

void FindSignalDialog::replaceAll()
{
	generateListIfNeeded();

	bool allAreReplaceable = true;

	for (int row = 0; row < m_foundListModel->rowCount(); row++)
	{
		if (isReplaceable(row) == false)
		{
			allAreReplaceable = false;
			break;
		}
	}

	if (allAreReplaceable == false)
	{
		QMessageBox msgBox(
					QMessageBox::Warning,
					"Warning",
					"Some signals have not replaceable values",
					QMessageBox::Yes | QMessageBox::Cancel,
					this);

		msgBox.setButtonText(QMessageBox::Yes, "Replace all possible");

		if (msgBox.exec() == QMessageBox::Cancel)
		{
			return;
		}
	}

	for (int row = 0; row < m_foundListModel->rowCount(); row++)
	{
		if (isReplaceable(row) == true)
		{
			replace(row);
		}
	}
}

void FindSignalDialog::replaceAndFindNext()
{
	generateListIfNeeded();

	int row = getSelectedRow();

	if (isReplaceable(row) == true)
	{
		replace(row);
		selectRow(row + 1);
	}
	else
	{
		QMessageBox msgBox(
					QMessageBox::Warning,
					"Warning",
					"Value could not be replaced",
					QMessageBox::Yes | QMessageBox::Cancel,
					this);

		msgBox.setButtonText(QMessageBox::Yes, "Skip and goto next");

		if (msgBox.exec() == QMessageBox::Yes)
		{
			selectRow(row + 1);
		}
	}
}

void FindSignalDialog::findPrevious()
{
	generateListIfNeeded();

	int row = getSelectedRow();

	if (row > 0)
	{
		selectRow(row - 1);
	}
}

void FindSignalDialog::findNext()
{
	generateListIfNeeded();

	int row = getSelectedRow();

	if (row < m_foundListModel->rowCount() - 1)
	{
		selectRow(row + 1);
	}
}

void FindSignalDialog::selectCurrentSignalOnAppSignalsTab()
{
	QModelIndex index = m_foundList->currentIndex();
	if (index.isValid())
	{
		m_signalTable->clearSelection();
		int signalId = m_foundListModel->data(m_foundListModel->index(index.row(), 0), Qt::UserRole).toInt();
		emit signalSelected(signalId);
	}
}

void FindSignalDialog::blinkReplaceableSignalQuantity()
{
	if (m_replaceableSignalQuantityBlinkIsOn)
	{
		m_canBeReplacedQuantityLabel->setStyleSheet("QLabel {background-color : red; color : yellow;}");
	}
	else
	{
		m_canBeReplacedQuantityLabel->setStyleSheet("QLabel {background-color : yellow; color : red;}");
	}

	m_replaceableSignalQuantityBlinkIsOn = !m_replaceableSignalQuantityBlinkIsOn;
}
