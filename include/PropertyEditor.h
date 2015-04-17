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
#include <QComboBox>
#include <QStringList>

class QtTreePropertyBrowser;
class QtProperty;


struct FilePathPropertyType
{
	FilePathPropertyType()
	{
		filter = "*.*";
	}

	QString filePath;
	QString filter;

	static int filePathTypeId();
};
Q_DECLARE_METATYPE(FilePathPropertyType)

struct EnumPropertyType
{
    EnumPropertyType()
    {
    }

	std::vector<std::pair<QString, int>> items;
	int value = -1;

    static int enumTypeId();
};
Q_DECLARE_METATYPE(EnumPropertyType)


class QtMultiFilePathEdit : public QWidget
{
	Q_OBJECT

public:
	explicit QtMultiFilePathEdit(QWidget* parent);
	void setValue(QVariant value);

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
	bool m_escape = false;
	QVariant m_oldPath;

};

class QtMultiEnumEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QtMultiEnumEdit(QWidget* parent);
	void setItems(QVariant value);
	void setValue(QVariant value);

public slots:
    void indexChanged(int index);

signals:
    void valueChanged(QVariant value);

//private slots:
    //void onButtonPressed();

//private:
    //bool eventFilter(QObject* watched, QEvent* event);

private:
    QComboBox* m_combo = nullptr;
    QVariant m_oldValue;

};
class QtMultiColorEdit : public QWidget
{
	Q_OBJECT

public:
	explicit QtMultiColorEdit(QWidget* parent);
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
	QColor m_oldColor;

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
	virtual void closeEvent(QCloseEvent *event);
};


class QtMultiCheckBox : public QWidget
{
	Q_OBJECT

public:
	explicit QtMultiCheckBox(QWidget* parent);
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
	explicit QtMultiTextEdit(QWidget* parent);
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
	bool m_escape = false;
	QString m_oldValue;
};


class QtMultiDoubleSpinBox : public QWidget
{
	Q_OBJECT

public:
	explicit QtMultiDoubleSpinBox(QWidget* parent);
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


class QtMultiIntSpinBox : public QWidget
{
    Q_OBJECT
public:
	explicit QtMultiIntSpinBox(QWidget* parent);
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


class QtMultiUIntSpinBox : public QWidget
{
	Q_OBJECT
public:
	explicit QtMultiUIntSpinBox(QWidget* parent);
	void setValue(quint32 value);

public slots:
	void onValueChanged(quint32 value);

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
	explicit QtMultiVariantPropertyManager(QObject* parent);

	QVariant attribute(const QtProperty* property, const QString& attribute) const;
	bool hasAttribute(const QtProperty* property, const QString& attribute) const;

	QVariant value(const QtProperty* property) const;
	int valueType(const QtProperty* property) const;

	bool sameValue(const QtProperty* property) const;

	QSet<QtProperty*> propertyByName(const QString& propertyName);

	void emitSetValue(QtProperty* property, const QVariant& value);

private:
	virtual QString displayText(const QtProperty *property) const;

private:

	struct Data
    {
        QVariant value;
		QMap<QString, QVariant> attributes;
	};
    QMap<const QtProperty*, Data> values;

public slots:
	void setValue(QtProperty* property, const QVariant& value);
	void setAttribute (QtProperty* property, const QString& attribute, const QVariant& value);

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
	explicit QtMultiVariantFactory(QObject* parent);

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
	explicit PropertyEditor(QWidget* parent);

	void updateProperty(const QString& propertyName);
	// Public functions
	//
public:
	void setObjects(QList<std::shared_ptr<QObject>>& objects);
	void clearProperties();

protected:
    virtual void valueChanged(QtProperty* property, QVariant newValue);

protected slots:
	void updateProperties();

private slots:
	void onValueChanged(QtProperty* property, QVariant value);
	void onShowErrorMessage (QString message);
	void onCurrentItemChanged(QtBrowserItem* current);

signals:
    void showErrorMessage(QString message);
	void propertiesChanged(QList<std::shared_ptr<QObject>> objects);

	// Protected functions and structs
	//
protected:
	struct PropertyItem
	{
		std::shared_ptr<QObject> object;
		int type;
		QVariant value;
	};

	// Data
	//
protected:
	QtGroupPropertyManager* m_propertyGroupManager = nullptr;

	QtMultiVariantPropertyManager* m_propertyVariantManager = nullptr;

	QMap<QtProperty*, std::shared_ptr<QObject>> m_propToClassMap;   //Property Name to Class Map

	//Private Data
	//
private:
	void createValuesMap(const QSet<QtProperty*>& props, QMap<QtProperty*, QVariant>& values);
	QtProperty* createProperty(QtProperty *parentProperty, const QString& name, const QString& fullName, const QVariant& value, int type, bool sameValue);
};

#endif // PROPERTYEDITOR_H
