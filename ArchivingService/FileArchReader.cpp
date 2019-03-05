#include "FileArchReader.h"

bool operator < (const FileArchReader::ArchPartition& afi1, const FileArchReader::ArchPartition& afi2)
{
	return afi1.systemTime < afi2.systemTime;
}

// ---------------------------------------------------------------------------------------------
//
// FileArchReader class implementation
//
// ---------------------------------------------------------------------------------------------

FileArchReader::FileArchReader(Archive* archive, const ArchRequestParam& param) :
	m_archive(archive),
	m_requestParam(param)
{
}

bool FileArchReader::findData()
{


	return true;
}


