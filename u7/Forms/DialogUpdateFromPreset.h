#ifndef DIALOGUPDATEFROMPRESET_H
#define DIALOGUPDATEFROMPRESET_H
#include <QDialog>

class QTextEdit;
class QListWidget;
class QPushButton;

class DialogUpdateFromPreset : public QDialog
{
public:
	DialogUpdateFromPreset(bool expertOptions, QStringList& availablePresets, QWidget* parent);

public:
	QStringList forceUpdateProperties() const;
	QStringList selectedPresets() const;

private slots:
	void clearPresetBittonClicked();

private:
	QTextEdit* m_forceUpdateEdit = nullptr;
	QListWidget* m_selectedPresetsList = nullptr;
	QPushButton* m_clearPresetButton = nullptr;

	QStringList m_availablePresets;
};

#endif // DIALOGUPDATEFROMPRESET_H
