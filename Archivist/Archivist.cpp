#include "Archivist.h"

Archivist::Archivist(const RequestParams& rp) :
	m_reqParams(rp)
{
}

void Archivist::printRequestParams()
{
	print.newLine();

	print << "Archive copy parameters:\n\n";

	print << QString("Archive location:\t%1\n").arg(m_reqParams.archiveLocation);
	print << QString("Begin time:\t\t%1\n").
			 arg(m_reqParams.begin.toString("hh:mm:ss dd.MM.yyyy"));
	print << QString("End time:\t\t%1\n").
			 arg(m_reqParams.end.toString("hh:mm:ss dd.MM.yyyy"));
	print << QString("Signals filter:\t\t%1\n").arg(m_reqParams.signalsList.join(" "));
	print << QString("Copy location:\t\t%1\n").arg(m_reqParams.destLocation);

	print.newLine();
}
