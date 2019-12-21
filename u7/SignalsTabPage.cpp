#include "SignalsTabPage.h"
#include "Settings.h"
#include "SignalPropertiesDialog.h"
#include "BusStorage.h"
#include "../lib/DbController.h"
#include "../lib/SignalProperties.h"
#include "../lib/WidgetUtils.h"
#include "../lib/WUtils.h"
#include "../lib/StandardColors.h"
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

SignalPropertyManager::SignalPropertyManager()
{
	for (size_t i = 0; i < m_propertyDescription.size(); i++)
	{
		m_propertyName2IndexMap[m_propertyDescription[i].name] = static_cast<int>(i);
	}
}

int SignalPropertyManager::count() const
{
	return static_cast<int>(m_propertyDescription.size());
}

int SignalPropertyManager::index(const QString& name)
{
	for (size_t i = 0; i < m_propertyDescription.size(); i++)
	{
		if (m_propertyDescription[i].name == name)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

QString SignalPropertyManager::caption(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].caption;
}

QString SignalPropertyManager::name(int propertyIndex)
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].name;
}

QVariant SignalPropertyManager::value(const Signal* signal, int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QVariant();
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);
	if (isHidden(behaviour))
	{
		return QVariant();
	}

	const SignalPropertyDescription& property = m_propertyDescription[static_cast<size_t>(propertyIndex)];
	if (property.enumValues.size() == 0)
	{
		return property.valueGetter(signal);
	}
	else
	{
		int value = property.valueGetter(signal).toInt();
		for (const auto& enumValue : property.enumValues)
		{
			if (value == enumValue.first)
			{
				return enumValue.second;
			}
		}
		return QString("Unknown value (%1)").arg(value);
	}
}

const std::vector<std::pair<int, QString> > SignalPropertyManager::values(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return {};
	}

	return m_propertyDescription[static_cast<size_t>(propertyIndex)].enumValues;
}

void SignalPropertyManager::setValue(Signal* signal, int propertyIndex, const QVariant& value)
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);
	if (isHidden(behaviour) || isReadOnly(behaviour))
	{
		assert(false);
	}

	m_propertyDescription[static_cast<size_t>(propertyIndex)].valueSetter(signal, value);
}

QVariant::Type SignalPropertyManager::type(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QVariant::Invalid;
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].type;
}

E::PropertyBehaviourType SignalPropertyManager::getBehaviour(const Signal& signal, const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return defaultBehaviour;
	}

	return getBehaviour(signal.signalType(), signal.inOutType(), propertyIndex);
}

E::PropertyBehaviourType SignalPropertyManager::getBehaviour(E::SignalType type, E::SignalInOutType directionType, const int propertyIndex) const
{
	int behaviourIndex = m_propertyIndex2BehaviourIndexMap.value(propertyIndex, -1);
	if (behaviourIndex == -1)
	{
		return defaultBehaviour;
	}

	auto typeEnum = QMetaEnum::fromType<E::SignalType>();
	auto inOutTypeEnum = QMetaEnum::fromType<E::SignalInOutType>();
	for (int i = 0; i < SIGNAL_TYPE_COUNT; i++)
	{
		if (type != typeEnum.value(i))
		{
			continue;
		}
		for (int j = 0; j < IN_OUT_TYPE_COUNT; j++)
		{
			if (directionType == static_cast<E::SignalInOutType>(inOutTypeEnum.value(j)))
			{
				return m_propertyBehaviorDescription[static_cast<size_t>(behaviourIndex)].behaviourType[static_cast<size_t>(i * typeEnum.keyCount() + j)];
			}
		}
	}

	return defaultBehaviour;
}

bool SignalPropertyManager::dependsOnPrecision(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return false;
	}

	int behaviourIndex = m_propertyIndex2BehaviourIndexMap.value(propertyIndex, -1);
	if (behaviourIndex == -1)
	{
		return false;
	}

	return m_propertyBehaviorDescription[static_cast<size_t>(behaviourIndex)].dependsOnPrecision;
}

bool SignalPropertyManager::isHiddenFor(E::SignalType type, const int propertyIndex) const
{
	auto inOutTypeEnum = QMetaEnum::fromType<E::SignalInOutType>();
	for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
	{
		E::SignalInOutType directionType = static_cast<E::SignalInOutType>(inOutTypeEnum.value(i));
		E::PropertyBehaviourType behaviour = getBehaviour(type, directionType, propertyIndex);
		if (isHidden(behaviour) == false)
		{
			return false;
		}
	}
	return true;
}

