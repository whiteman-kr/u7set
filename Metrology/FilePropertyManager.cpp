#include "FilePropertyManager.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QFocusEvent>

// -------------------------------------------------------------------------------------------------------------------

FileEdit::FileEdit(QWidget *parent)
    : QWidget(parent)
{
    m_edit = new QLineEdit(this);
    m_edit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_button = new QToolButton(this);
    m_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    m_button->setText(QLatin1String("..."));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_edit);
    layout->addWidget(m_button);

    setFocusProxy(m_edit);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);

    connect(m_edit, &QLineEdit::textEdited, this, &FileEdit::filePathChanged);
    connect(m_button, &QToolButton::clicked, this, &FileEdit::buttonClicked);
}

void FileEdit::buttonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select file"), m_edit->text(), m_filter);
    if (filePath.isEmpty() == true)
    {
        return;
    }

    m_edit->setText(filePath);

    emit filePathChanged(filePath);
}

void FileEdit::focusInEvent(QFocusEvent *e)
{
    m_edit->event(e);

    if (e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason)
    {
        m_edit->selectAll();
    }

    QWidget::focusInEvent(e);
}

void FileEdit::focusOutEvent(QFocusEvent *e)
{
    m_edit->event(e);

    QWidget::focusOutEvent(e);
}

void FileEdit::keyPressEvent(QKeyEvent *e)
{
    m_edit->event(e);
}

void FileEdit::keyReleaseEvent(QKeyEvent *e)
{
    m_edit->event(e);
}

// -------------------------------------------------------------------------------------------------------------------

VariantFactory::~VariantFactory()
{
    QList<FileEdit *> editors = m_editorToPropertyMap.keys();
    QListIterator<FileEdit *> it(editors);

    while (it.hasNext() == true)
    {
        delete it.next();
    }
}

void VariantFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, &QtVariantPropertyManager::valueChanged, this, &VariantFactory::slotPropertyChanged);
    connect(manager, &QtVariantPropertyManager::attributeChanged, this, &VariantFactory::slotPropertyAttributeChanged);

    QtVariantEditorFactory::connectPropertyManager(manager);
}

void VariantFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    disconnect(manager, &QtVariantPropertyManager::valueChanged, this, &VariantFactory::slotPropertyChanged);
    disconnect(manager, &QtVariantPropertyManager::attributeChanged, this, &VariantFactory::slotPropertyAttributeChanged);

    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

QWidget *VariantFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent)
{
    if (manager->propertyType(property) == VariantManager::filePathTypeId())
    {
        FileEdit *editor = new FileEdit(parent);

        editor->setFilePath(manager->value(property).toString());
        editor->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());

        m_createdEditorsMap[property].append(editor);
        m_editorToPropertyMap[editor] = property;

        connect(editor, &FileEdit::filePathChanged, this, &VariantFactory::slotSetValue);
        connect(editor, &FileEdit::destroyed, this, &VariantFactory::slotEditorDestroyed);

        return editor;
    }

    return QtVariantEditorFactory::createEditor(manager, property, parent);
}


void VariantFactory::slotPropertyChanged(QtProperty *property, const QVariant &value)
{
    if (m_createdEditorsMap.contains(property) == false)
    {
        return;
    }

    QList<FileEdit *> editors = m_createdEditorsMap[property];
    QListIterator<FileEdit *> itEditor(editors);

    while (itEditor.hasNext() == true)
    {
        itEditor.next()->setFilePath(value.toString());
    }
}

void VariantFactory::slotPropertyAttributeChanged(QtProperty *property, const QString &attribute, const QVariant &value)
{
    if (m_createdEditorsMap.contains(property) == false)
    {
        return;
    }

    if (attribute != QLatin1String("filter"))
    {
        return;
    }

    QList<FileEdit *> editors = m_createdEditorsMap[property];
    QListIterator<FileEdit *> itEditor(editors);

    while (itEditor.hasNext() == true)
    {
        itEditor.next()->setFilter(value.toString());
    }
}

void VariantFactory::slotSetValue(const QString &value)
{
    QObject *object = sender();
    QMap<FileEdit *, QtProperty *>::ConstIterator itEditor = m_editorToPropertyMap.constBegin();

    while (itEditor != m_editorToPropertyMap.constEnd())
    {
        if (itEditor.key() == object)
        {
            QtProperty *property = itEditor.value();
            QtVariantPropertyManager *manager = propertyManager(property);
            if (manager == nullptr)
            {
                return;
            }

            manager->setValue(property, value);

            return;
        }

        itEditor++;
    }
}

