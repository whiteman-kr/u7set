#include "MainWindow.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QMenu>
#include <QDir>
#include <QSettings>
#include <QTranslator>
#include <QMessageBox>
#include <QPushButton>
#include <QMenuBar>
#include <QToolBar>
#include <QActionGroup>
#include <QApplication>
#include "ScanOptionsWidget.h"
#include "ServiceTableModel.h"
#include "../OnlineLib/UdpSocket.h"
#include <functional>
#include <QHeaderView>
#include <UiLib/DialogAbout.h>

MainWindow::MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow(parent),
	m_serviceModel(new ServiceTableModel(softwareInfo, this)),
	m_serviceTable(new QTableView(this))
{
	qRegisterMetaType<Network::ServiceInfo>("ServiceInformation");

	m_serviceTable->setModel(m_serviceModel);
	connect(m_serviceTable, &QTableView::doubleClicked, m_serviceModel, &ServiceTableModel::openServiceStatusWidget);
	setCentralWidget(m_serviceTable);

	m_serviceTable->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_serviceTable->fontMetrics().height() * 1.4 * 4));
	m_serviceTable->horizontalHeader()->setDefaultSectionSize(250);

	m_serviceTable->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	// Manage Connections
	//
	QMenu* menu = menuBar()->addMenu(tr("Connections"));
	menu->addAction(tr("Scan network..."), this, SLOT(scanNetwork()));

	menu->addSeparator();

	menu->addSeparator();
	menu->addAction(tr("Remove host"), this, SLOT(removeHost()));

	menu->addSeparator();
	menu->addAction(tr("Exit"), qApp, SLOT(quit()));

	//Languages
	//
	QActionGroup* languageActionGroup = new QActionGroup(this);
	connect(languageActionGroup, &QActionGroup::triggered, this, &MainWindow::switchLanguage);

	QString qmPath = ":/translations";
	QDir dir(qmPath);
	QStringList fileNames = dir.entryList(QStringList("ServiceControlManager_*.qm"));

	if (!fileNames.isEmpty())
	{
		menu = menuBar()->addMenu(tr("&Language"));
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
			bool loadResult = translator.load(fileNames[i], qmPath);
			Q_UNUSED(loadResult);

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
	//
	menu = menuBar()->addMenu(tr("&?"));
	menu->addAction(tr("About &Qt..."), qApp, SLOT(aboutQt()));
	menu->addAction(tr("About &Service Control Manager..."), this, SLOT(aboutScm()));
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

void MainWindow::closeEvent(QCloseEvent *)
{
	if (qApp->quitOnLastWindowClosed())
	{
		qApp->quit();
	}
}

void MainWindow::openEditor()
{
	showNormal();
	showMaximized();
	raise();
	activateWindow();
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
	std::sort(hostsForRemoving.begin(), hostsForRemoving.end(), std::greater<int>());
	for (int row : hostsForRemoving)
	{
		m_serviceModel->removeHost(row);
	}
}

void MainWindow::aboutScm()
{
	QString text;

	text += qApp->applicationName() + " provides tools for check RPCT services state.";

	UiLib::DialogAbout::show(this, text, ":/Logo/RadiyLogo.png",
							 qApp->organizationName(), QString(), QDate(), QUuid(), QString());


}
