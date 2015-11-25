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

	QCheckBox* cb = new QCheckBox("Convert from BigEndian", this);
	connect(cb, &QCheckBox::toggled, signalTableModel, &SignalTableModel::setNeedToSwapBytes);
	cb->setChecked(true);

	QTableView* bufferTable = new QTableView(this);
	bufferTable->setModel(packetBufferModel);
	bufferTable->resizeColumnsToContents();

	QTableView* signalTable = new QTableView(this);
	signalTable->setModel(signalTableModel);
	signalTable->resizeColumnsToContents();

	QHBoxLayout* hl = new QHBoxLayout;
	QVBoxLayout* vl = new QVBoxLayout;
	hl->addWidget(bufferTable);
	vl->addWidget(cb);
	vl->addWidget(signalTable);
	hl->addLayout(vl);
	setLayout(hl);
	resize(640, 480);
	show();
	setAttribute(Qt::WA_DeleteOnClose, true);
}

