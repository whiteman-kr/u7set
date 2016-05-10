#include "TuningDataSource.h"


// -------------------------------------------------------------------------------
//
// TuningDataSource class implementation
//
// -------------------------------------------------------------------------------

TuningDataSource::TuningDataSource()
{

}


// -------------------------------------------------------------------------------
//
// TuningDataSources class implementation
//
// -------------------------------------------------------------------------------

TuningDataSources::~TuningDataSources()
{
	clear();
}


void TuningDataSources::clear()
{
	for(TuningDataSource* ds : *this)
	{
		delete ds;
	}

	QHash<quint32, TuningDataSource*>::clear();
}
