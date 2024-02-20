#include "SignalsModels.h"
#include "AppSignalSetProvider.h"
#include "AppSignalPropertyManager.h"
#include "SignalsTabPage.h"
#include "Settings.h"
#include "../UtilsLib/WUtils.h"

// -------------------------------------------------------------------------------------------------------
//
// SignalsModel class implementation
//
// -------------------------------------------------------------------------------------------------------

SignalsModel::SignalsModel(AppSignalSetProvider* signalSetProvider,
						   AppSignalPropertyManager* propManager,
						   QWidget* parent) :
	QAbstractTableModel(parent),
	m_signalSetProvider(signalSetProvider),
	m_propManager(propManager),
	m_rowCount(signalSetProvider->signalCount()),
	m_columnCount(signalSetProvider->signalPropertyManager().count()),
	m_parentWidget(parent)

{
	TEST_PTR_RETURN(m_signalSetProvider);
	TEST_PTR_RETURN(m_propManager);

	connect(m_signalSetProvider, &AppSignalSetProvider::signalsCountChanged, this, &SignalsModel::slot_signalsCountChanged);
	connect(m_signalSetProvider, &AppSignalSetProvider::signalsUpdated, this, &SignalsModel::slot_signalsUpdated);

	connect(m_propManager, &AppSignalPropertyManager::propertyCountWillIncrease, this, &SignalsModel::beginIncreaseColumnCount, Qt::DirectConnection);
	connect(m_propManager, &AppSignalPropertyManager::propertyCountWillDecrease, this, &SignalsModel::beginDecreaseColumnCount, Qt::DirectConnection);
	connect(m_propManager, &AppSignalPropertyManager::propertyCountIncreased, this, &SignalsModel::endIncreaseColumnCount, Qt::DirectConnection);
	connect(m_propManager, &AppSignalPropertyManager::propertyCountDecreased, this, &SignalsModel::endDecreaseColumnCount, Qt::DirectConnection);
}

SignalsModel::~SignalsModel()
{
}

AppSignalSetProvider* SignalsModel::signalSetProvider()
{
	return m_signalSetProvider;
}

AppSignalPropertyManager* SignalsModel::propManager()
{
	return m_propManager;
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

	return m_columnCount;
}

QVariant SignalsModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	const AppSignal* signal = m_signalSetProvider->getLoadedSignalByIndex(row, false);

	TEST_PTR_RETURN_VALUE(signal, QVariant());

	if (row >= m_signalSetProvider->signalCount() || signal->isLoaded() == false)
	{
		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (signal->checkedOut() == true)
		{
			E::VcsItemAction action = signal->instanceAction();

			switch (action)
			{
			case E::VcsItemAction::Added:		return m_addedBrush;
			case E::VcsItemAction::Modified:	return m_modifiedBrush;
			case E::VcsItemAction::Deleted:		return m_deletedBrush;
			default:							Q_ASSERT(false);
			}
		}

		return m_checkedInBrush;
	}

	if (role == Qt::ForegroundRole)
	{
		if (signal->excludeFromBuild() == true)
		{
			return m_excludedFromBuildBrush;
		}
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		QVariant value = m_propManager->value(signal, col, theSettings.isExpertMode());

		if (value.isValid() == true &&
			m_propManager->dependsOnPrecision(col) == true &&
			(value.typeId() == QMetaType::Float || value.typeId() == QMetaType::Double))
		{
			return m_defaultLocale.toString(value.toDouble(), 'f', signal->decimalPlaces());
		}

		return value;
	}

	return QVariant();
}

QVariant SignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (orientation == Qt::Horizontal)
		{
			return m_propManager->name(section);
		}

		if (orientation == Qt::Vertical)
		{
			if (section < m_signalSetProvider->signalCount())
			{
				return m_signalSetProvider->signalID(section);
			}
		}
	}
	return QVariant();
}

bool SignalsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
		int row = index.row();

		if (row >= m_signalSetProvider->signalCount())
		{
			Q_ASSERT(false);
			return false;
		}

		const AppSignal* ls = m_signalSetProvider->getLoadedSignalByIndex(row, false);

		AppSignal s(*ls);

		m_propManager->setValue(&s, index.column(), value, theSettings.isExpertMode());

		// This should be done by SignalsDelegate::setModelData
		Q_ASSERT(false);
		m_signalSetProvider->saveSignal(&s,  m_parentWidget);
	}
	else
	{
		return QAbstractTableModel::setData(index, value, role);
	}

	return true;
}

