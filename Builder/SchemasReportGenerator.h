#pragma once

#include "../DbLib/DbController.h"
#include "../ReportLib/Report.h"
#include "../ReportLib/ReportAppSignalProvider.h"
#include "../ReportLib/ReportDiagStateProvider.h"
#include "../ReportLib/ReportPrinter.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/DiagStateController.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemLoopback.h"
#include "../VFrame30/SchemaItemSignal.h"

namespace Builder
{
	//
	// SchemasReportOptions
	//
	class SchemasReportOptions
	{

	public:
		static SchemasReportOptions optionsForSingleSchema();	// Creates options for schema with all options set to false to produce clear schema "as is"
		static SchemasReportOptions optionsForSchemasAlbum(DbController* db);

	private:
		SchemasReportOptions() = default;

		bool load(DbController* db);

	public:
		bool save(DbController* db);

		// Generic schemas report options
		//
		void setSignleFile(bool value);
		bool singleFile() const;

		void setFooters(bool value);
		bool footers() const;

		void setItemsLabels(bool value);
		bool itemsLabels() const;

		// Options for schemas album
		//
		void setFolders(bool value);
		bool folders() const;
		
		void setTableOfContents(bool value);
		bool tableOfContents() const;
		
		void setSignalsDetails(bool value);
		bool signalsDetails() const;
		
		void setStartPageNumber(int value);
		int startPageNumber() const;

		void setContentsTextFontSize(int value);
		int contentsTextFontSize() const;

		void setContentsTableFontSize(int value);
		int contentsTableFontSize() const;

		void setTextFontSize(int value);
		int textFontSize() const;

		void setTableFontSize(int value);
		int tableFontSize() const;

		void setSchemaTags(const std::set<QString>& tagsSet);
		const std::map<QString, bool>& schemaTags() const;
		std::map<QString, bool>& schemaTags();

		void setUserVariables(const std::map<QString, QString>& variables);
		const std::map<QString, QString>& userVariables() const;

		void setProjectVariables(const std::map<QString, QString>& variables);
		const std::map<QString, QString>& projectVariables() const;

	private:
		bool m_footers = false;					// Generate footers in reports (top and bottom) with schema name, project info and page number
		bool m_folders = false;					// Include schema folder to schema id in schemas album
		bool m_tableOfContents = false;			// Generate table of contents in schemas album
		bool m_itemsLabels = false;				// Generate items labels for debuging
		bool m_signalsDetails = false;			// Generate extra pages in schemas album with signals sources and targets
		
		int m_startPageNumber = 1;              // Start page number
		bool m_singleFile = false;				// Generare report to single file, do not split to schema types
		
		int m_contentsTextFontSize = 9;			// Table of contents text font size
		int m_contentsTableFontSize = 9;		// Table of contents table font size

		int m_textFontSize = 9;					// Normal text font size
		int m_tableFontSize = 9;				// Table font size

		std::map<QString, bool> m_schemaTags;   // Key is tag, value shows if this tag is set
		std::map<QString, QString> m_projectVariables;	// Key is variable name, value is variable value
		std::map<QString, QString> m_userVariables;		// Key is variable name, value is variable value
	};

	//
	// SchemaTypesParams
	//

	class SchemaTypesParams
	{
	public:
		SchemaTypesParams(int fileId, const QString& caption, bool selected, const QPageLayout& initPageLayout, const QStringList& layoutNames);
		
		int fileId() const;
		bool hasFileId() const;
		const QString& caption() const;

		QPageLayout pageLayoutWithMargins(int index) const;

		// Properties
		//
		bool selected() const;
		void setSelected(bool value);

		int pageLayoutCount() const;

		const QString& pageLayoutCaption(int index) const;
		void setPageLayoutCaption(int index, const QString& value);


		const QPageLayout& pageLayout(int index) const;
		void setPageLayout(int index, const QPageLayout& layout);

		bool noMargins(int index) const;
		void setNoMargins(int index, bool value);

		// Load/save
		//
		bool load(DbController* db);
		bool save(DbController* db) const;

