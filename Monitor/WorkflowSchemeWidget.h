#ifndef WORKFLOWSCHEMEWIDGET_H
#define WORKFLOWSCHEMEWIDGET_H


namespace VFrame30
{
	class Scheme;
}

class WorkflowSchemeView;


//
//
// WorkflowSchemeWidget
//
//
class WorkflowSchemeWidget : public QScrollArea
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
	QPointF widgetPointToDocument(const QPoint& widgetPoint) const;

protected:
	bool MousePosToDocPoint(const QPoint& mousePos, QPointF* destDocPos, int dpiX = 0, int dpiY = 0);

	// Signals
	//
signals:
	//	void closeTab(QWidget* tabWidget);		// Command to the owner to Close current tab

	// Slots
	//
protected slots:
	void zoomIn();
	void zoomOut();
	void zoom100();

	// Properties
	//
public:
	std::shared_ptr<VFrame30::Scheme> scheme();
	const std::shared_ptr<VFrame30::Scheme> scheme() const;
	void setScheme(std::shared_ptr<VFrame30::Scheme> scheme);

	WorkflowSchemeView* schemeView();
	const WorkflowSchemeView* schemeView() const;

	double zoom() const;
	void setZoom(double zoom, int horzScrollValue = -1, int vertScrollValue = -1);

	// Data
	//
private:
	WorkflowSchemeView* m_schemeView = nullptr;

	// Actions
	//
private:
};


#endif // WORKFLOWSCHEMEWIDGET_H
