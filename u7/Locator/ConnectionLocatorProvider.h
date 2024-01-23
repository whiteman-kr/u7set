#pragma once

#include "LocatorProvider.h"

namespace Locator
{
	class ConnectionLocatorThread : public QThread
	{
		Q_OBJECT

	public:
		explicit ConnectionLocatorThread(DbController* dbc);

		void updateText(const QString& text);

	public slots:
		void openConnection();
		void closeConnection();

		void newTask(const QString& text);

	signals:
		void resultReady(QString text, std::vector<LocatedItem> result);

	private:
		DbController* m_mainDbc = nullptr;	// This dbc is NOT used for communictation, it used for getting connection params only.
		DbController m_dbc;

		// m_text is used to skip messages which are in the queue.
		//
		QMutex m_textMutex;
		QString m_text;
	};

	class ConnectionLocatorProvider : public LocatorProvider
	{
		Q_OBJECT

	public:
		explicit ConnectionLocatorProvider(DbController* dbc);
		virtual ~ConnectionLocatorProvider();

		virtual QString name() const override;

	protected:
		virtual void locateFor(const QString& text) override;
		virtual void stopSearching() override;

	private slots:
		void taskResultReady(QString text, const std::vector<LocatedItem>& result);

	signals:
		void newTask(QString text);

	private:
		ConnectionLocatorThread m_thread;
		QString m_text;
	};
}
