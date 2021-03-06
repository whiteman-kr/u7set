#ifndef DIALOGOPTIONSMEASUREVIEWHEADER_H
#define DIALOGOPTIONSMEASUREVIEWHEADER_H

#include <QDebug>
#include <QDialog>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>

#include "Options.h"

// ==============================================================================================

const char* const		MvhColumn[] =
{
						QT_TRANSLATE_NOOP("DialogOptionsMvh", "Title"),
						QT_TRANSLATE_NOOP("DialogOptionsMvh", "Visible"),
						QT_TRANSLATE_NOOP("DialogOptionsMvh", "Width"),
};

const int				MVH_COLUMN_COUNT	= sizeof(MvhColumn)/sizeof(MvhColumn[0]);

const int				MVH_COLUMN_TITLE	= 0,
						MVH_COLUMN_VISIBLE	= 1,
						MVH_COLUMN_WIDTH	= 2;

// ----------------------------------------------------------------------------------------------

const int				MvhColumnWidth[MVH_COLUMN_COUNT] =
{
						200,
						90,
						90,
};

// ==============================================================================================

class DialogOptionsMeasureViewHeader : public QDialog
{
	Q_OBJECT

public:
	explicit DialogOptionsMeasureViewHeader(const MeasureViewOption& header, QWidget* parent = nullptr);
	virtual ~DialogOptionsMeasureViewHeader();

	MeasureViewOption	m_header;

public:

	Measure::Type		measureType() const { return m_measureType; }

private:

	Measure::Type		m_measureType = Measure::Type::Linearity;
	LanguageType		m_languageType = LanguageType::English;

	// elements of interface
	//
	QLabel*				m_measureTypeLabel = nullptr;
	QComboBox*			m_measureTypeList = nullptr;

	QTableWidget*		m_columnList = nullptr;

	bool				m_updatingList = false;

	void				setHeaderList();
	void				updateList();
	void				clearList();

protected:

	void				keyPressEvent(QKeyEvent* e);
	void				showEvent(QShowEvent* e);

signals:

	void				updateMeasureViewPage(bool isDialog);

private slots:

	void				setMeasureType(int measureType);

	void				cellChanged(int,int);
	void				currentCellChanged(int,int,int,int);

	void				onEdit(int row, int column);
};

// ==============================================================================================

#endif // DIALOGOPTIONSMEASUREVIEWHEADER_H
