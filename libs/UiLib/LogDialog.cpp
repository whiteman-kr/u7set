
#include <UiLib/LogDialog.h>
#include <UiLib/StandardColors.h>
#include <UiLib/UiTools.h>

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QScrollBar>
#include <QShortcut>
#include <QStyledItemDelegate>

namespace Log
{
	//
	// LogRecordModel
	//
	class LogRecordModel : public QAbstractItemModel
	{
	public:
		LogRecordModel(bool showTypeColumn, const QStringList& headerTitles, QString dateTimeFormat);
		~LogRecordModel();

	public:
		enum class Columns
		{
			Time = 0,
			Type,
			Text
		};

	public:
		void clear();

		void appendChunk(const std::shared_ptr<LogFileChunk>& chunk);
		void appendRecord(const LogFileRecord& record);

	public:
		const LogFileRecord& record(int row) const;
		QBrush color(const QModelIndex& index, bool selected) const;

	protected:
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		virtual QModelIndex parent(const QModelIndex& index) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		QStringList m_columnsNames;

		std::vector<std::shared_ptr<LogFileChunk>> m_chunks;

		bool m_showTypeColumn = true;

		int m_columnTime = -1;
		int m_columnType = -1;
		int m_columnText = -1;

		QString m_dateTimeFormat;
	};

	LogRecordModel::LogRecordModel(bool showTypeColumn, const QStringList& headerTitles, QString dateTimeFormat) :
		m_showTypeColumn(showTypeColumn),
		m_dateTimeFormat(dateTimeFormat)
	{
		int c = 0;

		m_columnsNames << tr("Time");
		m_columnTime = c++;

		if (m_showTypeColumn == true)
		{
			m_columnsNames << tr("Type");
			m_columnType = c++;
		}

		if (headerTitles.empty() == true)
		{
			m_columnsNames << tr("Message");
			m_columnText = c++;
		}
		else
		{
			for (const QString& s : headerTitles)
			{
				m_columnsNames << s;
			}

			m_columnText = c++;
		}
	}

	LogRecordModel::~LogRecordModel() {}

	void LogRecordModel::clear()
	{
		// Remove data from the model
		//
		if (rowCount() > 0)
		{
			beginResetModel();

			m_chunks.clear();

			endResetModel();
		}
	}

	void LogRecordModel::appendChunk(const std::shared_ptr<LogFileChunk>& chunk)
	{
		// Append data to the model
		//
		if (chunk->empty() == false)
		{
			int index = rowCount();

			int count = static_cast<int>(chunk->size());

			beginInsertRows(QModelIndex(), index, index + count - 1);

			m_chunks.insert(m_chunks.end(), chunk);

			endInsertRows();
		}
	}

	void LogRecordModel::appendRecord(const LogFileRecord& record)
	{
		if (m_chunks.empty() == true)
		{
			std::shared_ptr<LogFileChunk> chunk = std::make_shared<LogFileChunk>();
			m_chunks.insert(m_chunks.end(), chunk);
		}

		// Add record to last chunk
		//
		int index = rowCount();

		beginInsertRows(QModelIndex(), index, index);

		std::shared_ptr<LogFileChunk> ch = m_chunks[m_chunks.size() - 1];
		ch->push_back(record);

		endInsertRows();
	}

	const LogFileRecord& LogRecordModel::record(int row) const
	{
		int processedRows = 0;
		for (const auto& ch : m_chunks)
		{
			if (row >= processedRows && row < processedRows + static_cast<int>(ch->size()))
			{
				return (*ch)[row - processedRows];
			}
			processedRows += static_cast<int>(ch->size());
		}

		Q_ASSERT(false);
		static LogFileRecord emptyRecord;
		return emptyRecord;
	}

	QBrush LogRecordModel::color(const QModelIndex& index, bool selected) const
	{
		const LogFileRecord& rec = record(index.row());

		switch (rec.type)
		{
		case MessageType::Error:
		case MessageType::Alert:
			if (selected == true)
			{
				return QBrush{StandardColors::LogErrorForeground};
			}
			else
			{
				return QBrush{StandardColors::LogErrorForegroundDark};
			}
		case MessageType::Warning:
			if (selected == true)
			{
				return QBrush{StandardColors::LogWarningForegroundDark};
			}
			else
			{
				return QBrush{StandardColors::LogWarningForeground};
			}
		default:
			return {};
		}
	}

