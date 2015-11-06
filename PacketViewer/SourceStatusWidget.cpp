#include "SourceStatusWidget.h"
#include "PacketSourceModel.h"
#include <QTableView>
#include "PacketBufferTableModel.h"
#include "SignalTableModel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

SourceStatusWidget::SourceStatusWidget(Source &source, PacketBufferTableModel *packetBufferModel, SignalTableModel *signalTableModel, QWidget *parent) : QWidget(parent)
{
	setWindowTitle(source.fullAddress());
	connect(this, &QWidget::destroyed, &source, &Source::removeDependentWidget);

	QCheckBox* cb = new QCheckBox("Change byte order", this);
	connect(cb, &QCheckBox::toggled, packetBufferModel, &PacketBufferTableModel::setNeedToSwapBytes);
	cb->setChecked(true);
	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(cb);

	QTableView* bufferTable = new QTableView(this);
	bufferTable->setModel(packetBufferModel);
	bufferTable->resizeColumnsToContents();

	QTableView* signalTable = new QTableView(this);
	signalTable->setModel(signalTableModel);
	signalTable->resizeColumnsToContents();

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(bufferTable);
	hl->addWidget(signalTable);
	vl->addLayout(hl);
	setLayout(vl);
	resize(640, 480);
	show();
	setAttribute(Qt::WA_DeleteOnClose, true);
}

