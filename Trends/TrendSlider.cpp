#include "TrendSlider.h"
#include <QPushButton>
#include <QHBoxLayout>

TrendSlider::TrendSlider()
{
	m_lineLeftButton = new QPushButton(QChar(0x25C4), this);
	m_lineRightButton = new QPushButton(QChar(0x25BA), this);
	m_railSubcontrol = new TrendSliderRailSubcontrol(this);

	m_lineLeftButton->setAutoRepeat(true);
	m_lineRightButton->setAutoRepeat(true);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->setMargin(1);
	layout->setSpacing(1);

	layout->addWidget(m_lineLeftButton);
	layout->addWidget(m_railSubcontrol);
	layout->addWidget(m_lineRightButton);

	setLayout(layout);

	m_lineLeftButton->setMaximumSize(QSize(m_lineLeftButton->height(), m_lineLeftButton->height()));
	m_lineRightButton->setMaximumSize(QSize(m_lineRightButton->height(), m_lineRightButton->height()));
	m_railSubcontrol->setMaximumHeight(m_lineRightButton->height() - 2);

	connect(m_lineLeftButton, &QPushButton::clicked, this, &TrendSlider::lineLeftClicked);
	connect(m_lineRightButton, &QPushButton::clicked, this, &TrendSlider::lineRightClicked);

	return;
}

void TrendSlider::paintEvent(QPaintEvent* /*event*/)
{
//	// This calls the base class's paint calls
//	//
//	QScrollBar::paintEvent(event);

//	// The following is painted on top of it
//	//
//	QStyleOptionSlider opt;
//	initStyleOption(&opt);
//	opt.dialWrapping = true;
//	opt.subControls = QStyle::SC_ScrollBarSlider;

//	QPainter p(this);
//	p.setPen(QPen(Qt::black, 0));

//	QRect sliderRect = style()->proxy()->subControlRect(QStyle::ComplexControl::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, this);

//	QString text = "XX:XX:XX 17.01.2017";
//	p.drawText(sliderRect, Qt::AlignCenter, text, &sliderRect);

	return;
}

void TrendSlider::lineLeftClicked()
{
	qint64 prevValue = m_value;
	m_value -= m_singleStep;
	m_value = std::max(m_min, m_value);

	if (m_value != prevValue)
	{
		emit paramsChanged(m_value, m_min, m_max, m_singleStep, m_pageStep);

		TimeStamp ts(m_value);
		emit valueChanged(ts);
	}

	return;
}

void TrendSlider::lineRightClicked()
{
	qint64 prevValue = m_value;
	m_value += m_singleStep;
	m_value = std::min(m_max, m_value);

	if (m_value != prevValue)
	{
		emit paramsChanged(m_value, m_min, m_max, m_singleStep, m_pageStep);

		TimeStamp ts(m_value);
		emit valueChanged(ts);
	}

	return;
}

TimeStamp TrendSlider::value() const
{
	return TimeStamp(m_value);
}

void TrendSlider::setValue(const TimeStamp& value)
{
	if (value.timeStamp != m_value)
	{
		emit valueChanged(value);
	}

	m_value = value.timeStamp;
	emit paramsChanged(m_value, m_min, m_max, m_singleStep, m_pageStep);
	return;
}

TimeStamp TrendSlider::max() const
{
	return TimeStamp(m_max);
}

void TrendSlider::setMax(const TimeStamp& value)
{
	m_max = value.timeStamp;
	emit paramsChanged(m_value, m_min, m_max, m_singleStep, m_pageStep);
	return;
}

TimeStamp TrendSlider::min() const
{
	return TimeStamp(m_min);
}

void TrendSlider::setMin(const TimeStamp& value)
{
	m_min = value.timeStamp;
	emit paramsChanged(m_value, m_min, m_max, m_singleStep, m_pageStep);
	return;
}

qint64 TrendSlider::signleStep() const
{
	return m_singleStep;
}

void TrendSlider::setSingleStep(qint64 ms)
{
	m_singleStep = ms;
	emit paramsChanged(m_value, m_min, m_max, m_singleStep, m_pageStep);
	return;
}

qint64 TrendSlider::pageStep() const
{
	return m_pageStep;
}

void TrendSlider::setPageStep(qint64 ms)
{
	m_pageStep = ms;
	emit paramsChanged(m_value, m_min, m_max, m_singleStep, m_pageStep);
	return;
}

