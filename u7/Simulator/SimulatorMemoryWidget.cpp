#include "SimulatorMemoryWidget.h"

SimulatorMemoryWidget::SimulatorMemoryWidget(const Sim::Ram ram, QWidget* parent) :
	QWidget(parent),
	m_ram(ram)
{
	m_ramAreaCombo = new QComboBox;
	m_ramAreaCombo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

	m_areaInfoLabel = new QLabel;

	m_memoryWidget = new MemoryView;
	m_memoryWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_quickWatchLabel = new QLabel;
	m_quickWatchLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	m_quickWatchLabel->setFont(fixedFont);
	m_quickWatchLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	QGridLayout* layout = new QGridLayout;
	layout->setMargin(4);
	layout->setSpacing(4);

	layout->addWidget(m_ramAreaCombo, 0, 0, 1, 1);
	layout->addWidget(m_areaInfoLabel, 0, 1, 1, 1);

	m_splitter = new QSplitter;
	m_splitter->addWidget(m_memoryWidget);
	m_splitter->addWidget(m_quickWatchLabel);
	m_splitter->setCollapsible(0, false);
	m_splitter->setCollapsible(1, true);
	m_splitter->setStretchFactor(0, 2);

	layout->addWidget(m_splitter, 1, 0, 2, 2);

	setLayout(layout);

	// --
	//
	connect(m_ramAreaCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimulatorMemoryWidget::currentAreaChanged);

	// --
	//
	updateAreaCombo();
	updateQuickWatch();

	return;
}

SimulatorMemoryWidget::~SimulatorMemoryWidget()
{
}

void SimulatorMemoryWidget::updateAreaCombo()
{
	assert(m_ramAreaCombo);
	m_ramAreaCombo->clear();

	int lastIndex = m_ramAreaCombo->currentIndex();

	std::vector<Sim::RamAreaInfo> areas = m_ram.memoryAreasInfo();

	for (const Sim::RamAreaInfo& ma : areas)
	{
		m_ramAreaCombo->addItem(ma.name());
	}

	if (lastIndex == -1)
	{
		lastIndex = 0;
	}

	m_ramAreaCombo->setCurrentIndex(lastIndex);
	return;
}

void SimulatorMemoryWidget::updateAreaInfo()
{
	assert(m_areaInfoLabel);

	const auto& currentAreaInfo = m_memoryWidget->areaInfo();

	QString access;
	switch (currentAreaInfo.access())
	{
	case Sim::RamAccess::Read:		access = "Read";		break;
	case Sim::RamAccess::Write:		access = "Write";		break;
	case Sim::RamAccess::ReadWrite:	access = "ReadWrite";	break;
	default:
		assert(false);
	}

	QString text = QString("Access: %1, Offset: 0x%2, Size: 0x%3")
					.arg(access)
					.arg(currentAreaInfo.offset(), 4, 16, QChar('0'))
					.arg(currentAreaInfo.size(), 4, 16, QChar('0'));

	m_areaInfoLabel->setText(text);

	return;
}

void SimulatorMemoryWidget::updateQuickWatch()
{
	assert(m_quickWatchLabel);

	QString text = "WORD:  0x0000\n"
				   "       0\n"
				   "DWORD: 0x00000000\n"
				   "       0\n"
				   "SI:    0\n"
				   "FP:    0\n"
				   "DBL:   0\n";

	m_quickWatchLabel->setText(text);

	return;
}

void SimulatorMemoryWidget::currentAreaChanged(int index)
{
	m_memoryWidget->setAreaInfo(m_ram.memoryAreaInfo(index));
	updateAreaInfo();
}

MemoryView::MemoryView()
{
	setBackgroundRole(QPalette::Dark);
	setAutoFillBackground(true);

	m_scroll = new QScrollBar(Qt::Vertical);
	m_hexView = new MemoryHexView(&m_areaInfo, m_scroll);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->setSpacing(1);
	layout->setMargin(1);

	layout->addWidget(m_hexView);
	layout->addWidget(m_scroll);

	setLayout(layout);
	return;
}

void MemoryView::setAreaInfo(const Sim::RamAreaInfo& areaInfo)
{
	m_areaInfo = areaInfo;
	setScrollrange();
	return;
}

