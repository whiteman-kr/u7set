#pragma once

#include "../VFrame30/SchemaView.h"

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
	ReportSchemaView(QWidget* parent);

	virtual ~ReportSchemaView();

	void adjust(QPainter* painter, double startX, double startY, double zoom) const;

	void drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect, const std::map<QUuid, ReportSchemaCompareAction>& compareActions);
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

class ReportMarginItem
{
public:
	ReportMarginItem(const QString& text, int pageFrom, int pageTo, Qt::Alignment alignment, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat);

public:
	int m_pageFrom = -1;
	int m_pageTo = -1;

	QString m_text;

	Qt::Alignment m_alignment = Qt::AlignTop | Qt::AlignHCenter;

	// Format
	//
	QTextCharFormat m_charFormat;
	QTextBlockFormat m_blockFormat;

};

//
// ReportGenerator
//

class ReportGenerator : public QObject
{
public:
	ReportGenerator(ReportSchemaView* schemaView);

public:
	int resolution() const;

protected:

	// Margins functions

	void addMarginItem(const ReportMarginItem& item);
	void clearMarginItems();

	// Rendering functions

	void printDocument(QPdfWriter* pdfWriter, QTextDocument* textDocument, QPainter* painter, int* pageIndex, QMutex* pageIndexMutex, int pageCount);
	void printSchema(QPdfWriter* pdfWriter,
					 QTextDocument* textDocument,
					 QPainter* painter,
					 std::shared_ptr<VFrame30::Schema> schema,
					 const std::map<QUuid, ReportSchemaCompareAction>& itemActions);

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
	void drawMarginItems(int page, int totalPages, QPdfWriter* pdfWriter, QPainter* painter);

private:
	int m_resolution = 300;

	std::vector<ReportMarginItem> m_marginItems;
	ReportSchemaView* m_schemaView = nullptr;

	QTextCharFormat m_currentCharFormat;
	QTextBlockFormat m_currentBlockFormat;

	QTextCharFormat m_currentCharFormatSaved;
	QTextBlockFormat m_currentBlockFormatSaved;
};
