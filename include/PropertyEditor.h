#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QSpinBox>
#include <QCheckBox>
#include <QtTreePropertyBrowser>
#include <QtVariantPropertyManager>
#include "../qtpropertybrowser/src/qteditorfactory.h"
#include <memory>

class QtTreePropertyBrowser;
class QtProperty;
class QtStringPropertyManager;
class QtIntPropertyManager;
class QtDoublePropertyManager;
class QtGroupPropertyManager;

//-------------------------------------------------------------------------------------

class QtMultiCheckBox : public QWidget
{
	Q_OBJECT

public:
	QtMultiCheckBox(QWidget* parent);
	void setCheckState(Qt::CheckState state);

public slots:
	void onStateChanged(int state);

signals:
	void valueChanged(QVariant value);

private:
	void updateText();

private:
	QCheckBox* m_checkBox = nullptr;

};

class QtMultiTextEdit : public QWidget
{
	Q_OBJECT

public:
	QtMultiTextEdit(QWidget* parent);
	void setValue(QString value);

public slots:
	void onValueChanged(QString value);
	void onEditingFinished();

signals:
	void valueChanged(QVariant value);

private:
	bool eventFilter(QObject* watched, QEvent* event);

private:
	QLineEdit* m_lineEdit = nullptr;
	QString m_text;
	bool m_escape = false;
	bool m_editingFinished = false;
};

class QtMultiDoubleSpinBox : public QWidget
{
	Q_OBJECT

public:
	QtMultiDoubleSpinBox(QWidget* parent);
	void setValue(double value);

public slots:
	void onValueChanged(double value);

signals:
	void valueChanged(QVariant value);

private:
	bool eventFilter(QObject* watched, QEvent* event);

private:
	QDoubleSpinBox* m_spinBox = nullptr;
	bool m_escape = false;
};

class QtMultiIntSpinBox:public QWidget
{
    Q_OBJECT
public:
    QtMultiIntSpinBox(QWidget* parent);
    void setValue(int value);

public slots:
    void onValueChanged(int value);

signals:
	void valueChanged(QVariant value);

private:
	bool eventFilter(QObject* watched, QEvent* event);

private:
    QSpinBox* m_spinBox = nullptr;
    bool m_escape = false;
};

class QtMultiVariantPropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT

public:
	QtMultiVariantPropertyManager(QObject* parent, QVariant::Type type);

	QVariant value(const QtProperty* property) const;

	const QVariant::Type type() const;

	void emitSetValue(QtProperty* property, const QVariant& value);

private:
    struct Data
    {
        QVariant value;
    };
    QMap<const QtProperty*, Data> values;
	QVariant::Type m_type;

public slots:
	void setValue(QtProperty* property, const QVariant& value);

signals:
	void valueChanged(QtProperty* property, QVariant value);

protected:
    void initializeProperty(QtProperty* property);
    void uninitializeProperty(QtProperty* property);
	QIcon valueIcon(const QtProperty* property) const;
    QString valueText(const QtProperty* property) const;

};

class QtMultiVariantFactory : public QtAbstractEditorFactory<QtMultiVariantPropertyManager>
{
    Q_OBJECT

public:
	QtMultiVariantFactory(QObject* parent);

	void connectPropertyManager (QtMultiVariantPropertyManager* manager);
	QWidget* createEditor(QtMultiVariantPropertyManager* manager, QtProperty* property, QWidget* parent);
	void disconnectPropertyManager(QtMultiVariantPropertyManager* manager);

public slots:
	//void slotPropertyChanged(QtProperty* property, QVariant value);
	void slotSetValue(QVariant value);
	void slotEditorDestroyed(QObject* object);

private:
	QtMultiVariantPropertyManager* m_manager = nullptr;
    QtProperty* m_property = nullptr;
};

// -------------------------------------------------------------------------------

class PropertyEditor : public QWidget
{
    Q_OBJECT

public:
	PropertyEditor(QWidget* parent);

	// Public functions
	//
public:
	void setObjects(QList<std::shared_ptr<QObject>>& objects);
	void update();
	void clear();

protected slots:
	virtual void valueChanged(QtProperty* property, QVariant value);

private slots:
	void onShowErrorMessage (QString message);
	void onCurrentItemChanged(QtBrowserItem* current);

signals:
    void showErrorMessage(QString message);
	void propertiesChanged(QObjectList objects);

	// Private functions and structs
	//
private:
	void resizeEvent(QResizeEvent* event);
	void createValuesMap(QtAbstractPropertyManager* manager, QVariant::Type type, QMap<QtProperty*, QVariant>& values);
	bool propertyByName(const QObject* object, const QString& name, QMetaProperty& metaProperty);

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

	QtMultiVariantPropertyManager* m_propertyStringManager = nullptr;
	QtMultiVariantPropertyManager* m_propertyIntManager = nullptr;
	QtMultiVariantPropertyManager* m_propertyDoubleManager = nullptr;
	QtMultiVariantPropertyManager* m_propertyBoolManager = nullptr;

    QMap<QString, QObject*> m_propToClassMap;   //Property Name to Class Map
};

#endif // PROPERTYEDITOR_H
