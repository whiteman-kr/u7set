#include "TuningSrvClient.h"
#include "TuningGatewayServer.h"

TuningSrvClient::TuningSrvClient(std::shared_ptr<TgsSession> session,
	const SoftwareInfo& softwareInfo,
	const HostAddressPort& serverAddressPort1,
	const HostAddressPort& serverAddressPort2,
	const QString& clientDescription,
	const QString& serverEquipmentID) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2,
				clientDescription, serverEquipmentID)
{
}

void TuningSrvClient::onConnection()
{
	restartReceiveFile();
}

void TuningSrvClient::onDisconnection()
{
}

void TuningSrvClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case TDS_GET_TUNING_SOURCES_FILE:
		onGetNextFilePart(replyData, replyDataSize);
		break;

	default:
		Q_ASSERT(false);
	}
}

void TuningSrvClient::onGetNextFilePart(const char* replyData, quint32 replyDataSize)
{
	Network::GetTuningSourcesFileReply reply;

	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		restartReceiveFile();
		return;
	}

	quint64 partNo = reply.partno();

	if (partNo == 0)
	{
		m_tuningSourcesFileData.clear();
		m_tuningSourcesFileData.reserve(reply.filesize());
	}

	m_tuningSourcesFileData.insert(m_tuningSourcesFileData.end(),
								   reply.filepartdata().begin(),
								   reply.filepartdata().end());

	m_filePartNo++;

	if (m_filePartNo < reply.partscount())
	{
		requestNextFilePart();
		return;
	}

	quint64 crc64 = Crc::crc64(m_tuningSourcesFileData.data(),
							  TO_QINT64(m_tuningSourcesFileData.size()));

	if (reply.filecrc64() != crc64)
	{
		restartReceiveFile();
		return;
	}

	m_fileReady = true;
}

void TuningSrvClient::restartReceiveFile()
{
	m_filePartNo = 0;
	m_filePartsCount = 0;
	m_fileReady = false;
	m_tuningSourcesFileData.clear();
	requestNextFilePart();
}

void TuningSrvClient::requestNextFilePart()
{
	Network::GetTuningSourcesFileRequest request;

	request.set_partno(m_filePartNo);

	sendRequest(TDS_GET_TUNING_SOURCES_FILE, request);
}

TuningSrvClientThread::TuningSrvClientThread(std::shared_ptr<TgsSession> session,
											const SoftwareInfo& softwareInfo,
											const HostAddressPort& serverAddressPort1,
											const HostAddressPort& serverAddressPort2,
											const QString& clientDescription,
											const QString& serverEquipmentID)
{
	TuningSrvClient* client = new TuningSrvClient(session, softwareInfo,
												  serverAddressPort1, serverAddressPort2,
												  clientDescription, serverEquipmentID);
	addWorker(client);
}
