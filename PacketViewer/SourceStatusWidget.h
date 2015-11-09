#ifndef SOURCESTATUSWIDGET_H
#define SOURCESTATUSWIDGET_H

#include <QWidget>
class Source;
class PacketBufferTableModel;
class SignalTableModel;

class SourceStatusWidget : public QWidget
{
	Q_OBJECT
public:
	explicit SourceStatusWidget(Source& source, PacketBufferTableModel* packetBufferModel, SignalTableModel* signalTableModel, QWidget *parent = 0);

signals:

public slots:
};

#endif // SOURCESTATUSWIDGET_H
