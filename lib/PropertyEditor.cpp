#include "../include/PropertyEditor.h"

#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtStringPropertyManager>
#include <QtEnumPropertyManager>
#include <QtIntPropertyManager>
#include <QtDoublePropertyManager>
#include <QtBoolPropertyManager>
#include <QList>
#include <QMetaProperty>
#include <QDebug>
#include <QMap>
#include <QStringList>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include <QHBoxLayout>
#include <QStyle>
#include <QStyleOptionButton>
#include <QPainter>
#include <QApplication>

//
// ---------QtMultiTextEdit----------
//

QtMultiTextEdit::QtMultiTextEdit(QWidget* parent):
	QWidget(parent)
{
	m_lineEdit = new QLineEdit(parent);

	connect(m_lineEdit, &QLineEdit::textEdited, this, &QtMultiTextEdit::onValueChanged);
	connect(m_lineEdit, &QLineEdit::editingFinished, this, &QtMultiTextEdit::onEditingFinished);

	QHBoxLayout* lt = new QHBoxLayout;
	lt->setContentsMargins(0, 0, 0, 0);
	lt->addWidget(m_lineEdit);
	setLayout(lt);

	m_lineEdit->installEventFilter(this);
}

bool QtMultiTextEdit::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_lineEdit && event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		if (ke->key() == Qt::Key_Escape)
		{
			m_escape = true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

void QtMultiTextEdit::setValue(QString value)
{
	m_lineEdit->blockSignals(true);
	m_lineEdit->setText(value);
	m_lineEdit->blockSignals(false);
}

void QtMultiTextEdit::onValueChanged(QString value)
{
	m_text = value;
}

void QtMultiTextEdit::onEditingFinished()
{
	if (m_escape == false)
	{
		if (m_editingFinished == false)
		{
			emit valueChanged(m_text);
			m_editingFinished = true;	//a "static" value. On "Enter", onEditingFinished comes twice???
		}
	}

}

//
// ---------QtMultiDoubleSpinBox----------
//

QtMultiDoubleSpinBox::QtMultiDoubleSpinBox(QWidget* parent):
	QWidget(parent)
{
	m_spinBox = new QDoubleSpinBox(parent);
	m_spinBox->setKeyboardTracking(false);
	m_spinBox->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
	m_spinBox->setDecimals(2);

	connect(m_spinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &QtMultiDoubleSpinBox::onValueChanged);

	QHBoxLayout* lt = new QHBoxLayout;
	lt->setContentsMargins(0, 0, 0, 0);
	lt->addWidget(m_spinBox);
	setLayout(lt);

	m_spinBox->installEventFilter(this);
}

bool QtMultiDoubleSpinBox::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_spinBox && event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		if (ke->key() == Qt::Key_Escape)
		{
			m_escape = true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

void QtMultiDoubleSpinBox::setValue(double value)
{
	m_spinBox->blockSignals(true);
	m_spinBox->setValue(value);
	m_spinBox->blockSignals(false);
}

void QtMultiDoubleSpinBox::onValueChanged(double value)
{
	if (m_escape == false)
	{
		emit valueChanged(value);
	}
}

//
// ---------QtMultiIntSpinBox----------
//

QtMultiIntSpinBox::QtMultiIntSpinBox(QWidget* parent):
	QWidget(parent)
{
	m_spinBox = new QSpinBox(parent);
	m_spinBox->setKeyboardTracking(false);
	m_spinBox->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	connect(m_spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &QtMultiIntSpinBox::onValueChanged);

	QHBoxLayout *lt = new QHBoxLayout;
	lt->setContentsMargins(0, 0, 0, 0);
	lt->addWidget(m_spinBox);
	setLayout(lt);

	m_spinBox->installEventFilter(this);
}

bool QtMultiIntSpinBox::eventFilter(QObject * watched, QEvent * event)
{
	if (watched == m_spinBox && event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		if (ke->key() == Qt::Key_Escape)
		{
			m_escape = true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

void QtMultiIntSpinBox::setValue(int value)
{
	m_spinBox->blockSignals(true);
	m_spinBox->setValue(value);
	m_spinBox->blockSignals(false);
}

void QtMultiIntSpinBox::onValueChanged(int value)
{
	if (m_escape == false)
	{
		emit valueChanged(value);
	}
}

//
// ---------QtMultiCheckBox----------
//
static QIcon drawCheckBox(int state)
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

	opt.state |= QStyle::State_Enabled;
	const QStyle *style = QApplication::style();
	// Figure out size of an indicator and make sure it is not scaled down in a list view item
	// by making the pixmap as big as a list view icon and centering the indicator in it.
	// (if it is smaller, it can't be helped)
	const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
	const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
	const int listViewIconSize = indicatorWidth;
	const int pixmapWidth = indicatorWidth;
	const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

	opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
	QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
	pixmap.fill(Qt::transparent);
	{
		// Center?
		const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
		const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
		QPainter painter(&pixmap);
		painter.translate(xoff, yoff);
		painter.translate(1, 0);
		style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
	}
	return QIcon(pixmap);
}

QtMultiCheckBox::QtMultiCheckBox(QWidget* parent):
	QWidget(parent)
{
	m_checkBox = new QCheckBox(parent);

	connect(m_checkBox, &QCheckBox::stateChanged, this, &QtMultiCheckBox::onStateChanged);

	QHBoxLayout*lt = new QHBoxLayout;
	lt->setContentsMargins(4, 1, 0, 0);
	lt->addWidget(m_checkBox);
	setLayout(lt);
}

void QtMultiCheckBox::setCheckState(Qt::CheckState state)
{
	m_checkBox->blockSignals(true);
	m_checkBox->setCheckState(state);
	updateText();
	m_checkBox->blockSignals(false);
}

void QtMultiCheckBox::onStateChanged(int state)
{
	updateText();

	m_checkBox->setTristate(false);

	emit valueChanged(state);
}

void QtMultiCheckBox::updateText()
{
	switch (m_checkBox->checkState())
	{
		case Qt::Checked:           m_checkBox->setText("True");                break;
		case Qt::Unchecked:         m_checkBox->setText("False");               break;
		case Qt::PartiallyChecked:  m_checkBox->setText("<Different values>");  break;
		default:
			Q_ASSERT(false);
	}
}

//
// ---------QtMultiVariantFactory----------
//

QtMultiVariantFactory::QtMultiVariantFactory(QObject* parent):
	QtAbstractEditorFactory<QtMultiVariantPropertyManager>(parent)
{
}

QWidget* QtMultiVariantFactory::createEditor(QtMultiVariantPropertyManager* manager, QtProperty* property, QWidget* parent)
{
	m_manager = manager;
	m_property = property;

	QWidget* editor = nullptr;

	switch(manager->type())
	{
		case QVariant::Int:
			{
				QtMultiIntSpinBox* m_editor = new QtMultiIntSpinBox(parent);
				editor = m_editor;
				m_editor->setValue(manager->value(property).toInt());

				connect(m_editor, &QtMultiIntSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiIntSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;
		case QVariant::Double:
			{
				QtMultiDoubleSpinBox* m_editor = new QtMultiDoubleSpinBox(parent);
				editor = m_editor;
				m_editor->setValue(manager->value(property).toDouble());

				connect(m_editor, &QtMultiDoubleSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiDoubleSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;
		case QVariant::Bool:
			{
				QtMultiCheckBox* m_editor = new QtMultiCheckBox(parent);
				editor = m_editor;
				m_editor->setCheckState((Qt::CheckState)manager->value(property).toInt());

				connect(m_editor, &QtMultiCheckBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiCheckBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;

		case QVariant::String:
			{
				QtMultiTextEdit* m_editor = new QtMultiTextEdit(parent);
				editor = m_editor;
				m_editor->setValue(manager->value(property).toString());

				connect(m_editor, &QtMultiTextEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiTextEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;

		default:
			Q_ASSERT(false);
	}

	Q_ASSERT(editor);

	return editor;
}

void QtMultiVariantFactory::connectPropertyManager (QtMultiVariantPropertyManager* manager)
{
	Q_UNUSED(manager);
	//connect(manager, &QtMultiVariantPropertyManager::valueChanged, this, &QtMultiVariantFactory::slotPropertyChanged);
}

void QtMultiVariantFactory::disconnectPropertyManager(QtMultiVariantPropertyManager* manager)
{
	Q_UNUSED(manager);
	//disconnect(manager, &QtMultiVariantPropertyManager::valueChanged, this, &QtMultiVariantFactory::slotPropertyChanged);
}

/*void QtMultiVariantFactory::slotPropertyChanged(QtProperty* property, QVariant value)
{
	Q_UNUSED(property);
	Q_UNUSED(value);
}*/

void QtMultiVariantFactory::slotSetValue(QVariant value)
{
	m_manager->setValue(m_property, value);
	m_manager->emitSetValue(m_property, value);
}

void QtMultiVariantFactory::slotEditorDestroyed(QObject* object)
{
	Q_UNUSED(object);
}

//
// ---------QtMultiVariantPropertyManager----------
//

QtMultiVariantPropertyManager::QtMultiVariantPropertyManager(QObject* parent, QVariant::Type type) :
	QtAbstractPropertyManager(parent),
	m_type(type)
{

}

const QVariant::Type QtMultiVariantPropertyManager::type() const
{
	return m_type;
}

QVariant QtMultiVariantPropertyManager::value(const QtProperty* property) const
{
	const QMap<const QtProperty*, Data>::const_iterator it = values.constFind(property);
	if (it == values.end())
	{
		Q_ASSERT(false);
		return 0;
	}
	return it.value().value;
}

void QtMultiVariantPropertyManager::setValue(QtProperty* property, const QVariant& value)
{
	const QMap<const QtProperty*, Data>::iterator it = values.find(property);
	if (it == values.end())
	{
		Q_ASSERT(false);
		return;
	}

	if (value.isNull())
	{
		it.value().value = QVariant();
	}
	else
	{
		QVariant newValue = value;
		it.value().value = newValue;
	}


	emit propertyChanged(property);
}

void QtMultiVariantPropertyManager::emitSetValue(QtProperty* property, const QVariant& value)
{
	valueChanged(property, value);
}

void QtMultiVariantPropertyManager::initializeProperty(QtProperty* property)
{
	values[property] = QtMultiVariantPropertyManager::Data();
}
void QtMultiVariantPropertyManager::uninitializeProperty(QtProperty* property)
{
	values.remove(property);

}

QIcon QtMultiVariantPropertyManager::valueIcon(const QtProperty* property) const
{
	switch (type())
	{
		case QVariant::Bool:
			{
				Qt::CheckState checkState = (Qt::CheckState)value(property).toInt();
				return drawCheckBox(checkState);
			}
			break;
	}
	return QIcon();
}

QString QtMultiVariantPropertyManager::valueText(const QtProperty* property) const
{
	if (value(property).isNull() == false)
	{
		switch (type())
		{
			case QVariant::Int:
				{
					int val = value(property).toInt();
					return QString::number(val);
				}
				break;

			case QVariant::Double:
				{
					double val = value(property).toDouble();
					return QString::number(val);
				}
				break;
			case QVariant::Bool:
				{
					Qt::CheckState checkState = (Qt::CheckState)value(property).toInt();

					switch (checkState)
					{
						case Qt::Checked:           return("True");
						case Qt::Unchecked:         return("False");
						case Qt::PartiallyChecked:  return("<Different values>");
					}

					return "???";

				}
				break;
			case QVariant::String:
				{
					QString val = value(property).toString();
					return val;
				}
				break;

			default:
				Q_ASSERT(false);
		}
	}

	return QString("");
}



//
// ------- Property Editor ----------
//

PropertyEditor::PropertyEditor(QWidget* parent) :
	QWidget(parent)
{
	m_propertyEditor = new QtTreePropertyBrowser(parent);

	m_propertyGroupManager = new QtGroupPropertyManager(m_propertyEditor);
	m_propertyStringManager = new QtMultiVariantPropertyManager(m_propertyEditor, QVariant::String);
	m_propertyIntManager = new QtMultiVariantPropertyManager(m_propertyEditor, QVariant::Int);
	m_propertyDoubleManager = new QtMultiVariantPropertyManager(m_propertyEditor, QVariant::Double);
	m_propertyBoolManager = new QtMultiVariantPropertyManager(m_propertyEditor, QVariant::Bool);

	QtMultiVariantFactory* spinBoxFactory = new QtMultiVariantFactory(this);
	QtMultiVariantFactory* doubleSpinBoxFactory = new QtMultiVariantFactory(this);
	QtMultiVariantFactory* lineEditFactory = new QtMultiVariantFactory(this);
	QtMultiVariantFactory *checkBoxFactory = new QtMultiVariantFactory(this);

	m_propertyEditor->setFactoryForManager(m_propertyStringManager, lineEditFactory);
	m_propertyEditor->setFactoryForManager(m_propertyIntManager, spinBoxFactory);
	m_propertyEditor->setFactoryForManager(m_propertyDoubleManager, doubleSpinBoxFactory);
	m_propertyEditor->setFactoryForManager(m_propertyBoolManager, checkBoxFactory);

	connect(m_propertyIntManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::valueChanged);
	connect(m_propertyStringManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::valueChanged);
	connect(m_propertyDoubleManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::valueChanged);
	connect(m_propertyBoolManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::valueChanged);

	connect(this, &PropertyEditor::showErrorMessage, this, &PropertyEditor::onShowErrorMessage, Qt::QueuedConnection);

	connect(m_propertyEditor, &QtTreePropertyBrowser::currentItemChanged, this, &PropertyEditor::onCurrentItemChanged);

	return;
}

void PropertyEditor::onCurrentItemChanged(QtBrowserItem* current)
{
	if (current->property() == nullptr)
	{
		Q_ASSERT(current->property());
		return;
	}
}


void PropertyEditor::setObjects(QList<std::shared_ptr<QObject>>& objects)
{
	clear();

	QMap<QString, PropertyItem> propertyItems;
	QList<QString> propertyNames;

	// Create a map with all properties
	//
	for (auto pobject = objects.begin(); pobject != objects.end(); pobject++)
	{
		QObject* object = pobject->get();

		const QMetaObject* metaObject = object->metaObject();

		for (int i = 0; i < metaObject->propertyCount(); ++i)
		{
			QMetaProperty metaProperty = metaObject->property(i);

			const char* name = metaProperty.name();
			if (QString(name) == "objectName")
			{
				continue;
			}

			PropertyItem pi;

			pi.object = object;
			pi.type = metaProperty.type();
			pi.value = object->property(name);

			propertyItems.insertMulti(name, pi);

			if (propertyNames.indexOf(name) == -1)
			{
				propertyNames.append(name);
			}
		}
	}

	QtProperty* commonProperty = m_propertyGroupManager->addProperty(tr("Common"));

	// add only common properties with same type
	//
	for (auto n = propertyNames.begin(); n != propertyNames.end(); n++)
	{
		// take all properties witn the same name
		//
		QString name = *n;
		QList<PropertyItem> items = propertyItems.values(name);

		if (items.size() != objects.size())
		{
			continue;   // this property is not in all objects
		}

		// now check if all properties have the same type and values
		//
		QVariant::Type type;
		QVariant value;

		bool sameType = true;
		bool sameValue = true;

		for (auto p = items.begin(); p != items.end(); p++)
		{
			PropertyItem& pi = *p;

			if (p == items.begin())
			{
				// remember the first item params
				//
				type = pi.type;
				value = pi.value;
			}
			else
			{
				// compare with next item params
				//
				if (pi.type != type)
				{
					sameType = false;
					break;
				}

				if (pi.value != value)
				{
					sameValue = false;
				}
			}

			m_propToClassMap.insertMulti(name, pi.object);
		}

		if (sameType == false)
		{
			continue;   // properties are not the same type
		}

		// Add the property now
		//
		QtProperty* subProperty = nullptr;

		switch (type)
		{
			case QVariant::Int:
				subProperty = m_propertyIntManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyIntManager->setValue(subProperty, value.toInt());
				}
				break;

			case QVariant::String:
				subProperty = m_propertyStringManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyStringManager->setValue(subProperty, value.toString());
				}
				break;

			case QVariant::Double:
				subProperty = m_propertyDoubleManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyDoubleManager->setValue(subProperty, value.toDouble());
				}
				break;

			case QVariant::Bool:
				subProperty = m_propertyBoolManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyBoolManager->setValue(subProperty, value.toBool() == true ? Qt::Checked : Qt::Unchecked);
				}
				else
				{
					m_propertyBoolManager->setValue(subProperty, Qt::PartiallyChecked);
				}
				break;

			default:
				Q_ASSERT(false);
				continue;
		}

		if (subProperty == nullptr)
		{
			Q_ASSERT(subProperty);
			continue;
		}

		commonProperty->addSubProperty(subProperty);
	}

	m_propertyEditor->addProperty(commonProperty);

	return;
}

void PropertyEditor::update()
{
	QMap<QtProperty*, QVariant> vals;

	//m_propertyIntManager

	QSet<QtProperty*> props = m_propertyIntManager->properties();
	createValuesMap(m_propertyIntManager, m_propertyIntManager->type(), vals);

	for (auto p : props)
	{
		m_propertyIntManager->setValue(p, vals.value(p));
	}


	//m_propertyBoolManager

	props = m_propertyBoolManager->properties();
	createValuesMap(m_propertyBoolManager, m_propertyBoolManager->type(), vals);

	for (auto p = props.begin(); p != props.end(); p++)
	{
		m_propertyBoolManager->setValue(*p, vals.value(*p));
	}


	//m_propertyDoubleManager

	props = m_propertyDoubleManager->properties();
	createValuesMap(m_propertyDoubleManager, m_propertyDoubleManager->type(), vals);

	for (auto p = props.begin(); p != props.end(); p++)
	{
		m_propertyDoubleManager->setValue(*p, vals.value(*p));
	}

	//m_propertyStringManager

	props = m_propertyStringManager->properties();
	createValuesMap(m_propertyStringManager, m_propertyStringManager->type(), vals);

	for (auto p = props.begin(); p != props.end(); p++)
	{
		m_propertyStringManager->setValue(*p, vals.value(*p));
	}
}

void PropertyEditor::createValuesMap(QtAbstractPropertyManager* manager, QVariant::Type type, QMap<QtProperty*, QVariant>& values)
{
	values.clear();

	QSet<QtProperty*> props = manager->properties();

	for (auto p = props.begin(); p != props.end(); p++)
	{
		QtProperty* property = *p;

		bool sameValue = true;
		QVariant value;

		QList<QObject*> objects = m_propToClassMap.values(property->propertyName());
		for (auto i = objects.begin(); i != objects.end(); i++)
		{
			QObject* pObject = *i;
			QMetaProperty metaProperty;
			if (propertyByName(pObject, property->propertyName(), metaProperty) == false)
			{
				Q_ASSERT(false);
				continue;
			}

			QVariant val = metaProperty.read(pObject);

			if (type == QVariant::Bool)
			{
				if (val == true)
				{
					val = Qt::Checked;
				}
				else
				{
					val = Qt::Unchecked;
				}
			}

			if (i == objects.begin())
			{
				value = val;
			}
			else
			{
				if (value != val)
				{
					sameValue = false;
					break;
				}
			}
		}

		if (sameValue == true)
		{
			values.insert(property, value);
		}
		else
		{
			values.insert(property, QVariant());
		}
	}
}

void PropertyEditor::clear()
{
	m_propToClassMap.clear();
	m_propertyEditor->clear();
}

void PropertyEditor::valueChanged(QtProperty* property, QVariant value)
{
	// Set the new property value in all objects
	//
	QList<QObject*> objects = m_propToClassMap.values(property->propertyName());

	QString errorString;
	QMetaProperty writeProperty;

	for (auto i = objects.begin(); i != objects.end(); i++)
	{
		QObject* pObject = *i;

		if (propertyByName(pObject, property->propertyName(), writeProperty) == false)
		{
			Q_ASSERT(false);
			continue;
		}

		if (writeProperty.type() == QVariant::Bool)
		{
			if (value == Qt::Unchecked)
				value = false;
			else
				value = true;
		}

		writeProperty.write(pObject, value);

		if (writeProperty.read(pObject) != value && errorString.isEmpty() == true)
		{
			errorString = QString("Property: %1 - incorrect input value")
						  .arg(property->propertyName());
		}
	}

	if (errorString.isEmpty() == false)
	{
		emit showErrorMessage(errorString);
	}

	emit propertiesChanged(objects);

	return;
}

bool PropertyEditor::propertyByName(const QObject* object, const QString& name, QMetaProperty& metaProperty)
{
	const QMetaObject* metaObject = object->metaObject();

	int index = metaObject->indexOfProperty(name.toStdString().c_str());
	if (index == -1)
	{
		Q_ASSERT(false);
		return false;
	}
	metaProperty = metaObject->property(index);
	return true;
}

void PropertyEditor::onShowErrorMessage(QString message)
{
	QMessageBox::warning(this, "Error", message);
}

void PropertyEditor::resizeEvent(QResizeEvent * event)
{
	if (m_propertyEditor != nullptr)
	{
		m_propertyEditor->resize(event->size());
	}
}
