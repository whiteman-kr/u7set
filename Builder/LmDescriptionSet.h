#ifndef LMDESCRIPTIONSET_H
#define LMDESCRIPTIONSET_H

#include "../lib/LogicModuleSet.h"
#include "IssueLogger.h"

namespace Builder
{
	//
	// LogicModule Description Set
	//
	class LmDescriptionSet : public LogicModuleSet
	{
		Q_OBJECT

	public:
		bool loadFile(IssueLogger* log, DbController* db, QString objectId, QString fileName);
		std::pair<QString, bool> rowFile(QString fileName) const;

	private:
		std::map<QString, QString> m_rawLmDescriptions;		// Raw data (xml), not parsed file. Required to save to build result
	};
}

#endif // LMDESCRIPTIONSET_H