Qt::ItemFlags SignalsModel::flags(const QModelIndex &index) const
{
	if (index.isValid() == false)
	{
		return QAbstractTableModel::flags(index);
	}

	int row = index.row();
	int column = index.column();

	if (column >= m_propManager->count())
	{
		return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
	}

	assert(row < m_signalSetProvider->signalCount());

	const AppSignal* s = m_signalSetProvider->getLoadedSignalByIndex(row, false);

	TEST_PTR_RETURN_VALUE(s, Qt::NoItemFlags);

	if (m_propManager->getBehaviour(*s, index.column()) == E::PropertyBehaviourType::Write)
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	else
	{
		return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
	}
}

SignalsTablePropEditor* SignalsModel::createDelegate(SignalsProxyModel* signalsProxyModel)
{
	return new SignalsTablePropEditor(signalsProxyModel, parent());
}

QWidget* SignalsModel::parentWidget()
{
	return m_parentWidget;
}

void SignalsModel::prepareForReset()
{
	beginResetModel();
}

void SignalsModel::finishReset()
{
	m_rowCount = m_signalSetProvider->signalCount();
	m_columnCount = m_signalSetProvider->signalPropertyManager().count();
	endResetModel();
}

void SignalsModel::slot_signalsUpdated(const std::vector<int>& indexes)
{
	if (indexes.empty())
	{
		return;
	}

	if (indexes.size() == 1)
	{
		emit dataChanged(index(indexes[0], 0), index(indexes[0], m_columnCount));
		return;
	}

	std::vector<int> varIndexes(indexes.begin(), indexes.end());

	std::sort(varIndexes.begin(), varIndexes.end());

	std::vector<std::pair<int, int>> ranges;

	int rangeStartValue = 0;
	int prevValue = 0;

	rangeStartValue = prevValue = varIndexes[0];

	for(int i = 1; i < varIndexes.size(); i++)
	{
		int curValue = varIndexes[i];

		if (prevValue + 1 == curValue)
		{
			prevValue = curValue;
			continue;
		}

		// add range
		//
		ranges.emplace_back(rangeStartValue, prevValue);
		rangeStartValue = prevValue = curValue;
	}

	ranges.emplace_back(rangeStartValue, prevValue);

	for(const auto& p : ranges)
	{
		emit dataChanged(index(p.first, 0), index(p.second, m_columnCount));
	}
}

void SignalsModel::slot_signalsCountChanged()
{
	beginResetModel();
	m_rowCount = m_signalSetProvider->signalCount();
	m_columnCount = m_signalSetProvider->signalPropertyManager().count();
	endResetModel();
}

void SignalsModel::beginIncreaseColumnCount(int newColumnCount)
{
	assert(newColumnCount > m_columnCount);

	beginInsertColumns(QModelIndex(), m_columnCount, newColumnCount - 1);
	m_columnCount = newColumnCount;
}

void SignalsModel::beginDecreaseColumnCount(int newColumnCount)
{
	assert(newColumnCount < m_columnCount);

	beginRemoveColumns(QModelIndex(), newColumnCount, m_columnCount - 1);
	m_columnCount = newColumnCount;
}

void SignalsModel::endIncreaseColumnCount()
{
	endInsertColumns();
}

void SignalsModel::endDecreaseColumnCount()
{
	endRemoveColumns();
}

// -------------------------------------------------------------------------------------------------------
//
// SignalsProxyModel class implementation
//
// -------------------------------------------------------------------------------------------------------

SignalsProxyModel::SignalsProxyModel(SignalsModel* sourceModel, QObject *parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel),
	m_signalType(SignalsTabPage::FILTER_ST_ANY),
	m_idFilterField(SignalsTabPage::FILTER_STR_APP_SIGNAL_ID)
{
	TEST_PTR_RETURN(m_sourceModel);

	AppSignalSetProvider* provider = m_sourceModel->signalSetProvider();
	TEST_PTR_RETURN(provider);

	connect(this, &SignalsProxyModel::aboutToSort, provider, &AppSignalSetProvider::enforceAllSignalsLoading, Qt::DirectConnection);
	connect(this, &SignalsProxyModel::aboutToFilter, provider, &AppSignalSetProvider::enforceAllSignalsLoading, Qt::DirectConnection);
	setSourceModel(sourceModel);
}

