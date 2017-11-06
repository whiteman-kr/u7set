#ifndef TUNINGCLIENTFILTEREDITOR_H
#define TUNINGCLIENTFILTEREDITOR_H

#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilterEditor.h"
#include "TuningClientTcpClient.h"

class DialogFilterEditor : public QDialog
{
public:

	explicit DialogFilterEditor(TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, TuningFilterStorage* filterStorage,
																	  QWidget* parent);

    ~DialogFilterEditor();

private:

    TuningFilterEditor* m_tuningFilterEditor = nullptr;
	TuningClientTcpClient* m_tuningClientTcpClient = nullptr;
	TuningSignalManager* m_tuningSignalManager = nullptr;

    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

private slots:
	void onGetCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok);
};

#endif // TUNINGCLIENTFILTEREDITOR_H
