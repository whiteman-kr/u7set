#include "VDUFontGenerator.h"

void PreviewWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	int width = size().width() - 1;
	int height = size().height() - 1;

	painter.fillRect(0, 0, width, height, QColor(220, 220, 220));

	painter.setFont(m_font);
	QFontMetrics fm(m_font);
	
	painter.drawText(0, fm.height(), "Example");
}

const QFont& PreviewWidget::font()
{
	return m_font;
}

void PreviewWidget::refresh(const QFont& font)
{
	m_font = font;
	update();
}

Symbol::Symbol(quint16 width, quint16 height, char c, const QFontMetrics& fm, const QFont& font) :
	m_header{static_cast<quint32>(c), width, height, 0},
	m_image(QSize(width, height), QImage::Format_Grayscale8)
{
	QPainter painter(&m_image);
	painter.setFont(font);
	painter.fillRect(0, 0, width, height, Qt::white);
	painter.setPen(Qt::black);
	painter.drawText(0, fm.ascent(), QChar(c));
	bool bEnd = painter.end();

	// Render to m_data
	//
	m_data.resize(imageSize());

	const char* imageData = reinterpret_cast<const char*>(m_image.bits());
	char* data = m_data.data();

	for (int i = 0; i < m_image.height(); i++)
	{
		QString s;
		for (int j = 0; j < m_image.bytesPerLine(); j++)
		{
			if (j < m_image.width())
			{
				*data++ = ((0xff - *imageData) & 0xc0);
			}
			imageData++;
		}
	}
}

bool Symbol::saveToBmp(const QString& file) const
{
	return m_image.save(file);
}

bool Symbol::saveToVdu(const QString& file) const
{
	QFile fb(file);
	if (fb.open(QFile::WriteOnly) == false)
	{
		return false;
	}

	fb.write(m_data);
	return true;
}

bool Symbol::saveToVdut(const QString& file) const
{
	QFile ft(file);
	if (ft.open(QFile::WriteOnly) == false)
	{
		return false;
	}

	const char* data = m_data.data();

	for (int i = 0; i < m_image.height(); i++)
	{
		QString s;
		for (int j = 0; j < m_image.width(); j++)
		{
			QString c = QString::number((unsigned char)(*data++), 16).rightJustified(2, '0');
			s += c + " ";
		}
		ft.write(s.toLocal8Bit());
		ft.write("\n");
	}
	return true;
}

const QByteArray& Symbol::data() const
{
	return m_data;
}

const QImage& Symbol::image() const
{
	return m_image;
}

const SymbolHeader& Symbol::header() const
{
	return m_header;
}

int Symbol::imageSize() const
{
	int result = m_image.height() * m_image.width();	// NOT sizeInBytes()!
	return result;
}

void Symbol::setHeaderOffset(quint32 offset)
{
	m_header.offset = offset;
}
	
VduFontGenerator::VduFontGenerator() :
	QDialog(),
	m_font("Arial")
{
	setWindowTitle(tr("VDU Font Generator"));

	m_font.setPixelSize(24);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QGridLayout* grid = new QGridLayout();
	mainLayout->addLayout(grid);

	int row = 0;

	grid->addWidget(new QLabel(tr("Font Name:")), row, 0);
	m_fontNameCombo = new QComboBox(this);
	m_fontNameCombo->addItems(QFontDatabase().families());
	m_fontNameCombo->setEditable(false);
	int index = m_fontNameCombo->findText(m_font.family());
	if (index != -1)
	{
		m_fontNameCombo->setCurrentIndex(index);
	}
	connect(m_fontNameCombo, &QComboBox::currentIndexChanged, [this](int index)
			{
				QString family = m_fontNameCombo->currentText();
				m_font.setFamily(family);
				emit refresh(m_font);
		});
	grid->addWidget(m_fontNameCombo, row, 1);

	row++;

	grid->addWidget(new QLabel(tr("Font Size:")), row, 0);
	m_fontSizeCombo = new QComboBox(this);
	for (int i = 8; i <= 72; i++)
	{
		m_fontSizeCombo->addItem(QString::number(i));
	}
	index = m_fontSizeCombo->findText(QString::number(m_font.pixelSize()));
	if (index != -1)
	{
		m_fontSizeCombo->setCurrentIndex(index);
	}
	
	connect(m_fontSizeCombo, &QComboBox::currentIndexChanged, [this](int index)
			{
				bool ok = false;
				int number = m_fontSizeCombo->currentText().toInt(&ok);
				if (ok == true)
				{
					m_font.setPixelSize(number);
					emit refresh(m_font);
				}
		});
	grid->addWidget(m_fontSizeCombo, row, 1);

	row++;

	grid->addWidget(new QLabel(tr("Font Bold:")), row, 0);
	m_fontBoldCheck = new QCheckBox(this);
	connect(m_fontBoldCheck, &QCheckBox::clicked, [this](bool value) {
				m_font.setBold(value);
				emit refresh(m_font);
			});
	grid->addWidget(m_fontBoldCheck, row, 1);

	row++;

	grid->addWidget(new QLabel(tr("Font Italic:")), row, 0);
	m_fontItalicCheck = new QCheckBox(this);
	connect(m_fontItalicCheck, &QCheckBox::clicked, [this](bool value) {
				m_font.setItalic(value);
				emit refresh(m_font);
			});
	grid->addWidget(m_fontItalicCheck, row, 1);

	row++;

	grid->addWidget(new QLabel(tr("Font Underlined:")), row, 0);
	m_fontUnderlinedCheck = new QCheckBox(this);
	connect(m_fontUnderlinedCheck, &QCheckBox::clicked, [this](bool value) {
				m_font.setUnderline(value);
				emit refresh(m_font);
			});
	grid->addWidget(m_fontUnderlinedCheck, row, 1);

	row++;

	grid->addWidget(new QLabel(tr("Generate Demo Files:")), row, 0);
	m_generateDemoFiles = new QCheckBox(this);
	grid->addWidget(m_generateDemoFiles, row, 1);

	m_previewWidget = new PreviewWidget(m_font);
	m_previewWidget->setMinimumSize(640, 480);
	connect(this, &VduFontGenerator::refresh, m_previewWidget, &PreviewWidget::refresh);
	mainLayout->addWidget(m_previewWidget);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	mainLayout->addLayout(buttonsLayout);

	buttonsLayout->addStretch();

	QPushButton* b = new QPushButton(tr("Generate"));
	connect(b, &QPushButton::clicked, this, &VduFontGenerator::onGenerateClicked);
	buttonsLayout->addWidget(b);

	setLayout(mainLayout);
}


