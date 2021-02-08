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
#include "ProcessData.h"

// ==============================================================================================

const char* const	ErrorList =					QT_TRANSLATE_NOOP("CalibratorManager.h", "Error list");
const char* const	CalibratorNotConnected =	QT_TRANSLATE_NOOP("CalibratorManager.h", "Not connected");
const char* const	CalibratorStr =				QT_TRANSLATE_NOOP("CalibratorManager.h", "Calibrator");

// ==============================================================================================

class CalibratorManager : public QDialog
{
	Q_OBJECT

public:

	explicit CalibratorManager(Calibrator* pCalibrator, QWidget* parent = nullptr);
	virtual ~CalibratorManager();

public:

	// calibrator
	//
	Calibrator*		calibrator() const { return m_pCalibrator; }
	void			setCalibrator(Calibrator* pCalibrator) { m_pCalibrator = pCalibrator; }

	bool			calibratorIsConnected() const;
	int				calibratorChannel() const;
	QString			calibratorPort() const;

	bool			isReadyForManage() const;
	void			setReadyForManage(bool ready);
	void			waitReadyForManage();

	// function for manage
	//
	bool			setUnit(int mode, int unit);

	void			getValue();
	void			setValue(double getValue);

	void			stepDown();
	void			stepUp();

private:

	mutable QMutex	m_mutex;

	Calibrator*		m_pCalibrator = nullptr;
	bool			m_readyForManage = false;

	// elements of interface - Menu
	//
	QFont*			m_pFont;
	QLabel*			m_pMeasureLabel = nullptr;
	QLineEdit*		m_pMeasureEdit = nullptr;
	QLabel*			m_pSourceLabel = nullptr;
	QLineEdit*		m_pSourceEdit = nullptr;
	CompleterData	m_valueCompleter;
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

	void			createInterface();
	void			initDialog();
	void			setWindowCaption();
	void			enableInterface(bool enable);

	void			updateModeList();
	void			updateUnitList();
	void			updateValue();

signals:

	void			calibratorSetUnit(int mode, int unit);
	void			calibratorSetValue(double getValue);
	void			calibratorStepDown();
	void			calibratorStepUp();
	void			calibratorGetValue();
	void			calibratorRemoveControl(bool enable);

private slots:

	void			onCalibratorError(QString errorText);

	void			onCalibratorConnect();
	void			onCalibratorDisconnect();

	void			onSetMode(int modeIndex);
	void			onSetUnit(int unitIndex);
	void			onUnitChanged();

	void			onSetValue();
	void			onStepDown();
	void			onStepUp();

	void			onValueChanging();
	void			onValueChanged();

	void			onErrorList();

	void			onRemoveControl();
};

// ==============================================================================================

#endif // CALIBRATORMANAGER_H
