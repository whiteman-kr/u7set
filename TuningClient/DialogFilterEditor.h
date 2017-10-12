#ifndef TUNINGCLIENTFILTEREDITOR_H
#define TUNINGCLIENTFILTEREDITOR_H

#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilterEditor.h"

class DialogFilterEditor : public QDialog
{
public:

    explicit DialogFilterEditor(TuningSignalManager* tuningSignalManager, TuningFilterStorage* filterStorage, const TuningSignalStorage* objects,
                                                                      QWidget* parent);

    ~DialogFilterEditor();

private:

    TuningFilterEditor* m_tuningFilterEditor = nullptr;
	TuningSignalManager* m_tuningSignalManager = nullptr;

    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

private slots:
    void onGetCurrentSignalValue(Hash appSignalHash, float* value, bool* ok);
};

#endif // TUNINGCLIENTFILTEREDITOR_H