VduFontGenerator::~VduFontGenerator()
{
}

void VduFontGenerator::onGenerateClicked()
{

	const QString genericExtension = "vdf";
	const QString symbolExtension = "vds";
	const QString textSymbolExtension = "vdst";

	static QString lastDir{"."};

	static QString lastFile;
	if (lastFile.isEmpty() == true)
	{
		lastFile = tr("%1\\%2_%3.%4").arg(lastDir).arg(m_font.family()).arg(m_font.pixelSize()).arg(genericExtension);
	} 
	
	if (m_generateDemoFiles->isChecked() == true)
	{
		QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			lastDir,
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
		if (dir.isEmpty() == true)
		{
			return;
		}

		lastDir = dir;
		lastFile = tr("%1\\%2_%3.%4").arg(lastDir).arg(m_font.family()).arg(m_font.pixelSize()).arg(genericExtension);
	}
	else
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Choose file"), lastFile);
		if (fileName.isEmpty() == true)
		{
			return;
		}

		lastDir = QFileInfo(fileName).path();
		lastFile = fileName;
	}

	QFontMetrics fm(m_font);
	
	std::vector<Symbol> symbols;
	const int start = 32;
	const int end = 127;
	
	symbols.reserve(end - start);

	for (int i = start; i <= end; i++)
	{
		char c = (char)i;

		quint16 height = fm.height();
		quint16 width = fm.horizontalAdvance(c);

		Symbol s(width, height, c, fm, m_font);

		if (m_generateDemoFiles->isChecked() == true)
		{
			// Save to files
			//
			QString fileNameSuffix = QString::number(i).rightJustified(3, '0');
			
			QString f = tr("%1\\%2.bmp").arg(lastDir).arg(fileNameSuffix);
			if (s.saveToBmp(f) == false)
			{
				QMessageBox::critical(this, qAppName(), tr("Failed to save file: %1!").arg(f));
				return;
			}

			f = tr("%1\\%2.%3").arg(lastDir).arg(fileNameSuffix).arg(textSymbolExtension);
			if (s.saveToVdut(f) == false)
			{
				QMessageBox::critical(this, qAppName(), tr("Failed to save file: %1!").arg(f));
				return;
			}

			f = tr("%1\\%2.%3").arg(lastDir).arg(fileNameSuffix).arg(symbolExtension);
			if (s.saveToVdu(f) == false)
			{
				QMessageBox::critical(this, qAppName(), tr("Failed to save file: %1!").arg(f));
				return;
			}
		}

		symbols.push_back(s);
	}
	
	quint32 offset = symbols.size() * sizeof(SymbolHeader);

	for (Symbol& s : symbols)
	{
		s.setHeaderOffset(offset);
		offset += s.imageSize();
	}

	{

		QFile fAll(lastFile);
		if (fAll.open(QFile::WriteOnly) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to save file: %1!").arg(lastFile));
			return;
		}

		for (Symbol& s : symbols)
		{
			fAll.write(reinterpret_cast<const char*>(&s.header()), sizeof(SymbolHeader));
		}

		for (Symbol& s : symbols)
		{
			const QByteArray& data = s.data();
			fAll.write(data);
		}
	}
}