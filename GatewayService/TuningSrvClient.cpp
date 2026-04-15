#include "TuningSrvClient.h"
#include "TuningGatewayServer.h"

// -------------------------------------------------------------------------------------
//
// TuningSrvClient class implementation
//
// -------------------------------------------------------------------------------------

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
	clearReceiveFileVars();
}

bool TuningSrvClient::getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount)
{
	if (m_fileReady == false)
	{
		fileSize = 0;
		maxPartSize = 0;
		partCount = 0;
		return false;
	}

	fileSize = TO_QUINT64(m_tuningSourcesFileData.size());
	maxPartSize = TDS_TUNING_SOURCES_FILE_PART_SIZE;
	partCount = (fileSize + TDS_TUNING_SOURCES_FILE_PART_SIZE - 1) / TDS_TUNING_SOURCES_FILE_PART_SIZE;

	return true;
}

bool TuningSrvClient::getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize)
{
	if (m_fileReady == false)
	{
		return false;
	}

	quint64 fileSize = TO_UINT64(m_tuningSourcesFileData.size());

	quint64 partStart = partNo * TDS_TUNING_SOURCES_FILE_PART_SIZE;

	if (partStart >= fileSize)
	{
		return false;
	}

	partSize = std::min(fileSize - partStart, TDS_TUNING_SOURCES_FILE_PART_SIZE);

	fileData.insert(fileData.end(), m_tuningSourcesFileData.begin() + partStart,
					m_tuningSourcesFileData.begin() + partStart + partSize);
	return true;
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
	clearReceiveFileVars();
	requestNextFilePart();
}

void TuningSrvClient::requestNextFilePart()
{
	Network::GetTuningSourcesFileRequest request;

	request.set_partno(m_filePartNo);

	sendRequest(TDS_GET_TUNING_SOURCES_FILE, request);
}

void TuningSrvClient::clearReceiveFileVars()
{
	m_filePartNo = 0;
	m_filePartsCount = 0;
	m_fileReady = false;
	m_tuningSourcesFileData.clear();
}

// -------------------------------------------------------------------------------------
//
// TuningSrvClientThread class implementation
//
// -------------------------------------------------------------------------------------

TuningSrvClientThread::TuningSrvClientThread(std::shared_ptr<TgsSession> session,
											const SoftwareInfo& softwareInfo,
											const HostAddressPort& serverAddressPort1,
											const HostAddressPort& serverAddressPort2,
											const QString& clientDescription,
											const QString& serverEquipmentID)
{
	m_client = new TuningSrvClient(session, softwareInfo,
								  serverAddressPort1, serverAddressPort2,
								  clientDescription, serverEquipmentID);
	addWorker(m_client);
}

bool TuningSrvClientThread::getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount)
{
	if (m_client == nullptr)
	{
		return false;
	}

	return m_client->getTuningSourcesFileMetrics(fileSize, maxPartSize, partCount);
}

bool TuningSrvClientThread::getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize)
{
	if (m_client == nullptr)
	{
		return false;
	}

	return m_client->getTuningSourcesFilePart(partNo, fileData, partSize);
}

