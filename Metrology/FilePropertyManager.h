#ifndef FILEPROPERTYMANAGER_H
#define FILEPROPERTYMANAGER_H

#include <QMap>
#include <QLineEdit>

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

// ==============================================================================================

class FileEdit : public QWidget
{
    Q_OBJECT

public:

    FileEdit(QWidget *parent = 0);

    QString filePath() const { return m_edit->text(); }
    void setFilePath(const QString &filePath) { if (m_edit->text() != filePath) m_edit->setText(filePath); }


    QString filter() const { return m_filter; }
    void setFilter(const QString &filter) { m_filter = filter; }

signals:

    void filePathChanged(const QString &filePath);

protected:

    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

private slots:

    void buttonClicked();

private:

    QLineEdit *m_edit;
    QToolButton *m_button;

    QString m_filter;
};

// ==============================================================================================

class VariantFactory : public QtVariantEditorFactory
{
    Q_OBJECT

public:

    VariantFactory(QObject *parent = 0) : QtVariantEditorFactory(parent) { }
    virtual ~VariantFactory();

protected:

    virtual void connectPropertyManager(QtVariantPropertyManager *manager);
    virtual void disconnectPropertyManager(QtVariantPropertyManager *manager);

    virtual QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent);

private slots:

    void slotPropertyChanged(QtProperty *property, const QVariant &value);
    void slotPropertyAttributeChanged(QtProperty *property, const QString &attribute, const QVariant &value);

    void slotSetValue(const QString &value);
    void slotEditorDestroyed(QObject *object);

private:

    QMap<QtProperty *, QList<FileEdit *> > m_createdEditorsMap;
    QMap<FileEdit *, QtProperty *> m_editorToPropertyMap;
};

// ==============================================================================================

class VariantManager : public QtVariantPropertyManager
{
    Q_OBJECT

public:
    VariantManager(QObject *parent = 0) : QtVariantPropertyManager(parent) { }

    virtual QVariant value(const QtProperty *property) const;
    virtual int valueType(int propertyType) const;
    virtual bool isPropertyTypeSupported(int propertyType) const;

    virtual QStringList attributes(int propertyType) const;
    virtual int attributeType(int propertyType, const QString &attribute) const;
    virtual QVariant attributeValue(const QtProperty *property, const QString &attribute);

    static int filePathTypeId();

public slots:

    virtual void setValue(QtProperty *property, const QVariant &val);
    virtual void setAttribute(QtProperty *property, const QString &attribute, const QVariant &value);

protected:

    virtual QString valueText(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);

private:

    struct Data
    {
        QString value;
        QString filter;
    };

    QMap<const QtProperty *, Data> m_valuesMap;
};

// ==============================================================================================

#endif // FILEPROPERTYMANAGER_H
