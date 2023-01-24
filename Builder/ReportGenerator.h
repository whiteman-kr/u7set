#pragma once

#include "../DbLib/DbController.h"

#include "../VFrame30/SchemaView.h"
#include "../VFrame30/AppSignalController.h"
#include "ReportAppSignalProvider.h"

namespace Builder
{

	//
	// ReportFileTypeParams
	//

	struct ReportFileTypeParams
	{
		ReportFileTypeParams(int fileId, const QString& caption, bool selected)
		{
			this->fileId = fileId;
			this->caption = caption;
			this->selected = selected;
		}

		ReportFileTypeParams(int fileId, const QString& caption, bool selected, QPageLayout pageLayout)
			:ReportFileTypeParams(fileId, caption, selected)
		{
			this->pageLayout = pageLayout;
		}

		int fileId = -1;
		QString caption;
		bool selected = false;

		// Multiple-file report section page options
		//
		QPageLayout pageLayout = QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, QMarginsF(15, 15, 15, 15));
	};

	//
	// ReportSchemaCompareAction
	//

	enum class ReportSchemaCompareAction
	{
		Unmodified,
		Modified,
		Added,
		Deleted
	};

	//
	// ReportSchemaView
	//

	class ReportSchemaView : public VFrame30::SchemaView
	{
	public:
		ReportSchemaView();
		virtual ~ReportSchemaView();

		void adjust(QPainter* painter, double startX, double startY, double zoom) const;
		void drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect, const std::map<QUuid, ReportSchemaCompareAction>& compareActions);

		virtual VFrame30::DrawMode drawMode() const override;
	};

	//
	// ReportObjectContext
	//

	struct ReportObjectContext
	{
		QTextDocument* textDocument = nullptr;
		QTextCursor* textCursor = nullptr;
	};

	//
	// ReportObject
	//

	class ReportObject
	{
	public:
		ReportObject();	// New page constructor
		virtual ~ReportObject();

		bool isText() const;
		bool isSchema() const;
		bool isTable() const;
		bool isNewPage() const;

		virtual void render(const ReportObjectContext& context) const = 0;
	};

	//
	// ReportTable
	//

	class ReportTable : public ReportObject
	{
	public:
		ReportTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat);

		int columnCount() const;
		int rowCount() const;

		const QStringList& rowAt(int index) const;

		void insertRow(const QStringList& row);

		void sortByColumn(int column);

		void render(const ReportObjectContext& context) const override;

	private:

		QStringList m_headerLabels;
		std::vector<int> m_columnWidths;

		std::vector<QStringList> m_rows;

		// Format
		//
		QTextCharFormat m_charFormat;
	};

	//
	// ReportText
	//

	class ReportText : public ReportObject
	{
	public:
		ReportText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat);

		void render(const ReportObjectContext& context) const override;

	private:
		QString m_text;

		// Format
		//
		QTextCharFormat m_charFormat;
		QTextBlockFormat m_blockCharFormat;
	};

	//
	// ReportSection
	//

	class ReportSection
	{
	public:
		ReportSection(const QString& caption);
		virtual ~ReportSection();

		bool isEmpty() const;

		const QString& caption() const;

		// Add object functions

		void addText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat);

		void addTable(std::shared_ptr<ReportTable> table);
		std::shared_ptr<ReportTable> addTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat);

		static std::shared_ptr<ReportTable> createTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat);

		// Schema functions

		std::shared_ptr<VFrame30::Schema> schema() const;
		void setSchema(std::shared_ptr<VFrame30::Schema> schema);

		const std::map<QUuid, ReportSchemaCompareAction>& compareItemActions() const;
		void setCompareItemActions(const std::map<QUuid, ReportSchemaCompareAction>& itemsActions);

		// Render functions

		void render(QSizeF pageSize);

		int pageCount() const;	// filled after render()!!!

		QTextDocument* textDocument();

	private:
		QString m_caption;

		std::vector<std::shared_ptr<ReportObject>> m_objects;

		std::shared_ptr<VFrame30::Schema> m_schema;

		std::map<QUuid, ReportSchemaCompareAction> m_itemsActions;

		QTextDocument m_textDocument;

		int m_pageCount = 0; // filled after render()!!!
	};

	// ReportPageFooter

	struct ReportMarginItem
	{
		ReportMarginItem(const QString& text, int pageFrom, int pageTo, const QFont& font, Qt::Alignment alignment);

		QString m_text;

		int m_pageFrom = -1;
		int m_pageTo = -1;

		QFont m_font;
		Qt::Alignment m_alignment = Qt::AlignTop | Qt::AlignHCenter;
	};

	//
	// ReportGenerator
	//

	class ReportGenerator : public QObject
	{
	public:
		ReportGenerator(std::shared_ptr<ReportSchemaView> schemaView,
						const AppSignalSet *signalSet);

	public:
		QPageLayout pageLayout() const;
		void setPageLayout(const QPageLayout& value);

		int resolution() const;
		void setResolution(int value);

	protected:

		const VFrame30::AppSignalController& appSignalController() const;
		const ReportSchemaView* schemaView() const;

		// Margins functions

		void addMarginItem(const ReportMarginItem& item);
		void clearMarginItems();

		// Rendering functions

		void printDocument(QPdfWriter* pdfWriter,
						   QTextDocument* textDocument,
						   QPainter* painter,
						   const QString& objectName,
						   int* pageIndex, QMutex*
						   pageIndexMutex,
						   int pageCount) const;

		void printSchema(QPdfWriter* pdfWriter,
						 QPainter* painter,
						 std::shared_ptr<VFrame30::Schema> schema,
						 std::optional<const QTextDocument* const> textDocument,
						 std::optional<const std::map<QUuid, ReportSchemaCompareAction>* const> compareActions);

		// Formatting functions

		void saveFormat();
		void restoreFormat();

		void setFont(const QFont& font);
		void setTextForeground(const QBrush& brush);
		void setTextBackground(const QBrush& brush);
		void setTextAlignment(Qt::Alignment alignment);

		const QTextCharFormat& currentCharFormat() const;
		const QTextBlockFormat& currentBlockFormat() const;

	private:
		void drawMarginItems(const QString& objectName, int page, int totalPages, QPdfWriter* pdfWriter, QPainter* painter) const;

	private:
		// Page options
		//
		QPageLayout m_pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Orientation::Portrait, QMarginsF(15, 15, 15, 15));

		int m_pageResolution = 600;

		std::vector<ReportMarginItem> m_marginItems;
		std::shared_ptr<ReportSchemaView> m_schemaView;

		QTextCharFormat m_currentCharFormat;
		QTextBlockFormat m_currentBlockFormat;

		QTextCharFormat m_currentCharFormatSaved;
		QTextBlockFormat m_currentBlockFormatSaved;

	private:
		ReportAppSignalProvider m_appSignalProvider;
		VFrame30::AppSignalController m_appSignalController;

	};

	class SchemasReportGenerator : public ReportGenerator
	{
		Q_OBJECT

	public:
		SchemasReportGenerator(std::shared_ptr<ReportSchemaView> schemaView,
							   const AppSignalSet* signalSet,
							   const QString& serverIp,
							   int serverPort,
							   const QString& serverUserName,
							   const QString& serverPassword,
							   const QString& projectName,
							   const QString& userName,
							   const QString& userPassword,
							   std::vector<DbFileInfo> files,
							   const QString& filePath);

		virtual ~SchemasReportGenerator();

		void setReportFileTypeParams(const std::vector<ReportFileTypeParams>& reportFileTypeParams);

		static std::vector<ReportFileTypeParams> defaultFileTypeParams(DbController* db);

	public slots:
		void exportFilesToPdf();
		void exportFilesToAlbum();
		void exportAllSchemasToAlbums();

		void stop();
		void progressRequested();

		void getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText);

	signals:
		void progressChanged(int progress, int progressMin, int progressMax, const QString& progressText);
		void finished(const QString& errorMessage);

	public:
		enum class WorkerStatus
		{
			Idle,
			Loading,
			Parsing,
			Rendering
		};

		// Access to output data. Output data is filled if fileName is empty
		//
		QStringList outputFilesList() const;
		const QByteArray& outputData(const QString& fileName);

		// Statistics
		//
		WorkerStatus currentStatus() const;
		int schemasCount() const;
		int schemaIndex() const;
		QString currentSchemaType() const;
		QString currentSchemaId() const;

	private:
		struct SchemaFilesInfo
		{
			SchemaFilesInfo(int fileId, const QString& caption)
			{
				this->fileId = fileId;
				this->caption = caption;
			}

			int fileId = -1;
			QString caption;

			std::vector<DbFileInfo> schemasFiles;
			std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;	// Key is full path to schema file
		};

	private:
		DbController* db();
		const QString& filePath() const;

		void openProject();
		void closeProject();
		void loadSchemas(const std::vector<DbFileInfo>& files, std::map<QString, std::shared_ptr<VFrame30::Schema>>* schemas);

	private:
		DbController m_db;

		std::vector<ReportFileTypeParams> m_reportFileTypeParams;

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

		bool m_stop = false;	// Stop processing flag, set by stop()

		// Connection information

		QString m_serverIp;
		int m_serverPort = -1;
		QString m_serverUserName;
		QString m_serverPassword;

		QString m_projectName;
		QString m_userName;
		QString m_userPassword;

		QFont m_marginFont;

		// Statistics data
		//

		WorkerStatus m_currentStatus = WorkerStatus::Idle;

		mutable QMutex m_statisticsMutex;
		int m_schemasCount = 0;	// Calculated after text rendering
		int m_schemaIndex = 0;
		QString m_currentSchemaType;
		QString m_currentSchemaId;

	};
}
