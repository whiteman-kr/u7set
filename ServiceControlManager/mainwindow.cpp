#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QMenu>
#include <QDir>
#include <QSettings>
#include <QTranslator>
#include <QMessageBox>
#include <QPushButton>
#include <QHostAddress>
#include "scanoptionswidget.h"
#include "servicetablemodel.h"
#include "../include/UdpSocket.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    serviceModel(new ServiceTableModel(this)),
    trayIcon(new QSystemTrayIcon(this))
{
    trayIcon->setIcon(windowIcon());
    trayIcon->show();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    QMenu *contextMenu = new QMenu(this);

    contextMenu->addAction(tr("Open editor"), this, SLOT(openEditor()));
    contextMenu->addSeparator();

    // Manage Connections
    QMenu* menu = menuBar()->addMenu(tr("Connections"));
    contextMenu->addAction(menu->addAction(tr("Scan network..."), this, SLOT(scanNetwork())));
    contextMenu->addSeparator();
    //

    // Context menu connections list
    QTableView* serviceTable = new QTableView(this);
    serviceTable->setModel(serviceModel);
    connect(serviceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), serviceTable, SLOT(resizeColumnsToContents()));
    setCentralWidget(serviceTable);
    /*QActionGroup *serviceActionGroup = new QActionGroup(this);
    connect(serviceActionGroup, SIGNAL(triggered(QAction *)), this, SLOT(connectionClicked(QAction *)));

    contextMenu->addSeparator();*/
    //

    QAction* exitAction = menu->addAction(tr("Exit"), qApp, SLOT(quit()));

    //Languages
    QActionGroup *languageActionGroup = new QActionGroup(this);
    connect(languageActionGroup, SIGNAL(triggered(QAction *)), this, SLOT(switchLanguage(QAction *)));

    QString qmPath = ":/translations";
    QDir dir(qmPath);
    QStringList fileNames = dir.entryList(QStringList("ServiceControlManager_*.qm"));
    if (!fileNames.isEmpty())
    {
        menu = menuBar()->addMenu(tr("&Language"));
        contextMenu->addMenu(menu);
        QAction *action = new QAction("1 English", this);
        action->setCheckable(true);
        action->setData("en");
        menu->addAction(action);
        languageActionGroup->addAction(action);
        QSettings settings;
        if ("en" == settings.value("locale", QLocale::system().name()).toString().left(2))
        {
            action->setChecked(true);
        }

        for (int i = 0; i < fileNames.size(); ++i)
        {
            QString locale = fileNames[i];
            locale.remove(0, locale.indexOf('_') + 1);
            locale.truncate(locale.lastIndexOf('.'));
            QTranslator translator;
            translator.load(fileNames[i], qmPath);
            QString language = translator.translate("MainWindow", "English");
            action = new QAction(QString("&%1 %2").arg(i + 2).arg(language), this);
            action->setCheckable(true);
            action->setData(locale);
            menu->addAction(action);
            languageActionGroup->addAction(action);
            if (locale == settings.value("locale", QLocale::system().name()).toString().left(2))
            {
                action->setChecked(true);
            }
        }
    }

    // Help
    menu = menuBar()->addMenu(tr("&Help"));
    menu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    //

    contextMenu->addAction(exitAction);

    trayIcon->setContextMenu(contextMenu);

    //resize(serviceTable->maximumViewportSize());
    //ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    for (int i = 0; i < widgets.count(); i++)
    {
        widgets[i]->deleteLater();
    }
    delete ui;
}

void MainWindow::openConnectionInfo(QString text)
{
    for (int i = 0; i < widgets.count(); i++)
    {
        if (widgets[i]->windowTitle() == text)
        {
            widgets[i]->showNormal();
            widgets[i]->raise();
            widgets[i]->activateWindow();
            return;
        }
    }
    QWidget* w = new QWidget;
    w->setWindowTitle(text);
    w->showMaximized();
    w->showNormal();
    w->raise();
    w->activateWindow();
    widgets.append(w);
}

void MainWindow::openEditor()
{
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Context:
        trayIcon->contextMenu()->show();
        break;
    case QSystemTrayIcon::Trigger:
        openEditor();
        break;
    default:
        break;
    }
}

void MainWindow::switchLanguage(QAction* selectedAction)
{
    QString locale = selectedAction->data().toString();
    QMessageBox::information(this, tr("Message"), tr("New language \"%1\" will be loaded after restart ServiceControlManager").arg(selectedAction->text().mid(selectedAction->text().indexOf(' ') + 1)));
    QSettings settings;
    settings.setValue("locale", locale);
}

void MainWindow::connectionClicked(QAction *selectedAction)
{
    openConnectionInfo(selectedAction->text());
}

void MainWindow::scanNetwork()
{
    ScanOptionsWidget sow(nullptr);
    if (sow.exec() == QDialog::Rejected)
    {
        return;
    }
    QStringList address = sow.getSelectedAddress().split("/");
    if (address.count() == 1)
    {
        serviceModel->checkAddress(address[0]);
    }
    else
    {
        QHostAddress ha(address[0]);
        quint32 mask = ha.toIPv4Address();
        quint32 bitsCount = address[1].toUInt();
        if (bitsCount > 32)
        {
            bitsCount = 32;
        }
        mask &= (~0) << bitsCount;
        quint64 counter = 1 << bitsCount;
        for (quint32 i = 0; i < counter; i++)
        {
            QHostAddress peerAddress(mask + i);
            serviceModel->checkAddress(peerAddress.toString());
        }
    }
}