void SignalPropertyManager::detectNewProperties(const Signal &signal)
{
	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(signal.specPropStruct());

	if (result.first == false)
	{
		assert(false);
		return;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject.properties();

	SignalSpecPropValues spValues;

	for(std::shared_ptr<Property> specificProperty : specificProperties)
	{
		int index = m_propertyName2IndexMap.value(specificProperty->caption(), -1);
		if (index != -1)
		{
			continue;
		}

		SignalPropertyDescription newProperty;

		QString propertyName = specificProperty->caption();
		bool propertyIsEnum = specificProperty->isEnum();
		QVariant::Type type = specificProperty->value().type();

		newProperty.name = propertyName;
		newProperty.caption = SignalProperties::generateCaption(propertyName);
		newProperty.type = type;
		if (propertyIsEnum)
		{
			newProperty.enumValues = specificProperty->enumValues();
		}

		newProperty.valueGetter = [propertyIsEnum, propertyName, type](const Signal* s)
		{
			QVariant qv;

			bool isEnum = propertyIsEnum;
			QString name = propertyName;

			bool result = s->getSpecPropValue(name, &qv, &isEnum);

			if (result == false)
			{
				return QVariant();
			}

			assert(qv.type() == type);

			return qv;
		};

		newProperty.valueSetter = [propertyIsEnum, propertyName](Signal* s, const QVariant& v)
		{
			bool isEnum = propertyIsEnum;
			QString name = propertyName;

			bool result = s->setSpecPropValue(name, v, isEnum);

			assert(result == true);

			Q_UNUSED(result);
		};

		switch (newProperty.type)
		{
		case QVariant::String:
		case QVariant::Double:
		case QVariant::Int:
		case QVariant::UInt:
		case QVariant::Bool:
			break;
		default:
			assert(false);
			continue;
		}

		addNewProperty(newProperty);
	}
}

void SignalPropertyManager::loadNotSpecificProperties(const SignalProperties& signalProperties)
{
	std::vector<SignalPropertyDescription> propetyDescription = signalProperties.getProperties();

	for (SignalPropertyDescription& property : propetyDescription)
	{
		if (index(property.name) == -1)
		{
			auto propertyPtr = signalProperties.propertyByCaption(property.name);
			if (propertyPtr != nullptr && propertyPtr->category().isEmpty() == false)
			{
				addNewProperty(property);
			}
		}
	}
}

void SignalPropertyManager::reloadPropertyBehaviour(DbController* dbController, QWidget* parent)
{
	if (dbController == nullptr)
	{
		return;
	}

	DbFileInfo mcInfo = dbController->systemFileInfo(dbController->etcFileId());

	if (mcInfo.isNull() == true)
	{
		QMessageBox::critical(parent, "Error", QString("File \"%1\" is not found!").arg(Db::File::EtcFileName));
		return;
	}

	DbFileInfo propertyBehaviorFile;
	dbController->getFileInfo(mcInfo.fileId(), QString(Db::File::SignalPropertyBehaviorFileName), &propertyBehaviorFile, parent);

	if (propertyBehaviorFile.isNull() == true)
	{
		QMessageBox::critical(parent, "Error", QString("File \"%1\" is not found!").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	std::shared_ptr<DbFile> file;
	bool result = dbController->getLatestVersion(propertyBehaviorFile, &file, parent);
	if (result == false)
	{
		QMessageBox::critical(parent, "Error", QString("Could not load file \"%1\"").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}
	QString fileText = file->data();
	QStringList rows = fileText.split("\n", QString::SkipEmptyParts);

	if (rows.isEmpty() == true)
	{
		QMessageBox::critical(parent, "Error", QString("File \"%1\" is empty").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	QStringList fieldNameList = rows[0].split(';', QString::KeepEmptyParts);
	trimm(fieldNameList);

	rows.removeFirst();

	QString uncorrectFileMessage =  QString("Uncorrect format of file \"%1\"").arg(Db::File::SignalPropertyBehaviorFileName);

	int nameIndex = fieldNameList.indexOf("PropertyName");
	if (nameIndex < 0)
	{
		QMessageBox::critical(parent, "Error", uncorrectFileMessage + ": PropertyName column not found");
		return;
	}

	int precisionIndex = fieldNameList.indexOf("DependsOnPrecision");
	if (precisionIndex < 0)
	{
		QMessageBox::critical(parent, "Error", uncorrectFileMessage + ": DependosOnPrecision column not found");
		return;
	}

	std::vector<int> typeIndexes(static_cast<size_t>(TOTAL_SIGNAL_TYPE_COUNT), -1);
	for (int i = 0; i < SIGNAL_TYPE_COUNT; i++)
	{
		for (int j = 0; j < IN_OUT_TYPE_COUNT; j++)
		{
			typeIndexes[static_cast<size_t>(i * SIGNAL_TYPE_COUNT + j)] = fieldNameList.indexOf(typeName(i, j));
		}
	}

	m_propertyBehaviorDescription.clear();
	m_propertyIndex2BehaviourIndexMap.clear();

	for (QString row : rows)
	{
		row = row.trimmed();

		if (row.isEmpty() == true)
		{
			continue;
		}

		QStringList fields = row.split(';', QString::KeepEmptyParts);
		trimm(fields);

		if (nameIndex > fields.size())
		{
			continue;
		}

		PropertyBehaviourDescription behaviour;
		behaviour.name = fields[nameIndex];

		if (precisionIndex < fields.size())
		{
			behaviour.dependsOnPrecision = fields[precisionIndex].toLower() == "true";
		}

		for (size_t i = 0; i < static_cast<size_t>(TOTAL_SIGNAL_TYPE_COUNT); i++)
		{
			if (typeIndexes[i] < 0 && typeIndexes[i] >= fields.size())
			{
				continue;
			}

			bool ok = false;
			E::PropertyBehaviourType behaviourType = E::stringToValue<E::PropertyBehaviourType>(fields[typeIndexes[i]], &ok);
			if (ok == true)
			{
				behaviour.behaviourType[i] = behaviourType;
			}
		}

		m_propertyBehaviorDescription.push_back(behaviour);

		int propertyIndex = m_propertyName2IndexMap.value(behaviour.name, -1);
		if (propertyIndex != -1)
		{
			int behaviourIndex = static_cast<int>(m_propertyBehaviorDescription.size()) - 1;
			assert(m_propertyDescription[propertyIndex].name == m_propertyBehaviorDescription[behaviourIndex].name);

			m_propertyIndex2BehaviourIndexMap[propertyIndex] = behaviourIndex;
		}
	}
}

bool SignalPropertyManager::isNotCorrect(int propertyIndex) const
{
	if (propertyIndex < 0 || propertyIndex >= static_cast<int>(m_propertyDescription.size()))
	{
		return true;
	}
	return false;
}

QString SignalPropertyManager::typeName(E::SignalType type, E::SignalInOutType inOutType)
{
	return E::valueToString<E::SignalType>(type) + E::valueToString<E::SignalInOutType>(inOutType);
}

QString SignalPropertyManager::typeName(int typeIndex, int inOutTypeIndex)
{
	return typeName(IntToEnum<E::SignalType>(QMetaEnum::fromType<E::SignalType>().value(typeIndex)),
					IntToEnum<E::SignalInOutType>(QMetaEnum::fromType<E::SignalInOutType>().value(inOutTypeIndex)));
}

TuningValue SignalPropertyManager::variant2TuningValue(const QVariant& variant, TuningValueType type)
{
	TuningValue value;
	value.setType(type);

	bool ok = false;
	value.fromString(variant.toString(), &ok);
	assert(ok == true);

	return value;
}

bool SignalPropertyManager::isHidden(E::PropertyBehaviourType behaviour) const
{
	bool hidden = behaviour == E::PropertyBehaviourType::Hide;
	hidden |= behaviour == E::PropertyBehaviourType::Expert && theSettings.isExpertMode() == false;
	return hidden;
}

bool SignalPropertyManager::isReadOnly(E::PropertyBehaviourType behaviour) const
{
	bool readOnly = behaviour != E::PropertyBehaviourType::Write;
	readOnly |= behaviour == E::PropertyBehaviourType::Expert && theSettings.isExpertMode() == false;
	return readOnly;
}

void SignalPropertyManager::addNewProperty(const SignalPropertyDescription& newProperty)
{
	if (m_propertyName2IndexMap.contains(newProperty.name))
	{
		assert(false);
		return;
	}
	int propertyIndex = static_cast<int>(m_propertyDescription.size());
	emit beginAddProperty(propertyIndex);
	m_propertyDescription.push_back(newProperty);
	m_propertyName2IndexMap.insert(newProperty.name, propertyIndex);
	emit endAddProperty();

	for (size_t i = 0; i < m_propertyBehaviorDescription.size(); i++)
	{
		if (newProperty.name == m_propertyBehaviorDescription[i].name)
		{
			m_propertyIndex2BehaviourIndexMap[propertyIndex] = static_cast<int>(i);
			break;
		}
	}
}

void SignalPropertyManager::trimm(QStringList& stringList)
{
	for (QString& string : stringList)
	{
		string = string.trimmed();
	}
}


SignalsModel* SignalsModel::m_instance = nullptr;


SignalsDelegate::SignalsDelegate(SignalSet& signalSet, SignalsModel* model, SignalsProxyModel* proxyModel, QObject *parent) :
	QStyledItemDelegate(parent),
	m_signalSet(signalSet),
	m_model(model),
	m_proxyModel(proxyModel)
{
	connect(this, &QAbstractItemDelegate::closeEditor, this, &SignalsDelegate::onCloseEditorEvent);
}

QWidget *SignalsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();

	m_model->loadSignal(m_model->key(row), false);	// get current checkedOut state

	Signal& s = m_signalSet[row];

	SignalPropertyManager& manager = m_model->signalPropertyManager();
	manager.reloadPropertyBehaviour(m_model->dbController(), parent);

	E::PropertyBehaviourType behaviour = manager.getBehaviour(s, col);
	if (manager.isHidden(behaviour) || manager.isReadOnly(behaviour))
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

	if (!m_model->checkoutSignal(row))
	{
		return nullptr;
	}

	m_model->loadSignal(m_model->key(row), false);	// update new checkedOut state on view

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
			QRegExp rx4ID("^[#]?[A-Za-z\\d_]*$");
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
	if (row >= m_signalSet.count())
	{
		return;
	}

	QComboBox* cb = dynamic_cast<QComboBox*>(editor);

	Signal& s = m_signalSet[row];

	SignalPropertyManager& manager = m_model->signalPropertyManager();

	const auto values = manager.values(col);

	if (values.size() > 0)
	{
		if (cb == nullptr)
		{
			assert(false);
			return;
		}

		cb->setCurrentIndex(cb->findData(manager.value(&s, col)));
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

		cb->setCurrentIndex(manager.value(&s, col).toBool());
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
		le->setText(manager.value(&s, col).toString());
		break;
	default:
		if (type == qMetaTypeId<TuningValue>())
		{
			le->setText(manager.value(&s, col).toString());
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
	if (row >= m_signalSet.count())
	{
		return;
	}

	QComboBox* cb = dynamic_cast<QComboBox*>(editor);

	Signal& s = m_signalSet[row];

	SignalPropertyManager& manager = m_model->signalPropertyManager();

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
			manager.setValue(&s, col, data);
			m_model->saveSignal(s);

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

		manager.setValue(&s, col, cb->currentIndex());
		m_model->saveSignal(s);
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
		manager.setValue(&s, col, value);
		break;
	}
	case QVariant::Double:
		manager.setValue(&s, col, value.toDouble());
		break;
	case QVariant::Int:
		manager.setValue(&s, col, value.toInt());
		break;
	case QVariant::UInt:
		manager.setValue(&s, col, value.toUInt());
		break;
	default:
		if (type == qMetaTypeId<TuningValue>())
		{
			manager.setValue(&s, col, value);
		}
		else
		{
			assert(false);
			return;
		}
	}

	m_model->saveSignal(s);
	signalIdForUndoOnCancelEditing = -1;
}

void SignalsDelegate::onCloseEditorEvent(QWidget*, QAbstractItemDelegate::EndEditHint hint)
{
	if (hint == QAbstractItemDelegate::RevertModelCache && signalIdForUndoOnCancelEditing != -1)
	{
		m_model->undoSignal(signalIdForUndoOnCancelEditing);
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


SignalsModel::SignalsModel(DbController* dbController, SignalsTabPage* parent) :
	QAbstractTableModel(parent),
	m_parentWindow(parent),
	m_dbController(dbController)
{
	m_instance = this;
	connect(&m_propertyManager, &SignalPropertyManager::beginAddProperty, this, &SignalsModel::beginAddProperty, Qt::DirectConnection);
	connect(&m_propertyManager, &SignalPropertyManager::endAddProperty, this, &SignalsModel::endAddProperty, Qt::DirectConnection);
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
	return m_signalSet.count();
}

int SignalsModel::columnCount(const QModelIndex& parentIndex) const
{
	if (parentIndex.isValid())
	{
		return 0;
	}
	return m_propertyManager.count() + 1;	// Usual properties and "Last change user"
}

QString SignalsModel::getUserStr(int userID) const
{
	if (m_usernameMap.contains(userID))
	{
		return m_usernameMap[userID];
	}
	else
	{
		return tr("Unknown user ID = %1").arg(userID);
	}
}

void SignalsModel::changeCheckedoutSignalActionsVisibility()
{
	for (int i = 0; i < m_signalSet.count(); i++)
	{
		const Signal& signal = m_signalSet[i];
		if (signal.checkedOut() && (signal.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator()))
		{
			emit setCheckedoutSignalActionsVisibility(true);
			return;
		}
	}
	emit setCheckedoutSignalActionsVisibility(false);
}

bool SignalsModel::checkoutSignal(int index)
{
	Signal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	QVector<int> signalsIDs;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
	}
	else
	{
		signalsIDs << m_signalSet.key(index);
	}
	QVector<ObjectState> objectStates;
	dbController()->checkoutSignals(&signalsIDs, &objectStates, parentWindow());
	if (objectStates.count() == 0)
	{
		return false;
	}
	showErrors(objectStates);
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			return false;
		}
	}
	for (int id : signalsIDs)
	{
		loadSignal(id, true);
	}
	emit setCheckedoutSignalActionsVisibility(true);
	return true;
}

bool SignalsModel::checkoutSignal(int index, QString& message)
{
	Signal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	QVector<int> signalsIDs;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
	}
	else
	{
		signalsIDs << m_signalSet.key(index);
	}
	QVector<ObjectState> objectStates;
	dbController()->checkoutSignals(&signalsIDs, &objectStates, parentWindow());
	if (objectStates.count() == 0)
	{
		return false;
	}
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode != ERR_SIGNAL_OK)
		{
			message += errorMessage(objectState) + "\n";
		}
	}
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			return false;
		}
	}
	for (int id : signalsIDs)
	{
		loadSignal(id, true);
	}
	emit setCheckedoutSignalActionsVisibility(true);
	return true;
}

