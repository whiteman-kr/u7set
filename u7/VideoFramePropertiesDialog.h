#pragma once

#include <QDialog>
#include "../VFrame30/VFrame30.h"

namespace Ui {
	class VideoFramePropertiesDialog;
}

class VideoFramePropertiesDialog : public QDialog
{
	Q_OBJECT
	
public:
	VideoFramePropertiesDialog(std::shared_ptr<VFrame30::Scheme> videoFrame, QWidget* parent);
	~VideoFramePropertiesDialog();

protected slots:
	virtual void accept();
	
private:
	Ui::VideoFramePropertiesDialog *ui;

	std::shared_ptr<VFrame30::Scheme> m_videoFrame;
};

