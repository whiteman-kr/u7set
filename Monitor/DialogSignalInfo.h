#ifndef DIALOGSIGNALINFO_H
#define DIALOGSIGNALINFO_H

#include <QDialog>
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
	explicit DialogSignalInfo(QWidget *parent, const Signal& signal);
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
	Signal m_signal;
	Hash m_hash;

	int m_updateStateTimerId = -1;

	SignalFlagsWidget* m_signalFlags = nullptr;

	int m_precision = 0;

	ViewType m_viewType = ViewType::Dec;

	int m_currentFontSize = 20;

};

#endif // DIALOGSIGNALINFO_H