bool SignalsModel::undoSignal(int id)
{
	const Signal& s = m_signalSet[m_signalSet.keyIndex(id)];
	if (!s.checkedOut())
	{
		return false;
	}

	QVector<int> signalsIDs;
	if (s.signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(s.signalGroupID());
	}
	else
	{
		signalsIDs << id;
	}
	QVector<ObjectState> states;

	for (int signalId : signalsIDs)
	{
		ObjectState state;
		dbController()->undoSignalChanges(signalId, &state, parentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}

	if (!states.isEmpty())
	{
		showErrors(states);
	}

	for (int signalId : signalsIDs)
	{
		loadSignal(signalId, true);
	}

	return true;
}

QString SignalsModel::errorMessage(const ObjectState& state) const
{
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT: return tr("Signal %1 is not checked out").arg(state.id);
		case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER: return tr("Signal %1 is checked out by \"%2\"").arg(state.id).arg(m_usernameMap[state.userId]);
		case ERR_SIGNAL_DELETED: return tr("Signal %1 was deleted already").arg(state.id);
		case ERR_SIGNAL_NOT_FOUND: return tr("Signal %1 not found").arg(state.id);
		case ERR_SIGNAL_EXISTS: return "";				// error message is displayed by PGSql driver
		default:
			return tr("Unknown error %1").arg(state.errCode);
	}
}

