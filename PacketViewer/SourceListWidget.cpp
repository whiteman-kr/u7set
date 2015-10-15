#include "SourceListWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeView>
#include <QComboBox>
#include <QLineEdit>
#include <QNetworkInterface>
#include "PacketSourceModel.h"

SourceListWidget::SourceListWidget(QWidget *parent)
	: QWidget(parent)
{
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
		}
	}

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(m_netListCombo);
	hl->addWidget(m_portEditor = new QLineEdit("2000", this));

	QPushButton* addListenerButton = new QPushButton("Add listener", this);
	connect(addListenerButton, &QPushButton::clicked, this, &SourceListWidget::addListener);
	hl->addWidget(addListenerButton);

	QPushButton* removeListenerButton = new QPushButton("Remove selected listener", this);
	connect(removeListenerButton, &QPushButton::clicked, this, &SourceListWidget::removeListener);
	hl->addWidget(removeListenerButton);
	hl->addStretch(1);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addLayout(hl);

	m_packetSourceView = new QTreeView(this);
	m_listenerModel = new PacketSourceModel(this);
	m_packetSourceView->setModel(m_listenerModel);
	connect(m_listenerModel, &PacketSourceModel::contentChanged, m_packetSourceView, &QTreeView::resizeColumnToContents);
	connect(m_packetSourceView, &QTreeView::doubleClicked, m_listenerModel, &PacketSourceModel::openSourceStatusWidget);
	for (int i = 0; i < m_listenerModel->columnCount(); i++)
	{
		m_packetSourceView->resizeColumnToContents(i);
	}
	vl->addWidget(m_packetSourceView);
	setLayout(vl);
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
}
