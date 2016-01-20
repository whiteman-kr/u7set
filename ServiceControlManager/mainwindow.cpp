#include "mainwindow.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QMenu>
#include <QDir>
#include <QSettings>
#include <QTranslator>
#include <QMessageBox>
#include <QPushButton>
#include <QHostAddress>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#include "scanoptionswidget.h"
#include "servicetablemodel.h"
#include "../include/UdpSocket.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	m_trayIcon(new QSystemTrayIcon(this)),
	m_serviceModel(new ServiceTableModel(this)),
	m_serviceTable(new QTableView(this))
{
	qRegisterMetaType<ServiceInformation>("ServiceInformation");

	m_serviceTable->setModel(m_serviceModel);
	connect(m_serviceModel, &ServiceTableModel::dataChanged, m_serviceTable, &QTableView::resizeColumnsToContents);
	connect(m_serviceModel, &ServiceTableModel::serviceStateChanged, m_serviceTable, &QTableView::resizeRowToContents);
	connect(m_serviceModel, &ServiceTableModel::rowsAboutToBeInserted, m_serviceTable, &QTableView::resizeColumnsToContents);
	connect(m_serviceTable, &QTableView::doubleClicked, m_serviceModel, &ServiceTableModel::openServiceStatusWidget);
	m_serviceTable->resizeColumnsToContents();
	setCentralWidget(m_serviceTable);

	m_trayIcon->setIcon(windowIcon());
	m_trayIcon->show();
	connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

	QMenu *contextMenu = new QMenu(this);
	QToolBar *toolBar = addToolBar(tr("Main actions"));

	contextMenu->addAction(tr("Open service status window"), this, SLOT(openEditor()));
	contextMenu->addSeparator();

	// Manage Connections
	QMenu* menu = menuBar()->addMenu(tr("Connections"));
	QAction* scanNetworkAction = menu->addAction(tr("Scan network..."), this, SLOT(scanNetwork()));
	menu->addSeparator();
	toolBar->addAction(scanNetworkAction);
	contextMenu->addAction(scanNetworkAction);
	contextMenu->addSeparator();

	menu->addSeparator();
	toolBar->addAction(menu->addAction(tr("Remove host"), this, SLOT(removeHost())));
	toolBar->addSeparator();

	menu->addSeparator();
	QAction* exitAction = menu->addAction(tr("Exit"), qApp, SLOT(quit()));
	toolBar->addAction(exitAction);

	//Languages
	QActionGroup *languageActionGroup = new QActionGroup(this);
	connect(languageActionGroup, &QActionGroup::triggered, this, &MainWindow::switchLanguage);

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

	m_trayIcon->setContextMenu(contextMenu);

	//resize(serviceTable->maximumViewportSize());
}

MainWindow::~MainWindow()
{
	for (int i = 0; i < m_widgets.count(); i++)
	{
		m_widgets[i]->deleteLater();
	}
}

void MainWindow::openConnectionInfo(QString text)
{
	for (int i = 0; i < m_widgets.count(); i++)
	{
		if (m_widgets[i]->windowTitle() == text)
		{
			m_widgets[i]->showNormal();
			m_widgets[i]->raise();
			m_widgets[i]->activateWindow();
			return;
		}
	}
	QWidget* w = new QWidget;
	w->setWindowTitle(text);
	w->showMaximized();
	w->showNormal();
	w->raise();
	w->activateWindow();
	m_widgets.append(w);
}

void MainWindow::openEditor()
{
	showNormal();
	showMaximized();
	raise();
	activateWindow();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
		case QSystemTrayIcon::Context:
			m_trayIcon->contextMenu()->show();
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
	ScanOptionsWidget sow(m_serviceModel, this);
	sow.exec();
}

void MainWindow::removeHost()
{
	QModelIndexList selection = m_serviceTable->selectionModel()->selectedIndexes();
	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No service is selected!"));
	}
	QVector<int> hostsForRemoving;
	QVector<int> checkedHosts;
	for (int i = 0; i < selection.count(); i++)
	{
		bool checked = false;
		int row = selection[i].row();
		for (int j = 0; j < checkedHosts.count(); j++)
		{
			if (row == checkedHosts[j])
			{
				checked = true;
			}
		}
		if (checked)
		{
			continue;
		}
		checkedHosts.append(row);
		auto reply = QMessageBox::question(this,
										   tr("Confirmation"),
										   tr("Are you sure you want to delete %1 host").arg(m_serviceModel->headerData(row, Qt::Vertical).toString()),
										   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		switch (reply)
		{
			case QMessageBox::Yes:
				hostsForRemoving.append(row);
				break;
			case QMessageBox::No:
				continue;
			case QMessageBox::Cancel:
				return;
			default:
				assert(false);
		}
	}
    //std::sort(hostsForRemoving.begin(), hostsForRemoving.end(), std::greater<int>());
	for (int row : hostsForRemoving)
	{
		m_serviceModel->removeHost(row);
	}
}
