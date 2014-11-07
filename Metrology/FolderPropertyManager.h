#ifndef FILEPROPERTYMANAGER_H
#define FILEPROPERTYMANAGER_H

#include <QMap>
#include <QLineEdit>

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

// ==============================================================================================

class FolderEdit : public QWidget
{
    Q_OBJECT

public:

    FolderEdit(QWidget *parent = 0);

    QString folderPath() const { return m_edit->text(); }
    void setFolderPath(const QString &path) { if (m_edit->text() != path) m_edit->setText(path); }

signals:

    void folderPathChanged(const QString &folderPath);

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

    void slotSetValue(const QString &value);
    void slotEditorDestroyed(QObject *object);

private:

    QMap<QtProperty *, QList<FolderEdit *> > m_createdEditorsMap;
    QMap<FolderEdit *, QtProperty *> m_editorToPropertyMap;
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

    static int folerPathTypeId();

public slots:

    virtual void setValue(QtProperty *property, const QVariant &val);

protected:

    virtual QString valueText(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);

private:

    struct Data
    {
        QString value;
    };

    QMap<const QtProperty *, Data> m_valuesMap;
};

// ==============================================================================================

#endif // FILEPROPERTYMANAGER_H