	private:
		int m_fileId = -1;
		QString m_caption;

		bool m_selected = false;
		
		struct LayoutInfo
		{
			QString caption;
			QPageLayout layout;
			bool noMargins;
		};

		std::vector<LayoutInfo> m_pageLayouts;
	};

	//
	// SchemaInfo
	//
	struct SchemaInfo
	{
		SchemaInfo() = default;
		SchemaInfo(const QString& fullFileName, const std::shared_ptr<VFrame30::Schema>& schema);

		const QString& folder() const;
		const QString& fileName() const;
		const std::shared_ptr<VFrame30::Schema>& schema() const;

	private:
		QString m_folder;
		QString m_fileName;
		std::shared_ptr<VFrame30::Schema> m_schema;
	};

	//
	// SchemaSignalInfo
	//
	struct SchemaSignalInfo
	{
		SchemaSignalInfo(const VFrame30::FblItemRect* item, const QString& appSignalId, const QStringList& otherSchemasIds, IAppSignalManager& appSignals);
		QStringList toStringList() const;

		static bool less(const SchemaSignalInfo& a, const SchemaSignalInfo& b);

	private:
		bool input = true;
		bool commented = false;
		bool impact = false;
		bool received = false;	// Signal comes from receiver
		double x = 0;
		double y = 0;
		QString signalId;
		QString caption;
		QString schemasList;
		QString color;
	};

	//
	// SchemaLoopbackInfo
	//
	struct SchemaLoopbackInfo
	{
		SchemaLoopbackInfo(const VFrame30::SchemaItemLoopback* loopbackItem, const QStringList& otherSchemasIds);
		QStringList toStringList() const;

		static bool less(const SchemaLoopbackInfo& a, const SchemaLoopbackInfo& b);

	private:
		bool source = true;
		bool commented = false;
		double x = 0;
		double y = 0;
		QString loopbackId;
		QString schemasList;
		QString color;
	};

	//
	// SchemaLoopbackInfo
	//
	struct SchemaConnectionInfo
	{
		SchemaConnectionInfo(const VFrame30::SchemaItemConnection* connectionItem, const QString& connectionId, const QStringList& otherSchemasIds);
		QStringList toStringList() const;

		static bool less(const SchemaConnectionInfo& a, const SchemaConnectionInfo& b);

	private:
		bool transmitter = true;
		bool commented = false;
		double x = 0;
		double y = 0;
		QString connectionId;
		QString schemasList;
		QString color;
	};

	// SchemasReportGenerator
	//

	class SchemasReportGenerator : public QObject
	{
		Q_OBJECT

	public:
		SchemasReportGenerator(std::shared_ptr<ReportLib::ReportSchemaView> schemaView,
							   const AppSignalSet* signalSet,
							   const QString& serverIp,
							   int serverPort,
							   const QString& serverUserName,
							   const QString& serverPassword,
							   const QString& projectName,
							   const QString& userName,
							   const QString& userPassword,
							   std::vector<DbFileInfo> files,
							   const QString& filePath,
							   const SchemasReportOptions& options,
							   const std::vector<SchemaTypesParams>& schemaTypesParams);

		virtual ~SchemasReportGenerator();

		static std::vector<SchemaTypesParams> defaultFileTypesParams(DbController* db);

	public slots:
		void exportSchemasToMultiplePdf();
		void exportSchemasToSinglePdf();
		void exportSchemasToAlbums();

		void stop();
		void progressRequested();

		void getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText);

	signals:
		void progressChanged(int progress, int progressMin, int progressMax, const QString& progressText);
		void finished(const QString& errorMessage);

	public:
		// Access to output data. Output data is filled if fileName is empty
		//
		QStringList outputFilesList() const;
		const QByteArray& outputData(const QString& fileName);

