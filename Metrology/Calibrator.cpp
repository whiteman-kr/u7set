#include "Calibrator.h"

#include <assert.h>
#include <QThread>


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Calibrator::Calibrator(int channel, QObject* parent) :
	QObject(parent),
	m_channel(channel),
	m_port(this)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

Calibrator::~Calibrator()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::clear()
{
	m_enableWaitResponse = false;

	if (m_port.isOpen() == true)
	{
		m_port.clear();
		m_port.close();
	}

	m_connected = false;

	m_caption.clear();
	m_serialNo.clear();

	m_timeout = 0;

	m_mode = CalibratorMode::NoMode;
	m_measureUnit = CalibratorUnit::NoUnit;
	m_sourceUnit = CalibratorUnit::NoUnit;

	m_measureValue = 0;
	m_sourceValue = 0;

	m_lastResponse.clear();
	m_lastError.clear();

	m_busy = false;
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::setConnected(bool connect)
{
	m_connected = connect;

	if (connect == true)
	{
		emit connected();
	}
	else
	{
		emit disconnected();
	}

}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::open()
{
	clear();

	if (m_portName.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, "
							  "Error description: Don't defined port name").
								arg(__FUNCTION__);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (openPort() == false)
	{
		return false;
	}

	if (getIDN() == false)
	{
		return false;
	}

	setConnected(true);

	setRemoteControl(true);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::openPort()
{
	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	CalibratorParam param = getParam(m_type);
	if (param.isValid() == false)
	{
		return false;
	}

	m_port.setPortName(m_portName);

	m_port.setBaudRate(param.baudRate);

	m_port.setDataBits(QSerialPort::Data8);
	m_port.setParity(QSerialPort::NoParity);
	m_port.setStopBits(QSerialPort::OneStop);
	m_port.setFlowControl(QSerialPort::NoFlowControl);

	if (m_port.open(QIODevice::ReadWrite) == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: %3 (%4)").arg(__FUNCTION__).
								arg(m_portName).
								arg(m_port.errorString()).
								arg(m_port.error());
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	setWaitResponse(true);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::getIDN()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	bool result = send(CALIBRATOR_IDN);
	if (result == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command send is failed").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (recv() == false)
	{
		return false;
	}

	// Extracts from the string of the last response from the calibrator name and serial number
	//
	int begPos = 0, endPos = 0;

	begPos = m_lastResponse.indexOf(',', 0);
	endPos = m_lastResponse.indexOf(',', begPos+1);

	// Calibrator name
	//
	m_caption = m_lastResponse.left(endPos);
	m_caption = m_caption.right(endPos - begPos - 1);

	if (m_caption.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration name").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	for(int type = 0; type < CalibratorTypeCount; type++)
	{
		if (m_caption == CalibratorIdnCaption(type))
		{
			m_type = static_cast<CalibratorType>(type);
			break;
		}
	}

	begPos = endPos;
	endPos = m_lastResponse.indexOf(',', begPos+1);

	// Calibrator serial number
	//
	m_serialNo = m_lastResponse.left(endPos);
	m_serialNo = m_serialNo.right(endPos - begPos -1);

	if (m_serialNo.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration serial number").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorParam Calibrator::getParam(int type)
{
	if (ERR_CALIBRATOR_TYPE(type) == true)
	{
		return CalibratorParam();
	}

	CalibratorParam param;

	for (int t = 0; t < CalibratorTypeCount; t++)
	{
		const CalibratorParam& cp = CalibratorParams[t];
		if (cp.isValid() == false)
		{
			continue;
		}

		if (cp.type != type)
		{
			continue;
		}

		param = cp;

		break;
	}

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorLimit Calibrator::getLimit(CalibratorMode mode, CalibratorUnit unit)
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return CalibratorLimit();
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return CalibratorLimit();
	}

	if (ERR_CALIBRATOR_MODE(mode) == true)
	{
		return CalibratorLimit();
	}

	if (ERR_CALIBRATOR_UNIT(unit) == true)
	{
		return CalibratorLimit();
	}

	CalibratorLimit limit;

	for(int l = 0; l < CalibratorLimitCount; l++)
	{
		const CalibratorLimit& cl = CalibratorLimits[l];
		if (cl.isValid() == false)
		{
			continue;
		}

		if (cl.type != m_type)
		{
			continue;
		}

		if (cl.mode != mode)
		{
			continue;
		}

		if (cl.unit != unit)
		{
			continue;
		}

		limit = cl;

		break;
	}

	return limit;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorLimit Calibrator::currentMeasureLimit()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return CalibratorLimit();
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return CalibratorLimit();
	}

	if (ERR_CALIBRATOR_UNIT(m_measureUnit) == true)
	{
		return CalibratorLimit();
	}

	return getLimit(CalibratorMode::Measure, m_measureUnit);
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorLimit Calibrator::currentSourceLimit()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return CalibratorLimit();
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return CalibratorLimit();
	}

	if (ERR_CALIBRATOR_UNIT(m_sourceUnit) == true)
	{
		return CalibratorLimit();
	}

	return getLimit(CalibratorMode::Source, m_sourceUnit);
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::send(QString cmd)
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	CalibratorParam param = getParam(m_type);
	if (param.isValid() == false)
	{
		return false;
	}

	if (cmd.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Empty command").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	cmd.append(param.terminamtor);	// add ending

	QByteArray cmdData = cmd.toLocal8Bit();

	// send
	//
	qint64 writtenBytes = m_port.write(cmdData);
	if (writtenBytes != cmdData.count())
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command is sent is not fully").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	emit commandIsSent(cmd);

	while(m_port.waitForBytesWritten((CALIBRATOR_TIMEOUT_STEP)));

	//qDebug("Serial Port: " + m_portName.toLocal8Bit() + " send: "+ cmd.toLocal8Bit());

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::recv()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	CalibratorParam param = getParam(m_type);
	if (param.isValid() == false)
	{
		return false;
	}

	m_lastResponse.clear();

	QByteArray requestData;

	m_timeout = 0;
	while(m_timeout < CALIBRATOR_TIMEOUT)
	{
		if (m_enableWaitResponse == false)
		{
			m_timeout = CALIBRATOR_TIMEOUT;
			break;
		}

		while (m_port.waitForReadyRead(CALIBRATOR_TIMEOUT_STEP))
		{
			requestData += m_port.readAll();
		}

		if (requestData.isEmpty() == false)
		{
			if (memcmp(requestData.data() + requestData.count() - param.terminamtor.length(),
					   param.terminamtor.toLocal8Bit(),
					   static_cast<size_t>(param.terminamtor.length())) == 0)
			{
				break;
			}
		}

		m_timeout += CALIBRATOR_TIMEOUT_STEP;
	}

	setWaitResponse(true);

	if (m_timeout == CALIBRATOR_TIMEOUT && requestData.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Calibrator don't sent a response").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);

		close();

		return false;
	}

	m_lastResponse = requestData;
	m_lastResponse.remove(param.terminamtor);

	qDebug("Function: %s, Serial Port: " + m_portName.toLocal8Bit() + ", Timeout: %d", __FUNCTION__, m_timeout);

	emit responseIsReceived(m_lastResponse);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString Calibrator::typeStr() const
{
	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		return QString ();
	}

	return CalibratorTypeCaption(m_type);
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::setUnit(int mode, int unit)
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_MODE(mode) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined mode").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError)); emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_UNIT(unit) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined unit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	CalibratorLimit limit = getLimit(static_cast<CalibratorMode>(mode), static_cast<CalibratorUnit>(unit));
	if (limit.isValid() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't find limit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (limit.cmdSetUnit.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command for set unit is empty").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	bool result = send(limit.cmdSetUnit);
	if (result == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command send is failed").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	switch(mode)
	{
		case CalibratorMode::Measure:

			m_mode = static_cast<CalibratorMode>(mode);
			m_measureUnit = static_cast<CalibratorUnit>(unit);
			break;

		case CalibratorMode::Source:

			m_mode = static_cast<CalibratorMode>(mode);
			m_sourceUnit = static_cast<CalibratorUnit>(unit);
			break;

		default:

			assert(false);
			m_mode = CalibratorMode::NoMode;
			return false;
	}

	//qDebug("Function: %s, Serial port: " + m_portName.toLocal8Bit() + ", Mode: %s, Unit: %s", __FUNCTION__, CalibratorModeCaption(mode), CalibratorUnitCaption(unit));

	emit unitIsChanged();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::setValue(double value)
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_MODE(m_mode) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined mode").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_UNIT(m_sourceUnit) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined unit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	CalibratorParam param = getParam(m_type);
	if (param.isValid() == false)
	{
		return false;
	}

	CalibratorLimit limit = getLimit(m_mode, m_sourceUnit);
	if (limit.isValid() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't find limit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	QString cmdSetValue;

	switch(m_type)
	{
		case CalibratorType::TrxII:
			{
				cmdSetValue = QString("%1%2").arg(param.cmdSetValue).arg(QString::number(value, 'f', limit.precesion));
			}
			break;

		case CalibratorType::Calys75:
			{
				switch (m_sourceUnit)
				{
					case CalibratorUnit::Hz:
						cmdSetValue = QString("%1%2").arg(param.cmdSetValue).arg(QString::number(value/1000, 'f', 4));
						break;

					default:
						cmdSetValue = QString("%1%2").arg(param.cmdSetValue).arg(QString::number(value, 'f', limit.precesion));
						break;
				}
			}
			break;

		case CalibratorType::Ktl6221:
			{
				int decimal = 0;

				switch (m_sourceUnit)
				{
					case CalibratorUnit::mA:	decimal = 3;	break;
					case CalibratorUnit::uA:	decimal = 6;	break;
					case CalibratorUnit::nA:	decimal = 9;	break;
				}

				if (decimal == 0)
				{
					break;
				}

				cmdSetValue = QString("%1%2e-%3").arg(param.cmdSetValue).arg(QString::number(value, 'f', limit.precesion + 6)).arg(decimal);
			}
			break;

		default:
			assert(false);
	}

	if (cmdSetValue.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Empty command").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	bool result = send(cmdSetValue);
	if (result == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command send is failed").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	getValue();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::stepDown()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_MODE(m_mode) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined mode").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_UNIT(m_sourceUnit) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined unit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	CalibratorLimit limit = getLimit(m_mode, m_sourceUnit);
	if (limit.isValid() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't find limit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	//double stepSize = 1 / pow(10.0, limit.precesion);
	//m_sourceValue -= stepSize;


	switch (m_type)
	{
		case CalibratorType::TrxII:
		case CalibratorType::Calys75:

			switch(m_sourceUnit)
			{
				case CalibratorUnit::Hz:		m_sourceValue -= 5;		break;
				case CalibratorUnit::OhmLow:	m_sourceValue -= 0.01;	break;
				case CalibratorUnit::OhmHigh:	m_sourceValue -= 0.1;	break;
				default:						m_sourceValue -= 0.001;	break;
			}

			break;

		case CalibratorType::Ktl6221:

			if (m_sourceValue > KTHL6221_LIMIT_FOR_SWITCH)
			{
				m_sourceValue -= 0.01;
			}
			else
			{
				m_sourceValue -= 0.001;
			}

			break;

		default:
			assert(0);
			break;
	}

	return setValue(m_sourceValue);
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::stepUp()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_MODE(m_mode) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined mode").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	if (ERR_CALIBRATOR_UNIT(m_sourceUnit) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined unit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	CalibratorLimit limit = getLimit(m_mode, m_sourceUnit);
	if (limit.isValid() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't find limit").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	//double stepSize = 1 / pow(10.0, limit.precesion);
	//m_sourceValue += stepSize;

	switch (m_type)
	{
		case CalibratorType::TrxII:
		case CalibratorType::Calys75:

			switch(m_sourceUnit)
			{
				case CalibratorUnit::Hz:		m_sourceValue += 5;		break;
				case CalibratorUnit::OhmLow:	m_sourceValue += 0.01;	break;
				case CalibratorUnit::OhmHigh:	m_sourceValue += 0.1;	break;
				default:						m_sourceValue += 0.001;	break;
			}

			break;

		case CalibratorType::Ktl6221:

			if (m_sourceValue >= KTHL6221_LIMIT_FOR_SWITCH)
			{
				m_sourceValue += 0.01;
			}
			else
			{
				m_sourceValue += 0.001;
			}

			break;

		default:
			assert(0);
			break;
	}

	return setValue(m_sourceValue);
}

// -------------------------------------------------------------------------------------------------------------------

double Calibrator::getValue()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return 0;
	}

	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return 0;
	}

	CalibratorParam param = getParam(m_type);
	if (param.isValid() == false)
	{
		return 0;
	}

	QString cmdGetValue = param.cmdGetValue;
	if (cmdGetValue.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Empty command").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return 0;
	}

	setBusy(true);

		bool result = send(cmdGetValue);
		if (result == false)
		{
			setBusy(false);
			m_lastError = QString("Calibrator error! "
								  "Function: %1, Serial port: %2, "
								  "Error description: Command send is failed").
									arg(__FUNCTION__).
									arg(m_portName);
			qDebug("%s", qPrintable(m_lastError));
			emit error(m_lastError);
			return false;
		}

		recv();
		parseResponse();

	setBusy(false);

	return m_sourceValue;
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::setBusy(bool busy)
{
	m_busy = busy;

	if (busy == true)
	{
		emit valueIsRequested();
	}
	else
	{
		emit valueIsReceived();
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::beep()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	bool result = send(CALIBRATOR_BEEP);
	if (result == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command send is failed").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::reset()
{
	if (m_port.isOpen() == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Port is not open").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	bool result = send(CALIBRATOR_RESET);
	if (result == false)
	{
		setBusy(false);
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command send is failed").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	recv();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::setRemoteControl(bool enable)
{
	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	QString cmd;

	switch (m_type)
	{
		case CalibratorType::TrxII:		return true;
		case CalibratorType::Calys75:	cmd = enable == true ? CALYS75_REMOTE_CONTROL : CALYS75_MANUAL_CONTROL;	break;
		case CalibratorType::Ktl6221:	cmd = enable == true ? KTHL6221_OUTPUT_ON : cmd = KTHL6221_OUTPUT_OFF;	break;
		default:						assert(0);																break;
	}

	bool result = send(cmd);
	if (result == false)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Command send is failed").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return false;
	}

	// timeout for switch on or off remote control
	//
	QThread::msleep(100);

	return true;
}


// -------------------------------------------------------------------------------------------------------------------

void Calibrator::parseResponse()
{
	if (ERR_CALIBRATOR_TYPE(m_type) == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined calibration type").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);
		return;
	}

	if (m_lastResponse.isEmpty() == true)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Empty response").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError));
		emit error(m_lastError);

		m_measureValue = 0;
		m_sourceValue = 0;

		return;
	}


	QString value, valueMeasure;
	int begPos, endPos;

	// Extracts from the string of the last response from the calibrator current electrical values
	//
	switch(m_type)
	{
		case CalibratorType::TrxII:

			value = m_lastResponse;
			value.remove(' ');

			begPos = value.indexOf(',');
			if (begPos == -1)
			{
				break;
			}

			valueMeasure = value.left(begPos);
			value.remove(0, begPos + 1);

			m_measureValue = valueMeasure.toDouble();
			m_sourceValue = value.toDouble();

			break;

		case CalibratorType::Calys75:

			value = m_lastResponse;

			begPos = value.indexOf("<MEAS>", 0);
			if (begPos == -1)
			{
				break;
			}

			value.remove(0, begPos + 6);

			endPos = value.indexOf("</MEAS>", 0);
			if (endPos == -1)
			{
				break;
			}

			value.remove(endPos, value.count());

			m_measureValue = value.toDouble();

			value = m_lastResponse;

			begPos = value.indexOf("<MEAS>", begPos + 1);
			if (begPos == -1)
			{
				break;
			}

			value.remove(0, begPos + 6);

			endPos = value.indexOf("</MEAS>", 0);
			if (endPos == -1)
			{
				break;
			}

			value.remove(endPos, value.count());

			m_sourceValue = value.toDouble();

			switch (m_sourceUnit)
			{
				case CalibratorUnit::Hz:	m_sourceValue *= 1000; break;
			}

			break;

		case CalibratorType::Ktl6221:

			value = m_lastResponse;
			value.remove(" \n\r");

			m_measureValue = 0;
			m_sourceValue = value.toDouble();

			switch (m_sourceUnit)
			{
				case CalibratorUnit::mA:	m_sourceValue *= 1e+3;		break;
				case CalibratorUnit::uA:	m_sourceValue *= 1e+6;		break;
				case CalibratorUnit::nA:	m_sourceValue *= 1e+9;		break;
			}

			break;

		default:
			assert(0);
	}


}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::close()
{
	if (m_port.isOpen() == true)
	{
		setRemoteControl(false);
	}

	clear();

	setConnected(false);
}


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CalibratorParam::isValid() const
{
	if (ERR_CALIBRATOR_TYPE(type) == true)
	{
		return false;
	}

	if (baudRate == 0)
	{
		return false;
	}

	if (cmdGetValue.isEmpty() == true || cmdSetValue.isEmpty() == true)
	{
		return false;
	}

	if (terminamtor.isEmpty() == true)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CalibratorLimit::isValid() const
{
	if (ERR_CALIBRATOR_TYPE(type) == true)
	{
		return false;
	}

	if (ERR_CALIBRATOR_MODE(mode) == true)
	{
		return false;
	}

	if (lowLimit == 0.0 && highLimit == 0.0)
	{
		return false;
	}

	if (ERR_CALIBRATOR_UNIT(unit) == true)
	{
		return false;
	}

	if (cmdSetUnit.isEmpty() == true)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QString CalibratorTypeCaption(int сalibratorType)
{
	QString caption;

	switch (сalibratorType)
	{
		case CalibratorType::TrxII:		caption = "TRX-II";			break;
		case CalibratorType::Calys75:	caption = "CALYS-75";		break;
		case CalibratorType::Ktl6221:	caption = "KEITHLEY-6221";	break;
		default:
			Q_ASSERT(0);
			caption = QT_TRANSLATE_NOOP("Calibrator", "Unknown");
	}

	return caption;
};

QString CalibratorIdnCaption(int сalibratorType)
{
	QString caption;

	switch (сalibratorType)
	{
		case CalibratorType::TrxII:		caption = "TRX-IIR";		break;
		case CalibratorType::Calys75:	caption = "Calys 75";		break;
		case CalibratorType::Ktl6221:	caption = "MODEL 6221";		break;
		default:
			Q_ASSERT(0);
			caption = QT_TRANSLATE_NOOP("Calibrator", "Unknown");
	}

	return caption;
};

QString CalibratorModeCaption(int сalibratorMode)
{
	QString caption;

	switch (сalibratorMode)
	{
		case CalibratorMode::Measure:	caption = QT_TRANSLATE_NOOP("Calibrator", "Measure");	break;
		case CalibratorMode::Source:	caption = QT_TRANSLATE_NOOP("Calibrator", "Source");	break;
		default:
			Q_ASSERT(0);
			caption = QT_TRANSLATE_NOOP("Calibrator", "Unknown");
	}

	return caption;
};

QString CalibratorUnitCaption(int сalibratorUnit)
{
	QString caption;

	switch (сalibratorUnit)
	{
		case CalibratorUnit::mV:		caption = QT_TRANSLATE_NOOP("Calibrator", "mV");			break;
		case CalibratorUnit::mA:		caption = QT_TRANSLATE_NOOP("Calibrator", "mA");			break;
		case CalibratorUnit::uA:		caption = QT_TRANSLATE_NOOP("Calibrator", "μA");			break;
		case CalibratorUnit::nA:		caption = QT_TRANSLATE_NOOP("Calibrator", "nA");			break;
		case CalibratorUnit::V:			caption = QT_TRANSLATE_NOOP("Calibrator", "V");				break;
		case CalibratorUnit::Hz:		caption = QT_TRANSLATE_NOOP("Calibrator", "Hz");			break;
		case CalibratorUnit::OhmLow:	caption = QT_TRANSLATE_NOOP("Calibrator", "Ohm (Low)");		break;
		case CalibratorUnit::OhmHigh:	caption = QT_TRANSLATE_NOOP("Calibrator", "Ohm (High)");	break;
		default:
			Q_ASSERT(0);
			caption = QT_TRANSLATE_NOOP("Calibrator", "Unknown");
	}

	return caption;
};

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