void SignalsModel::showError(const ObjectState& state) const
{
	if (state.errCode != ERR_SIGNAL_OK)
	{
		QString message = errorMessage(state);
		if (!message.isEmpty())
		{
			QMessageBox::critical(m_parentWindow, tr("Error"), errorMessage(state));
		}
	}
}

void SignalsModel::showErrors(const QVector<ObjectState>& states) const
{
	QString message;

	foreach (const ObjectState& state, states)
	{
		if (state.errCode != ERR_SIGNAL_OK)
		{
			if (message.isEmpty() == false)
			{
				message += "\n";
			}

			message += errorMessage(state);
		}
	}

	if (!message.isEmpty())
	{
		QMessageBox::critical(m_parentWindow, tr("Error"), message);
	}
}


QVariant SignalsModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	if (row == m_signalSet.count() || m_signalSet[row].isLoaded() == false)
	{
		return QVariant();
	}

	const Signal& signal = m_signalSet[row];

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

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (col >= m_propertyManager.count())
		{
			return signal.checkedOut() ? getUserStr(signal.userID()) : "";
		}

		QVariant value = m_propertyManager.value(&signal, col);

		if (value.isValid() && signal.isAnalog() && m_propertyManager.dependsOnPrecision(col))
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
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (orientation == Qt::Horizontal)
		{
			if (section == m_propertyManager.count())
			{
				return "Last change user";
			}
			return m_propertyManager.caption(section);
		}
		if (orientation == Qt::Vertical)
		{
			if (section < m_signalSet.count())
			{
				return m_signalSet.key(section);
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

		assert(row < m_signalSet.count());

		Signal& s = m_signalSet[row];

		m_propertyManager.setValue(&s, index.column(), value);

		// This should be done by SignalsDelegate::setModelData
		saveSignal(s);

		loadSignal(s.ID());
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

	if (column >= m_propertyManager.count())
	{
		return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
	}

	assert(row < m_signalSet.count());

	const Signal& s = m_signalSet[row];

	if (m_propertyManager.getBehaviour(s, index.column()) == E::PropertyBehaviourType::Write)
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	else
	{
		return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
	}
}

void SignalsModel::loadSignals()
{
	emit signalsLoadingFinished();

	bool signalsCleared = false;

	if (m_signalSet.count() != 0)
	{
		signalsCleared = true;
		emit aboutToClearSignals();
	}
	clearSignals();

	loadUsers();

	SignalSet temporarySignalSet;
	if (!dbController()->getSignals(&temporarySignalSet, false, m_parentWindow))
	{
		QMessageBox::warning(m_parentWindow, tr("Warning"), tr("Could not load signals"));
	}

	for (int i = 0; i < temporarySignalSet.count(); i++)
	{
		detectNewProperties(temporarySignalSet[i]);
	}

	if (temporarySignalSet.count() > 0)
	{
		beginInsertRows(QModelIndex(), 0, temporarySignalSet.count() - 1);
		std::swap(m_signalSet, temporarySignalSet);
		endInsertRows();

		if (signalsCleared)
		{
			emit signalsRestored();
		}
	}

	changeCheckedoutSignalActionsVisibility();
}

void SignalsModel::loadSignal(int signalId, bool updateView)
{
	int row = keyIndex(signalId);
	if (row == -1)
	{
		return;
	}
	dbController()->getLatestSignal(signalId, &m_signalSet[row], parentWindow());

	detectNewProperties(m_signalSet[row]);

	if (updateView)
	{
		emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
	}
}

void SignalsModel::loadSignalSet(QVector<int> keys, bool updateView)
{
	for (int i = 0; i < keys.count(); i++)
	{
		int row = keyIndex(keys[i]);
		dbController()->getLatestSignal(keys[i], &m_signalSet[row], parentWindow());

		detectNewProperties(m_signalSet[row]);

		if (updateView)
		{
			emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
		}
	}
}

void SignalsModel::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		beginRemoveRows(QModelIndex(), 0, m_signalSet.count() - 1);
		m_signalSet.clear();
		endRemoveRows();
	}
}

Signal*SignalsModel::getSignalByStrID(const QString signalStrID)
{
	if (m_signalSet.ID2IndexMapIsEmpty())
	{
		m_signalSet.buildID2IndexMap();
	}
	return m_signalSet.getSignal(signalStrID);
}

QVector<int> SignalsModel::getSameChannelSignals(int row)
{
	QVector<int> sameChannelSignalRows;
	if (m_signalSet[row].signalGroupID() != 0)
	{
		QVector<int> sameChannelSignalIDs = m_signalSet.getChannelSignalsID(m_signalSet[row].signalGroupID());
		foreach (const int id, sameChannelSignalIDs)
		{
			sameChannelSignalRows.append(m_signalSet.keyIndex(id));
		}
	}
	else
	{
		sameChannelSignalRows.append(row);
	}
	return sameChannelSignalRows;
}

bool SignalsModel::isEditableSignal(int row)
{
	Signal& s = m_signalSet[row];
	if (s.checkedOut() && (s.userID() != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator()))
	{
		return false;
	}
	return true;
}

