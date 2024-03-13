#include "DialogMatsUsersEditor.h"
#include "../lib/Ui/ChooseTagsWidget.h"


//
//
// MatsUsersEditorDelegate
//
//
 MatsUsersEditorDelegate::MatsUsersEditorDelegate(QObject* parent) :
	QItemDelegate(parent)
{
}

QWidget* MatsUsersEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == static_cast<int>(DialogMatsUsersEditor::Columns::Login))
    {
        QLineEdit* edit = new QLineEdit(parent);

		QRegularExpression rx("^[A-Za-z][A-Za-z\\d]*$");
		edit->setValidator(new QRegularExpressionValidator(rx, edit));

        return edit;
    }

	if(index.column() == static_cast<int>(DialogMatsUsersEditor::Columns::Description))
	{
		return QItemDelegate::createEditor(parent, option, index);
	}

	return nullptr;
}

void MatsUsersEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == static_cast<int>(DialogMatsUsersEditor::Columns::Login) || 
		index.column() == static_cast<int>(DialogMatsUsersEditor::Columns::Description))
	{
        QString s = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
        edit->setText(s);
    }
    else
    {
        QItemDelegate::setEditorData(editor, index);
    }
}

void MatsUsersEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == static_cast<int>(DialogMatsUsersEditor::Columns::Login) || 
		index.column() == static_cast<int>(DialogMatsUsersEditor::Columns::Description))
    {
        QLineEdit* edit = qobject_cast<QLineEdit*>(editor);
        model->setData(index, edit->text(), Qt::EditRole);
    }
    else
    {
        QItemDelegate::setModelData(editor, model, index);
    }
}

//
//
// DialogMatsUsersEditor
//
//

DialogMatsUsersEditor::DialogMatsUsersEditor(DbController *pDbController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_dbController(pDbController)
{
	assert(db());

	setWindowTitle(tr("MATS User Editor"));

	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	m_list = new QTreeWidget();
	m_list->setColumnCount(4);
	m_list->setRootIsDecorated(false);
	QStringList l;
	l << tr("Login");
	l << tr("Description");
	l << tr("Enabled");
	l << tr("TuningTags");
	m_list->setHeaderLabels(l);
	m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::Login), 70);
    m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::Description), 130);
	m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::Enabled), 70);
    m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::TuningTags), 130);
	connect(m_list, &QTreeWidget::itemChanged, this, &DialogMatsUsersEditor::onListItemChanged);

	m_editorDelegate = new MatsUsersEditorDelegate(this);
	m_list->setItemDelegate(m_editorDelegate);

	connect(m_list, &QTreeWidget::itemDoubleClicked, this, &DialogMatsUsersEditor::onListItemDoubleClicked);

	QVBoxLayout* addRemoveLayout = new QVBoxLayout();
	addRemoveLayout->setContentsMargins(0, 0, 0, 0);

	QPushButton* b = new QPushButton(tr("Add"));
	connect(b, &QPushButton::clicked, this, &DialogMatsUsersEditor::onAddClicked);
	addRemoveLayout->addWidget(b);

	b = new QPushButton(tr("Remove"));
	connect(b, &QPushButton::clicked, this, &DialogMatsUsersEditor::onRemoveClicked);
	addRemoveLayout->addWidget(b);

	addRemoveLayout->addStretch();

	QHBoxLayout* topLayout = new QHBoxLayout();

	topLayout->addWidget(m_list);
	topLayout->addLayout(addRemoveLayout);

	//
	QHBoxLayout* okCancelLayout = new QHBoxLayout();
	okCancelLayout->setContentsMargins(0, 0, 0, 0);

	okCancelLayout->addStretch();

	b = new QPushButton(tr("Ok"));
	connect(b, &QPushButton::clicked, this, &DialogMatsUsersEditor::onOkClicked);
	okCancelLayout->addWidget(b);

	b = new QPushButton(tr("Cancel"));
	connect(b, &QPushButton::clicked, this, &DialogMatsUsersEditor::onCancelClicked);
	okCancelLayout->addWidget(b);

	
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(okCancelLayout);

	//
	QString errorCode;

	Builder::DbMatsUserStorage storage;

	if (storage.load(db(), errorCode) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load MATS users!"));
		return;
	}

	for (int i = 0; i < storage.count(); i++)
	{
		const auto& user = storage.get(i);

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << user.login() << user.description() << QString() << user.appSignalTagsToString(' '));
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setCheckState(static_cast<int>(DialogMatsUsersEditor::Columns::Enabled), user.enabled() ? Qt::Checked : Qt::Unchecked);
		item->setData(0, Qt::UserRole, i);
		m_list->insertTopLevelItem(i, item);
	}

	QSettings settings;
	QByteArray ba = settings.value("DialogMatsUsersEditor/Geometry").toByteArray();
	if (ba.isEmpty() == false)
	{
		restoreGeometry(ba);
	}
	else
	{
		// Resize depends on monitor size, DPI, resolution
		//
		QRect screen = parentWidget()->screen()->availableGeometry();

		resize(static_cast<int>(screen.width() * 0.30),
			   static_cast<int>(screen.height() * 0.60));
		move(screen.center() - rect().center());
	}

	return;
}

