#include "DialogAfblEditor.h"
#include "ui_DialogAfblEditor.h"
#include "DialogAfbProperties.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>

DialogAfblEditor::DialogAfblEditor(DbController* pDbController, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    m_pDbController(pDbController),
    ui(new Ui::DialogAfblEditor)
{
    ui->setupUi(this);
    setWindowTitle(tr("AFB Library Editor"));

    ui->m_afbTree->setColumnCount(3);
    QStringList l(tr("File Name"));
	l << tr("State");
	l << tr("Last Check-in Time");
    ui->m_afbTree->setHeaderLabels(l);
    ui->m_afbTree->setColumnWidth(0, 150);
    ui->m_afbTree->setColumnWidth(1, 100);

    refreshFiles();
}

DialogAfblEditor::~DialogAfblEditor()
{
    delete ui;
}

void DialogAfblEditor::refreshFiles()
{
	// save selected indexes
	//
    std::vector<int> oldSelectedIndexes;
    QModelIndexList selectedIndexList = ui->m_afbTree->selectionModel()->selectedIndexes();
    for (auto i = selectedIndexList.begin(); i != selectedIndexList.end(); i++)
    {
        QModelIndex mi = *i;
        if (mi.column() != 0)
            continue;

        oldSelectedIndexes.push_back(mi.row());
    }
    int oldItemsCount = ui->m_afbTree->topLevelItemCount();

    // refresh list
	//
    ui->m_afbTree->clear();

	QList<QTreeWidgetItem*> items;

    std::vector<DbFileInfo> afbfiles;
    if (m_pDbController->getFileList(&afbfiles, m_pDbController->afblFileId(), "afb", this) == false)
    {
        QMessageBox::critical(this, "Error", "Could not get afb files list!");
        return;
    }

    std::vector<DbFileInfo> xsdfiles;
    if (m_pDbController->getFileList(&xsdfiles, m_pDbController->afblFileId(), "xsd", this) == false)
    {
        QMessageBox::critical(this, "Error", "Could not get xsd files list!");
        return;
    }

	files.clear();

	files.insert(files.begin(), xsdfiles.begin(), xsdfiles.end());
    files.insert(files.begin(), afbfiles.begin(), afbfiles.end());

    for (int i = 0; i < files.size(); i++)
    {
        DbFileInfo& fi = files[i];

        QStringList l(fi.fileName());
        l.append(fi.state().text());
        l.append(fi.lastCheckIn().toString());

		QTreeWidgetItem* pItem = new QTreeWidgetItem(ui->m_afbTree, l);
        items.append(pItem);

    }

    ui->m_afbTree->insertTopLevelItems(0, items);

	// restore selection
	//
    if (items.size() == oldItemsCount)
    {
        for (auto j = oldSelectedIndexes.begin(); j != oldSelectedIndexes.end(); j++)
        {
			ui->m_afbTree->selectionModel()->select( ui->m_afbTree->model()->index( *j, 0 ),
				QItemSelectionModel::Select | QItemSelectionModel::Rows );
        }
    }
}

void DialogAfblEditor::on_m_add_clicked()
{
    bool ok = false;
	QString caption = QInputDialog::getText(this, tr("Add AFB"), tr("Enter name:"), QLineEdit::Normal, tr(""), &ok);
    if (ok == false)
    {
        return;
    }

    Afb afb;
    afb.setCaption(caption);

    QByteArray byteArray;

    QXmlStreamWriter wr(&byteArray);
    afb.storeToXml(wr);

    std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
    pf->setFileName(caption + ".afb");
    pf->swapData(byteArray);

	addFile(pf);
}

void DialogAfblEditor::on_m_addXsd_clicked()
{
    bool ok = false;
    QString caption = QInputDialog::getText(this, tr("Add XSD"), tr("Enter the scheme name:"), QLineEdit::Normal, tr(""), &ok);
    if (ok == false)
    {
        return;
    }

	QByteArray byteArray = QString("Enter schema here...").toUtf8();


    std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
    pf->setFileName(caption + ".xsd");
    pf->swapData(byteArray);

    addFile(pf);
}

void DialogAfblEditor::on_m_edit_clicked()
{
    std::vector<DbFileInfo*> selectedFiles = getSelectedFiles();

    if (selectedFiles.size() != 1)
    {
        return;
    }

    DbFileInfo* pFi = *selectedFiles.begin();
    Q_ASSERT(pFi);

    if (pFi->state() != VcsState::CheckedOut)
    {
        if (QMessageBox::question(this, "Edit", "File is not checked out! Do you wish to check out and edit this file?") != QMessageBox::Yes)
        {
            return;
        }

		std::vector<DbFileInfo> f;
        f.push_back(*pFi);

		if (m_pDbController->checkOut(f, this) == false)
		{
            QMessageBox::critical(this, "Error", "Check Out error!");
            return;
        }

        refreshFiles();
    }

    std::shared_ptr<DbFile> f;

    if (m_pDbController->getWorkcopy(*pFi, &f, this) == false)
    {
        QMessageBox::critical(this, "Error", "Get work copy error!");
        return;
    }

    QByteArray data;
    f->swapData(data);

    DialogAfbProperties d(pFi->fileName(), &data, m_pDbController);
    if (d.exec() != QDialog::Accepted)
    {
        return;
    }

    f->swapData(data);


    if (m_pDbController->setWorkcopy(f, this) == false)
    {
        QMessageBox::critical(this, "Error", "Set work copy error!");
        return;
    }

	return;
}

void DialogAfblEditor::on_m_checkOut_clicked()
{
    std::vector<DbFileInfo*> selectedFiles = getSelectedFiles();
    if (selectedFiles.empty())
        return;

    for (auto it = selectedFiles.begin(); it != selectedFiles.end(); it++)
    {
        DbFileInfo* pFi = *it;
        if (pFi->state() != VcsState::CheckedIn)
        {
            QMessageBox::critical(this, "Error", "File " + pFi->fileName() + " is already checked out!");
            continue;
        }

        if (m_pDbController->checkOut(*pFi, this) == false)
        {
            QMessageBox::critical(this, "Error", pFi->fileName() + " Check Out error!");
            continue;
        }
    }

    refreshFiles();
}

void DialogAfblEditor::on_m_checkIn_clicked()
{
    std::vector<DbFileInfo*> selectedFiles = getSelectedFiles();
    if (selectedFiles.empty())
        return;

    bool ok = false;
    QString comment = QInputDialog::getText(this, tr("Check In"), tr("Enter the comment:"), QLineEdit::Normal, tr(""), &ok);
    if (ok == false)
        return;

    for (auto it = selectedFiles.begin(); it != selectedFiles.end(); it++)
    {
        DbFileInfo* pFi = *it;
        if (pFi->state() != VcsState::CheckedOut)
        {
            QMessageBox::critical(this, "Error", "File " + pFi->fileName() + " is already checked in!");
            continue;
        }

        if (m_pDbController->checkIn(*pFi, comment, this) == false)
        {
            QMessageBox::critical(this, "Error", pFi->fileName() + " Check In error!");
            continue;
        }
    }

    refreshFiles();
}

void DialogAfblEditor::on_m_Undo_clicked()
{
    std::vector<DbFileInfo*> selectedFiles = getSelectedFiles();
    if (selectedFiles.empty())
        return;

    if (QMessageBox::question(this, "Undo", "Are you sure you want to undo the changes?") != QMessageBox::Yes)
        return;

    for (auto it = selectedFiles.begin(); it != selectedFiles.end(); it++)
    {
        DbFileInfo* pFi = *it;
        if (pFi->state() != VcsState::CheckedOut)
        {
            QMessageBox::critical(this, "Error", "File " + pFi->fileName() + " is not checked out!");
            continue;
        }

        if (m_pDbController->undoChanges(*pFi, this) == false)
        {
            QMessageBox::critical(this, "Error", pFi->fileName() + " Undo error!");
            continue;
        }
    }

    refreshFiles();
}

void DialogAfblEditor::addFile(const std::shared_ptr<DbFile>& pf)
{
	if (m_pDbController->addFile(pf, m_pDbController->afblFileId(), this) == false)
    {
        QMessageBox::critical(this, "Error", "Error adding file!");
        return;
    }

    refreshFiles();

	// Select the new file
	//
    QList<QTreeWidgetItem*> added = ui->m_afbTree->findItems(pf->fileName(), Qt::MatchFixedString);

	Q_ASSERT(added.size() == 1);

    for (auto i = added.begin(); i != added.end(); i++)
    {
        QTreeWidgetItem* pItem = *i;
        pItem->setSelected(true);
    }

	return;
}

std::vector<DbFileInfo*> DialogAfblEditor::getSelectedFiles()
{
    std::vector<DbFileInfo*> selectedFiles;

    QModelIndexList selectedIndexList = ui->m_afbTree->selectionModel()->selectedIndexes();
    for (auto i = selectedIndexList.begin(); i != selectedIndexList.end(); i++)
    {
        QModelIndex mi = *i;
        if (mi.column() != 0)
            continue;

        selectedFiles.push_back(&files[mi.row()]);
    }

    return selectedFiles;
}



void DialogAfblEditor::on_m_afbTree_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(item);
    Q_UNUSED(column);
    on_m_edit_clicked();
}

void DialogAfblEditor::on_m_afbTree_itemSelectionChanged()
{
    std::vector<DbFileInfo*> selectedFiles = getSelectedFiles();

    bool enableEdit = false;
    if (selectedFiles.size() == 1 && selectedFiles[0]->state() == VcsState::CheckedOut)
    {
        enableEdit = true;
    }

    bool enableCheckOut = false;
    int count = 0;
    for (auto i = selectedFiles.begin(); i != selectedFiles.end(); i++)
    {
        if ((*i)->state() == VcsState::CheckedIn)
            count++;
    }
    enableCheckOut = count != 0 && count == selectedFiles.size();

    bool enableCheckIn = false;
    count = 0;
    for (auto i = selectedFiles.begin(); i != selectedFiles.end(); i++)
    {
        if ((*i)->state() == VcsState::CheckedOut)
            count++;
    }
    enableCheckIn = count != 0 && count == selectedFiles.size();


    ui->m_edit->setEnabled(enableEdit);
    ui->m_checkOut->setEnabled(enableCheckOut);
    ui->m_checkIn->setEnabled(enableCheckIn);
    ui->m_Undo->setEnabled(enableCheckIn);
    //ui->m_remove
}

