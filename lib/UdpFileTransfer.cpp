#include "../lib/UdpFileTransfer.h"

/*

quint32 ReceivedFile::m_serialID = 0;


ReceivedFile::ReceivedFile(const QString& fileName, quint32 fileSize)
{
	if (fileSize > SEND_FILE_MAX_SIZE)
	{
		assert(fileSize <= SEND_FILE_MAX_SIZE);
		return;
	}

	m_serialID++;

	m_ID = m_serialID;

	m_fileName = fileName;
	m_fileSize = fileSize;

	m_data = new char [fileSize];

	assert(m_data != nullptr);
}


ReceivedFile::~ReceivedFile()
{
	delete m_data;
}


bool ReceivedFile::appendData(const char* ptr, quint32 len)
{
	if (m_dataSize + len > m_fileSize)
	{
		assert(m_dataSize + len <= m_fileSize);
		return false;
	}

	if (m_data == nullptr)
	{
		assert(m_data != nullptr);
		return false;
	}

	memcpy(m_data + m_dataSize, ptr, len);

	m_dataSize += len;

	return true;
}


quint32 ReceivedFile::CRC32()
{
	assert(m_dataSize == m_fileSize);

	if (m_data == nullptr)
	{
		assert(m_data != nullptr);
		return 0;
	}

	return ::CRC32(m_data, m_dataSize);
}


*/

/*
BaseServiceWorker::~BaseServiceWorker()
{
	delete m_sendFileClientSocketThread;

	QHashIterator<quint32, ReceivedFile*> i(m_receivedFile);

	while (i.hasNext())
	{
		i.next();

		delete i.value();
	}

	m_receivedFile.clear();
}

*/



/*
void BaseServiceWorker::onThreadStarted()
{
	m_serverSocketThread = new UdpSocketThread;

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, serviceTypesInfo[m_serviceType].port);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &BaseServiceWorker::onBaseRequest);
	connect(this, &BaseServiceWorker::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	connect(this, &BaseServiceWorker::startMainFunction, m_baseServiceController, &BaseServiceController::startMainFunction);
	connect(this, &BaseServiceWorker::stopMainFunction, m_baseServiceController, &BaseServiceController::stopMainFunction);
	connect(this, &BaseServiceWorker::restartMainFunction, m_baseServiceController, &BaseServiceController::restartMainFunction);

	connect(m_baseServiceController, &BaseServiceController::sendFile, this, &BaseServiceWorker::onSendFile);
	connect(this, &BaseServiceWorker::endSendFile, m_baseServiceController, &BaseServiceController::onEndSendFile);

	connect(this, &BaseServiceWorker::fileReceived, m_baseServiceController, &BaseServiceController::onFileReceived);
	connect(m_baseServiceController, &BaseServiceController::freeReceivedFile, this, &BaseServiceWorker::onFreeReceivedFile);

	m_serverSocketThread->run(serverSocket);

	threadStarted();
}
*/


/*
void BaseServiceWorker::onBaseRequest(UdpRequest request)
{
	UdpRequest ack;

	ack.initAck(request);

	switch(request.ID())
	{

............

		case RQID_SEND_FILE_START:
			onSendFileStartRequest(request, ack);
			break;

		case RQID_SEND_FILE_NEXT:
			onSendFileNextRequest(request, ack);
			break;

............

		default:
			assert(false);
			ack.setErrorCode(RQERROR_UNKNOWN_REQUEST);
			break;
	}

	emit ackBaseRequest(ack);
}
*/

