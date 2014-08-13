#include "VideoFramePropertiesDialog.h"
#include "ui_VideoFramePropertiesDialog.h"

VideoFramePropertiesDialog::VideoFramePropertiesDialog(std::shared_ptr<VFrame30::CVideoFrame> videoFrame, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::VideoFramePropertiesDialog),
	m_videoFrame(videoFrame)
{
	assert(m_videoFrame.get() != nullptr);
	ui->setupUi(this);

	ui->strdIdEdit->setText(m_videoFrame->strID());
	ui->captionEdit->setText(m_videoFrame->caption());

	ui->widthEdit->setText(QString::number(m_videoFrame->docWidth() * 25.4, 'f', 2));
	ui->heightEdit->setText(QString::number(m_videoFrame->docHeight() * 25.4, 'f', 2));

	return;
}

VideoFramePropertiesDialog::~VideoFramePropertiesDialog()
{
	delete ui;
}

void VideoFramePropertiesDialog::accept()
{
	// StrdID
	//
	QString strID = ui->strdIdEdit->text();
	if (strID.isEmpty() == true)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid StrID."));
		msgBox.exec();

		ui->strdIdEdit->setFocus();
		return;
	}

	// Caption
	//
	QString caption = ui->captionEdit->text();
	if (caption.isEmpty() == true)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid caption."));
		msgBox.exec();

		ui->captionEdit->setFocus();
		return;
	}

	// Width
	//
	bool widthResult = false;
	double width = ui->widthEdit->text().toDouble(&widthResult);

	if (widthResult == false ||
		width <= 0.0)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid width."));
		msgBox.exec();

		ui->widthEdit->setFocus();
		return;
	}

	// Height
	//
	bool heightResult = false;
	double height = ui->heightEdit->text().toDouble(&heightResult);

	if (heightResult == false ||
		height <= 0.0)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid height."));
		msgBox.exec();

		ui->heightEdit->setFocus();
		return;
	}

	// Assign values to the videoframe
	//
	m_videoFrame->setStrID(strID);
	m_videoFrame->setCaption(caption);

	m_videoFrame->setDocWidth(width / 25.4);
	m_videoFrame->setDocHeight(height / 25.4);

	QDialog::accept();
}
