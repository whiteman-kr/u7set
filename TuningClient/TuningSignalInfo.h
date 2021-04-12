#ifndef TUNINGSIGNALINFO_H
#define TUNINGSIGNALINFO_H

#include <QDialog>
#include "../UtilsLib/Hash.h"
#include "../lib/Types.h"

namespace Ui {
	class TuningSignalInfo;
}

class TuningSignalManager;
class TuningClientTcpClient;

class TuningSignalInfo : public QDialog
{
	Q_OBJECT

public:
	explicit TuningSignalInfo(Hash appSignalHash, E::AnalogFormat analogFormat, Hash instanceIdHash, TuningSignalManager* signalManager, QWidget* parent = 0);
	~TuningSignalInfo();

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void updateInfo();

private:
	Ui::TuningSignalInfo *ui;

	int m_timerId = -1;

	Hash m_appSignalHash = UNDEFINED_HASH;
	E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
	Hash m_instanceIdHash = UNDEFINED_HASH;

	TuningSignalManager* m_signalManager = nullptr;

	QString m_textEditText;
};

#endif // TUNINGSIGNALINFO_H
