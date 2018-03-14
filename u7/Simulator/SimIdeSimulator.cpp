#include "SimIdeSimulator.h"
#include <QDir>

SimIdeSimulator::SimIdeSimulator() :
	Sim::Simulator(),
	m_appSignalManager(this),
	m_controlThread(this)
{
}

SimIdeSimulator::~SimIdeSimulator()
{
}

bool SimIdeSimulator::load(QString buildPath)
{
	bool ok = true;

	ok &= Sim::Simulator::load(buildPath);
	ok &= loadSchemaDetails(buildPath);
	ok &= loadAppSignals(buildPath);

	return ok;
}

std::vector<VFrame30::SchemaDetails> SimIdeSimulator::schemasForLm(QString equipmentId) const
{
	return m_schemaDetails.schemasDetails(equipmentId);
}

bool SimIdeSimulator::loadSchemaDetails(QString buildPath)
{
	QString fileName = QDir::fromNativeSeparators(buildPath);
	if (fileName.endsWith(QChar('/')) == false)
	{
		fileName.append(QChar('/'));
	}

	fileName += "LogicSchemas/SchemaDetails.pbuf";

	writeMessage(tr("Load logic schema detais file: %1").arg(fileName));

	bool ok = m_schemaDetails.Load(fileName);
	if (ok == false)
	{
		writeError(tr("File loading error, file name %1.").arg(fileName));
	}

	return ok;
}

bool SimIdeSimulator::loadAppSignals(QString buildPath)
{
	QString fileName = QDir::fromNativeSeparators(buildPath);
	if (fileName.endsWith(QChar('/')) == false)
	{
		fileName.append(QChar('/'));
	}

	fileName += "Common/AppSignals.asgs";

	writeMessage(tr("Loading AppSignals.asgs").arg(fileName));

	bool ok = m_appSignalManager.load(fileName);
	if (ok == false)
	{
		writeError(tr("File loading error, file name %1.").arg(fileName));
	}

	return ok;
}

SimAppSignalManager& SimIdeSimulator::appSignalManager()
{
	return m_appSignalManager;
}

const SimAppSignalManager& SimIdeSimulator::appSignalManager() const
{
	return m_appSignalManager;
}

TuningSignalManager& SimIdeSimulator::tuningSignalManager()
{
	return m_tuningSignalManager;
}

const TuningSignalManager& SimIdeSimulator::tuningSignalManager() const
{
	return m_tuningSignalManager;
}
