#ifndef APPLOGICBUILDER_H
#define APPLOGICBUILDER_H

// Forware delcarations
//
class QThread;
class OutputLog;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class McFirmwareOld;
}

namespace VFrame30
{
	class LogicScheme;
	class SchemeLayer;
}

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------
	class ApplicationLogicBuilder : QObject
	{
		Q_OBJECT

	public:
		ApplicationLogicBuilder() = delete;
		ApplicationLogicBuilder(DbController* db, OutputLog* log, int changesetId, bool debug);
		virtual ~ApplicationLogicBuilder();

		bool build();

	protected:
		bool loadApplicationLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicScheme>>* out);
		bool compileApplicationLogicScheme(VFrame30::LogicScheme* logicScheme);
		bool compileApplicationLogicLayer(VFrame30::LogicScheme* logicScheme, VFrame30::SchemeLayer* layer);

	protected:
		DbController* db();
		OutputLog* log();
		int changesetId() const;
		bool debug() const;
		bool release() const;

	private:
		DbController* m_db = nullptr;
		OutputLog* m_log = nullptr;
		int m_changesetId = 0;
		int m_debug = false;
	};
}

#endif // APPLOGICBUILDER_H
