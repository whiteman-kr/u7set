#include "LogFile.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>
#include <QAbstractItemModel>
#include <QComboBox>
#include <QUuid>
#include <QTableView>
#include <QFileDialog>
#include <QHeaderView>
#include <QDesktopWidget>
#include "Ui/UiTools.h"
#include "../UtilsLib/Hash.h"

//#define LOGFILE_USE_HEADER	// Uncomment this to use header

namespace Log
{
	//
	// LogFileRecord
	//

	const char* messageTypeTextShort[] = {"ALL", "ERR", "WRN", "MSG", "ALERT", "TXT", "DATA"};

	const char* messageTypeTextLong[] = {"All", "Error", "Warning", "Message", "Alert", "Text", "DataInvisible"};

	const int messageTypeCount = sizeof(messageTypeTextShort) / sizeof(messageTypeTextShort[0]);

	const int messageTypeLongCount = sizeof(messageTypeTextLong) / sizeof(messageTypeTextLong[0]);

	const char* messageTimeFormat = {"dd.MM.yyyy hh:mm:ss.zzz"};

	QString LogFileRecord::toString(const QString& sessionHashString)
	{
		if (type == MessageType::Text)
		{
			return QString("%1\t%2\r\n").arg(sessionHashString).arg(text);
		}

		int intType = static_cast<int>(type);
		if (intType < 0 || intType >= messageTypeCount)
		{
			assert(false);
			return QString();
		}

		if (type == MessageType::Data)
		{
			return QString("%1\t%2\t\t%3\t%4\r\n").arg(sessionHashString).arg(time.toString(messageTimeFormat)).arg(messageTypeTextShort[intType]).arg(textArray.join('\t'));
		}
		else
		{
			return QString("%1\t%2\t\t%3\t%4\r\n").arg(sessionHashString).arg(time.toString(messageTimeFormat)).arg(messageTypeTextShort[intType]).arg(text);
		}
	}

	bool LogFileRecord::loadFromString(const QString& source, quint64 currentSessionHash)
	{

		QString str = source;

		// Session Hash

		int tabPos = str.indexOf('\t');
		if (tabPos == -1)
		{
			return false;
		}

		QString s = str.left(tabPos).trimmed();
		str.remove(0, tabPos + 1);
		str = str.trimmed();

		sessionHash = s.toULongLong();

		if (currentSessionHash != 0 && sessionHash != currentSessionHash)
		{
			// Wrong session
			return false;
		}

		type = MessageType::Text;

		// Time

		tabPos = str.indexOf('\t');
		if (tabPos == -1)
		{
			// This is simple text
			text = str;
			return true;
		}

		s = str.left(tabPos).trimmed();
		str.remove(0, tabPos + 1);
		str = str.trimmed();

		time = QDateTime::fromString(s, messageTimeFormat);

		// Type

		tabPos = str.indexOf('\t');
		if (tabPos == -1)
		{
			// This is simple text
			text = str;
			return true;
		}

		s = str.left(tabPos).trimmed();
		str.remove(0, tabPos + 1);
		str = str.trimmed();

		if (s == messageTypeTextShort[static_cast<int>(MessageType::Error)])
		{
			type = MessageType::Error;
		}
		else
		{
			if (s == messageTypeTextShort[static_cast<int>(MessageType::Warning)])
			{
				type = MessageType::Warning;
			}
			else
			{
				if (s == messageTypeTextShort[static_cast<int>(MessageType::Message)])
				{
					type = MessageType::Message;
				}
				else
				{
					if (s == messageTypeTextShort[static_cast<int>(MessageType::Alert)])
					{
						type = MessageType::Alert;
					}
					else
					{
						if (s == messageTypeTextShort[static_cast<int>(MessageType::Data)])
						{
							type = MessageType::Data;
						}
						else
						{
							assert(false);
							type = MessageType::Text;
							text = QObject::tr("UNKNOWN TYPE %1, %2").arg(s).arg(str);
							return true;
						}
					}
				}
			}
		}

		// Text

		if (type == MessageType::Data)
		{
			textArray = str.split('\t');
		}
		else
		{
			text = str;
		}

		return true;
	}

