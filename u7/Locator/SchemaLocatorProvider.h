#pragma once

#include "LocatorProvider.h"
#include <VFrame30/SchemaDetails.h>

namespace Locator
{
	class SchemaLocatorThread : public QThread
	{
		Q_OBJECT

	public:
		explicit SchemaLocatorThread(DbController* dbc);

		void updateText(const QString& text);

	public slots:
		void openConnection();
		void closeConnection();

		void newTask(const QString& text);

	signals:
		void resultReady(QString text, std::vector<LocatedItem> result);

	private:
		bool updateFiles();

	private:
		DbController* m_mainDbc = nullptr;	// This dbc is NOT used for communictation, it used for getting connection params only.
		DbController m_dbc;

		QElapsedTimer m_lastSearchTimer;
		DbFileTree m_files;
		std::unordered_map<int, VFrame30::SchemaDetails> m_schemaDetails;		// Key is fileId

		// m_text is used to skip messages which are in the queue.
		//
		QMutex m_textMutex;
		QString m_text;
	};

	class SchemaLocatorProvider : public LocatorProvider
	{
		Q_OBJECT

	public:
		explicit SchemaLocatorProvider(DbController* dbc);
		virtual ~SchemaLocatorProvider();

		virtual QString name() const override;

	protected:
		virtual void locateFor(const QString& text) override;
		virtual void stopSearching() override;

	private slots:
		void taskResultReady(QString text, const std::vector<LocatedItem>& result);

	signals:
		void newTask(QString text);

	private:
		SchemaLocatorThread m_thread;
		QString m_text;
	};
}
