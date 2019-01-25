#ifndef DIALOGUPDATEFROMPRESET_H
#define DIALOGUPDATEFROMPRESET_H

class QTextEdit;
class QListWidget;
class QPushButton;
class QCheckBox;

class DialogUpdateFromPreset : public QDialog
{
public:
	DialogUpdateFromPreset(bool expertOptions, QStringList& availablePresets, QWidget* parent);

public:
	QStringList forceUpdateProperties() const;
	QStringList selectedPresets() const;
	bool updateAppSignlas() const;

private slots:
	void clearPresetBittonClicked();

private:
	QTextEdit* m_forceUpdateEdit = nullptr;
	QListWidget* m_selectedPresetsList = nullptr;
	QPushButton* m_clearPresetButton = nullptr;

	QCheckBox* m_updateAppSignalsCheckBox = nullptr;

	QStringList m_availablePresets;

	QObject m_parent;
};

#endif // DIALOGUPDATEFROMPRESET_H
