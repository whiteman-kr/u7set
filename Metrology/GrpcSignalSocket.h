#pragma once

#include <CommonLib/HostAddressPort.h>
#include <AdsConnectionLib/AdsConnection.h>
#include <AppSignalLibStd/IAppSignalUpdater.h>
#include <ClientLib/AppSignalManager.h>
#include <ClientLib/LoggerStdAdapter.h>
#include <ClientLib/ServiceEndpoint.h>

#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/CircularLogger.h"

class LoggerAdapter : public ILoggerStd
{
public:
	explicit LoggerAdapter(CircularLoggerShared log) :
		m_log(log)
	{
		return;
	}

	virtual void writeAlert(std::string_view message) override
	{
		writeWarning(message);
	}

	virtual void writeError(std::string_view message) override
	{
		QString msg = QString::fromUtf8(message.data(), TO_INT(message.size()));
		DEBUG_LOG_ERR(m_log, msg);
	}

	virtual void writeWarning(std::string_view message) override
	{
		QString msg = QString::fromUtf8(message.data(), TO_INT(message.size()));
		DEBUG_LOG_WRN(m_log, msg);
	}
	virtual void writeMessage(std::string_view message) override
	{
		QString msg = QString::fromUtf8(message.data(), TO_INT(message.size()));
		DEBUG_LOG_MSG(m_log, msg);
	}

private:
	CircularLoggerShared m_log;
};

class AppSignalUpdater : public ClientLib::IAppSignalUpdater
{
public:
	using SourceIdType = uintptr_t;

	// Reset all signal params and states.
	//
	virtual void reset() override
	{
	}

	// This function must notify (emit signal?) that signal params where updated.
	// This signal is out of scope of this interface, it is up to implementation which signal to emit and how to use it.
	//
	virtual void notifySignalParamsUpdated() override
	{
	}

	// Set signal params.
	//
	virtual void addSignals(std::span<const ::Proto::AppSignal> appSignals, const std::string& appDataServiceId) override
	{
	}

	// Invalidate all signal states by source sourceThreadId.
	//
	virtual void invalidateSignalStates(SourceIdType sourceThreadId) override
	{
	}

	// Set signal states by sources.
	//
	virtual void setStates(std::span<const ::Proto::AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId) override
	{
	}
};


class GrpcSignalSocket
{
public:
	GrpcSignalSocket(const SoftwareInfo& swInfo,
					const QString& equipmentId1,
					const HostAddressPort& serverAddressPort1,
					const QString& equipmentId2,
					const HostAddressPort& serverAddressPort2,
					CircularLoggerShared log);
	virtual ~GrpcSignalSocket();

	bool isConnected() const;
	int selectedServerIndex() const;

private:
	AppSignalUpdater m_updater;
	std::unique_ptr<ClientLib::AdsConnection> m_adsConnection;
};