bool SignalsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const
{
	if (m_signalType == SignalsTabPage::FILTER_ST_ANY && m_strIdMasks.isEmpty())
	{
		return true;
	}

	AppSignalSetProvider* provider = m_sourceModel->signalSetProvider();
	TEST_PTR_RETURN_FALSE(provider);

	const AppSignal* currentSignal = provider->getLoadedSignalByIndex(sourceRow, false);

	TEST_PTR_RETURN_FALSE(currentSignal);

	if (m_signalType != SignalsTabPage::FILTER_ST_ANY &&
		m_signalType != TO_INT(currentSignal->signalType()))
	{
		return false;
	}

	if (m_strIdMasks.isEmpty())
	{
		return true;
	}

	for (const QString& idMask : m_strIdMasks)
	{
		QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(idMask.trimmed()));

		bool result = false;

		switch (m_idFilterField)
		{
			case SignalsTabPage::FILTER_STR_ANY:
				result = rx.match(currentSignal->appSignalID().trimmed()).hasMatch() ||
						 rx.match(currentSignal->customAppSignalID().trimmed()).hasMatch() ||
						 rx.match(currentSignal->equipmentID().trimmed()).hasMatch() ||
						 rx.match(currentSignal->caption().trimmed()).hasMatch();
				break;

			case SignalsTabPage::FILTER_STR_APP_SIGNAL_ID:
				result = rx.match(currentSignal->appSignalID().trimmed()).hasMatch();
				break;

			case SignalsTabPage::FILTER_STR_CUSTOM_APP_SIGNAL_ID:
				result = rx.match(currentSignal->customAppSignalID().trimmed()).hasMatch();
				break;

			case SignalsTabPage::FILTER_STR_EQUIPMENT_ID:
				result = rx.match(currentSignal->equipmentID().trimmed()).hasMatch();
				break;

			case SignalsTabPage::FILTER_STR_CAPTION:
				result = rx.match(currentSignal->caption().trimmed()).hasMatch();
				break;

			case SignalsTabPage::FILTER_STR_TAGS:
				result = rx.match(currentSignal->tagsStr().trimmed()).hasMatch();
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

bool SignalsProxyModel::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
{
	QVariant l = m_sourceModel->data(sourceLeft);
	QVariant r = m_sourceModel->data(sourceRight);

	if (l == r)
	{
		AppSignalSetProvider* provider = m_sourceModel->signalSetProvider();
		TEST_PTR_RETURN_FALSE(provider);

		const AppSignal* sl = provider->getSignalByIndex(sourceLeft.row());
		const AppSignal* sr = provider->getSignalByIndex(sourceRight.row());

		return sl->appSignalID() < sr->appSignalID();
	}
	else
	{
		return QSortFilterProxyModel::lessThan(sourceLeft, sourceRight);
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

// -------------------------------------------------------------------------------------------------------
//
// SignalsTablePropEditor class implementation
//
// -------------------------------------------------------------------------------------------------------

SignalsTablePropEditor::SignalsTablePropEditor(SignalsProxyModel* proxyModel, QObject* parent) :
	QStyledItemDelegate(parent),
	m_proxyModel(proxyModel),
	m_provider(AppSignalSetProvider::getInstance()),
	m_propManager(AppSignalPropertyManager::getInstance()),
	m_dblValidatorEx(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), 1000, false)
{
	connect(this, &QAbstractItemDelegate::closeEditor, this, &SignalsTablePropEditor::onCloseEditorEvent);
}

SignalsTablePropEditor::~SignalsTablePropEditor()
{
}

QWidget* SignalsTablePropEditor::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& proxyIndex) const
{
	Q_ASSERT(proxyIndex.model() == m_proxyModel);

	TEST_PTR_RETURN_NULLPTR(m_provider);
	TEST_PTR_RETURN_NULLPTR(m_propManager);

	// always reinit with vars on createEditor()!

	m_editingSignalId = AppSignalSet::BAD_ID;
	m_signalCheckedOut = false;
	m_valueChanged = false;

	//

	int col = proxyIndex.column();
	int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();

	int editedSignalID = m_provider->signalID(sourceRow);

	const AppSignal* s = m_provider->loadSignal(editedSignalID, true);	// get current checkedOut state

	TEST_PTR_RETURN_NULLPTR(s);

	if (m_provider->isEditableSignal(s) == false)
	{
		QMessageBox::warning(parent, tr("Warning"),
							 QString("Signal %1\nis checked out by user %2.\n\nEditing impossible!").
									arg(s->appSignalID()).arg(m_provider->signalCheckedOutByUser(*s)));
		return nullptr;
	}

	Q_ASSERT(editedSignalID == s->ID());

	const AppSignalPropertyDescription& propDesc = m_propManager->getPropertyDescription(col);

	if (propDesc.isValid() == false)
	{
		Q_ASSERT(false);
		return nullptr;
	}

	if (AppSignalProperties::isPropertyExists(*s, propDesc.name()) == false)
	{
		return nullptr;
	}

	bool isExpert = theSettings.isExpertMode();

	E::PropertyBehaviourType behaviour = m_propManager->getBehaviour(*s, col);

	if (m_propManager->isHidden(behaviour, isExpert) || m_propManager->isReadOnly(behaviour, isExpert))
	{
		return nullptr;
	}

	if (s->checkedOut() == false)
	{
		if (m_provider->checkoutSignalByIndex(sourceRow, nullptr) == false)
		{
			return nullptr;
		}

		m_signalCheckedOut = true;
	}

	m_editingSignalId = s->ID();

	const AppSignal* appSignal = m_provider->loadSignal(editedSignalID, true);	// update new checkedOut state on view

	TEST_PTR_RETURN_VALUE(appSignal, nullptr);

	// QComboBox editor creation
	//
	if (m_propManager->isEnumProperty(col) == true)
	{
		std::vector<std::pair<int, QString>> enumPropValues;

		m_propManager->getSignalEnumPropertyValues(*appSignal, col, &enumPropValues);

		QComboBox* cb = new QComboBox(parent);

		for (const auto& value : enumPropValues)
		{
			cb->addItem(value.second, value.first);
		}

		return cb;
	}

	// QLineEdit editor creation
	//
	switch (m_propManager->type(col))
	{
	case QMetaType::QString:
	{
		QLineEdit* le = new QLineEdit(parent);

		if (m_propManager->name(col).right(2) == "ID")
		{
			QRegularExpression rx4ID(AppSignal::IDENTIFICATORS_VALIDATOR);
			le->setValidator(new QRegularExpressionValidator(rx4ID, le));
		}
		else
		{
			QRegularExpression rx4Name("^.+$");
			le->setValidator(new QRegularExpressionValidator(rx4Name, le));
		}

		return le;
	}
	case QMetaType::Float:
	case QMetaType::Double:
	{
		QLineEdit* le = new QLineEdit(parent);
		le->setValidator(&m_dblValidatorEx);
		return le;
	}
	case QMetaType::Int:
	case QMetaType::UInt:
	{
		QLineEdit* le = new QLineEdit(parent);
		le->setValidator(new QIntValidator(le));
		return le;
	}
	case QMetaType::Bool:
	{
		QComboBox* cb = new QComboBox(parent);
		cb->addItem("false", false);
		cb->addItem("true", true);
		return cb;
	}
	default:
		if (m_propManager->type(col) == qMetaTypeId<TuningValue>())
		{
			QLineEdit* le = new QLineEdit(parent);
			if (s->isAnalog())
			{
				le->setValidator(&m_dblValidatorEx);
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
			return QStyledItemDelegate::createEditor(parent, option, proxyIndex);
		}
	}
}

void SignalsTablePropEditor::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(index);

	editor->setGeometry(option.rect);

	QComboBox* cb = dynamic_cast<QComboBox*>(editor);

	if (cb != nullptr)
	{
		cb->showPopup();
	}
}

void SignalsTablePropEditor::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	TEST_PTR_RETURN(m_provider);
	TEST_PTR_RETURN(m_propManager);

	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();
	if (row >= m_provider->signalCount())
	{
		return;
	}

	QComboBox* cb = dynamic_cast<QComboBox*>(editor);

	const AppSignal* s = m_provider->getLoadedSignalByIndex(row, true);

	TEST_PTR_RETURN(s);

	bool isExpert = theSettings.isExpertMode();

	if (m_propManager->isEnumProperty(col))
	{
		if (cb == nullptr)
		{
			assert(false);
			return;
		}

		int curIndex = cb->findText(m_propManager->value(s, col, isExpert).toString());
		cb->setCurrentIndex(curIndex);
		return;
	}

	QMetaType::Type type = m_propManager->type(col);

	if (type == QMetaType::Bool)
	{
		if (cb == nullptr)
		{
			assert(false);
			return;
		}

		cb->setCurrentIndex(cb->findData(m_propManager->value(s, col, isExpert).toBool()));
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
	case QMetaType::Float:
	case QMetaType::Double:
	case QMetaType::QString:
	case QMetaType::Int:
	case QMetaType::UInt:
		le->setText(m_propManager->value(s, col, isExpert).toString());
		break;
	default:
		if (type == qMetaTypeId<TuningValue>())
		{
			le->setText(m_propManager->value(s, col, isExpert).toString());
		}
		else
		{
			assert(false);
			return;
		}
	}
}

void SignalsTablePropEditor::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& proxyIndex) const
{
	Q_ASSERT(proxyIndex.model() == m_proxyModel);

	TEST_PTR_RETURN(m_provider);
	TEST_PTR_RETURN(m_propManager);

	Q_UNUSED(model);

	int col = proxyIndex.column();
	int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();

	AppSignal* ls = m_provider->getLoadedSignalByIndex(sourceRow, true);

	TEST_PTR_RETURN(ls);

	AppSignal s(*ls);

	bool isExpert = theSettings.isExpertMode();

	// Set Enum property value
	//
	if (m_propManager->isEnumProperty(col) == true)
	{
		QComboBox* cb = dynamic_cast<QComboBox*>(editor);

		TEST_PTR_RETURN(cb);

		QVariant data = cb->currentData();

		if (data.isValid())
		{
			m_valueChanged = m_propManager->setValue(&s, col, data, isExpert);
		}
	}
	else
	{
		// Set Bool property value
		//
		QMetaType::Type type = m_propManager->type(col);

		if (type == QMetaType::Bool)
		{
			QComboBox* cb = dynamic_cast<QComboBox*>(editor);

			TEST_PTR_RETURN(cb);

			m_valueChanged = m_propManager->setValue(&s, col, cb->currentData(), isExpert);
		}
		else
		{
			QLineEdit* le = dynamic_cast<QLineEdit*>(editor);

			TEST_PTR_RETURN(le);

			QString value = le->text();

			switch (type)
			{
			case QMetaType::QString:
			{
				QString name = m_propManager->name(col);

				if (name == AppSignalPropNames::APP_SIGNAL_ID &&
						(value.isEmpty() || value[0] != '#'))
				{
					value = ('#' + value).trimmed();
				}
				if ((name == AppSignalPropNames::CUSTOM_APP_SIGNAL_ID ||
					 name == AppSignalPropNames::BUS_TYPE_ID ||
					 name == AppSignalPropNames::EQUIPMENT_ID) &&
						(value.isEmpty() == false && value[0] == '#'))
				{
					value = value.mid(1).trimmed();
				}
				if (name == AppSignalPropNames::CAPTION)
				{
					value = value.trimmed();
				}

				m_valueChanged = m_propManager->setValue(&s, col, value, isExpert);

				break;
			}

			case QMetaType::Double:
			case QMetaType::Float:
				{
					bool ok = false;

					double dbl = m_defaultLocale.toDouble(value, &ok);

					m_valueChanged = m_propManager->setValue(&s, col, dbl, isExpert);
				}
				break;

			case QMetaType::Int:
				m_valueChanged = m_propManager->setValue(&s, col, value.toInt(), isExpert);
				break;

			case QMetaType::UInt:
				m_valueChanged = m_propManager->setValue(&s, col, value.toUInt(), isExpert);
				break;

			default:
				if (type == qMetaTypeId<TuningValue>())
				{
					m_valueChanged = m_propManager->setValue(&s, col, value, isExpert);
				}
				else
				{
					Q_ASSERT(false);
				}
			}
		}
	}

	if (m_valueChanged == true)
	{
		m_provider->saveSignal(&s, nullptr);
	}
}

