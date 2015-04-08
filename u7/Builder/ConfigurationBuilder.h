#ifndef CONFIGURATIONBUILDER_H
#define CONFIGURATIONBUILDER_H

// Forware delcarations
//
class QThread;
class OutputLog;
class DbController;

namespace Builder
{

	class ConfigurationBuilder : QObject
	{
		Q_OBJECT
	public:
		ConfigurationBuilder() = delete;
		ConfigurationBuilder(DbController* db, OutputLog* log, int changesetId, bool debug);
		virtual ~ConfigurationBuilder();

		bool build();

	protected:


	private:
		DbController* db();
		OutputLog* log() const;
		int changesetId() const;
		bool debug() const;
		bool release() const;

	private:
		DbController* m_db = nullptr;
		mutable OutputLog* m_log = nullptr;
		int m_changesetId = 0;
		int m_debug = false;

	};

}

#endif // CONFIGURATIONBUILDER_H