	//
	// LogFileWorker
	//

	LogFileWorker::LogFileWorker(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount, quint64 sessionHash)
		:m_logName(logName),
		  m_path(path),
		  m_maxFileSize(maxFileSize),
		  m_maxFilesCount(maxFilesCount),
		  m_sessionHash(sessionHash),
		  m_sessionHashString(QString::number(sessionHash).leftJustified(21, ' '))
	{
		assert(messageTypeCount == static_cast<int>(MessageType::Count));
		assert(messageTypeLongCount == static_cast<int>(MessageType::Count));
	}

	LogFileWorker::~LogFileWorker()
	{
	}

	bool LogFileWorker::write(MessageType type, const QString& text)
	{
		QMutexLocker l(&m_queueMutex);

		LogFileRecord r;

		r.time = QDateTime::currentDateTime();
		r.type = type;
		r.sessionHash = m_sessionHash;
		r.text = text;

		emit recordArrived(r);

		m_queue.push_back(r);

		return true;
	}

	bool LogFileWorker::writeArray(const QStringList& textArray)
	{
		QMutexLocker l(&m_queueMutex);

		LogFileRecord r;

		r.time = QDateTime::currentDateTime();
		r.type = MessageType::Data;
		r.sessionHash = m_sessionHash;
		r.textArray = textArray;

		emit recordArrived(r);

		m_queue.push_back(r);

		return true;
	}

	void LogFileWorker::read(bool currentSessionOnly)
	{
		emit readStart(currentSessionOnly);
	}

	void LogFileWorker::getLoadedData(std::vector<LogFileRecord> *result)
	{
		QMutexLocker l(&m_readMutex);
		result->swap(m_readResult);
	}

	QString LogFileWorker::logName() const
	{
		return m_logName;
	}

	void LogFileWorker::onThreadStarted()
	{
		// Initialize path

		if (m_path.isEmpty() == true)
		{
			QString localAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
			m_path = QDir::toNativeSeparators(localAppDataPath);
		}

		if (QDir().exists(m_path) == false)
		{
			if (QDir().mkpath(m_path) == false)
			{
				QString errorString = tr("LogFileWorker: can't create path %1").arg(m_path);
				emit writeFailure(errorString);
			}
		}

		// Start timer

		m_timer = new QTimer(this);

		connect(m_timer, &QTimer::timeout, this, &LogFileWorker::slot_onTimer);
		m_timer->start(500);

		// Get the number of last log file

		QDir dir(m_path);

		QStringList filters;

		filters << QString("%1_????.log").arg(m_logName);

		QStringList existingFiles = dir.entryList(filters, QDir::Files, QDir::Name);

		m_currentFileNumber = 0;

		if (existingFiles.isEmpty() == false)
		{
			QString lastFile = existingFiles.last();

			lastFile.remove(m_logName);
			lastFile.remove('_');
			lastFile.remove(".log");

			bool ok = false;
			int value = lastFile.toInt(&ok);
			if (ok == true)
			{
				m_currentFileNumber = value;
			}
		}

		// Connect load event to load slot

		connect(this, &LogFileWorker::readStart, this, &LogFileWorker::slot_load);

	}

	void LogFileWorker::onThreadFinished()
	{
		m_timer->stop();

		QString errorString;

		if (flush(&errorString) == false)
		{
			emit writeFailure(errorString);
		}
	}

	QString LogFileWorker::getLogFileName(int index) const
	{
		QString fileNumber = QString::number(index).rightJustified(4, '0');

		return QString("%1%2%3_%4.log").arg(m_path).arg(QDir::separator()).arg(m_logName).arg(fileNumber);
	}

