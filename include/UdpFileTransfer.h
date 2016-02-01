#pragma once

/*
class ReceivedFile
{
private:
	static quint32 m_serialID;

	quint32 m_ID = 0;
	QString m_fileName;
	quint32 m_fileSize = 0;
	quint32 m_CRC32 = CRC32_INITIAL_VALUE;
	quint32 m_dataSize = 0;
	char* m_data = nullptr;

public:
	ReceivedFile(const QString& fileName, quint32 fileSize);
	~ReceivedFile();

	quint32 ID() { return m_ID; }

	bool appendData(const char *ptr, quint32 len);

	QString fileName() const { return m_fileName; }

	quint32 CRC32();
};

*/

/*

class UdpFileTransfer
{
private:
	// members needed for file sending
	//
	UdpSocketThread* m_sendFileClientSocketThread = nullptr;

	QFile m_fileToSend;
	QFileInfo m_fileToSendInfo;

	SendFileStart m_sendFileStart;
	SendFileNext m_sendFileNext;

	bool m_sendFileReadNextPartOK = false;
	bool m_sendFileFirstRead = true;

	bool sendFileReadNextPart();
	void stopSendFile(quint32 errorCode);

	// members needed for file receiving
	//
	QHash<quint32, ReceivedFile*> m_receivedFile;

	void onSendFileStartRequest(const UdpRequest &request, UdpRequest &ack);
	void onSendFileNextRequest(const UdpRequest &request, UdpRequest &ack);

signals:

		void endSendFile(bool result, QString fileName);
		void fileReceived(ReceivedFile* receivedFile);

		void sendFileRequest(UdpRequest request);

private slots:
	void onSendFileAckReceived(UdpRequest udpRequest);
	void onSendFileAckTimeout();

public slots:
		void onSendFile(QHostAddress address, quint16 port, QString fileName);
		void onFreeReceivedFile(quint32 fileID);
};
*/
