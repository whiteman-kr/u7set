#ifndef WORKFLOWSCHEMEVIEW_H
#define WORKFLOWSCHEMEVIEW_H

#include "../VFrame30/SchemeView.h"


class WorkflowSchemeView : public VFrame30::SchemeView
{
	Q_OBJECT

public:
	explicit WorkflowSchemeView(QWidget* parent = nullptr);
	virtual ~WorkflowSchemeView();

	// Painting
	//
protected:
	virtual void paintEvent(QPaintEvent*) override;

signals:

public slots:
};

#endif // WORKFLOWSCHEMEVIEW_H