	bool LogFileWorker::readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount)
	{
		QFile file(fileName);

		if (file.exists() == false)
		{
			return false;
		}

		if (file.open(QIODevice::ReadOnly | QIODevice::Text ) == false)
		{
			return false;
		}

		QTextStream logStream(&file);

		// Check the header line length

		QString s = logStream.readLine();
		if (s.isNull() == true)
		{
			return false;
		}
		s = s.trimmed();

		if (s.length() != m_serviceStringLength)
		{
			assert(false);
			return false;
		}

		int serviceLineLength = s.length();

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}

		// Read start time

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}
		s = s.trimmed();

		s = s.right(s.length() - s.indexOf('\t') - 1);

		startTime = QDateTime::fromString(s, "dd.MM.yyyy hh:mm:ss");

		if (startTime.isValid() == false)
		{
			assert(false);
			return false;
		}

		// Read end time

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}
		s = s.trimmed();

		s = s.right(s.length() - s.indexOf('\t') - 1);

		endTime = QDateTime::fromString(s, "dd.MM.yyyy hh:mm:ss");

		if (endTime.isValid() == false)
		{
			assert(false);
			return false;
		}

		// Read records count

		s = logStream.readLine();
		if (s.isNull() || s.length() != serviceLineLength)
		{
			assert(false);
			return false;
		}
		s = s.trimmed();

		s = s.right(s.length() - s.indexOf('\t') - 1);

		bool ok = false;

		recordsCount = s.toInt(&ok);

		if (ok == false)
		{
			assert(false);
			return false;
		}

		return true;
	}

	bool LogFileWorker::writeLogFileInfo(QFile& file, const QDateTime& startTime, const QDateTime& endTime, int recordsCount)
	{
		// Seek to the start
		//
		bool ok = file.seek(0);
		if (ok == false)
		{
			assert(ok);
			return false;
		}

		// Write header
		//

		QString str;

		str = str.leftJustified(m_serviceStringLength, '-').append("\r\n");

		file.write(str.toLocal8Bit());

		str = tr("Application:\t%1").arg(qAppName());

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toLocal8Bit());

		str = tr("Start Time:\t%1").arg(startTime.toString("dd.MM.yyyy hh:mm:ss"));

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toLocal8Bit());

		str = tr("End Time:\t%1").arg(endTime.toString("dd.MM.yyyy hh:mm:ss"));

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toLocal8Bit());

		str = tr("Records Count:\t%1").arg(recordsCount);

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toLocal8Bit());

		str.clear();

		str = str.leftJustified(m_serviceStringLength, '-').append("\r\n");

		file.write(str.toLocal8Bit());

		str = "\r\n";

		file.write(str.toLocal8Bit());

		// Seek to the end of the file
		//
		ok = file.seek(file.size());
		if (ok == false)
		{
			assert(false);
			return false;
		}

		return true;
	}

	bool LogFileWorker::flush(QString* errorString)
	{
		if (errorString == nullptr)
		{
			assert(errorString);
			return false;
		}

		errorString->clear();

		QMutexLocker l(&m_queueMutex);

		if (m_queue.size() == 0)
		{
			return true;
		}

		QString fileName = getLogFileName(m_currentFileNumber);

		// Check current file size and switch to the next file if needed
		{
			QFile file(fileName);

			if (file.size() > m_maxFileSize)
			{
				if (switchToNextLogFile(errorString) == false)
				{
					m_queue.clear();

					return false;
				}

				fileName = getLogFileName(m_currentFileNumber);
			}
		}


#ifdef LOGFILE_USE_HEADER
		// Read current file information

		QDateTime startTime;
		QDateTime endTime;
		int recordsCount = 0;

		if (readLogFileInfo(fileName, startTime, endTime, recordsCount) == false)
		{
			startTime = QDateTime::currentDateTime();
			endTime = startTime;
		}
#endif

		// Open file for writing

		QFile file(fileName);

		if (file.open(QIODevice::Append | QIODevice::Text ) == false)
		{
			*errorString = tr("Log file %1 open error: %2").arg(fileName).arg(file.errorString());

			m_queue.clear();

			return false;
		}

		// Write header data

#ifdef LOGFILE_USE_HEADER
		endTime = QDateTime::currentDateTime();

		recordsCount += static_cast<int>(m_queue.size());

		writeLogFileInfo(file, startTime, endTime, recordsCount);
#endif

		// Write records

		for (LogFileRecord& record : m_queue)
		{
			if (file.write(record.toString(m_sessionHashString).toLocal8Bit()) == -1)
			{
				*errorString = tr("Log file %1 write error: %2").arg(fileName).arg(file.errorString());

				file.close();

				m_queue.clear();

				return false;
			}
		}

		file.close();

		m_queue.clear();

		return true;
	}

	bool LogFileWorker::switchToNextLogFile(QString* errorString)
	{
		if (errorString == nullptr)
		{
			assert(errorString);
			return false;
		}

		// If current file is less than max count, switch to the next file

		if (m_currentFileNumber < m_maxFilesCount - 1)
		{
			m_currentFileNumber++;
			return true;
		}

		// Delete file number 0

		{
			QString fileName = getLogFileName(0);

			QFile f(fileName);

			if (f.exists() == false)
			{
				*errorString = tr("LogFileWorker::switchToNextLogFile, file %1 does not exist.").arg(fileName);
				return false;
			}

			if (f.remove() == false)
			{
				*errorString = tr("LogFileWorker::switchToNextLogFile, can't remove file %1.").arg(fileName);
				return false;
			}
		}

		// Rename others

		for (int i = 1; i < m_maxFilesCount; i++)
		{
			QString fileNameOld = getLogFileName(i);
			QString fileNameNew = getLogFileName(i - 1);

			QFile f(fileNameOld);

			if (f.rename(fileNameNew) == false)
			{
				*errorString = tr("LogFileWorker::switchToNextLogFile, can't rename file %1 -> %2.").arg(fileNameOld).arg(fileNameNew);
				return false;
			}
		}

		return true;
	}

	bool LogFileWorker::readFileRecords(const QString& fileName, bool currentSessionOnly, std::vector<LogFileRecord>* result)
	{
		if (result == nullptr)
		{
			assert(false);
			return false;
		}

		QFile f(fileName);

		if (f.exists() == false)
		{
			return true;
		}

		if (f.open(QFile::ReadOnly) == false)
		{
			return false;
		}

		QTextStream stream(&f);

#ifdef LOGFILE_USE_HEADER
		// Read and skip header

		const int headerLinesCount = 7;

		for (int i = 0; i < headerLinesCount; i++)
		{
			if (stream.readLine().isNull() == true)
			{
				return false;
			}
		}
#endif

		QString str;

		LogFileRecord record;

		while (true)
		{
			str = stream.readLine();
			if (str.isNull())
			{
				break;
			}

			if (record.loadFromString(str, currentSessionOnly == true ? m_sessionHash : 0) == true)
			{
				result->push_back(record);
			}
		}


		return true;

	}


	void LogFileWorker::slot_load(bool currentSessionOnly)
	{
		std::vector<LogFileRecord> readResult;

		for (int i = 0; i < m_maxFilesCount; i++)
		{
			readFileRecords(getLogFileName(i), currentSessionOnly, &readResult);
		}

		{
			QMutexLocker l(&m_readMutex);
			readResult.swap(m_readResult);
		}

		emit readComplete();
	}

	void LogFileWorker::slot_onTimer()
	{
		QString errorString;

		if (flush(&errorString) == false)
		{
			emit writeFailure(errorString);
		}
	}


	//
	// LogRecordModel
	//
	LogRecordModel::LogRecordModel(bool showTypeColumn, std::vector<std::pair<QString, double> > headerTitles):
		m_showTypeColumn(showTypeColumn)
	{
		int c = 0;

		double usedWidth = 0;

		m_columnsNames << tr("Time");
		m_columnsWidthPercent.push_back(0.15);
		usedWidth += 0.15;
		m_columnTime = c++;

		if (m_showTypeColumn == true)
		{
			m_columnsNames << tr("Type");
			m_columnsWidthPercent.push_back(0.05);
			usedWidth += 0.05;
			m_columnType = c++;
		}

		if (headerTitles.size() == 0)
		{
			m_columnsNames << tr("Message");
			m_columnsWidthPercent.push_back(1 - usedWidth);
			m_columnText = c++;
		}
		else
		{
			int count = static_cast<int>(headerTitles.size());

			double totalWidth = (1 - usedWidth);

			for (int i = 0; i < count; i++)
			{
				const QString& s = headerTitles[i].first;

				m_columnsNames << s;
				m_columnsWidthPercent.push_back(totalWidth * headerTitles[i].second);
			}

			m_columnText = c++;
		}
	}

	LogRecordModel::~LogRecordModel()
	{

	}

	void LogRecordModel::setRecords(std::vector<LogFileRecord>* records)
	{
		m_records.swap(*records);

		fillRecords();
	}

	void LogRecordModel::addRecord(const LogFileRecord& record)
	{
		m_records.push_back(record);

		if (processRecordFilter(record) == true)
		{
			int index = static_cast<int>(m_filteredRecordsIndex.size());

			beginInsertRows(QModelIndex(), index, index);

			m_filteredRecordsIndex.push_back(static_cast<int>(m_records.size() - 1));

			insertRows(index, 1);

			endInsertRows();
		}

	}

	void LogRecordModel::setFilter(MessageType messageType, const QString& text)
	{
		m_filterMessageType = messageType;
		m_filterText = text;

		fillRecords();
	}

	void LogRecordModel::fillRecords()
	{
		// Remove data from the model

		if (rowCount() > 0)
		{
			beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

			removeRows(0, rowCount());

			m_filteredRecordsIndex.clear();

			endRemoveRows();
		}

		// Process filters

		int count = static_cast<int>(m_records.size());

		m_filteredRecordsIndex.reserve(count);

		for (int i = 0; i < count; i++)
		{
			const LogFileRecord& rec = m_records[i];

			if (processRecordFilter(rec) == true)
			{
				m_filteredRecordsIndex.push_back(i);
			}
		}

		// Set data to the model

		if (m_filteredRecordsIndex.empty() == false)
		{
			int filterRecordsCount = static_cast<int>(m_filteredRecordsIndex.size());

			beginInsertRows(QModelIndex(), 0, filterRecordsCount - 1);

			insertRows(0, static_cast<int>(filterRecordsCount));

			endInsertRows();
		}

	}

	bool LogRecordModel::processRecordFilter(const LogFileRecord& record) const
	{
		if (m_filterMessageType != MessageType::All)
		{
			if (m_filterMessageType != record.type)
			{
				return false;
			}
		}

		if (m_filterText.isEmpty() == false)
		{
			if (record.type == MessageType::Data)
			{
				if (record.textArray.join(';').contains(m_filterText) == false)
				{
					return false;
				}
			}
			else
			{
				if (record.text.contains(m_filterText) == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	double LogRecordModel::columnWidthPercent(int index)
	{
		if (index < 0 || index >= m_columnsWidthPercent.size())
		{
			assert(false);
			return 100;
		}

		return m_columnsWidthPercent[index];
	}

	int LogRecordModel::rowCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(m_filteredRecordsIndex.size());
	}

	int LogRecordModel::columnCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(m_columnsNames.size());
	}

	QModelIndex LogRecordModel::index(int row, int column, const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return createIndex(row, column);
	}

	QModelIndex LogRecordModel::parent(const QModelIndex& index) const
	{
		Q_UNUSED(index);
		return QModelIndex();
	}

	QVariant LogRecordModel::data(const QModelIndex& index, int role) const
	{
		if (role == Qt::DisplayRole)
		{

			int col = index.column();

			if (col < 0 || col >= m_columnsNames.size())
			{
				assert(false);
				return QVariant();
			}

			int row = index.row();
			if (row >= m_filteredRecordsIndex.size())
			{
				assert(false);
				return QVariant();
			}

			int recordIndex = m_filteredRecordsIndex[row];

			const LogFileRecord& rec = m_records[recordIndex];

			int displayIndex = col;

			if (displayIndex == m_columnTime)
			{
				return rec.time.toString(messageTimeFormat);
			}

			if (displayIndex == m_columnType)
			{
				int intType = static_cast<int>(rec.type);
				if (intType < 0 || intType >= messageTypeCount)
				{
					assert(false);
					return QString();
				}

				return messageTypeTextShort[intType];
			}

			if (displayIndex >= m_columnText)
			{
				if (rec.type == MessageType::Data)
				{
					int textColumnNo = displayIndex - m_columnText;

					if (textColumnNo < 0)
					{
						assert(false);
						return QVariant();
					}

					if (textColumnNo < rec.textArray.size())
					{
						return rec.textArray[textColumnNo];
					}
				}
				else
				{
					return rec.text;
				}
			}
		}
		return QVariant();
	}

	QVariant LogRecordModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		{
			if (section < 0 || section >= m_columnsNames.size())
			{
				assert(false);
				return QVariant();
			}

			return m_columnsNames.at(section);
		}

		return QVariant();
	}

	//
	// LogFileDialog
	//

	LogFileDialog::LogFileDialog(LogFileWorker* worker, QWidget* parent, bool showType, std::vector<std::pair<QString, double>> headerTitles)
		:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
		  m_worker(worker),
		  m_model(showType, headerTitles)
	{
		setAttribute(Qt::WA_DeleteOnClose);

		setWindowTitle(tr("Log View - %1").arg(worker->logName()));

		QVBoxLayout* mainLayout = new QVBoxLayout();
		setLayout(mainLayout);

		QHBoxLayout* topLayout = new QHBoxLayout();
		mainLayout->addLayout(topLayout);

		//

		if (showType == true)
		{
			topLayout->addWidget(new QLabel("Type:"));

			//

			m_typeCombo = new QComboBox();

			m_typeCombo->blockSignals(true);

			connect(m_typeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LogFileDialog::onTypeComboIndexChanged);

			for (int i = 0; i < messageTypeCount; i++)
			{
				if (QString(messageTypeTextLong[i]).contains("Invisible") == false)
				{
					m_typeCombo->addItem(messageTypeTextLong[i]);
				}
			}
			m_typeCombo->setCurrentIndex(0);

			m_typeCombo->blockSignals(false);

			topLayout->addWidget(m_typeCombo);

			//

			topLayout->addStretch();
		}

		//

		m_filterLineEdit = new QLineEdit();
		topLayout->addWidget(m_filterLineEdit);

		connect(m_filterLineEdit, &QLineEdit::returnPressed, this, &LogFileDialog::onFilter);

		//

		QPushButton* b = new QPushButton(tr("Filter"));
		topLayout->addWidget(b);

		connect(b, &QPushButton::clicked, this, &LogFileDialog::onFilter);

		topLayout->addStretch();

		//

		m_allSessions = new QPushButton(tr("All Sessions"));
		m_allSessions->setCheckable(true);
		m_allSessions->setChecked(false);
		topLayout->addWidget(m_allSessions);
		connect(m_allSessions, &QPushButton::clicked, this, &LogFileDialog::onAllSessionsClicked);

		//

		m_autoScroll = new QPushButton(tr("Auto Scroll"));
		m_autoScroll->setCheckable(true);
		topLayout->addWidget(m_autoScroll);

		//

		m_table = new QTableView();
		m_table->setModel(&m_model);
		m_table->verticalHeader()->hide();
		m_table->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
		m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
		m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		m_table->setSortingEnabled(false);

		//

		mainLayout->addWidget(m_table);

		//

		m_counterLabel = new QLabel();

		m_export = new QPushButton(tr("Export"));
		connect(m_export, &QPushButton::clicked, this, &LogFileDialog::onExport);

		QHBoxLayout* bottomLayout = new QHBoxLayout();
		bottomLayout->addWidget(m_counterLabel);
		bottomLayout->addStretch();
		bottomLayout->addWidget(m_export);

		mainLayout->addLayout(bottomLayout);

		QSize defaultSize = QSize(1024, 600);

		setMinimumSize(defaultSize);

		//

		connect(m_worker, &LogFileWorker::readComplete, this, &LogFileDialog::onReadComplete);

		connect(m_worker, &LogFileWorker::recordArrived, this, &LogFileDialog::onRecordArrived);

		m_worker->read(true);
		enableControls(false);

		// Restore settings

		QSettings s;

		QPoint windowPos = s.value("LogFileDialog/windowPos", QPoint(-1, -1)).toPoint();
		QSize windowSize = s.value("LogFileDialog/windowSize", QSize(-1, -1)).toSize();

		if (windowPos.x() != -1 && windowPos.y() != -1)
		{
			move(windowPos);
		}
		else
		{
			QRect desktopRect = QApplication::desktop()->availableGeometry(this);
			int x = (desktopRect.width() - width()) / 2;
			int y = (desktopRect.height() - height()) / 2;
			move(QPoint(x, y));
		}

		if (windowSize.width() != -1 && windowSize.height() != -1)
		{
			resize(windowSize);
		}
		else
		{
			resize(defaultSize);
		}

		bool autoScroll = s.value("LogFileDialog/autoScroll", true).toBool();
		m_autoScroll->setChecked(autoScroll);

	}

	LogFileDialog::~LogFileDialog()
	{
		QSettings s;

		QPoint windowPos = pos();
		QSize windowSize = size();

		s.setValue("LogFileDialog/windowPos", windowPos);
		s.setValue("LogFileDialog/windowSize", windowSize);

		if (m_autoScroll != nullptr)
		{
			bool autoScroll = m_autoScroll->isChecked();
			s.setValue("LogFileDialog/autoScroll", autoScroll);
		}
	}

	void LogFileDialog::resizeEvent(QResizeEvent *event)
	{

		Q_UNUSED(event);

		if (m_table == nullptr)
		{
			return;
		}

		QSize s = m_table->size();

		double totalWidth = s.width() - 25;

		for (int c = 0; c < m_table->horizontalHeader()->count() ; c++)
		{
			int columnWidth = static_cast<int>(totalWidth * m_model.columnWidthPercent(c));

			if (columnWidth >= s.width())
			{
				columnWidth = 100;
			}

			m_table->setColumnWidth(c, columnWidth);
		}

	}

	void LogFileDialog::enableControls(bool enable)
	{
		if (m_typeCombo != nullptr)
		{
			m_typeCombo->setEnabled(enable);
		}

		m_filterLineEdit->setEnabled(enable);
		m_allSessions->setEnabled(enable);
		m_autoScroll->setEnabled(enable);
	}

	void LogFileDialog::onTypeComboIndexChanged(int index)
	{
		Q_UNUSED(index);
		onFilter();
	}

	void LogFileDialog::onFilter()
	{
		MessageType filterMessageType = MessageType::All;

		if (m_typeCombo != nullptr)
		{
			int typeComboIndex = m_typeCombo->currentIndex();

			if (typeComboIndex < 0 || typeComboIndex >= messageTypeCount)
			{
				assert(false);
				return;
			}

			filterMessageType = static_cast<MessageType>(typeComboIndex);
		}

		QString filterText = m_filterLineEdit->text();

		m_model.setFilter(filterMessageType, filterText);

		m_counterLabel->setText(tr("Total records: %1").arg(m_model.rowCount()));
	}

	void LogFileDialog::onAllSessionsClicked()
	{
		m_worker->read(m_allSessions->isChecked() == false);
		enableControls(false);
	}

	void LogFileDialog::onReadComplete()
	{
		std::vector<LogFileRecord> loadResult;

		m_worker->getLoadedData(&loadResult);

		m_model.setRecords(&loadResult);

		m_counterLabel->setText(tr("Total records: %1").arg(m_model.rowCount()));

		m_table->scrollToBottom();

		enableControls(true);
	}

	void LogFileDialog::onRecordArrived(LogFileRecord record)
	{
		if (isVisible() == false)
		{
			return;
		}

		m_model.addRecord(record);

		m_counterLabel->setText(tr("Total records: %1").arg(m_model.rowCount()));

		if (m_autoScroll->isChecked() == true)
		{
			m_table->scrollToBottom();
		}
	}

	void LogFileDialog::onExport()
	{
		QString fileName = QFileDialog::getSaveFileName(this,
														tr("Save File"),
														"untitled.csv",
														tr("CSV Files, semicolon separated (*.csv)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}

		QFile data(fileName);
		if (data.open(QFile::WriteOnly | QFile::Truncate) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("File creation error!"));
			return;
		}

		QTextStream out(&data);

		QAbstractItemModel* model = m_table->model();

		int colCount = model->columnCount();
		int rowCount = model->rowCount();

		for (int c = 0; c < colCount; c++)
		{
			QString s = model->headerData(c, Qt::Horizontal).toString();

			out << s;
			if (c != colCount - 1)
			{
				out << ";";
			}
		}
		out << Qt::endl;

		for (int r = 0; r < rowCount; r++)
		{
			for (int c = 0; c < colCount; c++)
			{
				QString s = model->data(model->index(r, c)).toString();
				out << s;
				if (c != colCount - 1)
				{
					out << ";";
				}
			}
			out << Qt::endl;
		}
	}

	//
	// LogFile
	//

	LogFile::LogFile(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount)
	{
		QUuid uuid = QUuid::createUuid();

		m_sessionHash = ::calcHash(uuid.toString());

		m_logFileWorker = new LogFileWorker(logName, path, maxFileSize, maxFilesCount, m_sessionHash);

		connect(m_logFileWorker, &LogFileWorker::writeFailure, this, &LogFile::onFlushFailure);

		m_logThread.addWorker(m_logFileWorker);
		m_logThread.start();


		// Register LogFileRecord meta type
		//
		static int regLogFileRecordMetaType = false;

		if (regLogFileRecordMetaType == false)
		{
			regLogFileRecordMetaType = true;
			qRegisterMetaType<LogFileRecord>();
		}
	}

	LogFile::~LogFile()
	{
		m_logFileWorker = nullptr;

		bool ok = m_logThread.quitAndWait(10000);

		if (ok == false)
		{
			// Thread termination timeout
			assert(ok);
		}

	}

	bool LogFile::writeMessage(const QString& text)
	{
		return write(MessageType::Message, text);
	}

	bool LogFile::writeAlert(const QString& text)
	{
		m_alertAckCounter++;

		emit alertArrived(text);

		return write(MessageType::Alert, text);
	}

	bool LogFile::writeError(const QString& text)
	{
		m_errorAckCounter++;

		return write(MessageType::Error, text);
	}

	bool LogFile::writeWarning(const QString& text)
	{
		m_warningAckCounter++;

		return write(MessageType::Warning, text);
	}

	bool LogFile::writeText(const QString& text)
	{
		return write(MessageType::Text, text);
	}

	bool LogFile::writeArray(const QStringList& textArray)
	{
		if (m_logFileWorker == nullptr)
		{
			return true;
		}

		return m_logFileWorker->writeArray(textArray);
	}

	bool LogFile::write(MessageType type, const QString& text)
	{
		if (m_logFileWorker == nullptr)
		{
			return true;
		}

		return m_logFileWorker->write(type, text);
	}

	void LogFile::view(QWidget* parent, bool showType, std::vector<std::pair<QString, double>> headerTitles)
	{
		m_alertAckCounter = 0;
		m_errorAckCounter = 0;
		m_warningAckCounter = 0;

		if (m_logDialog != nullptr)
		{
			if (m_logDialog->isActiveWindow() == false)
			{
				m_logDialog->activateWindow();
			}
		}
		else
		{
			m_logDialog = new LogFileDialog(m_logFileWorker, parent, showType, headerTitles);

			connect(m_logDialog, &QDialog::finished, this, &LogFile::onDialogFinished);

			m_logDialog->show();
		}

		UiTools::adjustDialogPlacement(m_logDialog);

		return;
	}

	int LogFile::alertAckCounter() const
	{
		return m_alertAckCounter;
	}

	int LogFile::errorAckCounter() const
	{
		return m_errorAckCounter;
	}

	int LogFile::warningAckCounter() const
	{
		return m_warningAckCounter;
	}

	void LogFile::onFlushFailure(QString errorString)
	{
		emit writeFailure(errorString);
	}

	void LogFile::onDialogFinished(int result)
	{
		Q_UNUSED(result);

		m_logDialog = nullptr;
	}

}

