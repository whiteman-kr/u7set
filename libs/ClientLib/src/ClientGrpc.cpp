#include "ClientGrpc.h"

#include <CommonLib/u7_vld.h>

#include <grpcpp/create_channel.h>

namespace ClientLib
{
	// gRPC channel cache to reuse channels per address and avoid repeated allocations
	//
	std::shared_ptr<grpc::Channel> GrpcChannelCache::get(const std::string& address)
	{
		static std::mutex mtx;
		static std::unordered_map<std::string, std::weak_ptr<grpc::Channel>> cache;
		std::lock_guard lock{mtx};

		if (auto existing = cache[address].lock(); existing != nullptr)
		{
			return existing;
		}

		grpc::ChannelArguments args;
		// Reduce instrumentation overhead & auxiliary allocations
		args.SetInt(GRPC_ARG_ENABLE_CHANNELZ, 0);
		args.SetInt(GRPC_ARG_MINIMAL_STACK, 1);
		// Keepalive to clean dead connections sooner
		args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 10000);
		args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5000);
		args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);

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