DialogMatsUsersEditor::~DialogMatsUsersEditor()
{
	QSettings settings;

	settings.setValue("DialogMatsUsersEditor/Geometry", saveGeometry());
}

void DialogMatsUsersEditor::showEvent(QShowEvent*)
{
	// --
	//
	assert(m_list);
	assert(m_list->columnCount() == 4);

	m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::Login), static_cast<int>(m_list->width() * 0.15));
	m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::Description), static_cast<int>(m_list->width() * 0.30));
	m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::Enabled), static_cast<int>(m_list->width() * 0.15));
	m_list->setColumnWidth(static_cast<int>(DialogMatsUsersEditor::Columns::TuningTags), static_cast<int>(m_list->width() * 0.30));

	return;
}

bool DialogMatsUsersEditor::askForSaveChanged()
{
	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, "MATS User Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (result == QMessageBox::Yes)
	{
		if (saveChanges() == false)
		{
			return false;
		}
		return true;
	}

	if (result == QMessageBox::No)
	{
		return true;
	}

	return false;
}

bool DialogMatsUsersEditor::saveChanges()
{
	Builder::DbMatsUserStorage storage;

	for (int i = 0; i < m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_list->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return false;
		}

        QString login = item->text(static_cast<int>(DialogMatsUsersEditor::Columns::Login));

        int count = storage.count();
        for (int s = 0; s < count; s++)
        {
            const auto& user = storage.get(s);

            if (user.login() == login)
            {
                QMessageBox::warning(this, "MATS User Editor", tr("Login '%1' already exists!").arg(login));
                return false;
            }
        }

		OnlineLib::MatsUser user{login, item->text(static_cast<int>(DialogMatsUsersEditor::Columns::Description))};
		user.setEnabled(item->checkState(static_cast<int>(DialogMatsUsersEditor::Columns::Enabled)) == Qt::Checked);
		user.setAppSignalTagsFromString(item->text(static_cast<int>(DialogMatsUsersEditor::Columns::TuningTags)));
		storage.add(user);
	}


    bool ok;
    QString comment = QInputDialog::getText(this, tr("MATS User Editor"),
                                            tr("Please enter comment:"), QLineEdit::Normal,
                                            tr("comment"), &ok);

    if (ok == false)
    {
        return false;
    }
    if (comment.isEmpty())
    {
		QMessageBox::warning(this, "MATS User Editor", "No comment supplied! Please provide a comment.");
        return false;
    }

    // save to db
	//
	if (storage.save(db(), comment) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save MATS users."));
		return false;
	}

	m_modified = false;

	return true;
}

DbController* DialogMatsUsersEditor::db()
{
	return m_dbController;
}

void DialogMatsUsersEditor::closeEvent(QCloseEvent* e)
{
	if (askForSaveChanged() == true)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
}

