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
    l<<tr("State");
    l<<tr("Last Check-in Time");
    ui->m_afbTree->setHeaderLabels(l);

    refreshFiles();
}

DialogAfblEditor::~DialogAfblEditor()
{
    delete ui;
}

void DialogAfblEditor::refreshFiles()
{
    ui->m_afbTree->clear();

    QList<QTreeWidgetItem *> items;

    if (m_pDbController->getFileList(&files, m_pDbController->afblFileId(), "afb", this) == false)
    {
        QMessageBox::critical(this, "Error", "Could not get files list!");
        return;
    }

    for (int i = 0; i < files.size(); i++)
    {
        DbFileInfo& fi = files[i];

        QStringList l(fi.fileName());
        l.append(fi.state().text());
        l.append(fi.lastCheckIn().toString());

        items.append(new QTreeWidgetItem((QTreeWidget*)0, l));
    }

    ui->m_afbTree->insertTopLevelItems(0, items);
}

void DialogAfblEditor::on_m_add_clicked()
{
    bool ok = false;
    QString caption = QInputDialog::getText(this, tr("Add AFB"), tr("Enter the name:"), QLineEdit::Normal, tr(""), &ok);
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

    std::vector<std::shared_ptr<DbFile>> f;
    f.push_back(pf);

    if (m_pDbController->addFiles(&f, m_pDbController->afblFileId(), this) == true)
    {
        refreshFiles();
    }
    else
    {
        QMessageBox::critical(this, "Error", "Error adding file!");
    }
}


void DialogAfblEditor::on_m_edit_clicked()
{

    DbFileInfo* pFi = getCurrentFileInfo();
    if (pFi == nullptr)
    {
        return;
    }

    if (pFi->state() != VcsState::CheckedOut)
    {
        if (QMessageBox::question(this, "Edit", "File is not checked out! Do you wish to check out and edit this file?") != QMessageBox::Yes)
        {
            return;
        }

        std::vector <DbFileInfo> f;
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

    DialogAfbProperties d(pFi->fileName(), &data);
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

}

void DialogAfblEditor::on_m_checkOut_clicked()
{
    DbFileInfo* pFi = getCurrentFileInfo();
    if (pFi == nullptr)
    {
        return;
    }

    if (pFi->state() != VcsState::CheckedIn)
    {
        QMessageBox::critical(this, "Error", "File is already checked out!");
        return;
    }

    std::vector <DbFileInfo> f;
    f.push_back(*pFi);

    if (m_pDbController->checkOut(f, this) == false)
    {
        QMessageBox::critical(this, "Error", "Check Out error!");
        return;
    }

    refreshFiles();
}

void DialogAfblEditor::on_m_checkIn_clicked()
{
    DbFileInfo* pFi = getCurrentFileInfo();
    if (pFi == nullptr)
    {
        return;
    }

    if (pFi->state() != VcsState::CheckedOut)
    {
        QMessageBox::critical(this, "Error", "File is already checked in!");
        return;
    }

    bool ok = false;
    QString comment = QInputDialog::getText(this, tr("Check In"), tr("Enter comment:"), QLineEdit::Normal, tr(""), &ok);
    if (ok == false)
    {
        return;
    }

    std::vector <DbFileInfo> f;
    f.push_back(*pFi);

    if (m_pDbController->checkIn(f, comment, this) == false)
    {
        QMessageBox::critical(this, "Error", "Check In error!");
        return;
    }

    refreshFiles();

}

void DialogAfblEditor::on_m_Undo_clicked()
{
    DbFileInfo* pFi = getCurrentFileInfo();
    if (pFi == nullptr)
    {
        return;
    }

    if (pFi->state() != VcsState::CheckedOut)
    {
        QMessageBox::critical(this, "Error", "File is not checked out!");
        return;
    }

    std::vector <DbFileInfo> f;
    f.push_back(*pFi);

    if (m_pDbController->undoChanges(f, this) == false)
    {
        QMessageBox::critical(this, "Error", "Undo error!");
        return;
    }

    refreshFiles();
}

DbFileInfo* DialogAfblEditor::getCurrentFileInfo()
{
    if (ui->m_afbTree->currentIndex().isValid() == false)
        return nullptr;

    int row = ui->m_afbTree->currentIndex().row();

    if (row < 0 || row >= files.size())
        return nullptr;

    return &files[row];
}


