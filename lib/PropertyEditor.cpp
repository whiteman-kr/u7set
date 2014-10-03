#include "../include/PropertyEditor.h"
#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtStringPropertyManager>
#include <QtEnumPropertyManager>
#include <QtIntPropertyManager>
#include <QtDoublePropertyManager>
#include <QtBoolPropertyManager>
#include <QtSpinBoxFactory>
#include <QList>
#include <QMetaProperty>
#include <QDebug>
#include <QMap>
#include <QStringList>
#include <QMessageBox>


PropertyEditor::PropertyEditor(QWidget* parent) :
	QWidget(parent)
{
    m_propertyEditor = new QtTreePropertyBrowser(this);

    m_propertyGroupManager = new QtGroupPropertyManager(m_propertyEditor);
    m_propertyStringManager = new QtStringPropertyManager(m_propertyEditor);
    m_propertyIntManager = new QtIntPropertyManager(m_propertyEditor);
    m_propertyDoubleManager = new QtDoublePropertyManager(m_propertyEditor);
    m_propertyBoolManager = new QtBoolPropertyManager(m_propertyEditor);

    QtSpinBoxFactory* spinBoxFactory = new QtSpinBoxFactory(this);
    QtDoubleSpinBoxFactory* doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);
    QtLineEditFactory* lineEditFactory = new QtLineEditFactory(this);
    QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(this);

    m_propertyEditor->setFactoryForManager(m_propertyStringManager, lineEditFactory);
    m_propertyEditor->setFactoryForManager(m_propertyIntManager, spinBoxFactory);
    m_propertyEditor->setFactoryForManager(m_propertyDoubleManager, doubleSpinBoxFactory);
    m_propertyEditor->setFactoryForManager(m_propertyBoolManager, checkBoxFactory);

	connect(m_propertyIntManager, &QtIntPropertyManager::valueChanged, this, &PropertyEditor::intValueChanged);
	connect(m_propertyStringManager, &QtStringPropertyManager::valueChanged, this, &PropertyEditor::stringValueChanged);
	connect(m_propertyDoubleManager, &QtDoublePropertyManager::valueChanged, this, &PropertyEditor::doubleValueChanged);
	connect(m_propertyBoolManager, &QtBoolPropertyManager::valueChanged, this, &PropertyEditor::boolValueChanged);

	connect(this, &PropertyEditor::showErrorMessage, this, &PropertyEditor::onShowErrorMessage, Qt::QueuedConnection);

	return;
}

void PropertyEditor::setObjects(QList<QObject*>& objects)
{
	clear();

	QMap<QString, PropertyItem> propertyItems;
	QList<QString> propertyNames;

	// Create a map with all properties
	//
	for (auto object = objects.begin(); object != objects.end(); object++)
	{
		const QMetaObject* metaObject = (*object)->metaObject();

		for (int i = 0; i < metaObject->propertyCount(); ++i)
		{
			QMetaProperty metaProperty = metaObject->property(i);

			const char* name = metaProperty.name();
			if (QString(name) == "objectName")
			{
				continue;
			}

			PropertyItem pi;

			pi.object = *object;
			pi.type = metaProperty.type();
			pi.value = (*object)->property(name);

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
				m_propertyBoolManager->setValue(subProperty, value.toBool());
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

void PropertyEditor::clear()
{
	m_propToClassMap.clear();
	m_propertyEditor->clear();
}


void PropertyEditor::intValueChanged(QtProperty* property, int value)
{
    valueChanged(property, value);
}

void PropertyEditor::stringValueChanged(QtProperty* property, QString value)
{
    valueChanged(property, value);
}

void PropertyEditor::doubleValueChanged(QtProperty* property, double value)
{
    valueChanged(property, value);
}

void PropertyEditor::boolValueChanged(QtProperty* property, bool value)
{
    valueChanged(property, value);
}

void PropertyEditor::valueChanged(QtProperty* property, QVariant value)
{
    QList<QObject*> objects = m_propToClassMap.values(property->propertyName());

    QString errorString;

    for (auto i = objects.begin(); i != objects.end(); i++)
    {
        QObject* pObject = *i;
        const QMetaObject* metaObject = pObject->metaObject();

        int index = metaObject->indexOfProperty(property->propertyName().toStdString().c_str());
        if (index == -1)
        {
            Q_ASSERT(false);
            continue;
        }

        QMetaProperty writeProperty = metaObject->property(index);
        writeProperty.write(pObject, value);

        if (writeProperty.read(pObject) != value && errorString.isEmpty() == true)
        {
			errorString = QString("Object: %1, property: %2 - incorrect input value")
						  .arg(metaObject->className())
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

void PropertyEditor::onShowErrorMessage(QString message)
{
    QMessageBox::warning(this, "Error", message);
}
