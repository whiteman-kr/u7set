#include "DialogConnections.h"
#include "../lib/PropertyEditorDialog.h"
#include "Settings.h"
#include <QMessageBox>
#include <QFileDialog>

DialogConnections* theDialogConnections = nullptr;

DialogConnections::DialogConnections(DbController *pDbController, QWidget *parent)
    :QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
      m_dbController(pDbController)
{

    setWindowTitle(tr("Connections Editor"));

    setAttribute(Qt::WA_DeleteOnClose);

    // Create user interface
    //

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* maskLayout = new QHBoxLayout();

    m_mask = new QLineEdit();

    m_maskApply = new QPushButton(tr("Search"));

    connect(m_mask, &QLineEdit::returnPressed, this, &DialogConnections::onMaskReturn);
    connect(m_maskApply, &QPushButton::clicked, this, &DialogConnections::onMaskApply);

    m_connectionsTree = new QTreeWidget();

    QStringList l;
    l << tr("#");
    l << tr("ConnectionID");
    l << tr("Type");
    l << tr("State");
    l << tr("Action");

    m_connectionsTree->setColumnCount(l.size());
    m_connectionsTree->setHeaderLabels(l);

    int il = 0;
    m_connectionsTree->setColumnWidth(il++, 50);
    m_connectionsTree->setColumnWidth(il++, 140);
    m_connectionsTree->setColumnWidth(il++, 80);
    m_connectionsTree->setColumnWidth(il++, 40);
    m_connectionsTree->setColumnWidth(il++, 40);
    m_connectionsTree->setSortingEnabled(true);
    m_connectionsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_connectionsTree->setContextMenuPolicy(Qt::CustomContextMenu);

    m_connectionsTree->setRootIsDecorated(false);

    connect(m_connectionsTree, &QTreeWidget::itemSelectionChanged, this, &DialogConnections::onItemSelectionChanged);
    connect(m_connectionsTree, &QWidget::customContextMenuRequested, this, &DialogConnections::onCustomContextMenuRequested);

    m_connectionPropertyEditor = new ExtWidgets::PropertyEditor(this);

    connect(m_connectionPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogConnections::onPropertiesChanged);

    maskLayout->addWidget(m_mask);
    maskLayout->addWidget(m_maskApply);

    m_splitter = new QSplitter(Qt::Horizontal);

    m_splitter->addWidget(m_connectionsTree);
    m_splitter->addWidget(m_connectionPropertyEditor);

    m_splitter->setChildrenCollapsible(false);


    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    m_btnAdd = new QPushButton(tr("Add"));
    m_btnRemove = new QPushButton(tr("Remove"));
    m_btnCheckOut = new QPushButton(tr("Check Out"));
    m_btnCheckIn = new QPushButton(tr("Check In"));
    m_btnUndo = new QPushButton(tr("Undo"));
    m_btnExport = new QPushButton(tr("Export"));
    m_btnClose = new QPushButton(tr("Close"));

    buttonsLayout->addWidget(m_btnAdd);
    buttonsLayout->addWidget(m_btnRemove);
    buttonsLayout->addWidget(m_btnCheckOut);
    buttonsLayout->addWidget(m_btnCheckIn);
    buttonsLayout->addWidget(m_btnUndo);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_btnExport);
    buttonsLayout->addWidget(m_btnClose);

    connect (m_btnAdd, &QPushButton::clicked, this, &DialogConnections::onAdd);
    connect (m_btnRemove, &QPushButton::clicked, this, &DialogConnections::onRemove);
    connect (m_btnCheckOut, &QPushButton::clicked, this, &DialogConnections::onCheckOut);
    connect (m_btnCheckIn, &QPushButton::clicked, this, &DialogConnections::onCheckIn);
    connect (m_btnUndo, &QPushButton::clicked, this, &DialogConnections::onUndo);
    connect (m_btnExport, &QPushButton::clicked, this, &DialogConnections::onExport);
    connect (m_btnClose, &QPushButton::clicked, this, &DialogConnections::close);

    mainLayout->addLayout(maskLayout);
    mainLayout->addWidget(m_splitter);
    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    // Mask and completer
    //
    m_completer = new QCompleter(theSettings.m_connectionEditorMasks, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_mask->setCompleter(m_completer);

    connect(m_mask, &QLineEdit::textEdited, [=](){m_completer->complete();});
    connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_mask, &QLineEdit::setText);

    // Popup menu
    //

    m_addAction = new QAction(tr("Add"), this);
    m_removeAction = new QAction(tr("Remove"), this);
    m_checkOutAction = new QAction(tr("Check Out"), this);
    m_checkInAction = new QAction(tr("Check In"), this);
    m_undoAction = new QAction(tr("Undo"), this);

    connect(m_addAction, &QAction::triggered, this, &DialogConnections::onAdd);
    connect(m_removeAction, &QAction::triggered, this, &DialogConnections::onRemove);
    connect(m_checkOutAction, &QAction::triggered, this, &DialogConnections::onCheckOut);
    connect(m_checkInAction, &QAction::triggered, this, &DialogConnections::onCheckIn);
    connect(m_undoAction, &QAction::triggered, this, &DialogConnections::onUndo);

    m_popupMenu = new QMenu(this);
    m_popupMenu->addAction(m_addAction);
    m_popupMenu->addAction(m_removeAction);
    m_popupMenu->addSeparator();
    m_popupMenu->addAction(m_checkOutAction);
    m_popupMenu->addAction(m_checkInAction);
    m_popupMenu->addAction(m_undoAction);


    // Load connections
    //
    QString errorCode;

    if (connections.load(m_dbController, errorCode) == false)
    {
        QMessageBox::critical(parent, QString("Error"), tr("Can't load connections!"));
        return;
    }

    // Load deprecated connections
    //

    Hardware::ConnectionStorage xmlConnections;

    if (xmlConnections.loadFromXmlDeprecated(m_dbController, errorCode) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't load connections from Connections.xml!"));
        return;
    }

    if (xmlConnections.count() > 0)
    {
        QMessageBox::warning(parent, QString("Warning"), tr("%1 connections have been imported from deprecated file Connections.xml.").arg(xmlConnections.count()));

        for (int i = 0; i < xmlConnections.count(); i++)
        {
            std::shared_ptr<Hardware::Connection> c = xmlConnections.get(i);

            connections.add(c);

            c->save(m_dbController);
        }

        if (xmlConnections.deleteXmlDeprecated(m_dbController) == false)
        {
            QMessageBox::critical(this, QString("Error"), tr("Can't delete file Connections.xml!"));
        }
    }

    // fill data
    //

    fillConnectionsList();

    updateButtonsEnableState();

    // sort items
    //

    for (int i = 0; i < m_connectionsTree->columnCount(); i++)
    {
        m_connectionsTree->resizeColumnToContents(i);
    }

    m_connectionsTree->sortByColumn(theSettings.m_connectionEditorSortColumn, theSettings.m_connectionEditorSortOrder);

    connect(m_connectionsTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogConnections::onSortIndicatorChanged);


    // Restore settings
    //

    if (theSettings.m_connectionEditorWindowPos.x() != -1 && theSettings.m_connectionEditorWindowPos.y() != -1)
    {
        move(theSettings.m_connectionEditorWindowPos);
        restoreGeometry(theSettings.m_connectionEditorWindowGeometry);

        m_splitter->restoreState(theSettings.m_connectionEditorSplitterState);
        m_connectionPropertyEditor->setSplitterPosition(theSettings.m_connectionEditorPeSplitterPosition);
    }

}

