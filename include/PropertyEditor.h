#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <QWidget>
#include <QMap>
#include <QVariant>

class QtTreePropertyBrowser;
class QtProperty;
class QtStringPropertyManager;
class QtEnumPropertyManager;
class QtIntPropertyManager;
class QtDoublePropertyManager;
class QtGroupPropertyManager;
class QtBoolPropertyManager;

class PropertyEditor : public QWidget
{
    Q_OBJECT

public:
	PropertyEditor(QWidget* parent);

	// Public functions
	//
public:
	void setObjects(QList<QObject*>& objects);
	void clear();

private slots:
    void intValueChanged (QtProperty* property, int value);
    void stringValueChanged (QtProperty* property, QString value);
    void doubleValueChanged (QtProperty* property, double value);
    void boolValueChanged (QtProperty* property, bool value);
    void onShowErrorMessage (QString message);

signals:
    void showErrorMessage(QString message);
	void propertiesChanged(QObjectList objects);

	// Private functions and structs
	//
private:
	void valueChanged(QtProperty* property, QVariant value);

	struct PropertyItem
	{
		QObject* object = nullptr;
		QVariant::Type type;
		QVariant value;
	};

	// Data
	//
private:
	QtTreePropertyBrowser* m_propertyEditor = nullptr;
	QtGroupPropertyManager* m_propertyGroupManager = nullptr;
	QtStringPropertyManager* m_propertyStringManager = nullptr;
	QtEnumPropertyManager* m_propertyEnumManager = nullptr;
	QtIntPropertyManager* m_propertyIntManager = nullptr;
	QtDoublePropertyManager* m_propertyDoubleManager = nullptr;
	QtBoolPropertyManager* m_propertyBoolManager = nullptr;

	QMap<QString, QObject*> m_propToClassMap;
};

#endif // PROPERTYEDITOR_H
