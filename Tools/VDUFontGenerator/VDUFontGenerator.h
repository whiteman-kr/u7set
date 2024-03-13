#pragma once

class PreviewWidget : public QWidget
{
	Q_OBJECT

public:
	PreviewWidget(const QFont& font) :
		m_font(font)
	{
	}

private:
	void paintEvent(QPaintEvent* event) override;

	const QFont& font();

public slots:
	void refresh(const QFont& font);

private:
	QFont m_font;

};

#pragma pack(1)

struct SymbolHeader
{
	quint32 code;
	quint16 width;
	quint16 height;
	quint32 offset;
};
#pragma pack()

struct Symbol
{
public:
	Symbol(quint16 width, quint16 height, char c, const QFontMetrics& fm, const QFont& font);

	[[nodiscard]] bool saveToBmp(const QString& file) const;
	[[nodiscard]] bool saveToVdu(const QString& file) const;
	[[nodiscard]] bool saveToVdut(const QString& file) const;
	const QByteArray& data() const;

	const QImage& image() const;
	const SymbolHeader& header() const;
	int imageSize() const;

	void setHeaderOffset(quint32 offset);

private:
	SymbolHeader m_header;
	QImage m_image;
	QByteArray m_data;

};


class VduFontGenerator : public QDialog
{
	Q_OBJECT
public:
	VduFontGenerator();
	virtual ~VduFontGenerator();

signals:
	void refresh(const QFont& font);


private slots:
	void onGenerateClicked();


private:
	QComboBox* m_fontNameCombo = nullptr;
	QComboBox* m_fontSizeCombo = nullptr;
	QCheckBox* m_fontBoldCheck = nullptr;
	QCheckBox* m_fontItalicCheck = nullptr;
	QCheckBox* m_fontUnderlinedCheck = nullptr;
	QCheckBox* m_generateDemoFiles = nullptr;
	PreviewWidget* m_previewWidget = nullptr;
	QFont m_font;

};
