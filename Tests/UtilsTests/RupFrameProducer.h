#pragma once

#pragma once

#include <atomic>
#include <thread>
#include <functional>
#include <vector>
#include <chrono>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>
#include <asio/steady_timer.hpp>

#include <HardwareLib/DataProtocols.h>
#include <CommonLib/Types.h>

class RupFrameProducer
{
public:
	using PayloadGenerator = std::function<void(Rup::Data& data,
												int frameNo,
												quint16 packetNumerator)>;

	RupFrameProducer(const std::string& targetIp,
					unsigned short targetPort,
					quint16 protocolVersion,
					quint32 dataId,
					int framesQuantity,
					int workcycleMs) :
			m_targetIp(targetIp),
			m_targetPort(targetPort),
			m_protocolVersion(protocolVersion),
			m_dataId(dataId),
			m_framesQuantity(framesQuantity),
			m_workcycleMs(workcycleMs)
	{
		Q_ASSERT(m_framesQuantity >= 1 && m_framesQuantity <= Rup::MAX_FRAME_COUNT);
		Q_ASSERT(m_workcycleMs > 0);
	}

	~RupFrameProducer()
	{
		stop();
	}

	void setPayloadGenerator(PayloadGenerator gen)
	{
		m_payloadGen = std::move(gen);
	}

	bool start()
	{
		bool expected = false;

		if (!m_running.compare_exchange_strong(expected, true))
		{
			return false;	// already running
		}

		try
		{
			m_thread = std::thread([this]()
								   {
									   runThread();
								   });
		}
		catch (...)
		{
			m_running.store(false);
			return false;
		}

		return true;
	}

	void stop()
	{
		bool expected = true;

		if (!m_running.compare_exchange_strong(expected, false))
		{
			return;			// already stoped
		}

		if (m_io)
		{
			m_io->stop();
		}

		if (m_thread.joinable())
		{
			m_thread.join();
		}

		m_socket.reset();
		m_timer.reset();
		m_io.reset();
	}

private:
	const std::string m_targetIp;
	const unsigned short m_targetPort;
	const quint16 m_protocolVersion;
	const quint32 m_dataId;
	const int m_framesQuantity;
	const int m_workcycleMs;

	std::atomic<bool> m_running { false };
	std::thread m_thread;

	std::unique_ptr<asio::io_context> m_io;
	std::unique_ptr<asio::ip::udp::socket> m_socket;
	std::unique_ptr<asio::steady_timer> m_timer;
	asio::ip::udp::endpoint m_endpoint;

	PayloadGenerator m_payloadGen;

	quint16 m_packetNumerator = 0;

private:
	void runThread()
	{
		m_io = std::make_unique<asio::io_context>();
		asio::io_context& io = *m_io;

		asio::ip::udp::resolver resolver(io);
		auto results = resolver.resolve(asio::ip::udp::v4(), m_targetIp, std::to_string(m_targetPort));
		m_endpoint = *results.begin();

		m_socket = std::make_unique<asio::ip::udp::socket>(io);
		m_socket->open(asio::ip::udp::v4());

		m_timer = std::make_unique<asio::steady_timer>(io);

		scheduleNextSend();

		io.run();
	}

	void scheduleNextSend()
	{
		using namespace std::chrono;

		if (!m_running.load())
		{
			return;
		}

		m_timer->expires_after(milliseconds(m_workcycleMs));
		m_timer->async_wait([this](const asio::error_code& ec)
							{
								if (ec || !m_running.load())
								{
									return;
								}

								sendOnePacket();
								scheduleNextSend();
							});
	}

	void sendOnePacket()
	{
		const quint16 numerator = nextPacketNumerator();

		QDateTime now = QDateTime::currentDateTime();

		for (int frameNo = 0; frameNo < m_framesQuantity; ++frameNo)
		{
			Rup::Frame frame{};
			fillHeader(now, frame.header, frameNo, numerator);
			fillPayload(frame.data, frameNo, numerator);

			frame.header.reverseBytes();

			const char* data = reinterpret_cast<const char*>(&frame);
			const std::size_t size = sizeof(Rup::Frame);

			asio::error_code ec;
			m_socket->send_to(asio::buffer(data, size), m_endpoint, 0, ec);
		}
	}

	void fillHeader(const QDateTime& now, Rup::Header& h, int frameNo, quint16 numerator)
	{
		h.frameSize       = static_cast<quint16>(sizeof(Rup::Frame));
		h.protocolVersion = m_protocolVersion;

		h.flags.all		  = Rup::APP_DATA;
		h.dataId          = m_dataId;

		h.moduleType      = 0;
		h.numerator       = static_cast<quint16>(numerator);
		h.framesQuantity  = static_cast<quint16>(m_framesQuantity);
		h.frameNumber     = static_cast<quint16>(frameNo);

		h.timeStamp.year        = static_cast<quint16>(now.date().year());
		h.timeStamp.month       = static_cast<quint8>(now.date().month());
		h.timeStamp.day         = static_cast<quint8>(now.date().day());
		h.timeStamp.hour        = static_cast<quint8>(now.time().hour());
		h.timeStamp.minute      = static_cast<quint8>(now.time().minute());
		h.timeStamp.second      = static_cast<quint8>(now.time().second());
		h.timeStamp.millisecond = static_cast<quint16>(now.time().msec());
	}

	void fillPayload(Rup::Data& d, int frameNo, quint16 numerator)
	{
		if (m_payloadGen)
		{
			m_payloadGen(d, frameNo, numerator);
			return;
		}

		// default Payload - zeros
		std::memset(&d, 0, sizeof(Rup::Data));
	}

	quint16 nextPacketNumerator()
	{
		const quint16 out = m_packetNumerator;
		m_packetNumerator = static_cast<quint16>(m_packetNumerator + 1);
		return out;
	}
};