	public:
		// Statistics data
		//
		enum class WorkerStatus
		{
			Idle,
			Loading,
			Parsing,
			Rendering,
			Printing
		};
		struct Statistics
		{
			WorkerStatus m_currentStatus = WorkerStatus::Idle;
			int m_schemasCount = 0;	// Calculated after text rendering
			int m_schemaIndex = 0;
			QString m_currentSchemaType;
			QString m_currentSchemaId;
		};

		Statistics statistics() const;

	private:
		DbController* db();
		const QString& filePath() const;

		void openProject();
		void closeProject();

		void loadSchemas(const DbFileTree& foldersTree,
						 const std::vector<DbFileInfo>& files,
						 std::vector<SchemaInfo>& schemas,
						 VFrame30::SchemaDetailsSet& detailsSet);

		void sortSchemas(std::vector<SchemaInfo>& schemas);

		bool renderSchemasToAlbums(const std::vector<SchemaInfo>& schemas,
								   const VFrame30::SchemaDetailsSet& detailsSet,
								   const QString& groupName,
								   const QPageLayout& schemaPageLayout,
								   const QPageLayout& textPageLayout);

		void createTableOfContents(const std::shared_ptr<ReportLib::Report> report,
								   const QPageLayout& pageLayout,
								   const std::vector<SchemaInfo>& schemas,
								   const QString& caption);

		[[nodiscard]] QPageLayout getSchemaPageLayout(const SchemaInfo& schemaInfo) const;

		void createLogicSchemaSignalsDetails(const std::shared_ptr<ReportLib::Report> report,
											 const QPageLayout& pageLayout,
											 const SchemaInfo& schemaInfo,
											 const std::vector<SchemaInfo>& allSchemas,
											 const VFrame30::SchemaDetailsSet& detailsSet);

		void createLogicSchemaIOSignalsDetails(const std::shared_ptr<ReportLib::ReportSection> section,
											   const VFrame30::LogicSchema* logicSchema,
											   const std::vector<SchemaInfo>& allSchemas,
											   const VFrame30::SchemaDetailsSet& detailsSet);

		void createLogicSchemaLoopbacksDetails(const std::shared_ptr<ReportLib::ReportSection> section,
											   const VFrame30::LogicSchema* logicSchema,
											   const std::vector<SchemaInfo>& allSchemas,
											   const VFrame30::SchemaDetailsSet& detailsSet);

		void createLogicSchemaConnectionsDetails(const std::shared_ptr<ReportLib::ReportSection> section,
												 const VFrame30::LogicSchema* logicSchema,
												 const std::vector<SchemaInfo>& allSchemas,
												 const VFrame30::SchemaDetailsSet& detailsSet);

	private:
		DbController m_db;
		const std::shared_ptr<ReportLib::ReportSchemaView> m_schemaView;
		ReportLib::ReportPrinter m_printer;

		ReportLib::ReportDiagStateProvider m_diagStateProvider;
		VFrame30::DiagStateController m_diagStateController;

		ReportLib::ReportAppSignalProvider m_appSignalProvider;
		VFrame30::AppSignalController m_appSignalController;

		// Report options, page sizes for different file groups etc
		//
		SchemasReportOptions m_options;
		std::vector<SchemaTypesParams> m_schemaTypesParams;

		// Input files for exportFilesToPdf() and exportFilesToAlbum()
		//
		std::vector<DbFileInfo> m_inputFiles;

		// Output file path
		//
		QString m_filePath;

		// Output Data
		//
		std::map<QString, QByteArray> m_outputData;

		// Report parameters

		std::atomic_bool m_stop = false;	// Stop processing flag, set by stop()

		// Connection information

		QString m_serverIp;
		int m_serverPort = -1;
		QString m_serverUserName;
		QString m_serverPassword;

		QString m_projectName;
		QString m_userName;
		QString m_userPassword;

		ReportLib::ReportFont m_contentsTextFont;
		ReportLib::ReportFont m_contentsTableFont;

		ReportLib::ReportFont m_textFont;
		ReportLib::ReportFont m_tableFont;

		ReportLib::ReportFont m_marginFont;

		mutable QMutex m_statisticsMutex;
		Statistics m_statistics;

	};
}
