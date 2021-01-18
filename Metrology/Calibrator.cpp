#include "Calibrator.h"

#include <assert.h>
#include <QThread>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CalibratorParam::isValid() const
{
	if (type < 0 || type > CALIBRATOR_TYPE_COUNT)
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
	if (type < 0 || type > CALIBRATOR_TYPE_COUNT)
	{
		return false;
	}

	if (mode < 0 || mode >= CALIBRATOR_MODE_COUNT)
	{
		return false;
	}

	if (lowLimit == 0.0 && highLimit == 0.0)
	{
		return false;
	}

	if (unit < 0 || unit >= CALIBRATOR_UNIT_COUNT)
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

Calibrator::Calibrator(int channel, QObject *parent) :
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

	m_mode = CALIBRATOR_MODE_UNDEFINED;
	m_measureUnit = CALIBRATOR_UNIT_UNDEFINED;
	m_sourceUnit = CALIBRATOR_UNIT_UNDEFINED;

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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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
	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	for(int type = 0; type < CALIBRATOR_TYPE_COUNT; type++)
	{
		if (m_caption == CalibratorIdnCaption[type])
		{
			m_type = type;
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
	if (type < 0 || type >= CALIBRATOR_TYPE_COUNT)
	{
		return CalibratorParam();
	}

	CalibratorParam param;

	for (int t = 0; t < CALIBRATOR_TYPE_COUNT; t++)
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

CalibratorLimit Calibrator::getLimit(int mode, int unit)
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (mode < 0 || mode >= CALIBRATOR_MODE_COUNT)
	{
		return CalibratorLimit();
	}

	if (unit < 0 || unit >= CALIBRATOR_UNIT_COUNT)
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (m_measureUnit < 0 || m_measureUnit >= CALIBRATOR_UNIT_COUNT)
	{
		return CalibratorLimit();
	}

	return getLimit(CALIBRATOR_MODE_MEASURE, m_measureUnit);
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (m_sourceUnit < 0 || m_sourceUnit >= CALIBRATOR_UNIT_COUNT)
	{
		return CalibratorLimit();
	}

	return getLimit(CALIBRATOR_MODE_SOURCE, m_sourceUnit);
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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
	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
	{
		return QString ();
	}

	return CalibratorType[ m_type ];
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (mode < 0 || mode >= CALIBRATOR_MODE_COUNT)
	{
		m_lastError = QString("Calibrator error! "
							  "Function: %1, Serial port: %2, "
							  "Error description: Don't defined mode").
								arg(__FUNCTION__).
								arg(m_portName);
		qDebug("%s", qPrintable(m_lastError)); emit error(m_lastError);
		return false;
	}

	if (unit < 0 || unit >= CALIBRATOR_UNIT_COUNT)
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

	CalibratorLimit limit = getLimit(mode, unit);
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
		case CALIBRATOR_MODE_MEASURE:

			m_mode = mode;
			m_measureUnit = unit;
			break;

		case CALIBRATOR_MODE_SOURCE:

			m_mode = mode;
			m_sourceUnit = unit;
			break;

		default:

			assert(false);
			m_mode = CALIBRATOR_MODE_UNDEFINED;
			return false;
	}

	//qDebug("Function: %s, Serial port: " + m_portName.toLocal8Bit() + ", Mode: %s, Unit: %s", __FUNCTION__, CalibratorMode[mode], CalibratorUnit[unit]);

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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (m_mode < 0 || m_mode >= CALIBRATOR_MODE_COUNT)
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

	if (m_sourceUnit < 0 || m_sourceUnit >= CALIBRATOR_UNIT_COUNT)
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
		case CALIBRATOR_TYPE_TRXII:
			{
				cmdSetValue = QString("%1%2").arg(param.cmdSetValue).arg(QString::number(value, 'f', limit.precesion));
			}
			break;

		case CALIBRATOR_TYPE_CALYS75:
			{
				switch (m_sourceUnit)
				{
					case CALIBRATOR_UNIT_HZ:
						cmdSetValue = QString("%1%2").arg(param.cmdSetValue).arg(QString::number(value/1000, 'f', 4));
						break;

					default:
						cmdSetValue = QString("%1%2").arg(param.cmdSetValue).arg(QString::number(value, 'f', limit.precesion));
						break;
				}
			}
			break;

		case CALIBRATOR_TYPE_KTHL6221:
			{
				int decimal = 0;

				switch (m_sourceUnit)
				{
					case CALIBRATOR_UNIT_MA:	decimal = 3;	break;
					case CALIBRATOR_UNIT_UA:	decimal = 6;	break;
					case CALIBRATOR_UNIT_NA:	decimal = 9;	break;
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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (m_mode < 0 || m_mode >= CALIBRATOR_MODE_COUNT)
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

	if (m_sourceUnit < 0 || m_sourceUnit >= CALIBRATOR_UNIT_COUNT)
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
		case CALIBRATOR_TYPE_TRXII:
		case CALIBRATOR_TYPE_CALYS75:

			switch(m_sourceUnit)
			{
				case CALIBRATOR_UNIT_HZ:		m_sourceValue -= 1;		break;
				case CALIBRATOR_UNIT_LOW_OHM:	m_sourceValue -= 0.01;	break;
				case CALIBRATOR_UNIT_HIGH_OHM:	m_sourceValue -= 0.1;	break;
				default:						m_sourceValue -= 0.001;	break;
			}

			break;

		case CALIBRATOR_TYPE_KTHL6221:

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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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

	if (m_mode < 0 || m_mode >= CALIBRATOR_MODE_COUNT)
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

	if (m_sourceUnit < 0 || m_sourceUnit >= CALIBRATOR_UNIT_COUNT)
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
		case CALIBRATOR_TYPE_TRXII:
		case CALIBRATOR_TYPE_CALYS75:

			switch(m_sourceUnit)
			{
				case CALIBRATOR_UNIT_HZ:		m_sourceValue += 1;		break;
				case CALIBRATOR_UNIT_LOW_OHM:	m_sourceValue += 0.01;	break;
				case CALIBRATOR_UNIT_HIGH_OHM:	m_sourceValue += 0.1;	break;
				default:						m_sourceValue += 0.001;	break;
			}

			break;

		case CALIBRATOR_TYPE_KTHL6221:

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

	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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
	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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
		case CALIBRATOR_TYPE_TRXII:		return true;
		case CALIBRATOR_TYPE_CALYS75:	cmd = enable == true ? CALYS75_REMOTE_CONTROL : CALYS75_MANUAL_CONTROL;	break;
		case CALIBRATOR_TYPE_KTHL6221:	cmd = enable == true ? KTHL6221_OUTPUT_ON : cmd = KTHL6221_OUTPUT_OFF;	break;
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
	if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
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
		case CALIBRATOR_TYPE_TRXII:

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

		case CALIBRATOR_TYPE_CALYS75:

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
				case CALIBRATOR_UNIT_HZ:	m_sourceValue *= 1000; break;
			}

			break;

		case CALIBRATOR_TYPE_KTHL6221:

			value = m_lastResponse;
			value.remove(" \n\r");

			m_measureValue = 0;
			m_sourceValue = value.toDouble();

			switch (m_sourceUnit)
			{
				case CALIBRATOR_UNIT_MA:	m_sourceValue *= 1e+3;		break;
				case CALIBRATOR_UNIT_UA:	m_sourceValue *= 1e+6;		break;
				case CALIBRATOR_UNIT_NA:	m_sourceValue *= 1e+9;		break;
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


