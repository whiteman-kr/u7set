#include "../include/AdsConnectionLib/ClientGrpc.h"

#include <CommonStdLib/u7_vld.h>

#include <grpcpp/create_channel.h>

namespace ClientLib
{
	// gRPC channel cache to reuse channels per address and avoid repeated allocations
	//
	std::shared_ptr<grpc::Channel> GrpcChannelCache::get(const std::string& address, bool forceToCreateNew)
	{
		static std::mutex mtx;
		static std::unordered_map<std::string, std::weak_ptr<grpc::Channel>> cache;

		std::lock_guard lock{mtx};

		if (forceToCreateNew == false)
		{
			auto existing = cache[address].lock();
			if (existing != nullptr)
			{
				return existing;
			}
		}
		else
		{
			// forceToCreateNew == true
			//
			cache.erase(address);
		}

		grpc::ChannelArguments args;
		// Reduce instrumentation overhead & auxiliary allocations
		args.SetInt(GRPC_ARG_ENABLE_CHANNELZ, 0);
		args.SetInt(GRPC_ARG_MINIMAL_STACK, 1);
		// Keepalive to clean dead connections sooner
		args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 10000);
		args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5000);
		args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
		args.SetInt(GRPC_ARG_MAX_RECONNECT_BACKOFF_MS, 5000);
		args.SetInt(GRPC_ARG_INITIAL_RECONNECT_BACKOFF_MS, 1000);

		// Suppress VLD during gRPC one-time global initialization happening on first channel creation
		//
#ifdef VLD_IS_INCLUDED
		::VLDDisable();
#endif // VLD_IS_INCLUDED

		auto channel = grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), args);

#ifdef VLD_IS_INCLUDED
		::VLDEnable();
#endif // VLD_IS_INCLUDED

		cache[address] = channel;
		return channel;
	}
} // namespace ClientLib