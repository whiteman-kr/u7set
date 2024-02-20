#ifndef TUNINGSIGNALINFO_H
#define TUNINGSIGNALINFO_H

#include <QDialog>
#include "../OnlineLib/SoftwareSettings.h"
#include "TuningConfigController.h"

namespace Ui {
	class TuningSignalInfo;
}

namespace ClientLib 
{
	class TuningConnection;
}

class TuningSignalManager;

class TuningSignalInfo : public QDialog
{
	Q_OBJECT

public:
	explicit TuningSignalInfo(TuningConfigController& configController,
							  const TuningSignalManager& signalManager,
							  const ClientLib::TuningConnection& tuningConnection,
							  Hash appSignalHash,
							  E::AnalogFormat analogFormat,
							  QWidget* parent = 0);
	~TuningSignalInfo();

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void updateInfo();

private slots:
	void onValueContextMenu();
	void onPropertiesContextMenu();

private:

	TuningConfigController& m_configController;

	Hash m_appSignalHash = UNDEFINED_HASH;
	E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
	AppSignalParam m_asp;
	int m_precision = 0;

	const TuningSignalManager& m_signalManager;
	const ClientLib::TuningConnection& m_tuningConnection;

	Hash m_clientHash;
	TuningClientSettings::LmStatusFlagMode m_lmStatusFlagMode = TuningClientSettings::LmStatusFlagMode::SOR;

	Ui::TuningSignalInfo *ui;
	int m_timerId = -1;
	QString m_textEditText;
};

#endif // TUNINGSIGNALINFO_H
