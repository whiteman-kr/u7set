#include "SimIdeSimulator.h"

SimIdeSimulator::SimIdeSimulator() :
	Sim::Simulator()
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

	fileName += "Schemas.als/SchemaDetails.pbuf";

	writeMessage(tr("Load logic schema detais file: %1").arg(fileName));

	bool ok = m_schemaDetails.Load(fileName);
	if (ok == false)
	{
		writeError(tr("File loading error, file name %1.").arg(fileName));
	}
	else
	{
		emit schemaDetailsUpdated();
	}

	return ok;
}

std::vector<VFrame30::SchemaDetails> SimIdeSimulator::schemasDetails() const
{
	std::vector<VFrame30::SchemaDetails> result = m_schemaDetails.schemasDetails();

	return result;
}

std::set<QString> SimIdeSimulator::schemaAppSignals(const QString& schemaId)
{
	std::shared_ptr<VFrame30::SchemaDetails> details = m_schemaDetails.schemaDetails(schemaId);

	if (details == nullptr)
	{
		return std::set<QString>();
	}

	return details->m_signals;
}


