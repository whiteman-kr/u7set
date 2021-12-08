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
	explicit DialogTuningSourceInfo(std::vector<TuningTcpClient*> tcpClients, QWidget* parent, Hash sourceHash, int channel);
	virtual ~DialogTuningSourceInfo();

	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

private:
	bool findActiveTuningTcpClient();

	void updateData() override;

private:
	std::vector<TuningTcpClient*> m_tcpClients;

	TuningTcpClient* m_activeTcpClient = nullptr;

	QString m_sourceEquipmentId;

	Hash m_tuningSourceHash = UNDEFINED_HASH;
	int m_channel = 0;

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
	const Hash selectedSourceHash() const;
	const int selectedSourceChannel() const;

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

	QWidget* m_parent = nullptr;
	QTreeWidget* m_treeWidget = nullptr;
	QPushButton* m_btnDetails = nullptr;
	QPushButton* m_btnEnableControl = nullptr;
	QPushButton* m_btnDisableControl = nullptr;

	int m_updateStateTimerId = -1;

	bool m_hasActivationControls = false;

	std::vector<TuningTcpClient*> m_tuningTcpClients;

	static const int columnIndex_ClientHash = 0;
	static const int columnIndex_SourceEquipmentIdHash = 1;
	static const int columnIndex_SourceChannel = 2;

	std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap;	// Used for managing details dialogs. Key is "Source ID + Channel" Hash.

	std::set<Hash> m_tuningClientsLansHashes;	// Used for comparing with current state, if it is changed - full refresh is performed. Key is "Client ID + Lan ID" Hash.
};



#endif // DIALOGTUNINGSOURCES_H
