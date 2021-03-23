#ifndef SOURCEOPTIONS_H
#define SOURCEOPTIONS_H

#include <QDesktopWidget>
#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "Options.h"

// ==============================================================================================

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OptionsDialog(const BuildOption& buildOption, QWidget *parent = nullptr);
	virtual ~OptionsDialog() override;

public:

	BuildOption& buildOption() { return m_buildOption; }

private:

	BuildOption m_buildOption;

	QLineEdit* m_cfgSrvIDEdit = nullptr;
	QLineEdit* m_cfgSrvIPEdit = nullptr;
	QLineEdit* m_cfgSrvPortEdit = nullptr;
	QLineEdit* m_appDataSrvIDEdit = nullptr;

	QLineEdit* m_ualTesterIPEdit = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;

	bool createInterface();

private slots:

	// slots of buttons
	//
	void onUalTesterIP(const QString &ip);

	void onOk();
};

// ==============================================================================================

#endif // SOURCEOPTIONS_H