void SignalsModel::addSignal()
{
	QDialog signalTypeDialog(m_parentWindow, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
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
	signalTypeDialog.setFixedSize(600, 200);

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
		QString newId = QString(E::valueToString<E::SignalType>(signal.signalType()).toUpper() + "_%1").arg(signalCounter, 3, 10, QLatin1Char('0'));
		signal.setAppSignalID('#' + newId);
		signal.setCustomAppSignalID(newId);
		signal.setCaption(newId);
	}

	SignalPropertiesDialog dlg(dbController(), QVector<Signal*>() << &signal, false, false, m_parentWindow);

	trimSignalTextFields(signal);

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

			if (dbController()->addSignal(E::SignalType(signalTypeCombo->currentIndex()), &signalVector, m_parentWindow))
			{
				for (int i = 0; i < signalVector.count(); i++)
				{
					resultSignalVector.append(signalVector[i]);
					m_propertyManager.detectNewProperties(signalVector[i]);
				}
			}
		}

		if (!resultSignalVector.isEmpty())
		{
			int firstInsertedSignalId = resultSignalVector[0].ID();

			beginInsertRows(QModelIndex(), m_signalSet.count(), m_signalSet.count() + resultSignalVector.count() - 1);

			for (int i = 0; i < resultSignalVector.count(); i++)
			{
				const Signal& s = resultSignalVector[i];

				m_signalSet.replaceOrAppendIfNotExists(s.ID(), s);
			}
			endInsertRows();
			emit dataChanged(createIndex(m_signalSet.count() - resultSignalVector.count(), 0), createIndex(m_signalSet.count() - 1, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
			emit signalsRestored(firstInsertedSignalId);
		}
	}

	emit setCheckedoutSignalActionsVisibility(true);
}

void SignalsModel::showError(QString message)
{
	if (!message.isEmpty())
	{
		QMessageBox::critical(m_parentWindow, tr("Error"), message);
	}
}

void SignalsModel::beginAddProperty(int propertyIndex)
{
	beginInsertColumns(QModelIndex(), propertyIndex, propertyIndex);
}

void SignalsModel::endAddProperty()
{
	endInsertColumns();
}

void SignalsModel::detectNewProperties(const Signal& signal)
{
	int oldColumnCount = columnCount();

	m_propertyManager.detectNewProperties(signal);

	if (oldColumnCount < columnCount())
	{
		emit updateColumnList();
	}
}

void SignalsModel::loadNotSpecificProperties(Signal& signal)
{
	int oldColumnCount = columnCount();

	SignalProperties signalProperties(signal, true);

	m_propertyManager.loadNotSpecificProperties(signalProperties);

	if (oldColumnCount < columnCount())
	{
		emit updateColumnList();
	}
}

bool SignalsModel::editSignals(QVector<int> ids)
{
	loadSignalSet(ids, false);

	bool readOnly = false;
	QVector<Signal*> signalVector;

	for (int i = 0; i < ids.count(); i++)
	{
		Signal& signal = m_signalSet[m_signalSet.keyIndex(ids[i])];
		if (signal.checkedOut() && signal.userID() != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			readOnly = true;
		}

		signalVector.append(&signal);
	}

	SignalPropertiesDialog dlg(dbController(), signalVector, readOnly, true, m_parentWindow);

	if (dlg.isValid() == false)
	{
		return false;
	}

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<ObjectState> states;
		for (int i = 0; i < ids.count(); i++)
		{
			if (dlg.isEditedSignal(ids[i]))
			{
				ObjectState state;
				trimSignalTextFields(*signalVector[i]);
				dbController()->setSignalWorkcopy(signalVector[i], &state, parentWindow());
				states.append(state);
			}
		}
		showErrors(states);

		loadSignalSet(ids);
		changeCheckedoutSignalActionsVisibility();
		return true;
	}

	if (dlg.hasEditedSignals())
	{
		loadSignalSet(ids);	//Signal could be checked out but not changed
		changeCheckedoutSignalActionsVisibility();
	}
	return false;
}

void SignalsModel::trimSignalTextFields(Signal& signal)
{
	signal.setAppSignalID(signal.appSignalID().trimmed());
	signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());
	signal.setEquipmentID(signal.equipmentID().trimmed());
	signal.setBusTypeID(signal.busTypeID().trimmed());
	signal.setCaption(signal.caption().trimmed());
	signal.setUnit(signal.unit().trimmed());
}

void SignalsModel::saveSignal(Signal& signal)
{
	ObjectState state;
	trimSignalTextFields(signal);

	dbController()->setSignalWorkcopy(&signal, &state, parentWindow());

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}

	loadSignal(signal.ID());
}

QVector<int> SignalsModel::cloneSignals(const QSet<int>& signalIDs)
{
	QVector<int> resultSignalIDs;
	m_signalSet.buildID2IndexMap();

	QSet<int> clonedSignalIDs;
	QList<int> signalIDsList = signalIDs.toList();
	qSort(signalIDsList);
	for (const int signalID : signalIDsList)
	{
		if (clonedSignalIDs.contains(signalID))
		{
			continue;
		}

		const Signal&& signal = m_signalSet.value(signalID);
		E::SignalType type = signal.signalType();
		QVector<int> groupSignalIDs;

		if (signal.signalGroupID() == 0)
		{
			groupSignalIDs.append(signal.ID());
		}
		else
		{
			groupSignalIDs = m_signalSet.getChannelSignalsID(signal);
		}
		qSort(groupSignalIDs);

		for (int groupSignalID : groupSignalIDs)
		{
			clonedSignalIDs.insert(groupSignalID);
		}

		QString suffix = "_CLONE";
		int suffixNumerator = 1;
		bool hasConflict;
		do
		{
			hasConflict = false;
			for (int groupSignalID : groupSignalIDs)
			{
				if (m_signalSet.contains(m_signalSet.value(groupSignalID).appSignalID() + suffix))
				{
					hasConflict = true;
					break;
				}
			}
			if (hasConflict)
			{
				suffixNumerator++;
				suffix = QString("_CLONE%1").arg(suffixNumerator);
			}
		}
		while (hasConflict && suffixNumerator < 1000);

		if (suffixNumerator >= 1000)
		{
			assert(false);
			return QVector<int>();
		}

		QVector<Signal> groupSignals(groupSignalIDs.count());
		for (int i = 0; i < groupSignalIDs.count(); i++)
		{
			const Signal&& groupSignal = m_signalSet.value(groupSignalIDs[i]);
			groupSignals[i] = groupSignal;
			trimSignalTextFields(groupSignals[i]);

			groupSignals[i].setAppSignalID(groupSignal.appSignalID() + suffix);
			groupSignals[i].setCustomAppSignalID(groupSignal.customAppSignalID() + suffix);
		}

		dbController()->addSignal(type, &groupSignals, m_parentWindow);

		int prevSize = resultSignalIDs.size();
		resultSignalIDs.resize(prevSize + groupSignals.count());

		for (int i = 0; i < groupSignals.count(); i++)
		{
			resultSignalIDs[prevSize + i] = groupSignals[i].ID();
		}
	}
	loadSignals();
	return resultSignalIDs;
}