void SignalsTablePropEditor::onCloseEditorEvent(QWidget*, QAbstractItemDelegate::EndEditHint hint)
{
	Q_UNUSED(hint);

	if (m_editingSignalId == AppSignalSet::BAD_ID)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_signalCheckedOut == true && m_valueChanged == false)
	{
		m_provider->undoSignal(m_editingSignalId);
	}
}

bool SignalsTablePropEditor::editorEvent(QEvent* event, QAbstractItemModel* model,
								  const QStyleOptionViewItem& option, const QModelIndex& index)
{
	Q_UNUSED(model);
	Q_UNUSED(option);
	Q_UNUSED(index);

	if (event->type() == QEvent::MouseButtonDblClick)
	{
		emit itemDoubleClicked();
		return true;
	}
	return false;
}

// -------------------------------------------------------------------------------------------------------
//
// CheckedOutSignalsModel class implementation
//
// -------------------------------------------------------------------------------------------------------

CheckedOutSignalsModel::CheckedOutSignalsModel(SignalsModel* sourceModel, QTableView* view, QObject* parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel),
	m_view(view)
{
	setSourceModel(sourceModel);
	m_checkStates.resize(rowCount());
}

QVariant CheckedOutSignalsModel::data(const QModelIndex& proxyIndex, int role) const
{
	if (proxyIndex.column() == 0 && role == Qt::CheckStateRole)
	{
		return m_checkStates[proxyIndex.row()];
	}

	return QSortFilterProxyModel::data(proxyIndex, role);
}

