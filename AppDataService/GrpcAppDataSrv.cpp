#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SocketIO.h"

#include "GrpcAppDataSrv.h"

GrpcAppDataSrv::GrpcAppDataSrv(	const AppDataServiceSettings& settings,
								CircularLoggerShared log) :
	m_log(log)
{
	ServerBuilder builder;

	QStringList ips;

	for(const RqCtrlSettings& rcs : settings.rcSettings)
	{
		if (rcs.enable() == false)
		{
			continue;
		}

		QString ipPort = rcs.clientRequestIP().addressStr() + ":" + QString::number(PORT_APP_DATA_SERVICE_GRPC_CLIENT_REQUEST);

		ips.append(ipPort);

		int selectedPort = 0;

		builder.AddListeningPort(ipPort.toStdString(), grpc::InsecureServerCredentials(), &selectedPort);
	}

	builder.RegisterService(this);

	m_server = builder.BuildAndStart();

	if (m_server == nullptr)
	{
		DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv NOT started!");
		return;
	}

	DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv started. Listening addresses: %1").arg(ips.join(", ")));
}

GrpcAppDataSrv::~GrpcAppDataSrv()
{
	if (m_server)
	{
		// Join thread explicitly, to log message only after thread has stopped.
		//
		m_thread.request_stop();
		m_thread.join();
	}

	DEBUG_LOG_MSG(m_log, "GrpcAppDataSrv finished.");
	return;
}

Status GrpcAppDataSrv::GetAppSignalList(ServerContext* context,
										const Grpc::GetAppSignalListRequest* request,
										ServerWriter<Grpc::GetAppSignalListReply>* writer)
{
	return Status::OK;
}


Status GrpcAppDataSrv::GetAppSignalParam(ServerContext* context,
										 const Grpc::GetAppSignalParamRequest *request,
										 ServerWriter<Grpc::GetAppSignalParamReply>* writer)
{
	return Status::OK;
}

