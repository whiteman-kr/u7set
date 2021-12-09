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
	explicit DialogTuningSourceInfo(std::vector<TuningTcpClient*> tcpClients, QWidget* parent, Hash m_sourceHash, Hash lanEquipmentHash);
	virtual ~DialogTuningSourceInfo();

	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

private:
	bool findActiveTuningTcpClient();

	void updateData() override;

	void updateInfo();
	void updateState();

private:
	std::vector<TuningTcpClient*> m_tcpClients;

	TuningTcpClient* m_activeTcpClient = nullptr;

	Hash m_sourceHash;
	Hash m_lanEquipmentHash;
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

	void tuningSourcesInfoArrived();

private:
	void updateAll();

	void fillTuningSourcesInfo();

	void updateTuningSourcesStates();

	void enableActivationControls();

	void activateControl(bool enable);

	Hash selectedSourceHash() const;
	Hash selectedLanControllerHash() const;

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

	static const int columnIndex_SourceHash = 0;

	static const int columnIndex_ControllerHash = 0;

	bool m_tuningSourcesInfoArrived = false;

	std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap;	// Used for managing details dialogs. Key is "Source ID + Channel" Hash.

	std::map<Hash, QTreeWidgetItem*> m_sourceHashToSourceItemMap;
	std::map<Hash, QTreeWidgetItem*> m_controllerHashToControllerItemMap;
};



#endif // DIALOGTUNINGSOURCES_H
