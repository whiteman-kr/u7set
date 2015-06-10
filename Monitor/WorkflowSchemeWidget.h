#ifndef WORKFLOWSCHEMEWIDGET_H
#define WORKFLOWSCHEMEWIDGET_H

#include "../VFrame30/BaseSchemeWidget.h"

class WorkflowSchemeView;

//
//
// WorkflowSchemeWidget
//
//
class WorkflowSchemeWidget : public VFrame30::BaseSchemeWidget
{
	Q_OBJECT

private:
	WorkflowSchemeWidget() = delete;

public:
	WorkflowSchemeWidget(std::shared_ptr<VFrame30::Scheme> scheme);
	virtual ~WorkflowSchemeWidget();

protected:
	void createActions();

	// Methods
	//
public:

	// Signals
	//
signals:
	//	void closeTab(QWidget* tabWidget);		// Command to the owner to Close current tab

	// Slots
	//
protected slots:

	// Properties
	//
public:
	//WorkflowSchemeView* schemeView();
	//const WorkflowSchemeView* schemeView() const;


	// Data
	//
private:

	// Actions
	//
private:
};


#endif // WORKFLOWSCHEMEWIDGET_H
