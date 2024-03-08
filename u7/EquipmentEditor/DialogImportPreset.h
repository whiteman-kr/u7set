#ifndef DIALOGIMPORTPRESET_H
#define DIALOGIMPORTPRESET_H

#include <QDialog>

namespace Ui {
class DialogImportPreset;
}

class DialogImportPreset : public QDialog
{
	Q_OBJECT

public:
	explicit DialogImportPreset(const Proto::ExportedDevicePreset2& message, QWidget* parent);
	~DialogImportPreset();

	const ::Proto::EnvelopeSet& chosenItems() const;

private:
	virtual void accept() override;

private:
	Ui::DialogImportPreset *ui;
	const ::Proto::ExportedDevicePreset2* m_message = nullptr;
	::Proto::EnvelopeSet m_chosenItems;
};

#endif // DIALOGIMPORTPRESET_H
