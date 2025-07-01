#pragma once

#include <QMutex>
#include <QStringList>
#include <QTextEdit>

namespace SimUi
{
	class SimOutputWidget : public QTextEdit
	{
		Q_OBJECT

	public:
		explicit SimOutputWidget(QWidget* parent);

	public:
		static void simulatorOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

	protected:
		virtual void timerEvent(QTimerEvent* event) override;
		virtual void contextMenuEvent(QContextMenuEvent* event) override;
		virtual void keyPressEvent(QKeyEvent*) override;

	private:
		static QtMessageHandler simOriginalMessageHandler;

		static QMutex m_mutex;
		static QStringList m_data;
	};
} // namespace SimUi