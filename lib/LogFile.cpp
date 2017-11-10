#include "LogFile.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>
#include <QAbstractItemModel>

#include "Windows.h"

namespace Log
{
	//
	// LogFileRecord
	//

	const int LogFileRecord::typeCont = 4;

	const char* LogFileRecord::typeTextShort[] = {"ERR", "WRN", "MSG", ""};

	QString LogFileRecord::toString()
	{
		if (type == MessageType::Text)
		{
			return QString("%1\r\n").arg(text);
		}

		int intType = static_cast<int>(type);
		if (intType < 0 || intType >= typeCont)
		{
			assert(false);
			return QString();
		}

		return QString("%1\t%2\t%3\t%4\r\n").arg(time.toString("dd.MM.yyyy hh:mm:ss")).arg(intType).arg(typeTextShort[intType]).arg(text);
	}

	//
	// LogFileWorker
	//

	LogFileWorker::LogFileWorker(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount)
		:m_logName(logName),
		  m_path(path),
		  m_maxFileSize(maxFileSize),
		  m_maxFilesCount(maxFilesCount)
	{
		if (m_path.isEmpty() == true)
		{
			QString localAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
			m_path = QDir::toNativeSeparators(localAppDataPath);
		}

		qDebug() << "Log path : " << m_path;

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
		r.text = text;

		emit recordArrived(r);

		m_queue.push_back(r);

		return true;
	}

	void LogFileWorker::read()
	{
		emit readStart();
	}

	void LogFileWorker::getLoadedData(std::vector<LogFileRecord> *result)
	{
		QMutexLocker l(&m_readMutex);
		result->swap(m_readResult);
	}

