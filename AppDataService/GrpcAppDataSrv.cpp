#include <chrono>
#include <thread>
#include <QStringList>

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/WUtils.h"

#include "GrpcAppDataSrv.h"

GrpcAppDataSrv::GrpcAppDataSrv(	const AppDataServiceSettings& settings,
								const AppSignals& appSignals,
								CircularLoggerShared log) :
	m_appSignals(appSignals),
	m_log(log)
{
	grpc::ServerBuilder builder;

	builder.RegisterService(this);

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

	m_server = builder.BuildAndStart();

	if (m_server == nullptr)
	{
		DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv NOT started!");
		return;
	}

	DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv started. Listening addresses: %1").
						 arg(ips.join(", ")));

	m_thread = std::jthread{[this](std::stop_token stoken, grpc::Server* server)
							{
								std::stop_callback stop_cb{stoken,
														   [server]()
														   {
															   server->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds(5));
														   }};
								try
								{
									DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv in Wait state"));
									server->Wait();		// unblocked by stop_token callback
								}
								catch (std::exception& e)
								{
									DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv thread exception: " + QString{e.what()});
								}
								catch (...)
								{
									DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv thread unknown exception");
								}
							},
							m_server.get()};
}

GrpcAppDataSrv::~GrpcAppDataSrv() noexcept
{
	if (m_server != nullptr)
	{
		m_thread.request_stop();

		if (m_thread.joinable())
		{
			m_thread.join();
		}

		m_server.reset();

		DEBUG_LOG_MSG(m_log, "GrpcAppDataSrv finished.");
	}

	return;
}

grpc::Status GrpcAppDataSrv::GetAppSignalList(grpc::ServerContext* context,
										const Grpc::GetAppSignalListRequest* request,
										grpc::ServerWriter<Grpc::GetAppSignalListReply>* writer)
{
	Q_UNUSED(request);

	Grpc::GetAppSignalListReply reply;

	constexpr int IDS_MAX_COUNT = 50000;

	reply.mutable_appsignalids()->Reserve(IDS_MAX_COUNT);

	grpc::Status writeStatus;

	auto writeReply = [context, writer](Grpc::GetAppSignalListReply& reply, grpc::Status& wrStatus) -> bool
	{
		wrStatus = grpc::Status::OK;

		if (context->IsCancelled())
		{
			wrStatus = grpc::Status::CANCELLED;
			return false;
		}

		if (writer->Write(reply) == false)
		{
			return false;
		}

		return true;
	};

	int ctr = 0;

	for(const AppSignal* appSignal : m_appSignals)
	{
		*reply.add_appsignalids() = appSignal->appSignalID().toStdString();

		ctr++;

		if ((ctr & 0x3FFF) == 0 && context->IsCancelled())
		{
			return grpc::Status::CANCELLED;
		}

		if (ctr >= IDS_MAX_COUNT)
		{
			if (writeReply(reply, writeStatus) == false)
			{
				return writeStatus;
			}

			reply.Clear();
			ctr = 0;
		}
	}

	if (ctr > 0)
	{
		if (writeReply(reply, writeStatus) == false)
		{
			return writeStatus;
		}
	}

	return grpc::Status::OK;
}

grpc::Status GrpcAppDataSrv::GetAppSignalParam(grpc::ServerContext* context,
										 const Grpc::GetAppSignalParamRequest* request,
										 grpc::ServerWriter<Grpc::GetAppSignalParamReply>* writer)
{
	Grpc::GetAppSignalParamReply reply;

	constexpr int PARAMS_MAX_COUNT = 256;

	reply.mutable_signalparams()->Reserve(PARAMS_MAX_COUNT);

	grpc::Status writeStatus;

	auto writeReply = [context, writer](Grpc::GetAppSignalParamReply& reply,
										size_t totalCount, int index,
										grpc::Status& wrStatus) -> bool
	{
		wrStatus = grpc::Status::OK;

		reply.set_totalsize(static_cast<::uint32_t>(totalCount));
		reply.set_replysignalindex(static_cast<::uint32_t>(index));

		if (context->IsCancelled())
		{
			wrStatus = grpc::Status::CANCELLED;
			return false;
		}

		if (writer->Write(reply) == false)
		{
			return false;
		}

		return true;
	};

	int ctr = 0;
	int index = 0;

	if (request->signalhashes_size() == 0)
	{
		for(const AppSignal* appSignal : m_appSignals)
		{
			appSignal->saveToProto(reply.add_signalparams());

			ctr++;

			if (ctr >= PARAMS_MAX_COUNT)
			{
				if (writeReply(reply, m_appSignals.count(), index, writeStatus) == false)
				{
					return writeStatus;
				}

				index += ctr;

				reply.Clear();
				ctr = 0;
			}
		}

		if (ctr > 0)
		{
			if (writeReply(reply, m_appSignals.count(), index, writeStatus) == false)
			{
				return writeStatus;
			}
		}

		return grpc::Status::OK;
	}
	else
	{
		for(auto h : request->signalhashes())
		{
			Hash hash = static_cast<Hash>(h);

			const AppSignal* appSignal = m_appSignals.getSignalByHash(hash);

			appSignal->saveToProto(reply.add_signalparams());

			ctr++;

			if (ctr >= PARAMS_MAX_COUNT)
			{
				if (writeReply(reply, request->signalhashes_size(), index, writeStatus) == false)
				{
					return writeStatus;
				}

				index += ctr;

				reply.Clear();
				ctr = 0;
			}
		}

		if (ctr > 0)
		{
			if (writeReply(reply, request->signalhashes_size(), index, writeStatus) == false)
			{
				return writeStatus;
			}
		}
	}

	return grpc::Status::OK;
}

