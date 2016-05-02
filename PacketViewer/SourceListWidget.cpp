#include "SourceListWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeView>
#include <QComboBox>
#include <QLineEdit>
#include <QNetworkInterface>
#include "PacketSourceModel.h"
#include <QSettings>
#include <QDirIterator>
#include <QSettings>
#include <QTimer>
#include "SendTuningFrameWidget.h"

SourceListWidget::SourceListWidget(QWidget *parent)
	: QWidget(parent)
{
	QSettings s("Radiy", "u7");
	m_rootPath = s.value("m_buildOutputPath", QDir::currentPath()).toString();

	m_packetSourceView = new QTreeView(this);
	m_listenerModel = new PacketSourceModel(this);
	m_packetSourceView->setModel(m_listenerModel);

	m_netListCombo = new QComboBox(this);
	m_netListCombo->addItem("0.0.0.0");
	QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
	for (int i = 0; i < interfaceList.count(); i++)
	{
		QList<QNetworkAddressEntry> addressList = interfaceList[i].addressEntries();
		for (int j = 0; j < addressList.count(); j++)
		{
			QHostAddress ip = addressList[j].ip();
			if (ip.protocol() != QAbstractSocket::IPv4Protocol)
			{
				continue;
			}
			m_netListCombo->addItem(ip.toString());
			QSettings settings;
			int count = settings.beginReadArray("PacketSourceModel/listenAddresses");
			for (int i = 0; i < count; i++)
			{
				settings.setArrayIndex(i);
				quint32 listenIp = settings.value("ip").toUInt();
				if (listenIp == ip.toIPv4Address())
				{
					m_listenerModel->addListener(ip.toString(), settings.value("port").toUInt(), false);
				}
			}
			settings.endArray();
		}
	}

	QVBoxLayout* vl = new QVBoxLayout;
	QHBoxLayout* hl = new QHBoxLayout;

	setLayout(vl);

	m_projectListCombo = new QComboBox(this);
	hl->addWidget(m_projectListCombo);

	loadProjectList();
	m_listenerModel->loadProject(m_rootPath + '/' + m_projectListCombo->currentText());

	QTimer* updateProjectListTimer = new QTimer(this);
	connect(updateProjectListTimer, &QTimer::timeout, this, &SourceListWidget::loadProjectList);
	updateProjectListTimer->start(10000);

	QPushButton* button = new QPushButton("Reload", this);
	connect(button, &QPushButton::pressed, this, &SourceListWidget::reloadFiles);
	hl->addWidget(button);

	button = new QPushButton("Send tuning frame", this);
	connect(button, &QPushButton::pressed, this, &SourceListWidget::sendTuningFrame);
	hl->addWidget(button);

	hl->addStretch();
	vl->addLayout(hl);
	hl = new QHBoxLayout;
	hl->addWidget(m_netListCombo);
	hl->addWidget(m_portEditor = new QLineEdit("2000", this));

	QPushButton* addListenerButton = new QPushButton("Add listener", this);
	connect(addListenerButton, &QPushButton::clicked, this, &SourceListWidget::addListener);
	hl->addWidget(addListenerButton);

	QPushButton* removeListenerButton = new QPushButton("Remove selected listener", this);
	connect(removeListenerButton, &QPushButton::clicked, this, &SourceListWidget::removeListener);
	hl->addWidget(removeListenerButton);
	hl->addStretch(1);

	vl->addLayout(hl);

	connect(m_listenerModel, &PacketSourceModel::contentChanged, m_packetSourceView, &QTreeView::resizeColumnToContents);
	connect(m_packetSourceView, &QTreeView::doubleClicked, m_listenerModel, &PacketSourceModel::openSourceStatusWidget);
	for (int i = 0; i < m_listenerModel->columnCount(); i++)
	{
		m_packetSourceView->resizeColumnToContents(i);
	}
	vl->addWidget(m_packetSourceView);
}

SourceListWidget::~SourceListWidget()
{

}

void SourceListWidget::addListener()
{
	m_listenerModel->addListener(m_netListCombo->currentText(), m_portEditor->text().toInt());
}

void SourceListWidget::removeListener()
{
	QModelIndexList selected = m_packetSourceView->selectionModel()->selectedRows();
	for (int i = 0; i < selected.count(); i++)
	{
		if (!selected[i].parent().isValid())
		{
			m_listenerModel->removeListener(selected[i].row());
		}
	}
	m_listenerModel->saveListenerList();
}

void SourceListWidget::reloadFiles()
{
	QSettings s;
	s.setValue("Selected project", m_projectListCombo->currentText());
	m_listenerModel->loadProject(m_rootPath + '/' + m_projectListCombo->currentText());
}

void SourceListWidget::loadProjectList()
{
	m_projectListCombo->clear();
	QDirIterator it(m_rootPath, QStringList() << "*build.xml", QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString path = it.next();
		int lastSlash = path.lastIndexOf('/');
		QString projectPath = path.left(lastSlash);
		if (QDirIterator(projectPath, QStringList() << "*appSignals.xml", QDir::Files, QDirIterator::Subdirectories).hasNext() &&
				QDirIterator(projectPath, QStringList() << "*equipment.xml", QDir::Files, QDirIterator::Subdirectories).hasNext())
		{
			int prevSlash = path.lastIndexOf('/', lastSlash - 1);
			m_projectListCombo->addItem(path.mid(prevSlash + 1, lastSlash - prevSlash - 1));
		}
	}
	QSettings s;
	m_projectListCombo->setCurrentText(s.value("Selected project", m_projectListCombo->itemText(m_projectListCombo->count() - 1)).toString());
}

void SourceListWidget::sendTuningFrame()
{
	SendTuningFrameWidget senderWidget(m_listenerModel);
	senderWidget.exec();
}
