#include "SchemaLocatorProvider.h"

namespace Locator
{
	SchemaLocatorThread::SchemaLocatorThread(DbController* dbc) :
		m_mainDbc{dbc}
	{
		Q_ASSERT(dbc);

		m_dbc.disableProgress();

		connect(dbc, &DbController::projectOpened, this, &SchemaLocatorThread::openConnection, Qt::QueuedConnection);
		connect(dbc, &DbController::projectClosed, this, &SchemaLocatorThread::closeConnection, Qt::QueuedConnection);

		return;
	}

	void SchemaLocatorThread::updateText(const QString& text)
	{
		m_textMutex.lock();
		m_text = text;
		m_textMutex.unlock();
	}

	void SchemaLocatorThread::openConnection()
	{
		m_dbc.setHost(m_mainDbc->host());
		m_dbc.setPort(m_mainDbc->port());
		m_dbc.setServerUsername(m_mainDbc->serverUsername());
		m_dbc.setServerPassword(m_mainDbc->serverPassword());

		bool opened = m_dbc.openProject(m_mainDbc->currentProject().projectName(),
										m_mainDbc->currentUser().username(),
										m_mainDbc->currentUser().password(),
										nullptr);
		if (opened == false)
		{
			qDebug() << "SchemaLocatorThread::openConnection(): Error opening project " <<
						m_mainDbc->currentProject().projectName() <<
						"\n\tError: " << m_mainDbc->lastError();
		}


		// Cache files right after open, make it available ot once.
		//
		updateFiles();
		return;
	}

	void SchemaLocatorThread::closeConnection()
	{
		if (m_dbc.isProjectOpened() == true)
		{
			m_files.clear();
			m_schemaDetails.clear();
			m_dbc.closeProject(nullptr);
		}

		return;
	}

	void SchemaLocatorThread::newTask(const QString& text)
	{
		if (text.isEmpty() == true)
		{
			emit resultReady(text, {});
			return;
		}

		{
			QMutexLocker locker{&m_textMutex};
			if (m_text != text)
			{
				// This is an outdated message
				//
				return;
			}
		}

		if (m_lastSearchTimer.isValid() == false || m_lastSearchTimer.hasExpired(60'000) == true)
		{
			updateFiles();
		}

		std::vector<LocatedItem> result;
		result.reserve(32);
		bool addMoreSchemas = false;

		for (const auto&[fileId, file] : m_files)
		{
			if (file->isFolder() == true)
			{
				continue;
			}

			if (QString name = file->fileName();
				name.contains(text, Qt::CaseInsensitive) == true)
			{
				QString path = m_files.filePath(file->fileId());
				if (path.startsWith(QChar{'/'}) == true)
				{
					path = path.right(path.size() - 1);
				}

				result.emplace_back(name, path, QVariant::fromValue(*file.get()));
			}

			if (auto schemaDetailsIt = m_schemaDetails.find(fileId);
				schemaDetailsIt != m_schemaDetails.end())
			{
				const VFrame30::SchemaDetails& schemaDetails = schemaDetailsIt->second;

				QUuid guid = QUuid::fromString(text);

				if (schemaDetails.hasSignal(text) == true ||
					schemaDetails.hasEquipmentId(text) == true ||
					schemaDetails.hasLoopback(text) == true ||
					schemaDetails.hasLabel(text) == true ||
					schemaDetails.hasConnection(text) == true ||
					(guid.isNull() == false && schemaDetails.hasGuid(guid) == true))
				{
					QString path = m_files.filePath(file->fileId());
					if (path.startsWith(QChar{'/'}) == true)
					{
						path = path.right(path.size() - 1);
					}

					result.emplace_back(text + " on schema " + file->fileName(), path, QVariant::fromValue(*file.get()));
				}
			}

			if (result.size() > 255)
			{
				// Enough
				//
				addMoreSchemas = true;
				break;
			}

		}

		// Sort by caption (path) first.
		//
		std::sort(result.begin(), result.end(), [](const LocatedItem& l, const LocatedItem& r) {
			return std::make_tuple(l.caption, l.what) < std::make_tuple(r.caption, r.what);
		});

		if (addMoreSchemas == true)
		{
			// This record must not be in std::sort.
			//
			result.emplace_back("...", tr("...more schemas..."));
		}

		// --
		//
		emit resultReady(text, std::move(result));
		return;
	}

	bool SchemaLocatorThread::updateFiles()
	{
		if (m_lastSearchTimer.isValid() == false)
		{
			m_lastSearchTimer.start();
		}
		else
		{
			m_lastSearchTimer.restart();
		}

		m_files.clear();
		m_schemaDetails.clear();

		bool ok = m_dbc.getFileListTree(&m_files, m_dbc.systemFileId(DbDir::SchemasDir), true, nullptr);
		if (ok == false)
		{
			return false;
		}

		m_files.removeFilesWithExtension(File::AlTemplExtension);
		m_files.removeFilesWithExtension(File::MvsTemplExtension);
		m_files.removeFilesWithExtension(File::UfbTemplExtension);
		m_files.removeFilesWithExtension(File::DvsTemplExtension);

		m_schemaDetails.reserve(m_files.size());
		for (const auto&[fileId, fileInfo] : m_files)
		{
			m_schemaDetails[fileId] = VFrame30::SchemaDetails{fileInfo->details()};
		}

		return true;
	}

	SchemaLocatorProvider::SchemaLocatorProvider(DbController* dbc) :
		m_thread{dbc}
	{
		m_thread.moveToThread(&m_thread);
		m_thread.start();

		connect(this, &SchemaLocatorProvider::newTask, &m_thread, &SchemaLocatorThread::newTask, Qt::QueuedConnection);
		connect(&m_thread, &SchemaLocatorThread::resultReady, this, &SchemaLocatorProvider::taskResultReady, Qt::QueuedConnection);
		return;
	}

	SchemaLocatorProvider::~SchemaLocatorProvider()
	{
		m_thread.exit();
		m_thread.wait(10000);
		return;
	}

	QString SchemaLocatorProvider::name() const
	{
		return QStringLiteral("Schema");
	}

	void SchemaLocatorProvider::locateFor(const QString& text)
	{
		if (text.size() < 1)
		{
			emit resultReady(text, this, {});
			return;
		}

		m_text = text;
		m_thread.updateText(text);

		emit newTask(text);
		return;
	}

	void SchemaLocatorProvider::stopSearching()
	{
		return;
	}

	void SchemaLocatorProvider::taskResultReady(QString text, const std::vector<LocatedItem>& result)
	{
		if (text != m_text)
		{
			// This is some old search
			//
			return;
		}

		emit resultReady(text, this, result);
		return;
	}
}

