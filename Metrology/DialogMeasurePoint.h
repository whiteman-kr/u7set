#ifndef DIALOGOPTIONSPOINT_H
#define DIALOGOPTIONSPOINT_H

#include <QDialog>
#include <QMenu>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>

#include "Options.h"

// ==============================================================================================

class DialogMeasurePoint : public QDialog
{
	Q_OBJECT

public:

	explicit DialogMeasurePoint(const LinearityOption& linearity, QWidget* parent = nullptr);
	virtual ~DialogMeasurePoint();

public:

	LinearityOption&	linearity() { return m_linearity; }
	void				setLinearity(const LinearityOption&	linearity) { m_linearity = linearity; }

private:

	LinearityOption		m_linearity;

	QAction*			m_pColumnAction[Measure::PointSensorCount];
	QMenu*				m_headerContextMenu = nullptr;

	// elements of interface
	//
	QLabel*				m_rangeTypeLabel = nullptr;
	QComboBox*			m_rangeTypeList = nullptr;

	QLabel*				m_pointCountLabel = nullptr;
	QLineEdit*			m_pointCountEdit = nullptr;
	QLabel*				m_lowRangeLabel = nullptr;
	QLineEdit*			m_lowRangeEdit = nullptr;
	QLabel*				m_highRangeLabel = nullptr;
	QLineEdit*			m_highRangeEdit = nullptr;


	QPushButton*		m_addButton = nullptr;
	QPushButton*		m_editButton = nullptr;
	QPushButton*		m_removeButton = nullptr;
	QPushButton*		m_upButton = nullptr;
	QPushButton*		m_downButton = nullptr;

	QTableWidget*		m_pointList = nullptr;

	bool				m_updatingList = false;

	void				setHeaderList();
	void				updateRangeType();
	void				updateList();
	void				clearList();

	void				hideColumn(int column, bool hide);

protected:

	void				keyPressEvent(QKeyEvent* e);
	void				showEvent(QShowEvent* e);

signals:

	void				updateLinearityPage(bool isDialog);

private slots:

	void				onAddPoint();
	void				onEditPoint();
	void				onRemovePoint();
	void				onUpPoint();
	void				onDownPoint();
	void				onRangeType(int type);
	void				onAutomaticCalculatePoints();

	void				cellChanged(int,int);
	void				currentCellChanged(int,int,int,int);

	void				onHeaderContextMenu(QPoint);
	void				onColumnAction(QAction* action);
};

// ==============================================================================================

#endif // DIALOGOPTIONSPOINT_H
