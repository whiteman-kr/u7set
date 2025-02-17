#include "NotificationPanel.h"

#include <QHBoxLayout>
#include <QToolButton>


namespace SimUi
{
	NotificationBanner::NotificationBanner(QWidget* parent) :
		QFrame{parent}
	{
		setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
		setStyleSheet("background-color: #ffffcc;");

		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		auto layout = new QHBoxLayout{this};
		layout->setContentsMargins(layout->contentsMargins() / 3);

		auto closeButton = new QToolButton(this);
		closeButton->setIcon(QIcon{":/SimulatorUi/Images/SimCloseButtonBlack.svg"});

		QLabel* label = new QLabel{this};
		label->setOpenExternalLinks(false);

		layout->addWidget(label);
		layout->addWidget(closeButton);

		connect(closeButton, &QToolButton::clicked, this, &NotificationBanner::closed);

		return;
	}

	void NotificationBanner::setText(const QString& message, std::function<void(QString)> func)
	{
		auto label = findChild<QLabel*>();
		Q_ASSERT(label);

		label->setText(message);

		label->disconnect();
		connect(label, &QLabel::linkActivated, this, std::move(func));

		return;
	}

	NotificationPanel::NotificationPanel(QWidget* parent) :
		QDockWidget{"Notifications", parent}
	{
		setFeatures(QDockWidget::NoDockWidgetFeatures);

		setTitleBarWidget(new QWidget{}); // Hides title bar
		setAllowedAreas(Qt::TopDockWidgetArea);

		m_banner = new NotificationBanner{this};
		setWidget(m_banner);

		connect(m_banner, &NotificationBanner::closed, this, &NotificationPanel::hide);

		return;
	}

	void NotificationPanel::showNotification(const QString& message, int maxHeight, std::function<void(QString)> func)
	{
		m_banner->setText(message, std::move(func));
		m_banner->setMaximumHeight(maxHeight);

		show();

		return;
	}
} // namespace SimUi
