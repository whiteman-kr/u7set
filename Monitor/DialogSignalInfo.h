#ifndef DIALOGSIGNALINFO_H
#define DIALOGSIGNALINFO_H

#include <QDialog>
#include <QLabel>
#include "../lib/AppSignalManager.h"

namespace Ui {
class DialogSignalInfo;
}

class SignalFlagsWidget : public QWidget
{
	Q_OBJECT

public:
	SignalFlagsWidget(QWidget *parent = 0);

	void updateControl(AppSignalStateFlags flags);

protected:
	void paintEvent(QPaintEvent *event);

private:
	AppSignalStateFlags m_flags;
};


class DialogSignalInfo : public QDialog
{
	Q_OBJECT

public:
	DialogSignalInfo(const AppSignalParam& signal, QWidget* parent);
	~DialogSignalInfo();

public:
	enum class ViewType
	{
		Dec, Hex, Bin16, Bin32, Bin64, Exp, Count
	};

	Q_ENUM(ViewType)

private slots:
	void prepareContextMenu(const QPoint& pos);

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	void mousePressEvent(QMouseEvent* event);

private:
	void updateData();
	void contextMenu(QPoint pos);

private:
	Ui::DialogSignalInfo *ui;

	QString m_appSignalId;
	AppSignalParam m_signal;
	Hash m_hash;

	int m_updateStateTimerId = -1;

	SignalFlagsWidget* m_signalFlags = nullptr;

	int m_precision = 0;

	ViewType m_viewType = ViewType::Dec;

	int m_currentFontSize = 20;
};

class QLabelAppSignalDragAndDrop : public QLabel
{
public:
	explicit QLabelAppSignalDragAndDrop(QWidget* parent = nullptr);

public:
	void setAppSignal(const AppSignalParam& signal);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	AppSignalParam m_appSignalParam;
	QPoint m_dragStartPosition;
};

#endif // DIALOGSIGNALINFO_H