DialogConnections::~DialogConnections()
{
    theSettings.m_connectionEditorWindowPos = pos();
    theSettings.m_connectionEditorWindowGeometry = saveGeometry();
    theSettings.m_connectionEditorSplitterState = m_splitter->saveState();
    theSettings.m_connectionEditorPeSplitterPosition = m_connectionPropertyEditor->splitterPosition();

    theDialogConnections = nullptr;
}

void DialogConnections::onMaskReturn()
{
    onMaskApply();
}

void DialogConnections::onMaskApply()
{
    // Get mask
    //
    QString maskText = m_mask->text();

    if (maskText.isEmpty() == false)
    {
        m_masks = maskText.split(';');

        for (auto mask : m_masks)
        {
            // Save filter history
            //
            if (theSettings.m_connectionEditorMasks.contains(mask) == false)
            {
                theSettings.m_connectionEditorMasks.append(mask);

                QStringListModel* model = dynamic_cast<QStringListModel*>(m_completer->model());
                if (model == nullptr)
                {
                    assert(model);
                    return;
                }
                model->setStringList(theSettings.m_connectionEditorMasks);
            }
        }
    }
    else
    {
        m_masks.clear();
    }

    fillConnectionsList();
}

void DialogConnections::fillConnectionsList()
{
    m_connectionsTree->clear();

    for (int i = 0; i < connections.count(); i++)
    {
        std::shared_ptr<Hardware::Connection> connection = connections.get(i);
        if (connection == nullptr)
        {
            assert(connection);
            break;
        }

        if (connection->mode() != Hardware::OptoPort::Mode::Optical)
        {
            continue;
        }

        if (m_masks.empty() == false)
        {
            QString s;
            bool result = false;
            for (QString mask : m_masks)
            {
                if (connection->connectionID().contains(mask, Qt::CaseInsensitive))
                {
                    result = true;
                    break;
                }
                if (connection->port1EquipmentID().contains(mask, Qt::CaseInsensitive))
                {
                    result = true;
                    break;
                }
                if (connection->port2EquipmentID().contains(mask, Qt::CaseInsensitive))
                {
                    result = true;
                    break;
                }
                QString numIndex = QString::number(connection->index()).rightJustified(4, '0');
                if (numIndex.contains(mask))
                {
                    result = true;
                    break;
                }

            }
            if (result == false)
            {
                continue;
            }
        }


        QTreeWidgetItem* item = new QTreeWidgetItem();
        setConnectionText(item, connection.get());

        item->setData(0, Qt::UserRole, QVariant::fromValue(connection));
        m_connectionsTree->addTopLevelItem(item);
    }
}

