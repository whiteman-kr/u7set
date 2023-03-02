#ifndef TUNINGSOURCESWIDGET_H
#define TUNINGSOURCESWIDGET_H

#include <QDialog>

#include "DialogSourceInfo.h"
#include "../CommonLib/Hash.h"

class TuningTcpClient;
class TuningSource;

//
// DialogTuningSourceInfo
//

class DialogTuningSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogTuningSourceInfo(std::vector<TuningTcpClient*> tcpClients, QWidget* parent, quint64 sourceId, Hash lanEquipmentHash);
	virtual ~DialogTuningSourceInfo();

	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

private:
	bool findActiveTuningTcpClient();

	void updateData() override;

	void updateInfo();
	void updateState();

private:
	std::vector<TuningTcpClient*> m_tcpClients;
	int m_noStateInfoTimeout = 0;

	TuningTcpClient* m_activeTcpClient = nullptr;

	Hash m_sourceHash;
	Hash m_lanEquipmentHash;
};

class TuningSourcesWidget : public QWidget
{
	Q_OBJECT
public:

	explicit TuningSourcesWidget(std::vector<TuningTcpClient*> tcpClients, bool hasActivationControls, QWidget* parent);
	virtual ~TuningSourcesWidget();

	bool treeIsFocused() const;

	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

public slots:
	void detailsClicked();
	void enableControlClicked();
	void disableControlClicked();

protected:
	void timerEvent(QTimerEvent* event);

	virtual bool login();	// Override this function to ask password before activating/deactivating sources

private slots:
	void treeWidgetItemSelectionChanged();
	void treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
	void detailsDialogClosed(Hash lanControllerHash);
	void tuningSourcesInfoArrived();
	void contextMenuRequested();

private:
	void updateAll();
	void fillTuningSourcesInfo();
	void updateTuningSourcesStates();
	void enableActivationControls();
	void activateControl(bool enable);

signals:
	void activationControlsAccessChanged(bool activateEnabled, bool deactivateEnabled);

private:
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
	QWidget* m_parent = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

	int m_updateStateTimerId = -1;

	bool m_hasActivationControls = false;
	bool m_buttonActivateEnabled = false;
	bool m_buttonDeactivateEnabled = false;


	std::vector<TuningTcpClient*> m_tuningTcpClients;

	static const int columnIndex_SourceHash = 0;
	static const int columnIndex_SourceEquipmentId = 1;
	static const int columnIndex_ControllerHash = 2;

	bool m_tuningSourcesInfoArrived = false;

	std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap;	// Used for managing details dialogs. Key is LAN controller hash
};



#endif // DIALOGTUNINGSOURCES_H