bool CheckedOutSignalsModel::setData(const QModelIndex& proxyIndex, const QVariant& value, int role)
{
	if (proxyIndex.column() == 0 && role == Qt::CheckStateRole)
	{
		QModelIndexList selIndexes = m_view->selectionModel()->selectedRows(0);

		for (const QModelIndex& selIndex : selIndexes)
		{
			setCheckState(selIndex.row(), Qt::CheckState(value.toInt()), AppSignalSet::BAD_INDEX);
		}

		return true;
	}

	return QSortFilterProxyModel::setData(proxyIndex, value, role);
}

Qt::ItemFlags CheckedOutSignalsModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = QSortFilterProxyModel::flags(index);

	flags &= ~Qt::ItemIsEditable;

	if (index.column() == 0)
	{
		flags |= Qt::ItemIsUserCheckable;
	}

	return flags;
}

bool CheckedOutSignalsModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const
{
	return AppSignalSetProvider::getInstance()->isCheckinableSignalForMe(sourceRow);
}

void CheckedOutSignalsModel::initCheckStates(const QModelIndexList& srcIndexes)
{
	for (const QModelIndex& srcIndex : srcIndexes)
	{
		QModelIndex proxyIndex = mapFromSource(srcIndex);

		if (proxyIndex.isValid())
		{
			// here srcIndex.row() == signalIndex in AppSignalSet
			//
			setCheckState(proxyIndex.row(), Qt::Checked, srcIndex.row());
		}
	}
}

void CheckedOutSignalsModel::setAllCheckStates(bool state)
{
	for (int i = 0; i < m_checkStates.count(); i++)
	{
		m_checkStates[i] = state ? Qt::Checked : Qt::Unchecked;
	}

	emit dataChanged(index(0, 0), index(static_cast<int>(m_checkStates.count()) - 1, 0), QVector<int>{Qt::CheckStateRole});
}

void CheckedOutSignalsModel::setCheckState(int proxyRow, Qt::CheckState state, int signalIndex)
{
	std::vector<int> sourceRows;

	if (signalIndex == AppSignalSet::BAD_INDEX)
	{
		signalIndex = mapToSource(index(proxyRow, 0)).row();
	}

	AppSignalSetProvider::getInstance()->getSameChannelSignalsIndexes(signalIndex, &sourceRows);

	for(int sourceRow : sourceRows)
	{
		QModelIndex changedIndex = mapFromSource(m_sourceModel->index(sourceRow, 0));

		if (!changedIndex.isValid())
		{
			continue;
		}

		m_checkStates[changedIndex.row()] = state;

		emit dataChanged(changedIndex, changedIndex, QVector<int>{Qt::CheckStateRole});
	}
}