void VariantFactory::slotEditorDestroyed(QObject *object)
{
    QMap<FileEdit *, QtProperty *>::ConstIterator itEditor = m_editorToPropertyMap.constBegin();

    while (itEditor != m_editorToPropertyMap.constEnd())
    {
        if (itEditor.key() == object)
        {
            FileEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();

            m_editorToPropertyMap.remove(editor);
            m_createdEditorsMap[property].removeAll(editor);

            if (m_createdEditorsMap[property].isEmpty() == true)
            {
                m_createdEditorsMap.remove(property);
            }
            return;
        }

        itEditor++;
    }
}

// -------------------------------------------------------------------------------------------------------------------

class FilePathPropertyType
{
};

Q_DECLARE_METATYPE(FilePathPropertyType)

int VariantManager::filePathTypeId()
{
    return qMetaTypeId<FilePathPropertyType>();
}

// -------------------------------------------------------------------------------------------------------------------

bool VariantManager::isPropertyTypeSupported(int propertyType) const
{
    if (propertyType == filePathTypeId())
    {
        return true;
    }

    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int VariantManager::valueType(int propertyType) const
{
    if (propertyType == filePathTypeId())
    {
        return QVariant::String;
    }

    return QtVariantPropertyManager::valueType(propertyType);
}

QVariant VariantManager::value(const QtProperty *property) const
{
    if (m_valuesMap.contains(property) == true)
    {
        return m_valuesMap[property].value;
    }

    return QtVariantPropertyManager::value(property);
}

QStringList VariantManager::attributes(int propertyType) const
{
    if (propertyType == filePathTypeId())
    {
        QStringList attr;
        attr << QLatin1String("filter");

        return attr;
    }

    return QtVariantPropertyManager::attributes(propertyType);
}

int VariantManager::attributeType(int propertyType, const QString &attribute) const
{
    if (propertyType == filePathTypeId())
    {
        if (attribute == QLatin1String("filter"))
        {
            return QVariant::String;
        }

        return 0;
    }
    return QtVariantPropertyManager::attributeType(propertyType, attribute);
}

QVariant VariantManager::attributeValue(const QtProperty *property, const QString &attribute)
{
    if (m_valuesMap.contains(property) == true)
    {
        if (attribute == QLatin1String("filter"))
        {
            return m_valuesMap[property].filter;
        }

        return QVariant();
    }

    return QtVariantPropertyManager::attributeValue(property, attribute);
}

QString VariantManager::valueText(const QtProperty *property) const
{
    if (m_valuesMap.contains(property) == true)
    {
        return m_valuesMap[property].value;
    }

    return QtVariantPropertyManager::valueText(property);
}

void VariantManager::setValue(QtProperty *property, const QVariant &val)
{
    if (m_valuesMap.contains(property) == true)
    {
        if (val.type() != QVariant::String && !val.canConvert(QVariant::String))
        {
            return;
        }

        QString str = qVariantValue<QString>(val);
        Data d = m_valuesMap[property];

        if (d.value == str)
        {
            return;
        }

        d.value = str;
        m_valuesMap[property] = d;

        emit propertyChanged(property);
        emit valueChanged(property, str);

        return;
    }

    QtVariantPropertyManager::setValue(property, val);
}

void VariantManager::setAttribute(QtProperty *property, const QString &attribute, const QVariant &val)
{
    if (m_valuesMap.contains(property) == true)
    {
        if (attribute == QLatin1String("filter"))
        {
            if (val.type() != QVariant::String && val.canConvert(QVariant::String) == false)
            {
                return;
            }

            QString str = qVariantValue<QString>(val);
            Data d = m_valuesMap[property];

            if (d.filter == str)
            {
                return;
            }

            d.filter = str;
            m_valuesMap[property] = d;

            emit attributeChanged(property, attribute, str);
        }
        return;
    }

    QtVariantPropertyManager::setAttribute(property, attribute, val);
}

void VariantManager::initializeProperty(QtProperty *property)
{
    if (propertyType(property) == filePathTypeId())
    {
        m_valuesMap[property] = Data();
    }

    QtVariantPropertyManager::initializeProperty(property);
}

void VariantManager::uninitializeProperty(QtProperty *property)
{
    m_valuesMap.remove(property);

    QtVariantPropertyManager::uninitializeProperty(property);
}

// -------------------------------------------------------------------------------------------------------------------