TrendSliderRailSubcontrol::TrendSliderRailSubcontrol(TrendSlider* threndSlider) :
	m_trendSlider(threndSlider)
{
	assert(m_trendSlider);

	connect(m_trendSlider, &TrendSlider::paramsChanged, this, &TrendSliderRailSubcontrol::paramsChanged);

	setMouseTracking(true);
	startTimer(100);

	return;
}

void TrendSliderRailSubcontrol::timerEvent(QTimerEvent*)
{
	QRect sliderRc = sliderRect();
	QPoint mousePos = mapFromGlobal(QCursor::pos());

	if (m_lastDrawHover != sliderRc.contains(mousePos))
	{
		update();
	}

	return;
}

void TrendSliderRailSubcontrol::mouseMoveEvent(QMouseEvent* /*event*/)
{
	QRect sliderRc = sliderRect();
	QPoint mousePos = mapFromGlobal(QCursor::pos());

	if (m_lastDrawHover != sliderRc.contains(mousePos))
	{
		update();
	}

	return;
}

void TrendSliderRailSubcontrol::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	QRect sliderRc = sliderRect();

	// Draw back area
	//
	p.fillRect(this->rect(), QColor(Qt::white));

	// Draw min text
	//
	p.setPen(QPen(Qt::darkGray, 0));

	QString minTimeText = TimeStamp(m_min).toDateTime().toString("hh:mm:ss [dd.MM.yyyy]");

	QRect minTextRect = sliderRc;
	minTextRect.moveLeft(0);

	p.drawText(minTextRect, Qt::AlignLeft | Qt::AlignVCenter, minTimeText);

	// Draw max text
	//
	QString maxTimeText = TimeStamp(m_max).toDateTime().toString("hh:mm:ss [dd.MM.yyyy]");

	QRect maxTextRect = sliderRc;
	maxTextRect.moveRight(this->rect().width());

	p.drawText(maxTextRect, Qt::AlignRight | Qt::AlignVCenter, maxTimeText);

	// Draw slider back
	//
	p.fillRect(sliderRc, qRgb(0xE0, 0xE0, 0xE0));

	QPoint mousePos = mapFromGlobal(QCursor::pos());
	if (sliderRc.contains(mousePos) == true)
	{
		p.setPen(Qt::lightGray);
		p.drawRect(sliderRc.left(), sliderRc.top(), sliderRc.width() - 1, sliderRc.height() - 1);

		m_lastDrawHover = true;
	}
	else
	{
		m_lastDrawHover = false;
	}
	// Draw text
	//
	p.setPen(QPen(Qt::black, 0));

	QDateTime dateTime = TimeStamp(m_value).toDateTime();
	QString text = dateTime.toString("hh:mm:ss [dd.MM.yyyy]");
	p.drawText(sliderRc, Qt::AlignCenter, text, &sliderRc);

	if (m_sliderWidth < static_cast<int>(sliderRc.width() * 1.2))
	{
		m_sliderWidth = static_cast<int>(sliderRc.width() * 1.2);
		update();
	}


	return;
}

QRect TrendSliderRailSubcontrol::sliderRect() const
{
	int width = this->width();					// [<-------------------width------------------->]
	int sliderWidth = m_sliderWidth;			// [------------------[<sliderWidth>]------------]
	int moveAreaWidth = width - sliderWidth;	// [<------------moveArea---------->[sliderWidth]]

	int sliderPos = 0;
	if (m_value < m_min ||
		m_max == m_min)
	{
		sliderPos = 0;
	}
	else
	{
		if (m_value > m_max)
		{
			sliderPos = moveAreaWidth;
		}
		else
		{
			sliderPos = static_cast<int>(
						static_cast<double>(m_value - m_min) /
						static_cast<double>(m_max - m_min) *
						static_cast<double>(moveAreaWidth));
		}
	}

	return QRect(sliderPos, 0, sliderWidth, this->height());
}

void TrendSliderRailSubcontrol::paramsChanged(qint64 value, qint64 min, qint64 max, qint64 singleStep, qint64 pageStep)
{
	m_value = value;
	m_max = max;
	m_min = min;
	m_singleStep = singleStep;
	m_pageStep = pageStep;

	update();
}
