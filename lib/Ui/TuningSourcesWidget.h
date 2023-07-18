#pragma once

#include <QDialog>

#include "DialogSourceInfo.h"
#include "../CommonLib/Hash.h"
#include "../ClientLib/TuningConnection.h"

namespace ClientLib
{
	class TuningTcpClient;
	class TuningSource;
}

//
// DialogTuningSourceInfo
//

class DialogTuningSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogTuningSourceInfo(ClientLib::TuningConnection& connection, QWidget* parent, quint64 sourceId, Hash lanEquipmentHash);
	virtual ~DialogTuningSourceInfo();

private:
	void updateData() override;

	void updateInfo(const ClientLib::TuningSource& ts);
	void updateState(const ClientLib::TuningSource& ts);

private:
	ClientLib::TuningConnection& m_tuningConnection;
	int m_noStateInfoTimeout = 0;

	Hash m_sourceHash;
	Hash m_lanEquipmentHash;
};

class TuningSourcesWidget : public QWidget
{
	Q_OBJECT
public:

	explicit TuningSourcesWidget(ClientLib::TuningConnection& tuningConnection, bool hasActivationControls, QWidget* parent);
	virtual ~TuningSourcesWidget();

	bool treeIsFocused() const;

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
	void contextMenuRequested();

private:
	void updateData();
	void updateTuningSourcesStates();
	void enableActivationControls();
	void activateControl(bool enable);

signals:
	void activationControlsAccessChanged(bool activateEnabled, bool deactivateEnabled);
	void activateSourceControl(const QString& sourceEquipmentId, bool activate);

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
		LmTime,
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


	ClientLib::TuningConnection& m_tuningConnection;

	static const int columnIndex_SourceHash = 0;
	static const int columnIndex_SourceEquipmentId = 1;
	static const int columnIndex_ControllerHash = 2;

	std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap;	// Used for managing details dialogs. Key is LAN controller hash
};
