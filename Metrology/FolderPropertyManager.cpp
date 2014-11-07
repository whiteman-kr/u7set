#include "FolderPropertyManager.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QFocusEvent>

// -------------------------------------------------------------------------------------------------------------------

FolderEdit::FolderEdit(QWidget *parent)
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

    connect(m_edit, &QLineEdit::textEdited, this, &FolderEdit::folderPathChanged);
    connect(m_button, &QToolButton::clicked, this, &FolderEdit::buttonClicked);
}

void FolderEdit::buttonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"), m_edit->text());
    if (path.isEmpty() == true)
    {
        return;
    }

    m_edit->setText(path);

    emit folderPathChanged(path);
}

void FolderEdit::focusInEvent(QFocusEvent *e)
{
    m_edit->event(e);

    if (e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason)
    {
        m_edit->selectAll();
    }

    QWidget::focusInEvent(e);
}

void FolderEdit::focusOutEvent(QFocusEvent *e)
{
    m_edit->event(e);

    QWidget::focusOutEvent(e);
}

void FolderEdit::keyPressEvent(QKeyEvent *e)
{
    m_edit->event(e);
}

void FolderEdit::keyReleaseEvent(QKeyEvent *e)
{
    m_edit->event(e);
}

// -------------------------------------------------------------------------------------------------------------------

VariantFactory::~VariantFactory()
{
    QList<FolderEdit *> editors = m_editorToPropertyMap.keys();
    QListIterator<FolderEdit *> it(editors);

    while (it.hasNext() == true)
    {
        delete it.next();
    }
}

void VariantFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, &QtVariantPropertyManager::valueChanged, this, &VariantFactory::slotPropertyChanged);

    QtVariantEditorFactory::connectPropertyManager(manager);
}

void VariantFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    disconnect(manager, &QtVariantPropertyManager::valueChanged, this, &VariantFactory::slotPropertyChanged);

    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

QWidget *VariantFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent)
{
    if (manager->propertyType(property) == VariantManager::folerPathTypeId())
    {
        FolderEdit *editor = new FolderEdit(parent);

        editor->setFolderPath(manager->value(property).toString());

        m_createdEditorsMap[property].append(editor);
        m_editorToPropertyMap[editor] = property;

        connect(editor, &FolderEdit::folderPathChanged, this, &VariantFactory::slotSetValue);
        connect(editor, &FolderEdit::destroyed, this, &VariantFactory::slotEditorDestroyed);

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

    QList<FolderEdit *> editors = m_createdEditorsMap[property];
    QListIterator<FolderEdit *> itEditor(editors);

    while (itEditor.hasNext() == true)
    {
        itEditor.next()->setFolderPath(value.toString());
    }
}

void VariantFactory::slotSetValue(const QString &value)
{
    QObject *object = sender();
    QMap<FolderEdit *, QtProperty *>::ConstIterator itEditor = m_editorToPropertyMap.constBegin();

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
    QMap<FolderEdit *, QtProperty *>::ConstIterator itEditor = m_editorToPropertyMap.constBegin();

    while (itEditor != m_editorToPropertyMap.constEnd())
    {
        if (itEditor.key() == object)
        {
            FolderEdit *editor = itEditor.key();
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

class FolderPathPropertyType
{
};

Q_DECLARE_METATYPE(FolderPathPropertyType)

int VariantManager::folerPathTypeId()
{
    return qMetaTypeId<FolderPathPropertyType>();
}

// -------------------------------------------------------------------------------------------------------------------

bool VariantManager::isPropertyTypeSupported(int propertyType) const
{
    if (propertyType == folerPathTypeId())
    {
        return true;
    }

    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int VariantManager::valueType(int propertyType) const
{
    if (propertyType == folerPathTypeId())
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

void VariantManager::initializeProperty(QtProperty *property)
{
    if (propertyType(property) == folerPathTypeId())
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



