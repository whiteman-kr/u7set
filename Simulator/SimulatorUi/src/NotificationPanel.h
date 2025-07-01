#pragma once

#include <QDockWidget>
#include <QFrame>

namespace SimUi
{
	class NotificationBanner : public QFrame
	{
		Q_OBJECT

	public:
		explicit NotificationBanner(QWidget* parent = nullptr);

	public:
		void setText(const QString& message, std::function<void(QString)> func);

	signals:
		void closed();
	};


	class NotificationPanel : public QDockWidget
	{
		Q_OBJECT

	public:
		explicit NotificationPanel(QWidget* parent = nullptr);

	public:
		void showNotification(const QString& message, int maxHeight, std::function<void(QString)> func);

	private:
		NotificationBanner* m_banner = nullptr;
	};

} // namespace SimUi