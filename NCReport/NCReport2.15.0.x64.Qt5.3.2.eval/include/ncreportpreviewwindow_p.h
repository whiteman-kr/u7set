#ifndef NCREPORTPREVIEWWINDOW_P_H
#define NCREPORTPREVIEWWINDOW_P_H

#include "ncreportpreviewwindow.h"

#include <QEventLoop>

class NCReportPreviewWindowPrivate
{
public:
	NCReportPreviewWindowPrivate(NCReportPreviewWindow* parent) : q_ptr(parent), eventLoop(NULL) {}
	~NCReportPreviewWindowPrivate() {}

private:
	Q_DECLARE_PUBLIC(NCReportPreviewWindow);
	NCReportPreviewWindow* q_ptr;

public:
	QEventLoop* eventLoop;
};

#endif // NCREPORTPREVIEWWINDOW_P_H