void SignalsModel::deleteSignalGroups(const QSet<int>& signalGroupIDs)
{
	for (const int groupID : signalGroupIDs)
	{
		QVector<int> signalIDs = m_signalSet.getChannelSignalsID(groupID);
		for (const int signalID : signalIDs)
		{
			deleteSignal(signalID);
		}
	}
	loadSignals();
}

void SignalsModel::deleteSignals(const QSet<int>& signalIDs)
{
	foreach (const int signalID, signalIDs)
	{
		deleteSignal(signalID);
	}
	loadSignals();
}

void SignalsModel::deleteSignal(int signalID)
{
	ObjectState state;
	dbController()->deleteSignal(signalID, &state, parentWindow());
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
	changeCheckedoutSignalActionsVisibility();
}

void SignalsModel::initLazyLoadSignals()
{
	m_partialLoading = true;

	loadUsers();

	m_propertyManager.reloadPropertyBehaviour(m_dbController, m_parentWindow);

	QVector<ID_AppSignalID> signalIds;
	dbController()->getSignalsIDAppSignalID(&signalIds, m_parentWindow);

	if (signalIds.count() == 0)
	{
		Signal signal;
		loadNotSpecificProperties(signal);

		return;
	}

	beginResetModel();
	for (const ID_AppSignalID& id : signalIds)
	{
		m_signalSet.replaceOrAppendIfNotExists(id.ID, Signal(id));
	}

	if (signalIds.count() > 0)
	{
		QVector<Signal> signalsArray;
		QVector<int> signalId;

		int SignalPortionCount = signalIds.count();

		if (SignalPortionCount > 250)
		{
			SignalPortionCount = 250;
		}

		signalsArray.reserve(SignalPortionCount);
		signalId.reserve(SignalPortionCount);

		for (int i = 0; i < SignalPortionCount; i++)
		{
			signalId.append(signalIds[i].ID);
		}
		dbController()->getLatestSignalsWithoutProgress(signalId, &signalsArray, m_parentWindow);

		for (const Signal& loadedSignal : signalsArray)
		{
			m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);

			detectNewProperties(loadedSignal);
		}

		loadNotSpecificProperties(signalsArray[0]);
	}

	endResetModel();
}

void SignalsModel::finishLoadSignals()
{
	if (m_partialLoading == true)
	{
		QVector<int> signalIds;
		for (int i = 0; i < m_signalSet.count(); i++)
		{
			if (m_signalSet[i].isLoaded() == false)
			{
				signalIds.push_back(m_signalSet.key(i));
			}
		}

		if (signalIds.count() > 0)
		{
			QVector<Signal> signalsToLoad;
			signalsToLoad.reserve(signalIds.count());

			dbController()->getLatestSignals(signalIds, &signalsToLoad, m_parentWindow);

			for (const Signal& loadedSignal: signalsToLoad)
			{
				m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);

				detectNewProperties(loadedSignal);
			}
		}
	}

	m_partialLoading = false;

	beginResetModel();
	endResetModel();
	emit signalsLoadingFinished();
}

void SignalsModel::loadNextSignalsPortion()
{
	int middleRow = m_parentWindow->getMiddleVisibleRow();

	QVector<int> signalIds;
	signalIds.reserve(250);
	int low = middleRow - 1;
	int high = middleRow;

	if (middleRow == -1)
	{
		high = 0;
	}

	while ((low >= 0 || high < rowCount()) && signalIds.count() <= 248)
	{
		while (low >= 0 && m_signalSet[low].isLoaded() == true)
		{
			low--;
		}

		if (low >= 0)
		{
			signalIds.push_back(m_signalSet.key(low));
			low--;
		}

		while (high < rowCount() && m_signalSet[high].isLoaded() == true)
		{
			high++;
		}

		if (high < rowCount())
		{
			signalIds.push_back(m_signalSet.key(high));
			high++;
		}
	}

	if (signalIds.count() > 0)
	{
		QVector<Signal> signalsToLoad;
		signalsToLoad.reserve(signalIds.count());

		dbController()->getLatestSignalsWithoutProgress(signalIds, &signalsToLoad, m_parentWindow);

		for (const Signal& loadedSignal: signalsToLoad)
		{
			m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);

			detectNewProperties(loadedSignal);
		}
	}
	else
	{
		m_partialLoading = false;

		emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
		emit signalsLoadingFinished();
	}
}

void SignalsModel::loadUsers()
{
	std::vector<DbUser> list;
	m_dbController->getUserList(&list, m_parentWindow);

	m_usernameMap.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		m_usernameMap[list[i].userId()] = list[i].username();
	}
}

DbController *SignalsModel::dbController()
{
	return m_dbController;
}

const DbController *SignalsModel::dbController() const
{
	return m_dbController;
}



//
//
// SignalsTabPage
//
//

SignalsTabPage* SignalsTabPage::m_instance = nullptr;


