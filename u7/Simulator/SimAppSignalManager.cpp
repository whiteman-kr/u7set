#include "SimAppSignalManager.h"
#include "SimIdeSimulator.h"
#include "../../Proto/serialization.pb.h"

SimAppSignalManager::SimAppSignalManager(SimIdeSimulator* simulator,
										 QObject* parent) :
	AppSignalManager(parent),
	Sim::Output("AppSignalManager"),
	m_simulator(simulator)
{
	assert(m_simulator);

	return;
}

SimAppSignalManager::~SimAppSignalManager()
{
}

bool SimAppSignalManager::load(QString fileName)
{
	reset();
	m_signals.clear();

	// Open, read, decompress file
	//
	QFile file(fileName);
	bool ok = file.open(QIODevice::ReadOnly);

	if (ok == false)
	{
		writeError(tr("Cannot open file %1, error %2 ").arg(fileName).arg(file.errorString()));
		return false;
	}

	QByteArray data = file.readAll();
	QByteArray uncompressedData = qUncompress(data);

	::Proto::AppSignalSet message;
	ok = message.ParseFromArray(uncompressedData.constData(), uncompressedData.size());

	if (ok == false)
	{
		writeError(tr("Cannot parse file %1").arg(fileName));
		return false;
	}

	std::vector<AppSignalParam> signalParams;
	signalParams.reserve(message.appsignal_size());

	std::map<QString, Signal> signalMap;

	for (int i = 0; i < message.appsignal_size(); ++i)
	{
		const ::Proto::AppSignal& signalMessage = message.appsignal(i);

		// Load AppSignalParam
		//
		ok &= signalParams.emplace_back().load(signalMessage);

		// Load Signal
		//
		QString appSignalId = QString::fromStdString(signalMessage.appsignalid());

		signalMap[appSignalId].serializeFrom(signalMessage);
	}

	if (ok == false)
	{
		writeError(tr("Cannot load Proto::AppSignal"));
		return false;
	}

	// --
	//
	addSignals(signalParams);
	std::swap(signalMap, m_signals);

	return ok;
}


