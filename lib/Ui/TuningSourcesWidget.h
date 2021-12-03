#ifndef TUNINGSOURCESWIDGET_H
#define TUNINGSOURCESWIDGET_H

#include <QDialog>

#include "DialogSourceInfo.h"

class TuningTcpClient;
class TuningSource;

//
// DialogTuningSourceInfo
//

class DialogTuningSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogTuningSourceInfo(std::vector<TuningTcpClient*> tcpClients, QWidget* parent, Hash sourceHash);
	virtual ~DialogTuningSourceInfo();

	Hash sourceHash() const;

	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

private:
	bool findActiveTuningTcpClient();

	void updateData() override;

private:
	std::vector<TuningTcpClient*> m_tcpClients;

	TuningTcpClient* m_activeTcpClient = nullptr;

	QString m_sourceEquipmentId;

};

class TuningSourcesWidget : public QWidget
{
	Q_OBJECT
public:

	explicit TuningSourcesWidget(std::vector<TuningTcpClient*> tcpClients, bool hasActivationControls, bool hasCloseButton, QWidget* parent);
	virtual ~TuningSourcesWidget();

	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

signals:
	void closeButtonPressed();

protected:
	void timerEvent(QTimerEvent* event);

	virtual bool login();	// Override this function to ask password before activating/deactivating sources

private slots:
	void closeClicked();

	void detailsClicked();

	void treeWidget_itemSelectionChanged();

	void treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

	void enableControl_clicked();

	void disableControl_clicked();

	void detailsDialogClosed(Hash hash);

private:
	bool checkTuningSourcesChanged() const;

	void update(bool refreshOnly);

	void activateControl(bool enable);

	TuningTcpClient* selectedClient() const;
	const std::optional<TuningSource> selectedSource() const;

	enum class Columns
	{
		EquipmentId,
		Ip,
		Port,
		Channel,
		SubsystemID,
		LmNumber,

		State,
		IsActive,
		HasUnappliedParams,
		RequestCount,
		ReplyCount,

		ColumnCount
	};

private:

	QTreeWidget* m_treeWidget = nullptr;
	QPushButton* m_btnDetails = nullptr;
	QPushButton* m_btnEnableControl = nullptr;
	QPushButton* m_btnDisableControl = nullptr;

	int m_updateStateTimerId = -1;

	bool m_hasActivationControls = false;

	std::vector<TuningTcpClient*> m_tuningTcpClients;

	QWidget* m_parent = nullptr;

	static const int columnIndex_Hash = 0;
	static const int columnIndex_EquipmentId = 1;

	std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap;

	std::set<Hash> m_tuningClientsSourcesHashes;	// used for comparing with current state, if it is changed - full refresh is performed
};



#endif // DIALOGTUNINGSOURCES_H