/*
void BaseServiceWorker::onSendFile(QHostAddress address, quint16 port, QString fileName)
{
	if (m_sendFileClientSocketThread != nullptr)
	{
		assert(m_sendFileClientSocketThread == nullptr);	// file send already running
		return;
	}

	m_fileToSend.setFileName(fileName);

	if (!m_fileToSend.open(QIODevice::ReadOnly))
	{
		emit endSendFile(false, fileName);
		return;
	}

	m_fileToSendInfo.setFile(m_fileToSend);

	if (m_fileToSendInfo.size() > SEND_FILE_MAX_SIZE)
	{
		m_fileToSend.close();
		emit endSendFile(false, fileName);
		return;
	}

	// run UDP client socket thread for send file
	//
	m_sendFileClientSocketThread = new UdpSocketThread;

	UdpClientSocket* clientSocket = new UdpClientSocket(address, port);

	connect(this, &BaseServiceWorker::sendFileRequest, clientSocket, &UdpClientSocket::sendRequest);
	connect(clientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceWorker::onSendFileAckReceived);
	connect(clientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceWorker::onSendFileAckTimeout);

	m_sendFileClientSocketThread->run(clientSocket);

	// compose SEND_FILE_START request
	//

	// set file  name
	//
	memset(m_sendFileStart.fileName, 0, sizeof(m_sendFileStart.fileName));

	QString fName = m_fileToSendInfo.fileName();

	QChar* strPtr = fName.data();

	int i = 0;

	int fileNameLen = sizeof(m_sendFileStart.fileName) / sizeof(ushort);

	do
	{
		if (strPtr->isNull())
		{
			break;
		}

		m_sendFileStart.fileName[i] = strPtr->unicode();

		strPtr++;

		i++;
	}
	while (i < fileNameLen - 1);

	m_sendFileStart.fileName[i] = 0;

	// set file size
	//
	m_sendFileStart.fileSize = m_fileToSendInfo.size();

	// send REND_FILE_START request
	//
	UdpRequest request;

	request.setID(RQID_SEND_FILE_START);

	request.initWrite();

	request.writeData(reinterpret_cast<char*>(&m_sendFileStart), sizeof(m_sendFileStart));

	sendFileRequest(request);

	m_sendFileNext.fileID = 0;

	APP_MSG(m_log, QString("Sending of file %1 was started").arg(fName));

	m_sendFileFirstRead = true;

	m_sendFileReadNextPartOK = sendFileReadNextPart();
}


bool BaseServiceWorker::sendFileReadNextPart()
{
	bool lastPart = false;

	if (m_sendFileFirstRead)
	{
		// init send file variables
		//
		m_sendFileNext.fileID = 0;
		m_sendFileNext.partNo = 0;
		m_sendFileNext.partCount = m_fileToSendInfo.size() / SEND_FILE_DATA_SIZE + (m_fileToSendInfo.size() % SEND_FILE_DATA_SIZE == 0 ? 0 : 1);
		m_sendFileNext.dataSize = 0;
		m_sendFileNext.CRC32 = CRC32_INITIAL_VALUE;

		m_sendFileFirstRead = false;
	}
	else
	{
		m_sendFileNext.partNo++;
	}

	if (m_sendFileNext.partNo == m_sendFileNext.partCount - 1)
	{
		lastPart = true;
	}

	quint64 readed = m_fileToSend.read(m_sendFileNext.data, SEND_FILE_DATA_SIZE);

	m_sendFileNext.dataSize = readed;

	m_sendFileNext.CRC32 = CRC32(m_sendFileNext.CRC32, m_sendFileNext.data, readed, lastPart);

	return true;
}


void BaseServiceWorker::onSendFileAckReceived(UdpRequest udpRequest)
{
	udpRequest.initRead();

	UdpRequest request;

	switch(udpRequest.ID())
	{
		case RQID_SEND_FILE_START:

			if (udpRequest.errorCode() == RQERROR_OK)
			{
				m_sendFileNext.fileID = udpRequest.readDword();

				request.setID(RQID_SEND_FILE_NEXT);

				request.initWrite();

				request.writeData(reinterpret_cast<char*>(&m_sendFileNext), sizeof(m_sendFileNext));

				sendFileRequest(request);

				if (m_sendFileNext.partNo != m_sendFileNext.partCount - 1)
				{
					sendFileReadNextPart();
				}
			}
			else
			{
				stopSendFile(udpRequest.errorCode());
				emit endSendFile(false, m_fileToSendInfo.fileName());
			}

			break;

		case RQID_SEND_FILE_NEXT:
			if (udpRequest.errorCode() == RQERROR_OK)
			{
				if (m_sendFileNext.partNo == m_sendFileNext.partCount)
				{
					// this is ack on last part send
					//
					stopSendFile(udpRequest.errorCode());
					emit endSendFile(true, m_fileToSendInfo.fileName());
				}
				else
				{
					request.setID(RQID_SEND_FILE_NEXT);

					request.initWrite();

					request.writeData(reinterpret_cast<char*>(&m_sendFileNext), sizeof(m_sendFileNext));

					sendFileRequest(request);

					if (m_sendFileNext.partNo != m_sendFileNext.partCount - 1)
					{
						// no last part, - read next part
						//
						sendFileReadNextPart();
					}
					else
					{
						// last part, set partNo equal to partCount
						//
						m_sendFileNext.partNo++;
					}
				}
			}
			else
			{
				stopSendFile(udpRequest.errorCode());
				emit endSendFile(false, m_fileToSendInfo.fileName());
			}
			break;
	}
}


void BaseServiceWorker::onSendFileAckTimeout()
{
	stopSendFile(RQERROR_TIMEOUT);
	emit endSendFile(false, m_fileToSendInfo.fileName());
}


void BaseServiceWorker::stopSendFile(quint32 errorCode)
{
	if (errorCode == RQERROR_OK)
	{
		APP_MSG(m_log, QString("Sending of file %1 (ID = %2) was finished OK.").arg(m_fileToSendInfo.fileName()).arg(m_sendFileNext.fileID));
	}
	else
	{
		APP_WRN(m_log, QString("Sending of file %1 (ID = %2) was finished. Error code = %3.").arg(m_fileToSendInfo.fileName()).arg(m_sendFileNext.fileID).arg(errorCode));
	}

	m_fileToSend.close();

	m_sendFileClientSocketThread->quit();

	delete m_sendFileClientSocketThread;

	m_sendFileClientSocketThread = nullptr;
}


void BaseServiceWorker::onSendFileStartRequest(const UdpRequest& request, UdpRequest& ack)
{
	const SendFileStart* sendFileStart = reinterpret_cast<const SendFileStart*>(request.data());

	QString fileName(reinterpret_cast<const QChar*>(sendFileStart->fileName));

	APP_MSG(m_log, QString("Receiving of file %1 was started.").arg(fileName));

	ReceivedFile* rf = new ReceivedFile(fileName, sendFileStart->fileSize);

	if (rf->ID() == 0)
	{
		// error
		//
		ack.writeDword(0);

		delete rf;
	}
	else
	{
		ack.writeDword(rf->ID());

		m_receivedFile.insert(rf->ID(), rf);
	}
}


void BaseServiceWorker::onSendFileNextRequest(const UdpRequest &request, UdpRequest &ack)
{
	const SendFileNext* sendFileNext = reinterpret_cast<const SendFileNext*>(request.data());

	if (m_receivedFile.contains(sendFileNext->fileID) == false)
	{
		ack.setErrorCode(RQERROR_UNKNOWN_FILE_ID);
		return;

	}

	ReceivedFile* rf = 	m_receivedFile.value(sendFileNext->fileID);

	if (rf == nullptr)
	{
		ack.setErrorCode(RQERROR_RECEIVE_FILE);

		m_receivedFile.remove(sendFileNext->fileID);

		APP_ERR(m_log, QString("Receiving of file was terminated. Internal error."));

		return;
	}

	if (!rf->appendData(sendFileNext->data, sendFileNext->dataSize))
	{
		qDebug() << "File Receive Error";

		ack.setErrorCode(RQERROR_RECEIVE_FILE);

		APP_WRN(m_log, QString("Receiving of file %1 (ID = %2) was terminated.").arg(rf->fileName()).arg(rf->ID()));

		m_receivedFile.remove(sendFileNext->fileID);

		delete rf;

		return;
	}

	if (sendFileNext->partNo < sendFileNext->partCount - 1)
	{
		return;
	}

	// file receive was completed
	//

	quint32 crc32 = rf->CRC32();

	if (crc32 != sendFileNext->CRC32)
	{
		ack.setErrorCode(RQERROR_RECEIVE_FILE);

		APP_WRN(m_log, QString("Receiving of file %1 (ID = %2) was terminated. Bad CRC32.").arg(rf->fileName()).arg(rf->ID()));

		m_receivedFile.remove(sendFileNext->fileID);

		delete rf;

		return;
	}

	APP_MSG(m_log, QString("Receiving of file %1 (ID = %2) was finished OK.").arg(rf->fileName()).arg(rf->ID()));

	emit fileReceived(rf);
}


void BaseServiceWorker::onFreeReceivedFile(quint32 fileID)
{
	if (m_receivedFile.contains(fileID))
	{
		ReceivedFile* rf = m_receivedFile.value(fileID);

		delete rf;

		m_receivedFile.remove(fileID);
	}
}*/


/*
void BaseServiceController::onEndSendFile(bool result, QString fileName)
{
	if (result == false)
	{
		qDebug() << "Send file error: " << fileName;
	}
	else
	{
		qDebug() << "File " << fileName << " sent OK";
	}
}


void BaseServiceController::onFileReceived(ReceivedFile* receivedFile)
{
	if (receivedFile == nullptr)
	{
		assert(receivedFile != nullptr);
		return;
	}

	qDebug() << "File " << receivedFile->fileName() << " received OK";

	emit freeReceivedFile(receivedFile->ID());
}*/

