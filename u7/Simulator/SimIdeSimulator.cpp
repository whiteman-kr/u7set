#include "SimIdeSimulator.h"
#include <QDir>

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

	fileName += "LogicSchemas/SchemaDetails.pbuf";

	writeMessage(tr("Load logic schema detais file: %1").arg(fileName));

	bool ok = m_schemaDetails.Load(fileName);
	if (ok == false)
	{
		writeError(tr("File loading error, file name %1.").arg(fileName));
	}

	return ok;
}