void DialogConnections::setPropertyEditorObjects()
{
    QList <QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

    if (selectedItems.isEmpty() == true)
    {
        m_connectionPropertyEditor->clear();

        updateButtonsEnableState();

        return;
    }

    bool readOnly = true;

    QList<std::shared_ptr<PropertyObject>> objects;

    for (auto item : selectedItems)
    {
        std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        VcsState state;
        VcsItemAction action;
        bool ok = connection->vcsStateAndAction(m_dbController, state, action);
        if (ok == false)
        {
            continue;
        }

        if (state == VcsState::CheckedOut)
        {
            readOnly = false;
        }

        objects.push_back(connection);
    }

    m_connectionPropertyEditor->setExpertMode(theSettings.isExpertMode());

    m_connectionPropertyEditor->setReadOnly(readOnly);

    m_connectionPropertyEditor->setObjects(objects);
}

bool DialogConnections::continueWithDuplicateCaptions()
{
    bool duplicated = false;
    QString duplicatedCaption;

    for (int i = 0; i < connections.count(); i++)
    {
        Hardware::Connection* c = connections.get(i).get();
        if (c->mode() != Hardware::OptoPort::Mode::Optical)
        {
            continue;
        }

        for (int j = 0; j < connections.count(); j++)
        {
            Hardware::Connection* e = connections.get(j).get();

            if (i == j)
            {
                continue;
            }

            if (e->connectionID() == c->connectionID())
            {
                duplicated = true;
                duplicatedCaption = e->connectionID();
                break;
            }
        }
        if (duplicated == true)
        {
            break;
        }
    }

    if (duplicated == true)
    {
        QString s = QString("Connection with ID '%1' already exists.\r\n\r\nAre you sure you want to continue?").arg(duplicatedCaption);
        if (QMessageBox::warning(this, "Connections Editor", s, QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        {
            return false;
        }
    }


    return true;
}

void DialogConnections::onSortIndicatorChanged(int column, Qt::SortOrder order)
{
    theSettings.m_connectionEditorSortColumn = column;
    theSettings.m_connectionEditorSortOrder = order;
}

void DialogConnections::onItemSelectionChanged()
{
    updateButtonsEnableState();

    setPropertyEditorObjects();
}

void DialogConnections::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
    for (auto object : objects)
    {
        Hardware::Connection* c = (Hardware::Connection*)object.get();
        if (c == nullptr)
        {
            assert(c);
            continue;
        }

        if (c->save(m_dbController) == false)
        {
            QMessageBox::critical(this, tr("Error"), tr("Failed to save connection %1").arg(c->connectionID()));
            continue;
        }

        for (int i = 0; i < m_connectionsTree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* item = m_connectionsTree->topLevelItem(i);
            if (item == nullptr)
            {
                assert(item);
                continue;
            }

            std::shared_ptr<Hardware::Connection> itemConnection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
            if (itemConnection == nullptr)
            {
                assert(itemConnection);
                continue;
            }

            if (itemConnection->index() == c->index())
            {
                setConnectionText(item, c);
                break;
            }
        }
    }
}

void DialogConnections::onAdd()
{
    std::shared_ptr<Hardware::Connection> connection = std::make_shared<Hardware::Connection>();

    connection->setConnectionID(tr("Connection%1").arg(QString::number(m_dbController->nextCounterValue()).rightJustified(4, '0')));

    connections.add(connection);

    connection->save(m_dbController);

    QTreeWidgetItem* item = new QTreeWidgetItem();
    setConnectionText(item, connection.get());

    item->setData(0, Qt::UserRole, QVariant::fromValue(connection));
    m_connectionsTree->addTopLevelItem(item);

    updateButtonsEnableState();
}

void DialogConnections::onRemove()
{

    QList <QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

    if (selectedItems.isEmpty() == true)
    {
        return;
    }

    if (QMessageBox::warning(this, tr("Warning"), tr("Are you sure you want to remove selected connections?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    std::vector<std::shared_ptr<Hardware::Connection>> connectionsToDelete;

    for (auto item : selectedItems)
    {
        std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        bool fileRemoved = false;

        connection->remove(m_dbController, fileRemoved);

        if (fileRemoved == true)
        {
            connectionsToDelete.push_back(connection);
        }
        else
        {
            setConnectionText(item, connection.get());
        }
    }

    deleteConnections(connectionsToDelete);

    updateButtonsEnableState();

    setPropertyEditorObjects();
}

void DialogConnections::onCheckOut()
{
    QList <QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

    if (selectedItems.isEmpty() == true)
    {
        return;
    }

    for (auto item : selectedItems)
    {
        std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        if (connection->checkOut(m_dbController) == false)
        {
            QMessageBox::critical(this, tr("Error"), tr("Failed to check out connection %1").arg(connection->connectionID()));
            continue;
        }

        setConnectionText(item, connection.get());
    }

    updateButtonsEnableState();

    setPropertyEditorObjects();
}

void DialogConnections::onCheckIn()
{
    QList <QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

    if (selectedItems.isEmpty() == true)
    {
        return;
    }

    bool ok;
    QString comment = QInputDialog::getText(this, tr("Connections Editor"),
                                            tr("Please enter comment:"), QLineEdit::Normal,
                                            tr("comment"), &ok);

    if (ok == false)
    {
        return;
    }
    if (comment.isEmpty())
    {
        QMessageBox::warning(this, "Connections Editor", "No comment supplied!");
        return;
    }

    for (auto item : selectedItems)
    {
        std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        if (connection->checkIn(m_dbController, comment) == false)
        {
            QMessageBox::critical(this, tr("Error"), tr("Failed to check in connection %1").arg(connection->connectionID()));
            continue;
        }

        setConnectionText(item, connection.get());
    }

    updateButtonsEnableState();

    setPropertyEditorObjects();
}

void DialogConnections::onUndo()
{
    QList <QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

    if (selectedItems.isEmpty() == true)
    {
        return;
    }

    if (QMessageBox::warning(this, tr("Warning"), tr("Are you sure you want to undo changes on selected connections?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    std::vector<std::shared_ptr<Hardware::Connection>> connectionsToDelete;

    for (auto item : selectedItems)
    {
        std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }


        bool fileRemoved = false;

        if (connection->undo(m_dbController, fileRemoved) == false)
        {
            QMessageBox::critical(this, tr("Error"), tr("Failed to undo on connection %1").arg(connection->connectionID()));
            continue;
        }

        if (fileRemoved == true)
        {
            connectionsToDelete.push_back(connection);
        }
        else
        {
            setConnectionText(item, connection.get());
        }
    }

    deleteConnections(connectionsToDelete);

    updateButtonsEnableState();

    setPropertyEditorObjects();
}

void DialogConnections::deleteConnections(std::vector<std::shared_ptr<Hardware::Connection>>& connectionsToDelete)
{

    for (auto connectionToDelete : connectionsToDelete)
    {
        connections.remove(connectionToDelete);

        bool found = false;

        int count = m_connectionsTree->topLevelItemCount();
        for (int i = 0; i < count; i++)
        {

            QTreeWidgetItem* item = m_connectionsTree->topLevelItem(i);

            std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
            if (connection == nullptr)
            {
                assert(connection);
                return;
            }

            if (connection->index() == connectionToDelete->index())
            {
                QTreeWidgetItem* item = m_connectionsTree->takeTopLevelItem(i);
                delete item;

                found = true;

                break;
            }
        }

        if (found == false)
        {
            assert(false);
        }
    }


}

void DialogConnections::closeEvent(QCloseEvent* e)
{
    if (continueWithDuplicateCaptions() == true)
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void DialogConnections::reject()
{
    if (continueWithDuplicateCaptions() == true)
    {
        QDialog::reject();
    }
}

void DialogConnections::onExport()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export"),
                                                    "./",
                                                    tr("Text files (*.txt);; All files (*.*)"));

    if (fileName.isNull() == true)
    {
        return;
    }

    QFile file(fileName);

    if (file.open(QFile::WriteOnly) == false)
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create file %1!").arg(fileName));
        return;
    }

    QTextStream textStream(&file);

    textStream << tr("Radiy Platform Configuration Tool\r\n");
    textStream << qApp->applicationName() << tr(" v") << qApp->applicationVersion() << "\r\n";

    textStream << "\r\n";

    textStream << tr("Project Name:\t") << m_dbController->currentProject().projectName() << "\r\n";
    textStream << tr("User Name:\t") << m_dbController->currentUser().username() << "\r\n";

    textStream << "\r\n";



    textStream << tr("Generated at:\t") << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") << "\r\n";

    textStream << "\r\n";

    int count = m_connectionsTree->topLevelItemCount();

    textStream << tr("Connections count: ") << count << "\r\n\r\n";

    for (int i = 0; i < count; i++)
    {
        QTreeWidgetItem* item = m_connectionsTree->topLevelItem(i);
        if (item == nullptr)
        {
            assert(item);
            return;
        }

        std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        textStream << tr("------------ Connection ") << connection->index() << tr(" ------------\r\n\r\n");
        textStream << tr("ConnectionID: ") << connection->connectionID() << "\r\n\r\n";
        textStream << tr("Port1 EquipmentID: ") << connection->port1EquipmentID() << "\r\n";
        textStream << tr("Port2 EquipmentID: ") << connection->port2EquipmentID() << "\r\n";

        if (connection->mode() == Hardware::OptoPort::Mode::Optical)
        {
            textStream << tr("Port mode: Optical")<<"\r\n";
        }

        if (connection->mode() == Hardware::OptoPort::Mode::Serial)
        {
            textStream << tr("Port mode: Serial") << "\r\n";

            textStream << tr("Serial mode enabled: ") << (connection->enableSerial() == true ? tr("Yes") : tr("No")) <<"\r\n";

            textStream << tr("Serial mode: ") << (connection->serialMode() == Hardware::OptoPort::SerialMode::RS232  ? tr("RS232") : tr("RS485")) <<"\r\n";

        }

        if (connection->manualSettings() == true)
        {
            textStream << tr("\r\nManual settings:\r\n");
            textStream << tr("Port1 start address: ") << connection->port1ManualTxStartAddress()<<"\r\n";
            textStream << tr("Port1 TX words quantity: ") << connection->port1ManualTxWordsQuantity()<<"\r\n";
            textStream << tr("Port1 RX words quantity: ") << connection->port1ManualRxWordsQuantity()<<"\r\n";

            textStream << tr("Port2 start address: ") << connection->port2ManualTxStartAddress()<<"\r\n";
            textStream << tr("Port2 TX words quantity: ") << connection->port2ManualTxWordsQuantity()<<"\r\n";
            textStream << tr("Port2 RX words quantity: ") << connection->port2ManualRxWordsQuantity()<<"\r\n";
        }

        textStream<<"\r\n";
    }

    textStream.flush();

    file.close();
}

void DialogConnections::onCustomContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);

    m_popupMenu->exec(this->cursor().pos());
}


void DialogConnections::setConnectionText(QTreeWidgetItem* item, Hardware::Connection* connection)
{
    if (item == nullptr || connection == nullptr)
    {
        assert(item);
        assert(connection);
        return;
    }

    VcsState state;
    VcsItemAction action;
    bool ok = connection->vcsStateAndAction(m_dbController, state, action);
    if (ok == false)
    {
        return;
    }

    int c = 0;
    QString numString = QString::number(connection->index()).rightJustified(4, '0');
    item->setText(c++, numString);
    item->setText(c++, connection->connectionID());
    item->setText(c++, connection->manualSettings() ? tr("Manual") : tr("Auto"));
    item->setText(c++, state.text());
    item->setText(c++, action.text());
}

void DialogConnections::updateButtonsEnableState()
{
    int selectedCount = 0;
    int checkedInCount = 0;
    int checkedOutCount = 0;

    QList <QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

    selectedCount = (int)selectedItems.size();

    for (auto item : selectedItems)
    {
        std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        VcsState state;
        VcsItemAction action;
        bool ok = connection->vcsStateAndAction(m_dbController, state, action);
        if (ok == false)
        {
            continue;
        }

        if (state == VcsState::CheckedOut)
        {
            checkedOutCount++;
        }
        else
        {
            checkedInCount++;
        }
    }

    m_btnRemove->setEnabled(selectedCount > 0);
    m_removeAction->setEnabled(selectedCount > 0);

    m_btnCheckOut->setEnabled(selectedCount > 0 && checkedInCount > 0);
    m_checkOutAction->setEnabled(selectedCount > 0 && checkedInCount > 0);

    m_btnCheckIn->setEnabled(selectedCount > 0 && checkedOutCount > 0);
    m_checkInAction->setEnabled(selectedCount > 0 && checkedOutCount > 0);

    m_btnUndo->setEnabled(selectedCount > 0 && checkedOutCount > 0);
    m_undoAction->setEnabled(selectedCount > 0 && checkedOutCount > 0);
}