SignalsTabPage::SignalsTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);
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
	m_signalsModel = new SignalsModel(dbcontroller, this);
	//new QAbstractItemModelTester(m_signalsModel, QAbstractItemModelTester::FailureReportingMode::Fatal, this);
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

	auto& propertyManager = m_signalsModel->signalPropertyManager();
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
	connect(m_signalsModel, &SignalsModel::updateColumnList, m_signalsColumnVisibilityController, &TableDataVisibilityController::checkNewColumns);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));
	m_signalsView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	connect(delegate, &SignalsDelegate::itemDoubleClicked, this, &SignalsTabPage::editSignal);
	connect(m_signalTypeFilterCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SignalsTabPage::changeSignalTypeFilter);

	connect(m_signalsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SignalsTabPage::changeSignalActionsVisibility);

	connect(m_signalsModel, &SignalsModel::aboutToClearSignals, this, &SignalsTabPage::saveSelection);
	connect(m_signalsModel, &SignalsModel::signalsRestored, this, &SignalsTabPage::restoreSelection);

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

	QHash<QString, int> signalIDsMap;

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
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setEnabled);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaAddFile.svg"), tr("New signal"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::addSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaClone.svg"), tr("Clone signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::cloneSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setEnabled);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaDelete.svg"), tr("Delete signal"), this);
	action->setShortcut(Qt::Key_Delete);
	connect(action, &QAction::triggered, this, &SignalsTabPage::deleteSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setEnabled);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	m_signalsView->addAction(toolBar->addSeparator());

	action = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("Check in signal(s)"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::checkIn);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setEnabled);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo changes"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::undoSignalChanges);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setEnabled);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaHistory.svg"), tr("History"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::viewSignalHistory);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setEnabled);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	m_signalsView->addAction(toolBar->addSeparator());

	action = new QAction(QIcon(":/Images/Images/SchemaRefresh.svg"), tr("Refresh"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::loadSignals);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	changeSignalActionsVisibility();
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
			selectedSignalIds.append(m_signalsModel->signal(row).appSignalID() + "\n");
		}

		QApplication::clipboard()->setText(selectedSignalIds);
	}
}

void SignalsTabPage::projectOpened()
{
	this->setEnabled(true);

	m_signalsModel->initLazyLoadSignals();

	if (m_loadSignalsTimer == nullptr && m_signalsModel->rowCount() > 0)
	{
		m_loadSignalsTimer = new QTimer(this);

		connect(m_loadSignalsTimer, &QTimer::timeout, m_signalsModel, &SignalsModel::loadNextSignalsPortion);
		connect(m_signalsProxyModel, &SignalsProxyModel::aboutToSort, m_signalsModel, &SignalsModel::finishLoadSignals, Qt::DirectConnection);
		connect(m_signalsProxyModel, &SignalsProxyModel::aboutToFilter, m_signalsModel, &SignalsModel::finishLoadSignals, Qt::DirectConnection);
		connect(m_signalsView->verticalScrollBar(), &QScrollBar::valueChanged, m_signalsModel, &SignalsModel::loadNextSignalsPortion, Qt::DirectConnection);
		connect(m_signalsModel, &SignalsModel::signalsLoadingFinished, this, &SignalsTabPage::stopLoadingSignals, Qt::DirectConnection);

		if (m_tabWidget != nullptr)
		{
			connect(m_tabWidget, &QTabWidget::currentChanged, this, &SignalsTabPage::onTabPageChanged);
		}

		m_signalsModel->loadNextSignalsPortion();
	}
}

void SignalsTabPage::projectClosed()
{
	m_signalsColumnVisibilityController->saveAllHeaderGeomery();

	this->setEnabled(false);

	m_signalsModel->clearSignals();

	resetSignalIdFilter();
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

	if (m_loadSignalsTimer == nullptr)
	{
		m_tabWidget = tabWidget;
		disconnect(tabWidget, &QTabWidget::currentChanged, this, &SignalsTabPage::onTabPageChanged);
		return;
	}

	if (tabWidget->currentWidget() == this)
	{
		m_loadSignalsTimer->start(100);
	}
	else
	{
		m_loadSignalsTimer->stop();
	}
}

void SignalsTabPage::stopLoadingSignals()
{
	// if loading already stopped
	//
	if (m_loadSignalsTimer == nullptr)
	{
		return;
	}

	disconnect(m_loadSignalsTimer, &QTimer::timeout, m_signalsModel, &SignalsModel::loadNextSignalsPortion);
	disconnect(m_signalsProxyModel, &SignalsProxyModel::aboutToSort, m_signalsModel, &SignalsModel::finishLoadSignals);
	disconnect(m_signalsProxyModel, &SignalsProxyModel::aboutToFilter, m_signalsModel, &SignalsModel::finishLoadSignals);
	disconnect(m_signalsView->verticalScrollBar(), &QScrollBar::valueChanged, m_signalsModel, &SignalsModel::loadNextSignalsPortion);
	disconnect(m_signalsModel, &SignalsModel::finishLoadSignals, this, &SignalsTabPage::stopLoadingSignals);

	m_loadSignalsTimer->deleteLater();
	m_loadSignalsTimer = nullptr;
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
	int currentId = m_signalsModel->key(currentRow);

	QVector<int> selectedSignalId;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		selectedSignalId.append(m_signalsModel->key(row));
	}

	m_signalsModel->editSignals(selectedSignalId);

	m_signalsView->scrollTo(m_signalsProxyModel->mapFromSource(m_signalsModel->index(m_signalsModel->keyIndex(currentId), currentColumn)));
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
		int id = m_signalsModel->key(row);
		clonedSignalIDs.insert(id);
	}

	m_selectedRowsSignalID = m_signalsModel->cloneSignals(clonedSignalIDs);
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
		int groupId = m_signalsModel->signal(row).signalGroupID();
		if (groupId != 0)
		{
			deletedSignalIDs.unite(m_signalsModel->getChannelSignalsID(groupId).toList().toSet());
		}
		else
		{
			deletedSignalIDs.insert(m_signalsModel->key(row));
		}
	}
	m_signalsModel->deleteSignals(deletedSignalIDs);
}

