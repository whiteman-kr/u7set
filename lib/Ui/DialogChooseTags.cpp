#include "DialogChooseTags.h"

QString ChooseTagsWidget::m_filterText = QString();

ChooseTagsWidget::ChooseTagsWidget(const QStringList &tags, QWidget* parent):
    QWidget(parent)
{
    for (const QString& s : tags)
    {
        m_tags.push_back({s, QString()});
    }

    setupUi(false/*hasDescriptions*/);

    fillTags();

    return;
}

ChooseTagsWidget::ChooseTagsWidget(const std::vector<std::pair<QString, QString>>& tags, QWidget* parent):
    QWidget(parent),
    m_tags(tags)
{
    setupUi(true/*hasDescriptions*/);

    fillTags();

    return;
}

ChooseTagsWidget::~ChooseTagsWidget()
{
    m_filterText = m_filterEdit->text();
}

QString ChooseTagsWidget::text() const
{
    return m_textEdit->text();
}

void ChooseTagsWidget::setText(const QString& text)
{
    m_textEdit->blockSignals(true);
    m_textEdit->setText(text);
    m_textEdit->blockSignals(false);

    updateChecks(text);

    return;
}

bool ChooseTagsWidget::readOnly() const
{
    return m_textEdit->isReadOnly();
}

void ChooseTagsWidget::setReadOnly(bool value)
{
    m_textEdit->setReadOnly(value);
    m_okButton->setEnabled(value == false);
}

void ChooseTagsWidget::tagsTextChanged(const QString& text)
{
    updateChecks(text);

    return;
}
void ChooseTagsWidget::tagsListItemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(item);
    Q_UNUSED(column);

    updateTags();

    return;
}

void ChooseTagsWidget::tagsListItemPressed(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (item == nullptr)
    {
        Q_ASSERT(item);
        return;
    }

    if (item->checkState(0) == Qt::Checked)
    {
        item->setCheckState(0, Qt::Unchecked);
    }
    else
    {
        item->setCheckState(0, Qt::Checked);
    }

    return;
}

void ChooseTagsWidget::filterTextChanged(const QString& text)
{
    Q_UNUSED(text);

    fillTags();

    return;
}

void ChooseTagsWidget::setupUi(bool hasDescriptions)
{
    // TextEditor
    //
    m_textEdit = new QLineEdit();
    connect(m_textEdit, &QLineEdit::textChanged, this, &ChooseTagsWidget::tagsTextChanged);

    // Tags list
    //
    m_tagsList = new QTreeWidget();
    m_tagsList->setRootIsDecorated(false);

    QStringList l;
    l << tr("Tag");
    l << tr("Description");
    m_tagsList->setHeaderLabels(l);
    m_tagsList->setColumnCount(static_cast<int>(l.size()));

    m_tagsList->setHeaderHidden(hasDescriptions == false);
    m_tagsList->setColumnHidden(1, hasDescriptions == false);

    connect(m_tagsList, &QTreeWidget::itemChanged, this, &ChooseTagsWidget::tagsListItemChanged);
    connect(m_tagsList, &QTreeWidget::itemPressed, this, &ChooseTagsWidget::tagsListItemPressed);

    // Buttons and Filter Layout
    //
    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    m_filterEdit = new QLineEdit();
    m_filterEdit->setClearButtonEnabled(true);
    m_filterEdit->setPlaceholderText(tr("Filter"));
    m_filterEdit->setToolTip(tr("Start typing to filter tags"));
    m_filterEdit->setText(m_filterText);
    connect(m_filterEdit, &QLineEdit::textEdited, this, &ChooseTagsWidget::filterTextChanged);
    buttonsLayout->addWidget(m_filterEdit);

    buttonsLayout->addStretch();

    m_okButton = new QPushButton(tr("OK"));
    connect(m_okButton, &QPushButton::clicked, [this](){
        emit okPressed();});
    buttonsLayout->addWidget(m_okButton);

    m_cancelButton = new QPushButton(tr("Cancel"));
    connect(m_cancelButton, &QPushButton::clicked, [this](){
        emit cancelPressed();});
    buttonsLayout->addWidget(m_cancelButton);

    // Main Layout
    //
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_textEdit);
    mainLayout->addWidget(new QLabel("Predefined tags:"));
    mainLayout->addWidget(m_tagsList);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    return;
}

void ChooseTagsWidget::fillTags()
{
    QString filterText = m_filterEdit->text();

    m_tagsList->clear();

    for (const std::pair<QString, QString>& tagPair :  m_tags)
    {
        if (filterText.isEmpty() == false)
        {
            if (tagPair.first.contains(filterText, Qt::CaseInsensitive) == false)
            {
                continue;
            }
        }

        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, tagPair.first);
        item->setText(1, tagPair.second);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Unchecked);

        m_tagsList->addTopLevelItem(item);
    }

    m_tagsList->resizeColumnToContents(0);

    updateChecks(m_textEdit->text());

    return;
}

void ChooseTagsWidget::updateChecks(const QString& text)
{
    // Get exitsing tags

    QStringList textTags = text.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    for (QString& t : textTags)
    {
        t = t.trimmed();
    }

    // Check existing tags in list

    m_tagsList->blockSignals(true);

    int count = m_tagsList->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
        QTreeWidgetItem* item = m_tagsList->topLevelItem(i);

        if (textTags.contains(item->text(0)) == true)
        {
            item->setCheckState(0, Qt::Checked);
        }
        else
        {
            item->setCheckState(0, Qt::Unchecked);
        }
    }

    m_tagsList->blockSignals(false);

    return;
}

void ChooseTagsWidget::updateTags()
{
    const QString& text = m_textEdit->text();

    // Get exitsing tags

    QStringList tags = text.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    for (QString& t : tags)
    {
        t = t.trimmed();
    }

    // Build tag checks map

    std::map<QString, bool> tagsListState;

    int count = m_tagsList->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
        QTreeWidgetItem* item = m_tagsList->topLevelItem(i);
        if (item == nullptr)
        {
            Q_ASSERT(item);
            continue;
        }

        tagsListState[item->text(0)] = item->checkState(0) == Qt::Checked;
    }

    // Add manually added and previously checked tags to the result

    QStringList resultTags;

    for (const QString& tag : tags)
    {
        auto it = tagsListState.find(tag);
        if (it == tagsListState.end())
        {
            resultTags.push_back(tag);
        }
        else
        {
            bool checked = it->second;
            if (checked == true)
            {
                resultTags.push_back(tag);
            }
        }
    }

    // Add newly checked tags to the result

    for (const auto& it : tagsListState)
    {
        bool checked = it.second;
        if (checked == true)
        {
            const QString& tag = it.first;
            if (resultTags.contains(tag) == false)
            {
                resultTags.push_back(tag);
            }
        }
    }

    m_textEdit->blockSignals(true);
    m_textEdit->setText(resultTags.join(' '));
    m_textEdit->blockSignals(false);

    return;
}
