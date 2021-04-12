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
	void reloadFiles();
	void loadProjectList();

	void sendTuningFrame();

private:
	QComboBox* m_netListCombo;
	QComboBox* m_projectListCombo;
	QLineEdit* m_portEditor;
	QTreeView* m_packetSourceView;
	PacketSourceModel* m_listenerModel;
	QString m_rootPath;
};

#endif // SOURCELISTWIDGET_H