void SignalsTabPage::undoSignalChanges()
{
	m_signalsModel->finishLoadSignals();

	UndoSignalsDialog dlg(m_signalsModel, m_signalsColumnVisibilityController, this);

	const QItemSelection& proxySelection = m_signalsView->selectionModel()->selection();
	const QItemSelection& sourceSelection = m_signalsProxyModel->mapSelectionToSource(proxySelection);
	dlg.setCheckStates(sourceSelection.indexes(), true);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::checkIn()
{
	const QItemSelection& proxySelection = m_signalsView->selectionModel()->selection();
	const QItemSelection& sourceSelection = m_signalsProxyModel->mapSelectionToSource(proxySelection);

	m_signalsModel->finishLoadSignals();

	CheckinSignalsDialog dlg(m_signalsModel, m_signalsColumnVisibilityController, sourceSelection.indexes(), this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::viewSignalHistory()
{
	int row = m_signalsView->currentIndex().row();

	if (row < 0 || row >= m_signalsModel->rowCount())
	{
		return;
	}

	const Signal& signal = m_signalsModel->signal(row);
	SignalHistoryDialog dlg(dbController(), signal.appSignalID(), signal.ID(), this);

	dlg.exec();
}

void SignalsTabPage::changeSignalActionsVisibility()
{
	if (m_changingSelectionManualy)
	{
		return;
	}
	if (!m_signalsView->selectionModel()->hasSelection())
	{
		emit setSignalActionsVisibility(false);
	}
	else
	{
		QModelIndexList&& selection = m_signalsView->selectionModel()->selectedRows();
		for (int i = 0; i < selection.count(); i++)
		{
			int row = m_signalsProxyModel->mapToSource(selection[i]).row();
			if (m_signalsModel->isEditableSignal(row))
			{
				emit setSignalActionsVisibility(true);
				return;
			}
		}
		emit setSignalActionsVisibility(false);
	}
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

	int focusedRow = m_signalsModel->keyIndex(focusedCellSignalID);

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
		m_selectedRowsSignalID[currentIdIndex++] = m_signalsModel->key(row);
	}
	QModelIndex index = m_signalsView->currentIndex();
	if (index.isValid())
	{
		int row = m_signalsProxyModel->mapToSource(index).row();
		m_focusedCellSignalID = m_signalsModel->key(row);
		m_focusedCellColumn = index.column();
	}
	m_lastHorizontalScrollPosition = m_signalsView->horizontalScrollBar()->value();
	m_lastVerticalScrollPosition = m_signalsView->verticalScrollBar()->value();
}

void SignalsTabPage::restoreSelection(int focusedSignalId)
{
	m_changingSelectionManualy = true;

	if (focusedSignalId != -1)
	{
		m_focusedCellSignalID = focusedSignalId;
		m_focusedCellColumn = 0;
	}

	QModelIndex currentSourceIndex = m_signalsModel->index(m_signalsModel->keyIndex(m_focusedCellSignalID), m_focusedCellColumn);
	QModelIndex currentProxyIndex = m_signalsProxyModel->mapFromSource(currentSourceIndex);

	/*QItemSelection selection;

	int selectionRowCount = 0;
	foreach (int id, m_selectedRowsSignalID)
	{
		int rowNo = m_signalsModel->keyIndex(id);

		QModelIndex leftIndex  = m_signalsModel->index(rowNo, 0);
		QModelIndex rightIndex = m_signalsModel->index(rowNo, m_signalsModel->columnCount() -1);

		QItemSelection rowSelection(leftIndex, rightIndex);
		selection.merge(rowSelection, QItemSelectionModel::Select);

		selectionRowCount++;

		if (selectionRowCount > 256)
		{
			// Selection limits has been added, because m_signalsView->selectionModel()->select(...) becomes extremely slow
			break;
		}
	}

	m_signalsView->selectionModel()->select(m_signalsProxyModel->mapSelectionFromSource(selection), QItemSelectionModel::Select | QItemSelectionModel::Rows);*/

	m_signalsView->selectionModel()->setCurrentIndex(currentProxyIndex, QItemSelectionModel::Select);
	m_signalsView->selectionModel()->select(currentProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	m_signalsView->horizontalScrollBar()->setValue(m_lastHorizontalScrollPosition);
	m_signalsView->verticalScrollBar()->setValue(m_lastVerticalScrollPosition);

	m_signalsView->scrollTo(currentProxyIndex);

	m_changingSelectionManualy = false;
	changeSignalActionsVisibility();
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
			m_signalsModel->signalPropertyManager().isHiddenFor(static_cast<E::SignalType>(signalType), i) == false)
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
		m_signalsModel->loadSignals();
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
	changeSignalIdFilter(m_filterEdit->text().trimmed().split("|", QString::SkipEmptyParts), false);
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
	const Signal& signal = m_sourceModel->signal(source_row);
	return signal.checkedOut() && (signal.userID() == m_sourceModel->dbController()->currentUser().userId() || m_sourceModel->dbController()->currentUser().isAdminstrator());
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
	QVector<int> sourceRows = m_sourceModel->getSameChannelSignals(mapToSource(index(row, 0)).row());
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

	const auto& propertyManager = m_sourceModel->signalPropertyManager();

	QSettings settings;
	m_signalsView->setColumnWidth(0, columnManager->getColumnWidth(0) + 30);	// basic column width + checkbox size

	for (int i = 1; i < propertyManager.count(); i++)
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
	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, 0);
		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();
		IDs << m_sourceModel->key(sourceRow);
	}
	if (IDs.count() == 0)
	{
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	states.resize(IDs.size());
	m_sourceModel->dbController()->checkinSignals(&IDs, commentText, &states, this);
	m_sourceModel->showErrors(states);

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

	const auto& propertyManager = m_sourceModel->signalPropertyManager();

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

	QVector<int> IDs;
	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, 0);
		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();
		IDs << m_sourceModel->key(sourceRow);
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
		m_sourceModel->dbController()->undoSignalChanges(ID, &state, m_sourceModel->parentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}
	if (!states.isEmpty())
	{
		m_sourceModel->showErrors(states);
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
	setSourceModel(sourceModel);
}

bool SignalsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
	const Signal& currentSignal = m_sourceModel->signal(source_row);
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
		const Signal& sl = m_sourceModel->signal(source_left.row());
		const Signal& sr = m_sourceModel->signal(source_right.row());

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

	QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::clicked, this, &QDialog::accept);
	vl->addWidget(buttonBox);

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
	historyView->verticalHeader()->setResizeMode(QHeaderView::Fixed);

	historyView->horizontalHeader()->setHighlightSections(false);
	historyView->horizontalHeader()->setDefaultSectionSize(150);
	historyView->horizontalHeader()->setStretchLastSection(true);

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

	SignalPropertyManager signalPropertyManager;
	signalPropertyManager.reloadPropertyBehaviour(dbController, parent);

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
			signalPropertyManager.detectNewProperties(signalInstance[0]);
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
	for (int propertyIndex = 0; propertyIndex < signalPropertyManager.count(); propertyIndex++)
	{
		if (signalInstances.count() == 0)
		{
			break;
		}

		QVariant previousValue = signalPropertyManager.value(&signalInstances[0], propertyIndex);

		QList<QStandardItem*> column;
		int columnIndex = m_historyModel->columnCount();

		for (int signalIndex = 0; signalIndex < signalInstances.count(); signalIndex++)
		{
			QVariant currentValue = signalPropertyManager.value(&signalInstances[signalIndex], propertyIndex);
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
		m_historyModel->setHeaderData(columnIndex, Qt::Horizontal, signalPropertyManager.caption(propertyIndex));
	}

	new TableDataVisibilityController(historyView, "SignalHistoryDialog", defaultColumns);
}

void SignalHistoryDialog::closeEvent(QCloseEvent* event)
{
	saveWindowPosition(this, "SignalHistoryDialog");

	QDialog::closeEvent(event);
}
