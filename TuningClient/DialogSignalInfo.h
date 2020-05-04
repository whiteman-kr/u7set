#ifndef DIALOGSIGNALINFO_H
#define DIALOGSIGNALINFO_H

#include <QDialog>
#include "../lib/Hash.h"
#include "../lib/Types.h"

namespace Ui {
	class DialogSignalInfo;
}

class TuningSignalManager;
class TuningClientTcpClient;

class DialogSignalInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSignalInfo(Hash appSignalHash, E::AnalogFormat analogFormat, Hash instanceIdHash, TuningSignalManager* signalManager, QWidget* parent = 0);
	~DialogSignalInfo();

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void updateInfo();

private:
	Ui::DialogSignalInfo *ui;

	int m_timerId = -1;

	Hash m_appSignalHash = UNDEFINED_HASH;
	E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
	Hash m_instanceIdHash = UNDEFINED_HASH;

	TuningSignalManager* m_signalManager = nullptr;

	QString m_textEditText;
};

#endif // DIALOGSIGNALINFO_H