	void LogFileWorker::onThreadStarted()
	{
		// Start timer

		m_timer = new QTimer(this);

		connect(m_timer, &QTimer::timeout, this, &LogFileWorker::slot_onTimer);
		m_timer->start(500);

		// Get the number of last log file

		QDir dir(m_path);

		QStringList filters;

		filters << QString("%1_????.log").arg(qAppName());

		QStringList existingFiles = dir.entryList(filters, QDir::Files, QDir::Name);

		m_currentFileNumber = 0;

		if (existingFiles.isEmpty() == false)
		{
			QString lastFile = existingFiles.last();

			lastFile.remove(qAppName());
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

		if (flush() == false)
		{
			emit flushFailure();
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

		file.write(str.toUtf8());

		str = tr("Application:\t%1").arg(qAppName());

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str = tr("Start Time:\t%1").arg(startTime.toString("dd.MM.yyyy hh:mm:ss"));

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str = tr("End Time:\t%1").arg(endTime.toString("dd.MM.yyyy hh:mm:ss"));

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str = tr("Records Count:\t%1").arg(recordsCount);

		str = str.leftJustified(m_serviceStringLength, ' ').append("\r\n");

		file.write(str.toUtf8());

		str.clear();

		str = str.leftJustified(m_serviceStringLength, '-').append("\r\n");

		file.write(str.toUtf8());

		str = "\r\n";

		file.write(str.toUtf8());

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

	bool LogFileWorker::flush()
	{
		QMutexLocker l(&m_queueMutex);

		if (m_queue.size() == 0)
		{
			return true;
		}

		QDateTime startTime;
		QDateTime endTime;
		int recordsCount = 0;

		QString fileName = getLogFileName(m_currentFileNumber);

		// Check current file size and switch to the next file if needed
		{
			QFile file(fileName);

			if (file.size() > m_maxFileSize)
			{
				if (switchToNextLogFile() == true)
				{
					fileName = getLogFileName(m_currentFileNumber);
				}
			}
		}

		// Read current file information

		if (readLogFileInfo(fileName, startTime, endTime, recordsCount) == false)
		{
			startTime = QDateTime::currentDateTime();
			endTime = startTime;
		}

		// Open file for writing

		QFile file(fileName);

		if (file.open(QIODevice::Append | QIODevice::Text ) == false)
		{
			return false;
		}

		// Write header data

		endTime = QDateTime::currentDateTime();

		recordsCount += static_cast<int>(m_queue.size());

		writeLogFileInfo(file, startTime, endTime, recordsCount);

		// Write records

		for (LogFileRecord& record : m_queue)
		{
			file.write(record.toString().toUtf8());
		}

		file.close();

		m_queue.clear();

		return true;
	}

	bool LogFileWorker::switchToNextLogFile()
	{
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
				assert(false);
				return false;
			}

			if (f.remove() == false)
			{
				assert(false);
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
				assert(false);
				return false;
			}
		}

		return true;
	}

	bool LogFileWorker::readFileRecords(const QString& fileName, std::vector<LogFileRecord>* result)
	{
		qDebug() << fileName;

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

		// Read and skip header

		const int headerLinesCount = 7;

		for (int i = 0; i < headerLinesCount; i++)
		{
			if (stream.readLine().isNull() == true)
			{
				return false;
			}
		}

		QString str;

		LogFileRecord record;

		while (true)
		{
			str = stream.readLine();
			if (str.isNull())
			{
				break;
			}

			// Time

			int tabPos = str.indexOf('\t');
			if (tabPos == -1)
			{
				record.type = MessageType::Text;
				record.text = str.trimmed();
				result->push_back(record);
				continue;
			}

			QString s = str.left(tabPos).trimmed();
			str.remove(0, tabPos + 1);

			record.time = QDateTime::fromString(s, "dd.MM.yyyy hh:mm:ss");

			// Type

			tabPos = str.indexOf('\t');
			if (tabPos == -1)
			{
				continue;
			}

			s = str.left(tabPos).trimmed();
			str.remove(0, tabPos + 1);

			record.type = static_cast<MessageType>(s.toInt());

			// Type text
			tabPos = str.indexOf('\t');
			if (tabPos == -1)
			{
				continue;
			}

			s = str.left(tabPos).trimmed();
			str.remove(0, tabPos + 1);

			// Text

			record.text = str.trimmed();

			result->push_back(record);
		}


		return true;

	}


	void LogFileWorker::slot_load()
	{
		std::vector<LogFileRecord> readResult;

		for (int i = 0; i < m_maxFilesCount; i++)
		{
			readFileRecords(getLogFileName(i), &readResult);
		}

		{
			QMutexLocker l(&m_readMutex);
			readResult.swap(m_readResult);
		}

		emit readComplete();
	}

	void LogFileWorker::slot_onTimer()
	{
		if (flush() == false)
		{
			emit flushFailure();
		}
	}

	//
	// LogFile
	//

	LogFile::LogFile(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount)
	{

		m_logFileWorker = new LogFileWorker(logName, path, maxFileSize, maxFilesCount);

		connect(m_logFileWorker, &LogFileWorker::flushFailure, this, &LogFile::onFlushFailure);

		m_logThread.addWorker(m_logFileWorker);
		m_logThread.start();
	}

	LogFile::~LogFile()
	{
		bool ok = m_logThread.quitAndWait(10000);

		if (ok == false)
		{
			// Thread termination timeout
			assert(ok);
		}
	}

	bool LogFile::writeMessage(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Message, text);
	}