const Sim::RamAreaInfo& MemoryView::areaInfo() const
{
	return m_areaInfo;
}

void MemoryView::setScrollrange()
{
	assert(m_scroll);

	int wordCount = m_hexView->wordsInLine();
	int lines = m_hexView->lineCount();

	int areaOffset = m_areaInfo.offset();
	int areaSize = m_areaInfo.size();

	m_scroll->setMinimum((areaOffset / wordCount) * wordCount);
	bool allValuesOnSingleScreen = m_scroll->minimum() + lines * wordCount > (areaOffset + areaSize);

	if (allValuesOnSingleScreen == true)
	{
		m_scroll->setMaximum(m_scroll->minimum());
	}
	else
	{
		m_scroll->setMaximum(areaOffset + areaSize - (lines - 1) * wordCount);
		m_scroll->setSingleStep(wordCount);
		m_scroll->setPageStep(lines * wordCount);
	}


	//m_scroll->setValue(qBound(m_scroll->minimum(), m_scroll->value(), m_scroll->maximum()));

	return;
}

void MemoryView::showEvent(QShowEvent*)
{
	// On show widget geometry will be recalculated and new steps must be applied to scroll
	//
	setScrollrange();
	return;
}

void MemoryView::resizeEvent(QResizeEvent*)
{
	setScrollrange();
	return;
}

MemoryHexView::MemoryHexView(Sim::RamAreaInfo* memoryArea, QScrollBar* scroll) :
	m_memoryArea(memoryArea),
	m_scroll(scroll)
{
	assert(memoryArea);
	assert(scroll);

	setBackgroundRole(QPalette::Light);
	setAutoFillBackground(true);

	setMinimumWidth(200);

	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	setFont(fixedFont);

	connect(m_scroll, &QScrollBar::valueChanged, this, [this](int){MemoryHexView::update();});

	return;
}

int MemoryHexView::wordsInLine() const
{
	int wigteWidth = rect().width();
	QFontMetrics fm = fontMetrics();

	int result = 1;
	for (int i = 0; i < 5; i++)
	{
		QString str = " 00000000 |";
		for (int f = 0; f < result * 2; f++)
		{
			str.append(" 0000");
		};

		if (fm.width(str) > wigteWidth)
		{
			return result;
		}

		result *= 2;
	}

	return 16;
}

int MemoryHexView::lineCount() const
{
	int widgetHeight = rect().height();

	QFontMetrics fm = fontMetrics();
	int lines = widgetHeight / fm.boundingRect("|").height();

	return lines;
}

void MemoryHexView::wheelEvent(QWheelEvent* event)
{
	QPoint numDegrees = event->angleDelta() / 8;

	m_scroll->setValue(m_scroll->value() - numDegrees.y() * 3);
	update();

	event->accept();
	return;
}

void MemoryHexView::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	int wordWidth = wordsInLine();
	int lines = lineCount();

	int address = (static_cast<int>(m_scroll->value()) / wordWidth) * wordWidth;

	int startAddress = m_memoryArea->offset();
	int maxAdderss = m_memoryArea->offset() + m_memoryArea->size();

	QFontMetrics fm = fontMetrics();

	double lineHeight = fm.boundingRect(" 00000000 | ").height();
	double addressWidth = fm.boundingRect(" 00000000 | ").width();
	double valueWidth = fm.boundingRect(" 0000").width();

	quint16 vvvvv = 0;

	for (int i = 0; i < lines + 1; i++)
	{
		QString addressStr = QString(" %1 |").arg(address, 8, 16, QChar('0'));

		double y = static_cast<double>(i) * lineHeight + lineHeight;
		double x = 0;

		painter.drawText(QPointF(x, y), addressStr);

		for (int f = 0; f < wordWidth; f++)
		{
			address ++;
			quint16 value = vvvvv++;

			QString valueStr;
			if (address < startAddress || address > maxAdderss)
			{
				valueStr = QString(" ----");
			}
			else
			{
				valueStr = QString(" %1").arg(value, 4, 16, QChar('0'));
			}

			x = addressWidth + valueWidth * f;

			painter.drawText(QPointF(x, y), valueStr);
		}
	}

	return;
}
