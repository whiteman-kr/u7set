#include "ChooseTagsWidget.h"

//
// ChooseTagsWidget
//

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

ChooseTagsWidget::ChooseTagsWidget(const std::vector<std::pair<QString, QString>>& tags,
								   const std::vector<OnlineLib::MatsUser>& users,
								   QWidget* parent,
								   QChar separator) :
    QWidget(parent),
    m_tags(tags),
	m_users(users),
	m_separator(separator)
{
    if (m_tags.empty() == true && m_users.empty() == false)
	{
        m_objectName = "User";
        m_objectNames = "users";
	}

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
    m_list = new QTreeWidget();
    m_list->setRootIsDecorated(false);

    QStringList l;
    l << m_objectName;
    l << tr("Description");
    m_list->setHeaderLabels(l);
    m_list->setColumnCount(static_cast<int>(l.size()));

    m_list->setHeaderHidden(hasDescriptions == false);
    m_list->setColumnHidden(1, hasDescriptions == false);

    connect(m_list, &QTreeWidget::itemChanged, this, &ChooseTagsWidget::tagsListItemChanged);
    connect(m_list, &QTreeWidget::itemPressed, this, &ChooseTagsWidget::tagsListItemPressed);

    // Buttons and Filter Layout
    //
    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    m_filterEdit = new QLineEdit();
    m_filterEdit->setClearButtonEnabled(true);
    m_filterEdit->setPlaceholderText(tr("Filter"));
    m_filterEdit->setToolTip(tr("Start typing to filter %1").arg(m_objectNames));
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
	mainLayout->addWidget(new QLabel(tr("Predefined %1:").arg(m_objectNames)));
    mainLayout->addWidget(m_list);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    return;
}

void ChooseTagsWidget::fillTags()
{
    QString filterText = m_filterEdit->text();

    m_list->clear();

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

        m_list->addTopLevelItem(item);
    }

    for (const OnlineLib::MatsUser& user: m_users)
    {
        if (filterText.isEmpty() == false)
        {
			if (user.login().contains(filterText, Qt::CaseInsensitive) == false)
            {
                continue;
            }
        }

        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, user.login());
        item->setText(1, user.description());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Unchecked);

        m_list->addTopLevelItem(item);
    }

    m_list->resizeColumnToContents(0);

    updateChecks(m_textEdit->text());

    return;
}

void ChooseTagsWidget::updateChecks(const QString& text)
{
    // Get exitsing tags

    QStringList textTags;
	if (m_separator == QChar::Null)
	{
    	textTags = text.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
	}
	else
	{
        textTags = text.split(m_separator, Qt::SkipEmptyParts);
	}
    for (QString& t : textTags)
    {
        t = t.trimmed();
    }

    // Check existing tags in list

    m_list->blockSignals(true);

    int count = m_list->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
        QTreeWidgetItem* item = m_list->topLevelItem(i);

        if (textTags.contains(item->text(0)) == true)
        {
            item->setCheckState(0, Qt::Checked);
        }
        else
        {
            item->setCheckState(0, Qt::Unchecked);
        }
    }

    m_list->blockSignals(false);

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

    int count = m_list->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
        QTreeWidgetItem* item = m_list->topLevelItem(i);
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
    m_textEdit->setText(resultTags.join(m_separator == QChar::Null ? ' ' : m_separator));
    m_textEdit->blockSignals(false);

    return;
}
