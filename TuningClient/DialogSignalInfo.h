#ifndef DIALOGSIGNALINFO_H
#define DIALOGSIGNALINFO_H

#include <QDialog>
#include "../lib/Hash.h"

namespace Ui {
	class DialogSignalInfo;
}

class TuningSignalManager;
class TuningClientTcpClient;

class DialogSignalInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSignalInfo(Hash appSignalHash, Hash instanceIdHash, TuningSignalManager* signalManager, QWidget *parent = 0);
	~DialogSignalInfo();

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void updateInfo();

private:
	Ui::DialogSignalInfo *ui;

	int m_timerId = -1;

	Hash m_appSignalHash;

	Hash m_instanceIdHash;

	TuningSignalManager* m_signalManager = nullptr;

	QString m_textEditText;
};

#endif // DIALOGSIGNALINFO_H