	int LogRecordModel::rowCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);

		int result = 0;
		for (const auto& ch : m_chunks)
		{
			result += static_cast<int>(ch->size());
		}

		return result;
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

	QVariant LogRecordModel::data(const QModelIndex& index, int role) const
	{
		size_t column = index.column();
		if (column >= static_cast<size_t>(m_columnsNames.size()))
		{
			assert(false);
			return {};
		}
		const LogFileRecord& rec = record(index.row());

		if (role == Qt::DisplayRole)
		{
			int displayIndex = static_cast<int>(column);
			if (displayIndex == m_columnTime)
			{
				return QDateTime().fromMSecsSinceEpoch(rec.time).toString(m_dateTimeFormat);
			}

			if (displayIndex == m_columnType)
			{
				size_t intType = static_cast<size_t>(rec.type);
				if (intType >= messageTypeTextShort.size())
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

					QStringList textArray = QString::fromLocal8Bit(rec.text.c_str()).split('\t');

					if (textColumnNo < textArray.size())
					{
						return textArray[textColumnNo];
					}
				}
				else
				{
					return QString::fromLocal8Bit(rec.text.c_str()).replace('\n', ' ');
				}
			}
		}

		if (role == Qt::ToolTipRole)
		{
			// Display tooltip only for text if it contains '\n'
			if (static_cast<int>(column) >= m_columnText && rec.type != MessageType::Data && rec.text.find('\n') != std::string::npos)
			{
				return QString::fromLocal8Bit(rec.text.c_str());
			}
		}

		return QVariant();
	}

	QModelIndex LogRecordModel::parent(const QModelIndex& index) const
	{
		Q_UNUSED(index);
		return QModelIndex();
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
	// LogRecordProxyModel
	//
	class LogRecordProxyModel : public QSortFilterProxyModel
	{
	public:
		LogRecordProxyModel(LogRecordModel* sourceModel);

		int recordCount() const;

		int sourceRow(int row) const;

		int errorCount() const;
		int warningCount() const;

		int filterRecordTypeMask() const;
		void setFilterRecordTypeMask(int value);

		QString filterText() const;
		void setFilterText(const QString& value);

		qint64 filterTimeFrom() const;
		qint64 filterTimeTo() const;
		void setFilterTime(qint64 from, quint64 to);

	private:
		virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

	private:
		LogRecordModel* m_sourceModel = nullptr;

		mutable int m_errorCount = 0;
		mutable int m_warningCount = 0;

		int m_filterRecordTypeMask = MessageType::All;
		std::string m_filterText;

		qint64 m_filterTimeFrom = -1;
		qint64 m_filterTimeTo = -1;
	};

    LogRecordProxyModel::LogRecordProxyModel(LogRecordModel* sourceModel):
        m_sourceModel(sourceModel)
    {
        connect(this, &QSortFilterProxyModel::modelAboutToBeReset, [this](){
            m_errorCount = 0;
            m_warningCount = 0;
        });

    }

    int LogRecordProxyModel::recordCount() const
    {
        return rowCount();
    }

    int LogRecordProxyModel::sourceRow(int row) const
    {
        return mapToSource(index(row, 0)).row();
    }

    int LogRecordProxyModel::errorCount() const
    {
        return m_errorCount;
    }

    int LogRecordProxyModel::warningCount() const
    {
        return m_warningCount;
    }


    int LogRecordProxyModel::filterRecordTypeMask() const
    {
        return m_filterRecordTypeMask;
    }

    void LogRecordProxyModel::setFilterRecordTypeMask(int value)
    {
        beginResetModel();

        m_filterRecordTypeMask = value;

        endResetModel();
    }

    QString LogRecordProxyModel::filterText() const
    {
        return QString::fromLocal8Bit(m_filterText.c_str());
    }

    void LogRecordProxyModel::setFilterText(const QString& value)
    {
        beginResetModel();

        m_filterText = value.toLocal8Bit().toStdString();

        endResetModel();
    }

    qint64 LogRecordProxyModel::filterTimeFrom() const
    {
        return m_filterTimeFrom;
    }

    qint64 LogRecordProxyModel::filterTimeTo() const
    {
        return m_filterTimeTo;
    }

    void LogRecordProxyModel::setFilterTime(qint64 from, quint64 to)
    {
        beginResetModel();

        m_filterTimeFrom = from;
        m_filterTimeTo = to;

        endResetModel();
    }

    bool LogRecordProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& /*sourceParent*/) const
    {
        const LogFileRecord& record = m_sourceModel->record(sourceRow);

        if (m_filterRecordTypeMask != MessageType::All)
        {
            if ((m_filterRecordTypeMask & (1 << record.type)) == 0)
            {
                return false;
            }
        }

        if (m_filterTimeFrom != -1 && m_filterTimeTo != -1)
        {
            if (record.time < m_filterTimeFrom || record.time > m_filterTimeTo)
            {
                return false;
            }
        }

        if (m_filterText.empty() == false)
        {
            if (record.text.find(m_filterText) == std::string::npos)
            {
                return false;
            }
        }

        if (record.type == MessageType::Error)
        {
            m_errorCount++;
        }

        if (record.type == MessageType::Warning)
        {
            m_warningCount++;
        }

        return true;
    }


	//
	// SelectionControlDelegate
	//
	class SelectionControlDelegate : public QStyledItemDelegate
	{
	public:
		SelectionControlDelegate(QObject* parent, LogRecordModel* model, LogRecordProxyModel* proxyModel);
		void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

	private:
		LogRecordModel* m_model = nullptr;
		LogRecordProxyModel* m_proxyModel = nullptr;
	};

	SelectionControlDelegate::SelectionControlDelegate(QObject* parent, LogRecordModel* model, LogRecordProxyModel* proxyModel) :
		QStyledItemDelegate(parent),
        m_model(model),
        m_proxyModel(proxyModel)
	{
	}

	void SelectionControlDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
	{
		QStyledItemDelegate::initStyleOption(option, index);
		
		bool active = option->state & QStyle::State_Active;
		bool selected = option->state & QStyle::State_Selected;

		QBrush br = m_model->color(m_proxyModel->mapToSource(index), selected);
		option->palette.setColor(QPalette::Text, br.color());
		
		// Set color for selected item (by default it is displayed by white)
		//
		if (selected == true)
		{
			if (br.style() == Qt::NoBrush && active == true)
			{
				// Use white color on selected items if control is active
				//
				option->palette.setColor(QPalette::HighlightedText, Qt::white);
			}
			else
			{
				option->palette.setColor(QPalette::HighlightedText, br.color());
			}
			if (active == true)
			{
				option->palette.setColor(QPalette::Highlight, qRgb(0x90, 0xc8, 0xf6));
			}
			else
			{
				option->palette.setColor(QPalette::Highlight, qRgb(0xe0, 0xe0, 0xe0));
			}
		}
		else
		{
			option->palette.setColor(QPalette::Base, Qt::white);
		}
	}

    //
	// DialogTimeFilter
	//

	class DialogTimeFilter : public QDialog
	{
		Q_OBJECT
	public:
		DialogTimeFilter(qint64 filterTimeFrom, qint64 filterTimeTo, QString dateTimeFormat, QWidget* parent);
		virtual ~DialogTimeFilter();

		qint64 filterTimeFrom() const;
		qint64 filterTimeTo() const;

	private:
		virtual void accept();

		qint64 m_filterTimeFrom = -1;
		qint64 m_filterTimeTo = -1;

		QDateTimeEdit* m_timeFromEdit = nullptr;
		QDateTimeEdit* m_timeToEdit = nullptr;
	};

	DialogTimeFilter::DialogTimeFilter(qint64 filterTimeFrom, qint64 filterTimeTo, QString dateTimeFormat, QWidget* parent) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
		m_filterTimeFrom(filterTimeFrom),
		m_filterTimeTo(filterTimeTo)
	{
		setWindowTitle(tr("Time Filter"));

		QVBoxLayout* mainLayout = new QVBoxLayout();

		mainLayout->addWidget(new QLabel(tr("Start Time:")));
		m_timeFromEdit = new QDateTimeEdit(this);
		m_timeFromEdit->setDisplayFormat(dateTimeFormat);
		mainLayout->addWidget(m_timeFromEdit);

		mainLayout->addWidget(new QLabel(tr("End Time:")));
		m_timeToEdit = new QDateTimeEdit(this);
		m_timeToEdit->setDisplayFormat(dateTimeFormat);
		mainLayout->addWidget(m_timeToEdit);

		if (m_filterTimeFrom != -1 && m_filterTimeTo != -1)
		{
			m_timeFromEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(m_filterTimeFrom));
			m_timeToEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(m_filterTimeTo));
		}
		else
		{
			QDateTime now = QDateTime::currentDateTime();
			m_timeFromEdit->setDateTime(now);
			m_timeToEdit->setDateTime(now);
		}

		QHBoxLayout* hb = new QHBoxLayout();
		hb->addStretch();

		QPushButton* b = new QPushButton(tr("Set"));
		connect(b, &QPushButton::clicked, this, &DialogTimeFilter::accept);
		hb->addWidget(b);

		b = new QPushButton(tr("Reset"));
		connect(b, &QPushButton::clicked, [this](){
			m_filterTimeFrom = -1;
			m_filterTimeTo = -1;
			QDialog::accept();
		});
		hb->addWidget(b);

		b = new QPushButton(tr("Cancel"));
		connect(b, &QPushButton::clicked, this, &DialogTimeFilter::reject);
		hb->addWidget(b);

		mainLayout->addLayout(hb);

		setLayout(mainLayout);

		return;
	}

	DialogTimeFilter::~DialogTimeFilter()
	{

	}

	qint64 DialogTimeFilter::filterTimeFrom() const
	{
		return m_filterTimeFrom;
	}

	qint64 DialogTimeFilter::filterTimeTo() const
	{
		return m_filterTimeTo;
	}

	void DialogTimeFilter::accept()
	{
		m_filterTimeFrom = m_timeFromEdit->dateTime().toMSecsSinceEpoch();
		m_filterTimeTo = m_timeToEdit->dateTime().toMSecsSinceEpoch();

		if (m_filterTimeFrom >= m_filterTimeTo)
		{
			m_timeFromEdit->setFocus();
			QMessageBox::critical(this, qAppName(), tr("Start Time should be earlier than End Time!"));
			return;
		}

		QDialog::accept();
		return;
	}


	//
	// LogTableView
	//

	class LogTableView : public QTableView
	{
		Q_OBJECT
	protected:
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void wheelEvent(QWheelEvent* event) override;

	signals:
		void copyKeyPressed();
		void turnOffAutoscroll();
	};

	void LogTableView::keyPressEvent(QKeyEvent* event)
	{
		if (event->key() == Qt::Key_F6 || event->key() == Qt::Key_F3)
		{
			return;
		}

		if (selectedIndexes().size() > 0)
		{
			if ((event->modifiers() & Qt::ControlModifier) != 0 && event->key() == Qt::Key_C)
			{
				emit copyKeyPressed();
				return;
			}
		}
		else
		{
			if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
			{
				return;
			}
		}

		if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown ||
			event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
			event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
		{
			emit turnOffAutoscroll();
		}

		QTableView::keyPressEvent(event);
		return;
	}

	void LogTableView::wheelEvent(QWheelEvent* event)
	{
		emit turnOffAutoscroll();

		QTableView::wheelEvent(event);
		return;
	}


	//
	// LogFileDialog
	//

    void LogFileDialog::view(LogFile& log, QWidget* parent, bool showType, bool headerVisible, const QStringList& headerTitles)
	{
		auto dialog = m_logDialogs[log.logName()];

		if (dialog != nullptr)
		{
			if (dialog->isActiveWindow() == false)
			{
				dialog->activateWindow();
			}
		}
		else
		{
			dialog = new LogFileDialog(log, parent, showType, headerVisible, headerTitles);
			connect(dialog, &QDialog::finished, dialog, &LogFileDialog::onDialogFinished);
			dialog->show();

			m_logDialogs[log.logName()] = dialog;
		}

		UiTools::adjustDialogPlacement(dialog);

		return;
	}

	LogFileDialog::LogFileDialog(LogFile& log,
								 QWidget* parent,
								 bool useMessageType,
								 bool headerVisible,
								 const QStringList& headerTitles) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
		m_log(log),
		m_model(std::make_unique<LogRecordModel>(useMessageType, headerTitles, m_log.dateTimeFormat())),
		m_proxyModel(std::make_unique<LogRecordProxyModel>(m_model.get()))
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle(tr("Log View - %1").arg(m_log.logName()));

		QVBoxLayout* mainLayout = new QVBoxLayout();
		setLayout(mainLayout);

		QHBoxLayout* topLayout = new QHBoxLayout();
		mainLayout->addLayout(topLayout);

		if (useMessageType == true)
		{
			topLayout->addWidget(new QLabel(tr("Type:")));

			m_recordTypeCombo = new QComboBox();
			m_recordTypeCombo->addItem(tr("All Messages"), MessageType::All);
			m_recordTypeCombo->addItem(tr("Errors"), 1 << MessageType::Error);
			m_recordTypeCombo->addItem(tr("Warnings"), 1 << MessageType::Warning);
			m_recordTypeCombo->addItem(tr("Errors and Warnings"), (1 << (MessageType::Error)) | (1 << (MessageType::Warning)));
			m_recordTypeCombo->addItem(tr("Alerts"), 1 << MessageType::Alert);
			m_recordTypeCombo->addItem(tr("Messages"), 1 << MessageType::Message);
			m_recordTypeCombo->addItem(tr("Text"), 1 << MessageType::Text);
			m_recordTypeCombo->setCurrentIndex(0);
			connect(m_recordTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LogFileDialog::onTypeComboIndexChanged);

			topLayout->addWidget(m_recordTypeCombo);
		}

		// --
		//
		m_timeFilterButton = new QPushButton(tr("Time Filter"));
		connect(m_timeFilterButton, &QPushButton::clicked, this, &LogFileDialog::onTimeFilter);
		topLayout->addWidget(m_timeFilterButton);

		topLayout->addStretch();

		// --
		//
		m_filterLineEdit = new QLineEdit();
		m_filterLineEdit->setPlaceholderText(tr("Search/Filter Text"));
		m_filterLineEdit->setClearButtonEnabled(true);
		topLayout->addWidget(m_filterLineEdit);
		connect(m_filterLineEdit, &QLineEdit::returnPressed, this, &LogFileDialog::onFilter);
		connect(m_filterLineEdit, &QLineEdit::textEdited, this, &LogFileDialog::onFilterEdited);

		QString text = QString(50, 'a');
		QFontMetrics fm(m_filterLineEdit->font());
		int pixelsWide = fm.boundingRect(text).width();
		m_filterLineEdit->setFixedWidth(pixelsWide);

		// --
		//
		QShortcut* shF3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
		connect(shF3, &QShortcut::activated, this, &LogFileDialog::onSearch);

		// --
		//
		m_search = new QPushButton(tr("Search <F3>"));
		m_search->setEnabled(false);
		m_search->setDefault(true);
		topLayout->addWidget(m_search);

		connect(m_search, &QPushButton::clicked, this, &LogFileDialog::onSearch);

		// --
		//
		m_filter = new QPushButton(tr("Filter"));
        m_filter->setEnabled(false);
		topLayout->addWidget(m_filter);

		connect(m_filter, &QPushButton::clicked, this, &LogFileDialog::onFilter);

		// --
		//
		if (useMessageType == true)
		{
			m_prevIssue = new QPushButton(tr("Prev Issue <Shift+F6>"));
			connect(m_prevIssue, &QPushButton::clicked, this, &LogFileDialog::onPrevIssue);
			topLayout->addWidget(m_prevIssue);

			QShortcut* shShiftF6 = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F6), this);
			connect(shShiftF6, &QShortcut::activated, this, &LogFileDialog::onPrevIssue);

			m_nextIssue = new QPushButton(tr("Next Issue <F6>"));
			connect(m_nextIssue, &QPushButton::clicked, this, &LogFileDialog::onNextIssue);
			topLayout->addWidget(m_nextIssue);

			QShortcut* shF6 = new QShortcut(QKeySequence(Qt::Key_F6), this);
			connect(shF6, &QShortcut::activated, this, &LogFileDialog::onNextIssue);
		}

		// --
		//
		topLayout->addStretch();

		// --
		//
		m_allSessions = new QPushButton(tr("All Sessions"));
		m_allSessions->setCheckable(true);
		m_allSessions->setChecked(false);
		topLayout->addWidget(m_allSessions);
		connect(m_allSessions, &QPushButton::clicked, this, &LogFileDialog::onAllSessionsClicked);

		// --
		//
		m_autoScroll = new QPushButton(tr("Auto Scroll"));
		m_autoScroll->setCheckable(true);
		topLayout->addWidget(m_autoScroll);

		// --
		//
		m_table = new LogTableView();
        m_proxyModel->setSourceModel(m_model.get());
        m_table->setModel(m_proxyModel.get());

		m_table->verticalHeader()->hide();
        m_table->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
        m_table->setItemDelegate(new SelectionControlDelegate(this, m_model.get(), m_proxyModel.get()));

        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        m_table->setStyleSheet("QTableView::item { margin: 10px }");
        m_table->horizontalHeader()->setStretchLastSection(true);

        if (headerVisible == false)
		{
            m_table->horizontalHeader()->hide();
        }

		m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

		m_table->setSortingEnabled(false);

		connect(m_table, &LogTableView::copyKeyPressed, this, &LogFileDialog::onCellCopyKeyPressed);
		connect(m_table->verticalScrollBar(), &QScrollBar::sliderMoved, [this](){
			turnOffAutoscroll();
		});
		connect(m_table, &LogTableView::turnOffAutoscroll, [this](){
			turnOffAutoscroll();
		});

		// --
		//
		mainLayout->addWidget(m_table);

		// --
		//
		QHBoxLayout* bottomLayout = new QHBoxLayout();

		m_counterLabel = new QLabel();
		bottomLayout->addWidget(m_counterLabel);

		m_logPathLabel = new QLabel();
		m_logPathLabel->setTextFormat(Qt::RichText);
		m_logPathLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

		if (m_log.noDiskLog() == true) 
		{
			m_logPathLabel->setText(tr("Log file writing is disabled"));
		}
		else
		{
			m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_log.getCurrentFileName()));
		}
		connect(m_logPathLabel, &QLabel::linkActivated, this, &LogFileDialog::onLinkActivated);

		bottomLayout->addWidget(m_logPathLabel);

		bottomLayout->addStretch();

		m_clear = new QPushButton(tr("Clear"));
		connect(m_clear, &QPushButton::clicked, this, &LogFileDialog::onClear);
		bottomLayout->addWidget(m_clear);

		m_load = new QPushButton(tr("Load..."));
		connect(m_load, &QPushButton::clicked, this, &LogFileDialog::onLoad);
		bottomLayout->addWidget(m_load);

		m_export = new QPushButton(tr("Export..."));
		connect(m_export, &QPushButton::clicked, this, &LogFileDialog::onExport);
		bottomLayout->addWidget(m_export);

		mainLayout->addLayout(bottomLayout);

		// --
        //
        connect(&m_log, &LogFile::loadChunkComplete, this, &LogFileDialog::onLoadChunkComplete);
		connect(&m_log, &LogFile::loadComplete, this, &LogFileDialog::onLoadComplete);
		connect(&m_log, &LogFile::recordArrived, this, &LogFileDialog::onRecordArrived, Qt::QueuedConnection);

		// Restore settings
		//
		QSettings s;

		QPoint windowPos = s.value("LogFileDialog/windowPos", QPoint(-1, -1)).toPoint();
		QByteArray windowGeometry = s.value("LogFileDialog/windowGeometry").toByteArray();

		if (windowPos.x() != -1 && windowPos.y() != -1)
		{
			move(windowPos);
		}
		else
		{
			QRect desktopRect = this->screen()->availableGeometry();
			int x = (desktopRect.width() - width()) / 2;
			int y = (desktopRect.height() - height()) / 2;
			move(QPoint(x, y));
		}

		if (windowGeometry.isEmpty() == false)
		{
			restoreGeometry(windowGeometry);
		}

		bool autoScroll = s.value("LogFileDialog/autoScroll", true).toBool();
		m_autoScroll->setChecked(autoScroll);

		// Fill log data
		//

		log.resetAckCounters();

		if (m_log.noDiskLog() == true) 
		{
			// Output the queue
			//
			auto queue = m_log.queue();
			std::shared_ptr<LogFileChunk> chunk = std::make_shared<LogFileChunk>();
			chunk->reserve(queue.size());
			for (const auto& rec : queue)
			{
				chunk->push_back(rec);
			}
			m_model->appendChunk(chunk);

            for (int i = 0; i < m_table->horizontalHeader()->count() - 1; i++)
			{
				m_table->resizeColumnToContents(i);
			}

			m_table->scrollToBottom();
			
			enableControls(true);
		}
		else
		{
			// Read existing log
			//
			m_log.load(true /*currentSessionOnly*/);
			enableControls(false);
		}

	}

	LogFileDialog::~LogFileDialog()
	{
		QSettings s;

		s.setValue("LogFileDialog/windowPos", pos());
		s.setValue("LogFileDialog/windowGeometry", saveGeometry());

		if (m_autoScroll != nullptr)
		{
			bool autoScroll = m_autoScroll->isChecked();
			s.setValue("LogFileDialog/autoScroll", autoScroll);
        }

        if (m_log.loadInProgress() == true)
        {
            m_log.cancelLoad();
        }
	}

	void LogFileDialog::enableControls(bool enable)
	{
        m_allSessions->setEnabled(enable && m_loadedFromFile == false);

		m_clear->setEnabled(enable);
        m_load->setEnabled(enable);
		m_export->setEnabled(enable);
	}

	void LogFileDialog::searchIssue(bool forward)
	{
		// Turn off autoscroll
		//
		m_autoScroll->setChecked(false);

		// Find an index with issue
		//
		int row = -1;

		QModelIndexList selectedIndexes = m_table->selectionModel()->selectedIndexes();
		if (selectedIndexes.empty() == true)
		{
            row = forward ? 0 : m_proxyModel->rowCount() - 1;
		}
		else
		{
			row = selectedIndexes[0].row();
		}

        int issueRow = searchIssue(row, forward);
		if (issueRow != -1)
		{
            m_table->selectionModel()->select(m_proxyModel->index(issueRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

            m_table->setCurrentIndex(m_proxyModel->index(issueRow, 0));

            m_table->scrollTo(m_proxyModel->index(issueRow, 0), QAbstractItemView::EnsureVisible);
		}
		else
		{
			QMessageBox::warning(this, qAppName(), tr("Issue was not found!"));
		}
	}

    int LogFileDialog::searchIssue(int startRow, bool forward) const
    {
        if (m_proxyModel->recordCount() == 0)
        {
            return -1;
        }

        int row = startRow;

        do
        {
            if (forward == true)
            {
                row++;
                if (row >= m_proxyModel->recordCount())	// Loop to start
                {
                    row = 0;
                }
            }
            else
            {
                row--;
                if (row < 0)	// Loop to end
                {
                    row = m_proxyModel->recordCount() - 1;
                }
            }

            int sourceRow =  m_proxyModel->sourceRow(row);

            const LogFileRecord& rec = m_model->record(sourceRow);

            if (rec.type == MessageType::Warning || rec.type == MessageType::Error)
            {
                return row;
            }
        }while (row != startRow);

        return -1;
    }

    int LogFileDialog::searchRecord(int startRow, const std::string& text) const
    {
        if (text.empty() == true)
        {
            return -1;
        }

        if (m_proxyModel->recordCount() == 0)
        {
            return -1;
        }

        int row = startRow;

        do
        {
            row++;
            if (row >= m_proxyModel->recordCount())	// Loop to start
            {
                row = 0;
            }

            int sourceRow =  m_proxyModel->sourceRow(row);

            const LogFileRecord& rec = m_model->record(sourceRow);

            if (rec.text.find(text) != std::string::npos)
            {
                return row;
            }
        }while (row != startRow);

        return -1;
    }

	void LogFileDialog::onDialogFinished(int result)
	{
		Q_UNUSED(result);
		m_logDialogs[m_log.logName()] = nullptr;
	}
	
	void LogFileDialog::onTypeComboIndexChanged(int index)
	{
		Q_UNUSED(index);

		int filterRecordTypeMask = MessageType::All;

		if (m_recordTypeCombo != nullptr)
		{
			int typeComboIndex = m_recordTypeCombo->currentIndex();
			if (typeComboIndex < 0)
			{
				assert(false);
				return;
			}

			filterRecordTypeMask = m_recordTypeCombo->itemData(typeComboIndex, Qt::UserRole).toInt();
		}

        m_proxyModel->setFilterRecordTypeMask(filterRecordTypeMask);

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel->rowCount()).arg(m_proxyModel->errorCount()).arg(m_proxyModel->warningCount()));
	}

	void LogFileDialog::onTimeFilter()
	{
        DialogTimeFilter d(m_proxyModel->filterTimeFrom(), m_proxyModel->filterTimeTo(), m_log.dateTimeFormat(), this);
		if (d.exec() == QDialog::Accepted)
		{
			qint64 filterTimeFrom = d.filterTimeFrom();
			qint64 filterTimeTo = d.filterTimeTo();

			if (filterTimeFrom != -1 && filterTimeTo != -1)
			{
				//m_timeFilterButton->setText(tr("Time Filter is on"));
				m_timeFilterButton->setStyleSheet("QPushButton { font-weight: bold; }");
			}
			else
			{
				//m_timeFilterButton->setText(tr("Time Filter is off"));
				m_timeFilterButton->setStyleSheet({});
			}

            m_proxyModel->setFilterTime(filterTimeFrom, filterTimeTo);

            m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel->rowCount()).arg(m_proxyModel->errorCount()).arg(m_proxyModel->warningCount()));

			return;
		}
	}

	void LogFileDialog::onFilter()
	{
        m_proxyModel->setFilterText(m_filterLineEdit->text());

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel->rowCount()).arg(m_proxyModel->errorCount()).arg(m_proxyModel->warningCount()));
	}

	void LogFileDialog::onFilterEdited(const QString& text)
	{
		m_search->setEnabled(text.isEmpty() == false);
        m_filter->setEnabled(text.isEmpty() == false);

        if (m_proxyModel->filterText().isEmpty() == false && text.isEmpty() == true)
        {
            onFilter();
        }
	}

	void LogFileDialog::onSearch()
	{
		// Turn off autoscroll
		//
		m_autoScroll->setChecked(false);

		// Find an index with issue
		//
		int row = 0;

		QModelIndexList selectedIndexes = m_table->selectionModel()->selectedIndexes();
		if (selectedIndexes.empty() == false)
		{
			row = selectedIndexes[0].row();
		}

		std::string findText = m_filterLineEdit->text().toLocal8Bit().toStdString();

		if (findText.empty() == true)
		{
			return;
		}

        int foundRow = searchRecord(row, findText);
		if (foundRow != -1)
		{
            m_table->selectionModel()->select(m_proxyModel->index(foundRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

            m_table->setCurrentIndex(m_proxyModel->index(foundRow, 0));

            m_table->scrollTo(m_proxyModel->index(foundRow, 0), QAbstractItemView::EnsureVisible);
		}
		else
		{
			QMessageBox::warning(this, qAppName(), tr("Text was not found!"));
		}
	}

	void LogFileDialog::onAllSessionsClicked()
	{
        if (m_loadedFromFile == true)
        {
            return;
        }

        m_model->clear();

        m_log.load(m_allSessions->isChecked() == false);

		enableControls(false);
	}

    void LogFileDialog::onLoadChunkComplete()
    {
        bool modelWasEmpty = m_proxyModel->rowCount() == 0;

        std::vector<std::shared_ptr<LogFileChunk>> result;

        m_log.getLoadedChunks(&result);

        if (result.empty() == true)
        {
            return;
        }

        for (const auto& chunk : result)
        {
			m_model->appendChunk(chunk);
        }

        if (modelWasEmpty == true)
        {
            for (int i = 0; i < m_table->horizontalHeader()->count() - 1; i++)
            {
                m_table->resizeColumnToContents(i);
            }
        }

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel->rowCount()).arg(m_proxyModel->errorCount()).arg(m_proxyModel->warningCount()));

        if (m_autoScroll->isChecked() == true)
		{
			m_table->scrollToBottom();
        }
	}

    void LogFileDialog::onLoadComplete()
	{
        for (const auto& rec : m_arrivedRecords)
        {
			m_model->appendRecord(rec);
        }
        m_arrivedRecords.clear();

        //

        if (m_loadedFromFile == false)
        {
            m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_log.getCurrentFileName()));
        }
        else
        {
            setWindowTitle(m_loadedFileName);
            m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_loadedFileName));
        }

        enableControls(true);

		m_table->scrollToBottom();
	}

	void LogFileDialog::onRecordArrived(LogFileRecord record)
	{
		if (isVisible() == false)
		{
			return;
		}

		if (m_loadedFromFile == true)
		{
			return;
		}

        if (m_log.loadInProgress() == true)
        {
            // Log loading is in progress, write the record to an array and add later
            //
            m_arrivedRecords.push_back(record);
            return;
        }

        bool modelWasEmpty = m_proxyModel->rowCount() == 0;

        m_model->appendRecord(record);

        if (modelWasEmpty == true)
		{
            for (int i = 0; i < m_table->horizontalHeader()->count() - 1; i++)
            {
                m_table->resizeColumnToContents(i);
            }
        }

		if (m_log.noDiskLog() == true)
		{
			m_logPathLabel->setText(tr("Log file writing is disabled"));
		}
		else
		{
			m_logPathLabel->setText(tr("File: <a href=\"%1\">%1</a>").arg(m_log.getCurrentFileName()));
		}

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel->rowCount()).arg(m_proxyModel->errorCount()).arg(m_proxyModel->warningCount()));

		if (m_autoScroll->isChecked() == true)
		{
			m_table->scrollToBottom();
		}
	}

	void LogFileDialog::onCellCopyKeyPressed()
	{
		QModelIndexList selectedIndexes = m_table->selectionModel()->selectedIndexes();
		if (selectedIndexes.empty() == true)
		{
			return;
		}

		std::sort(selectedIndexes.begin(), selectedIndexes.end(), [](const QModelIndex& left, const QModelIndex& right) -> bool
		{
			if (left.row() == right.row())
			{
				return left.column() < right.column();
			}

			return left.row() < right.row();
		}
		);

		// Calculate the leftmost column
		//
		int firstColumn = selectedIndexes[0].column();

		for (const QModelIndex& mi : selectedIndexes)
		{
			if (mi.column() < firstColumn)
			{
				firstColumn = mi.column();
			}
		}

		// Build the result
		//
		QString result;

		int lastRow = selectedIndexes[0].row();

		int lastColumn = firstColumn;

		for (const QModelIndex& mi : selectedIndexes)
		{
			if (lastRow != mi.row())
			{
				// Switch to the next row
				//
				lastRow = mi.row();
				lastColumn = firstColumn;

				result += "\n";
			}

			// Fill tabs between columns
			//
			for (int c = lastColumn; c < mi.column(); c++)
			{
				result += "\t";
			}

			lastColumn = mi.column();

            result += m_proxyModel->data(mi, Qt::DisplayRole).toString().trimmed();
		}

		QClipboard* clipboard = QApplication::clipboard();
		clipboard->clear();
		clipboard->setText(result);

		return;
	}

	void LogFileDialog::turnOffAutoscroll()
	{
		if (m_autoScroll->isChecked() == true)
		{
			m_autoScroll->setChecked(false);
		}
	}

	void LogFileDialog::onLoad()
	{
		static QString path{"."};
		QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), path, "Log Files (*.log)");
		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		m_loadedFromFile = true;
        m_loadedFileName = fileName;

		m_autoScroll->setChecked(false);

        m_model->clear();

        m_log.loadFromFile(QDir::toNativeSeparators(fileName));

        enableControls(false);

		return;
	}

	void LogFileDialog::onClear()
	{
		m_model->clear();

        m_allSessions->setChecked(false);

        m_counterLabel->setText(tr("Total records: %1, Errors: %2, Warnings: %3").arg(m_proxyModel->rowCount()).arg(m_proxyModel->errorCount()).arg(m_proxyModel->warningCount()));

        if (m_loadedFromFile == true)
        {
            m_loadedFromFile = false;

            m_allSessions->setEnabled(true);

            setWindowTitle(tr("Log View - %1").arg(m_log.logName()));
        }
    }

	void LogFileDialog::onExport()
	{
		static QString path{"."};
        QString fileName = QFileDialog::getSaveFileName(this,
														tr("Save File"),
														path + QDir::separator() + "Untitled.log",
														tr("Log files (*.log)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		std::vector<LogFileRecord> exportRecords;

        int count = m_proxyModel->recordCount();
        exportRecords.reserve(count);

        for (int i = 0; i < count; i++)
        {
            int sourceRow = m_proxyModel->sourceRow(i);
			const LogFileRecord& rec = m_model->record(sourceRow);
            exportRecords.push_back(rec);
        }

		if (exportRecords.empty() == true)
		{
			QMessageBox::warning(this, qAppName(), tr("No data to export!"));
			return;
		}

		QFile data(fileName);
		if (data.open(QFile::WriteOnly | QFile::Truncate) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("File creation error!"));
			return;
		}

		QTextStream out(&data);

		QString dateTimeformat = m_log.dateTimeFormat();

		for (const LogFileRecord& rec : exportRecords)
		{
			out << LogFileRecord::toString(rec, m_log.sessionHashString(), dateTimeformat);
		}

		QMessageBox::information(this, qAppName(), tr("Export complete."));

		return;
	}

	void LogFileDialog::onPrevIssue()
	{
		searchIssue(false);
	}

	void LogFileDialog::onNextIssue()
	{
		searchIssue(true);
	}

	void LogFileDialog::onLinkActivated(const QString& link)
	{
#if defined(Q_OS_WIN)
		QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(link)});
#else
		QFileInfo info(link);
		QDesktopServices::openUrl(QUrl::fromLocalFile(info.isDir()? link : info.path()));
#endif
	}
}

#include "LogDialog.moc"