void DialogMatsUsersEditor::onAddClicked()
{
	// --
	//
	int index = -1;

	QList<QTreeWidgetItem*> items = m_list->selectedItems();
	if (items.size() != 1)
	{
		index = m_list->topLevelItemCount();
	}
	else
	{
		index = items[0]->data(0, Qt::UserRole).toInt() + 1;
	}

	// Create unique login
	//
	QString login;
	{
		QStringList existingLogins;
		for (int i = 0; i < m_list->topLevelItemCount(); i++)
		{
			existingLogins.push_back(m_list->topLevelItem(i)->text(static_cast<int>(DialogMatsUsersEditor::Columns::Login)));
		}
		int userIndex = 1;
		do
		{
			login = tr("User%1").arg(userIndex++);
		} while (existingLogins.contains(login) == true);
	}

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << login << tr("Description") << QString() << QString());
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	item->setCheckState(static_cast<int>(DialogMatsUsersEditor::Columns::Enabled), Qt::Checked);
	m_list->insertTopLevelItem(index, item);

	// Renumber indexes
	//
	for (int i = 0; i < m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* ti = m_list->topLevelItem(i);
		ti->setData(0, Qt::UserRole, i);
	}

	// Select the created element
	//
	m_list->clearSelection();
	item->setSelected(true);

	m_modified = true;
}

void DialogMatsUsersEditor::onRemoveClicked()
{
	int index = -1;

	QList<QTreeWidgetItem*> items = m_list->selectedItems();
	if (items.size() != 1)
	{
		return;
	}
	else
	{
		index = m_list->indexOfTopLevelItem(items[0]);
	}

	QTreeWidgetItem* deletedItem = m_list->takeTopLevelItem(index);
	delete deletedItem;

	// Renumber indexes
	//
	for (int i = 0; i < m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* ti = m_list->topLevelItem(i);
		ti->setData(0, Qt::UserRole, i);
	}

	if (m_list->topLevelItemCount() > 0 && index != -1)
    {
        m_list->selectionModel()->select(m_list->model()->index (index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

	m_modified = true;
}

void DialogMatsUsersEditor::onOkClicked()
{
	if (m_modified == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}

	accept();
	return;
}

void DialogMatsUsersEditor::onCancelClicked()
{
	if (askForSaveChanged() == true)
	{
		reject();
	}
	return;
}

void DialogMatsUsersEditor::onListItemChanged(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);
	m_modified = true;
}

void DialogMatsUsersEditor::onListItemDoubleClicked(QTreeWidgetItem* item, int column)
{
	if (item == nullptr || column != static_cast<int>(DialogMatsUsersEditor::Columns::TuningTags))
	{
		return;
	}

	if (m_dbController == nullptr)
	{
		Q_ASSERT(m_dbController);
		return;
	}

    std::vector<std::pair<QString, QString>> tags;
	{
		std::vector<DbTag> dbTags;
		bool ok = m_dbController->getTags(&dbTags);
		if (ok == true)
		{
			tags.reserve(dbTags.size());
			for (const DbTag& dbt : dbTags)
			{
				tags.push_back({dbt.tag, dbt.description});
			}
		}
	}
	
	QDialog tagsSelectorDialog{this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint};

	ChooseTagsWidget te{tags, {}, this};
	te.setText(item->text(static_cast<int>(DialogMatsUsersEditor::Columns::TuningTags)));

	connect(&te, &ChooseTagsWidget::okPressed, &tagsSelectorDialog, &QDialog::accept);
	connect(&te, &ChooseTagsWidget::cancelPressed, &tagsSelectorDialog, &QDialog::reject);

	QHBoxLayout l;
	l.addWidget(&te);
	tagsSelectorDialog.setLayout(&l);

	if (tagsSelectorDialog.exec() == QDialog::Accepted)
	{
		item->setText(static_cast<int>(DialogMatsUsersEditor::Columns::TuningTags), te.text());
		m_modified = true;
	}
}

