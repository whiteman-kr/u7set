#ifndef TUNINGCLIENTFILTEREDITOR_H
#define TUNINGCLIENTFILTEREDITOR_H

#include "../lib/Tuning/TuningFilterEditor.h"
#include <ClientLib/TuningSignalManager.h>


class DialogFilterEditor : public QDialog
{
	Q_OBJECT
public:

	explicit DialogFilterEditor(ClientLib::TuningSignalManager& tuningSignalManager, TuningFilterStorage& filterStorage, QWidget* parent);

    ~DialogFilterEditor();

private:

	TuningFilterEditor* m_tuningFilterEditor = nullptr;
	ClientLib::TuningSignalManager& m_tuningSignalManager;

    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

private slots:
	void onGetCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok);
};

#endif // TUNINGCLIENTFILTEREDITOR_H
