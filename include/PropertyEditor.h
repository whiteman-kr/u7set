#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "../qtpropertybrowser/src/qteditorfactory.h"
#include <memory>
#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QSpinBox>
#include <QCheckBox>
#include <QtTreePropertyBrowser>
#include <QtVariantPropertyManager>
#include <QTextEdit>
#include <QDialog>
#include <QSet>

class QtTreePropertyBrowser;
class QtProperty;
class QtStringPropertyManager;
class QtIntPropertyManager;
class QtDoublePropertyManager;
class QtGroupPropertyManager;


class QtMultiColorEdit : public QWidget
{
	Q_OBJECT

public:
	QtMultiColorEdit(QWidget* parent);
	void setValue(QVariant value);

public slots:
	void onEditingFinished();

signals:
	void valueChanged(QVariant value);

private slots:
	void onButtonPressed();

private:
	bool eventFilter(QObject* watched, QEvent* event);
	QColor colorFromText(const QString& t);


private:
	QLineEdit* m_lineEdit = nullptr;
	bool m_escape = false;
	bool m_editingFinished = false;
};

class MultiLineEdit : public QDialog
{
public:
	MultiLineEdit(QWidget* parent, const QString& text);
	QString text();

private:
	QString m_text;
	QTextEdit* m_textEdit = nullptr;

	virtual void accept();
};

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
	void onEditingFinished();

signals:
	void valueChanged(QVariant value);

private slots:
	void onButtonPressed();

private:
	bool eventFilter(QObject* watched, QEvent* event);

private:
	QLineEdit* m_lineEdit = nullptr;
	//QString m_text;
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

	QSet<QtProperty*> propertyByName(const QString& propertyName);

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

//template <class Type>
class PropertyEditor : public QtTreePropertyBrowser
{
	Q_OBJECT

public:
	PropertyEditor(QWidget* parent);

	// Public functions
	//
public:
	void setObjects(QList<std::shared_ptr<QObject>>& objects);
	void updateProperties(const QString& propertyName);
	void updateProperties();
	void clearProperties();

protected slots:
	virtual void valueChanged(QtProperty* property, QVariant value);
	void onShowErrorMessage (QString message);
	void onCurrentItemChanged(QtBrowserItem* current);

signals:
    void showErrorMessage(QString message);
	void propertiesChanged(QList<std::shared_ptr<QObject>> objects);

	// Protected functions and structs
	//
protected:
	bool propertyByName(const std::shared_ptr<QObject>& object, const QString& name, QMetaProperty& metaProperty);

	struct PropertyItem
	{
		std::shared_ptr<QObject> object;
		QVariant::Type type;
		QVariant value;
	};

	// Data
	//
protected:
	QtGroupPropertyManager* m_propertyGroupManager = nullptr;

	QtMultiVariantPropertyManager* m_propertyStringManager = nullptr;
	QtMultiVariantPropertyManager* m_propertyIntManager = nullptr;
	QtMultiVariantPropertyManager* m_propertyDoubleManager = nullptr;
	QtMultiVariantPropertyManager* m_propertyBoolManager = nullptr;
	QtMultiVariantPropertyManager* m_propertyColorManager = nullptr;

	QMap<QString, std::shared_ptr<QObject>> m_propToClassMap;   //Property Name to Class Map

	//Private Data
	//
private:
	void createValuesMap(const QSet<QtProperty*>& props, QMap<QtProperty*, QVariant>& values);
};

#endif // PROPERTYEDITOR_H
