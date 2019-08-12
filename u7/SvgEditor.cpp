#include "SvgEditor.h"
#include <QSvgRenderer>
#include "Settings.h"

SvgWidget::SvgWidget(QWidget* parent)
	:QWidget(parent),
	  m_svgRenderer(this)
{

}

const QString& SvgWidget::svgData() const
{
	return m_svgData;
}

void SvgWidget::setSvgData(const QString& data)
{
	m_svgData = data;
	m_svgRenderer.load(data.toUtf8());

	update();
}

bool SvgWidget::isValid() const
{
	return m_svgRenderer.isValid();
}

bool SvgWidget::stretch() const
{
	return m_stretch;
}

void SvgWidget::setStretch(bool value)
{
	m_stretch = value;
	update();
}

void SvgWidget::paintEvent(QPaintEvent* pe)
{
	Q_UNUSED(pe);

	QRect r (rect());
	r.adjust(0, 0, -1, -1);

	QPainter p(this);
	p.fillRect(r, Qt::white);
	p.drawRect(r);

	if (m_svgRenderer.isValid() == true)
	{
		const QSize widgetSize = r.size();
		QSize svgSize = m_svgRenderer.defaultSize();

		if (m_stretch == true)
		{
			double kx = (double)widgetSize.width() / svgSize.width();
			double ky = (double)widgetSize.height() / svgSize.height();

			int w = widgetSize.width();
			int h = static_cast<int>(svgSize.height() * kx);

			if (h > widgetSize.height())
			{
				w = static_cast<int>(svgSize.width() * ky);
				h = widgetSize.height();
			}

			svgSize.setWidth(w);
			svgSize.setHeight(h);
		}

		QRect rectangle((widgetSize.width() - svgSize.width()) / 2,
						(widgetSize.height() - svgSize.height()) / 2,
						svgSize.width(),
						svgSize.height());

		m_svgRenderer.render(&p, rectangle);
	}
	else
	{
		p.drawText(r, Qt::AlignCenter, tr("Not valid SVG data."));
	}
}

//
// SvgEditor
//

SvgEditor::SvgEditor(QWidget* parent):
	PropertyTextEditor(parent),
	m_parent(parent),
	m_svgWidget(this)
{
	if (m_parent == nullptr)
	{
		Q_ASSERT(m_parent);
		return;
	}

	setHasOkCancelButtons(false);

	// TextEditor
	//
	m_textEdit = new QsciScintilla();
	m_textEdit->setUtf8(true);

#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
#elif defined(Q_OS_MAC)
		QFont f = QFont("Courier");
		//f.setPixelSize(font().pixelSize());
#else
		QFont f = QFont("Courier");
		//f.setPixelSize(font().pixelSize());
#endif

	m_lexerXml.setDefaultFont(f);
	m_textEdit->setLexer(&m_lexerXml);

	m_textEdit->setFont(f);
	m_textEdit->setTabWidth(4);
	m_textEdit->setAutoIndent(true);

	connect(m_textEdit, &QsciScintilla::textChanged, this, &SvgEditor::onTextChanged);

	// Top Layout
	//
	m_topSplitter = new QSplitter();
	m_topSplitter->addWidget(m_textEdit);
	m_topSplitter->addWidget(&m_svgWidget);
	m_topSplitter->setContentsMargins(0, 0, 0, 0);

	m_svgWidget.setStretch(theSettings.m_svgEditorStretch);


	// Buttons
	//
	QHBoxLayout* buttonLayout = new QHBoxLayout();

	QCheckBox* checkStretch = new QCheckBox(tr("Stretch"));
	checkStretch->setChecked(m_svgWidget.stretch());
	connect(checkStretch, &QCheckBox::clicked, this, &SvgEditor::onStretchCheckClicked);
	buttonLayout->addWidget(checkStretch);

	buttonLayout->addStretch();

	QPushButton* okButton = new QPushButton(tr("OK"));
	okButton->setDefault(true);
	connect(okButton, &QPushButton::clicked, this, &SvgEditor::onOkClicked);
	buttonLayout->addWidget(okButton);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, &QPushButton::clicked, this, &SvgEditor::onCancelClicked);
	buttonLayout->addWidget(cancelButton);

	// Main Layout
	//
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(m_topSplitter);
	mainLayout->addLayout(buttonLayout);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_topSplitter->restoreState(theSettings.m_svgEditorSplitterState);

	return;
}

SvgEditor::~SvgEditor()
{
	theSettings.m_svgEditorSplitterState = m_topSplitter->saveState();
}

void SvgEditor::setText(const QString& text)
{
	m_textEdit->blockSignals(true);
	m_textEdit->setText(text);
	m_textEdit->blockSignals(false);

	m_svgWidget.setSvgData(text);

	return;
}

QString SvgEditor::text()
{
	return m_svgWidget.svgData();
}

void SvgEditor::setReadOnly(bool value)
{
	m_textEdit->setReadOnly(value);
}

void SvgEditor::onTextChanged()
{
	m_svgWidget.setSvgData(m_textEdit->text());

	//

	textChanged();
	return;
}

void SvgEditor::onOkClicked()
{
	if (m_svgWidget.isValid() == false)
	{
		int result = QMessageBox::warning(m_parent, qAppName(), tr("SVG data is invalid. Do you want to save it anyway?"),
										   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (result == QMessageBox::Cancel)
		{
			return;
		}

		if (result == QMessageBox::No)
		{
			onCancelClicked();
			return;
		}
	}

	okButtonPressed();
	return;
}

void SvgEditor::onCancelClicked()
{
	cancelButtonPressed();
	return;
}

void SvgEditor::onStretchCheckClicked(bool checked)
{
	theSettings.m_svgEditorStretch = checked;
	m_svgWidget.setStretch(checked);
}
