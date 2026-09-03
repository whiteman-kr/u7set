#pragma once

#include <QWidget>

class RvduWidget : public QWidget
{
	Q_OBJECT
public:
	explicit RvduWidget(unsigned int* pHdmiBuffer, int width, int height, QWidget* parent = nullptr);
	~RvduWidget();

	void setRunningMode(bool running);
	void setErrorMode(bool error, const QString& errorMsg);

private:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);

private:
	unsigned int* m_pHdmiBuffer = nullptr;
	int m_width = 0;
	int m_height = 0; 

	bool m_isRunning = false;
	bool m_isError = false;
	QString m_errorMsg;
};
