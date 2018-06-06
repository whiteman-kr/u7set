#ifndef CALIBRATORMANAGER_H
#define CALIBRATORMANAGER_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QMutex>

#include "Calibrator.h"

// ==============================================================================================

class CalibratorManager : public QDialog
{
	Q_OBJECT

public:

	explicit CalibratorManager(Calibrator* pCalibrator, QWidget* parent = 0);
	virtual ~CalibratorManager();

private:

	mutable QMutex	m_mutex;

	Calibrator*		m_pCalibrator = nullptr;
	bool			m_readyForManage = false;

public:

	// calibrator
	//
	Calibrator*		calibrator() const { return m_pCalibrator; }
	void			setCalibrator(Calibrator* pCalibrator) { m_pCalibrator = pCalibrator;	}

	bool			calibratorIsConnected();
	int				calibratorChannel() const;
	QString			calibratorPort() const;

	bool			isReadyForManage() const;
	void			setReadyForManage(bool ready);
	void			waitReadyForManage();

	// elements of interface - Menu
	//
	QFont*			m_pFont;
	QLabel*			m_pMeasureLabel = nullptr;
	QLineEdit*		m_pMeasureEdit = nullptr;
	QLabel*			m_pSourceLabel = nullptr;
	QLineEdit*		m_pSourceEdit = nullptr;
	QLineEdit*		m_pValueEdit = nullptr;
	QPushButton*	m_pSetValueButton = nullptr;
	QPushButton*	m_pStepDownButton = nullptr;
	QPushButton*	m_pStepUpButton = nullptr;
	QComboBox*		m_pModeList = nullptr;
	QComboBox*		m_pUnitList = nullptr;
	QPushButton*	m_pErrorsButton = nullptr;
	QCheckBox*		m_pRemoteControlCheck = nullptr;

	QDialog*		m_pErrorDialog = nullptr;
	QTextEdit*		m_pErrorList = nullptr;

	void			createManageDialog();
	void			initDialog();
	void			enableInterface(bool enable);

	// function for manage
	//

	bool			setUnit(int mode, int unit);

	void			updateValue();
	void			value();
	void			setValue(double value);

	void			stepDown();
	void			stepUp();

	// options
	//
	void			loadSettings(Calibrator* pCalibrator);
	void			saveSettings(Calibrator* pCalibrator);

signals:

	void			calibratorSetUnit(int mode, int unit);
	void			calibratorSetValue(double value);
	void			calibratorStepDown();
	void			calibratorStepUp();
	void			calibratorGetValue();
	void			calibratorRemoveControl(bool enable);

private slots:

	void			onCalibratorError(QString text);

	void			onCalibratorConnect();
	void			onCalibratorDisconnect();

	void			onUnitChanged();
	void			onModeUnitList(int);

	void			onValueChanging();
	void			onValueChanged();

	void			onSetValue();
	void			onStepDown();
	void			onStepUp();

	void			onErrorList();

	void			onRemoveControl();
};

// ==============================================================================================

#endif // CALIBRATORMANAGER_H
