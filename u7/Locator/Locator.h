#pragma once
#include "LocatorProvider.h"

namespace Locator
{
	class LocatorListWidget;

	class Locator : public QObject
	{
		Q_OBJECT

	public:
		explicit Locator(LocatorListWidget& listWidget);

	public:
		void addProvider(LocatorProvider& locatorProvider);

	public:
		void setText(const QString& text);
		void stopSearching();

	signals:
		void signal_locateFor(QString text);
		void signal_stopSearching();

	private slots:
		void resultReady(QString text, LocatorProvider* provider, const std::vector<LocatedItem>& items);

	private:
		LocatorListWidget& m_listWidget;
		QString m_text;
	};
}

