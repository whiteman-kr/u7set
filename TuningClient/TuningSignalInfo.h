#ifndef TUNINGSIGNALINFO_H
#define TUNINGSIGNALINFO_H

#include <QDialog>
#include "../CommonLib/Hash.h"
#include "../CommonLib/Types.h"
#include "TuningClientTcpClient.h"

namespace Ui {
	class TuningSignalInfo;
}

class TuningSignalManager;
class TuningTcpClient;

class TuningSignalInfo : public QDialog
{
	Q_OBJECT

public:
	explicit TuningSignalInfo(Hash appSignalHash, E::AnalogFormat analogFormat, Hash instanceIdHash,
							  TuningSignalManager& signalManager,
							  std::vector<TuningTcpClient*> tuningTcpClients,
							  LmStatusFlagMode lmStatusFlagMode,
							  QWidget* parent = 0);
	~TuningSignalInfo();

	void setTuningTcpClients(std::vector<TuningTcpClient*> tuningTcpClients);

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void updateInfo();

private:

	Hash m_appSignalHash = UNDEFINED_HASH;
	E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
	Hash m_instanceIdHash = UNDEFINED_HASH;

	TuningSignalManager& m_signalManager;

	std::vector<TuningTcpClient*> m_signalTcpClients;
	LmStatusFlagMode m_lmStatusFlagMode = LmStatusFlagMode::SOR;

	Ui::TuningSignalInfo *ui;
	int m_timerId = -1;
	QString m_textEditText;
};

#endif // TUNINGSIGNALINFO_H
