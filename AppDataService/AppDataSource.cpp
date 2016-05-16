#include "AppDataSource.h"


// -------------------------------------------------------------------------------
//
// AppDataSource class implementation
//
// -------------------------------------------------------------------------------

AppDataSource::AppDataSource()
{

}


// -------------------------------------------------------------------------------
//
// AppDataSources class implementation
//
// -------------------------------------------------------------------------------

AppDataSources::~AppDataSources()
{
	clear();
}


void AppDataSources::clear()
{
	for(AppDataSource* dataSource : *this)
	{
		delete dataSource;
	}

	HashedVector<quint32, AppDataSource*>::clear();
}
