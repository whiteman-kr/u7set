#include "FileArchivist.h"
#include "../UtilsLib/WUtils.h"

FileArchivist::FileArchivist(const RequestParams& rp) :
	Archivist(rp)
{
}

bool FileArchivist::copyArchive()
{
	printRequestParams();

	bool result = true;

	result = readArchInfoProto();

	RETURN_IF_FALSE(result);

	return true;
}

bool FileArchivist::readArchInfoProto()
{
	QString fileName = m_reqParams.archiveLocation + "/ArchInfo.proto";

	QFile f(fileName);

	if (f.open(QIODevice::ReadOnly) == false)
	{
		print << QString("Read file error: %1\n").arg(fileName);
		return false;
	}

	QByteArray data = f.readAll();

	bool result = m_archInfo.ParseFromArray(data, data.size());

	if (result == false)
	{
		print << QString("File %1 parsing error!\n").arg(fileName);
		return false;
	}

	print << QString("File %1 parsed Ok\n").arg(fileName);

	return true;
}
