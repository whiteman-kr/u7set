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

    connect(m_mask, &QLineEdit::returnPressed, this, &DialogConnections::onMaskReturnPressed);
    connect(m_maskApply, &QPushButton::clicked, this, &DialogConnections::onMaskApplyClicked);

    m_connectionsTree = new QTreeWidget();

    QStringList l;
    l << tr("#");
    l << tr("ConnectionID");
    l << tr("Type");
    l << tr("State");

    m_connectionsTree->setColumnCount(l.size());
    m_connectionsTree->setHeaderLabels(l);

    int il = 0;
    m_connectionsTree->setColumnWidth(il++, 50);
    m_connectionsTree->setColumnWidth(il++, 80);
    m_connectionsTree->setColumnWidth(il++, 80);
    m_connectionsTree->setColumnWidth(il++, 40);
    m_connectionsTree->setSortingEnabled(true);
    m_connectionsTree->setSelectionMode(QAbstractItemView::SingleSelection);

    m_connectionsTree->setRootIsDecorated(false);

    connect(m_connectionsTree, &QTreeWidget::currentItemChanged, this, &DialogConnections::onConnectionItemChanged);

    m_connectionPropertyEditor = new ExtWidgets::PropertyEditor(this);


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

    // Load connections
    //
    QString errorCode;

    if (connections.load(m_dbController, errorCode) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't load connections!"));
        return;
    }

    fillConnectionsList();

    // sort items
    //

    for (int i = 0; i < m_connectionsTree->columnCount(); i++)
    {
        m_connectionsTree->resizeColumnToContents(i);
    }

    m_connectionsTree->sortByColumn(theSettings.m_connectionEditorSortColumn, theSettings.m_connectionEditorSortOrder);

    connect(m_connectionsTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogConnections::sortIndicatorChanged);


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

void DialogConnections::onMaskReturnPressed()
{
    onMaskApplyClicked();
}

void DialogConnections::onMaskApplyClicked()
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

    // Check for valid symbols in connection IDS
    //
    QStringList badList;

    for (int i = 0; i < connections.count(); i++)
    {
        Hardware::Connection* c = connections.get(i).get();
        if (c->mode() != Hardware::OptoPort::Mode::Optical)
        {
            continue;
        }

        QRegExp rx("^[A-Za-z0-9_]+$");
        if (rx.exactMatch(c->connectionID()) == false)
        {
            badList.push_back(c->connectionID());
        }
    }

    if (badList.empty() == false)
    {
        QString badString;
        int badCount = 0;

        for (auto b : badList)
        {
            badString += tr("'%1'\r\n").arg(b);

            if (badCount >= 10)
            {
                badString += tr("and %1 more connections.\r\n").arg(badList.size() - badCount);
                break;
            }

            badCount++;
        }

        QString s = QString("Invalid symbols were found in column ConnectionID:\r\n\r\n%1\r\nConnectionID can contain only letters, numbers and '_' symbol."
                            ".\r\n\r\nAre you sure you want to continue?").arg(badString);

        if (QMessageBox::warning(this, "Connections Editor", s, QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        {
            return false;
        }
    }

    return true;
}

void DialogConnections::sortIndicatorChanged(int column, Qt::SortOrder order)
{
    theSettings.m_connectionEditorSortColumn = column;
    theSettings.m_connectionEditorSortOrder = order;
}

void DialogConnections::onConnectionItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (current == nullptr)
    {
        m_connectionPropertyEditor->clear();
        return;
    }

    std::shared_ptr<Hardware::Connection> connection = current->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
    if (connection == nullptr)
    {
        assert(connection);
        return;
    }

    QList<std::shared_ptr<PropertyObject>> objects;
    objects.push_back(connection);

    m_connectionPropertyEditor->setExpertMode(theSettings.isExpertMode());

    m_connectionPropertyEditor->setObjects(objects);
}

void DialogConnections::setConnectionText(QTreeWidgetItem* item, Hardware::Connection* connection)
{
    if (item == nullptr || connection == nullptr)
    {
        assert(item);
        assert(connection);
        return;
    }

    int c = 0;
    QString numString = QString::number(connection->index()).rightJustified(4, '0');
    item->setText(c++, numString);
    item->setText(c++, connection->connectionID());
    item->setText(c++, connection->manualSettings() ? tr("Manual") : tr("Auto"));

}