	bool LogFile::writeError(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Error, text);
	}

	bool LogFile::writeWarning(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Warning, text);
	}

	bool LogFile::writeText(const QString& text)
	{
		return m_logFileWorker->write(MessageType::Text, text);
	}

	bool LogFile::write(MessageType type, const QString& text)
	{
		return m_logFileWorker->write(type, text);
	}

	void LogFile::view(QWidget* parent)
	{
		if (m_logDialog != nullptr)
		{
			m_logDialog->activateWindow();

			return;
		}

		m_logDialog = new LogFileDialog(m_logFileWorker, parent);

		connect(m_logDialog, &QDialog::finished, this, &LogFile::onDialogFinished);

		m_logDialog->show();

		return;

	}

	void LogFile::onFlushFailure()
	{
		qDebug() << "Log write failure!";
		assert(false);
		emit writeFailure();
	}

	void LogFile::onDialogFinished(int result)
	{
		Q_UNUSED(result);

		m_logDialog = nullptr;
	}

	//
	// LogRecordModel
	//
	LogRecordModel::LogRecordModel()
	{
		m_columnsNames << tr("Time");
		m_columnsNames << tr("Type");
		m_columnsNames << tr("Message");

		m_columnsWidthPercent.push_back(0.15);
		m_columnsWidthPercent.push_back(0.05);
		m_columnsWidthPercent.push_back(0.8);
	}

	LogRecordModel::~LogRecordModel()
	{

	}

	void LogRecordModel::setRecords(std::vector<LogFileRecord>* records)
	{
		if (rowCount() > 0)
		{
			beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

			removeRows(0, rowCount());

			m_records.clear();

			endRemoveRows();
		}

		//
		if (records->empty() == false)
		{
			int count = static_cast<int>(records->size());

			beginInsertRows(QModelIndex(), 0, count - 1);

			records->swap(m_records);

			insertRows(0, static_cast<int>(count));

			endInsertRows();
		}
	}

	void LogRecordModel::addRecord(const LogFileRecord& record)
	{
		int index = static_cast<int>(m_records.size() + 1);

		beginInsertRows(QModelIndex(), index, index);

		m_records.push_back(record);

		insertRows(index, 1);

		endInsertRows();
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
		return static_cast<int>(m_records.size());
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
			if (row >= m_records.size())
			{
				assert(false);
				return QVariant();
			}

			const LogFileRecord& rec = m_records[row];

			int displayIndex = col;

			if (displayIndex == static_cast<int>(Columns::Time))
			{
				return rec.time.toString("dd.MM.yyyy hh:mm:ss");
			}

			if (displayIndex == static_cast<int>(Columns::Type))
			{
				return LogFileRecord::typeTextShort[static_cast<int>(rec.type)];
			}

			if (displayIndex == static_cast<int>(Columns::Text))
			{
				return rec.text;
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

	LogFileDialog::LogFileDialog(LogFileWorker* worker, QWidget* parent)
		:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
		  m_worker(worker)
	{
		setAttribute(Qt::WA_DeleteOnClose);

		QVBoxLayout* mainLayout = new QVBoxLayout();
		setLayout(mainLayout);

		QHBoxLayout* topLayout = new QHBoxLayout();
		mainLayout->addLayout(topLayout);

		//

		topLayout->addWidget(new QLabel("Type:"));

		//

		m_typeCombo = new QComboBox();
		topLayout->addWidget(m_typeCombo);

		//

		QPushButton* b = new QPushButton(tr("Choose Period..."));
		topLayout->addWidget(b);

		connect(b, &QPushButton::clicked, this, &LogFileDialog::onChoosePeriod);

		topLayout->addStretch();

		//

		m_filter = new QLineEdit();
		topLayout->addWidget(m_filter);

		connect(m_filter, &QLineEdit::returnPressed, this, &LogFileDialog::onFilter);

		//

		b = new QPushButton(tr("Filter"));
		topLayout->addWidget(b);

		connect(b, &QPushButton::clicked, this, &LogFileDialog::onFilter);

		topLayout->addStretch();

		//

		m_autoScroll = new QPushButton(tr("Auto Scroll"));
		m_autoScroll->setCheckable(true);
		m_autoScroll->setChecked(true);
		topLayout->addWidget(m_autoScroll);

		connect(m_autoScroll, &QPushButton::clicked, this, &LogFileDialog::onAutoScroll);

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
		mainLayout->addWidget(m_counterLabel);

		setMinimumSize(1024, 600);

		//

		connect(m_worker, &LogFileWorker::readComplete, this, &LogFileDialog::onReadComplete);

		connect(m_worker, &LogFileWorker::recordArrived, this, &LogFileDialog::onRecordArrived);

		m_worker->read();


	}

	LogFileDialog::~LogFileDialog()
	{

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
			int columnWidth = totalWidth * m_model.columnWidthPercent(c);

			if (columnWidth >= s.width())
			{
				columnWidth = 100;
			}
			m_table->setColumnWidth(c, columnWidth);
		}

	}

	void LogFileDialog::onChoosePeriod()
	{

	}

	void LogFileDialog::onFilter()
	{

	}

	void LogFileDialog::onAutoScroll()
	{

	}


	void LogFileDialog::onReadComplete()
	{
		qDebug() << "onLoadComplete";

		std::vector<LogFileRecord> loadResult;

		m_worker->getLoadedData(&loadResult);

		m_model.setRecords(&loadResult);

		m_counterLabel->setText(tr("Total records: %1").arg(m_model.rowCount()));

		m_table->scrollToBottom();
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
}

