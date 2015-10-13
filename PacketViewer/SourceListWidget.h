#ifndef SOURCELISTWIDGET_H
#define SOURCELISTWIDGET_H

class QComboBox;
class QLineEdit;
class QTreeView;
class PacketSourceModel;

#include <QWidget>

class SourceListWidget : public QWidget
{
	Q_OBJECT

public:
	SourceListWidget(QWidget *parent = 0);
	~SourceListWidget();

public slots:
	void addListener();
	void removeListener();

private:
	QComboBox* m_netListCombo;
	QLineEdit* m_portEditor;
	QTreeView* m_packetSourceView;
	PacketSourceModel* m_listenerModel;
};

#endif // SOURCELISTWIDGET_H
