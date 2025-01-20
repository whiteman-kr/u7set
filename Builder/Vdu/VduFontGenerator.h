#pragma once

namespace Builder
{
#pragma pack(1)
	struct VduSymbolHeader
	{
		quint32 code;
		quint16 width;
		quint16 height;
		quint32 offset;
	};
#pragma pack()

	struct VduSymbol
	{
	public:
		VduSymbol(quint16 width, quint16 height, const QChar& c, const QFontMetrics& fm, const QFont& font);

		void saveToBmp(QByteArray& out) const;
		void saveToVdut(QByteArray& out) const;
		const QByteArray& data() const;

		const QImage& image() const;
		const VduSymbolHeader& header() const;
		int imageSize() const;

		void setHeaderOffset(quint32 offset);

	private:
		VduSymbolHeader m_header;
		QImage m_image;
		QByteArray m_data;
	};

	class IssueLogger;
	class Context;
	struct VduFontInfo;
	struct VduSymbolSubset;

	class VduFontGenerator
	{
	public:
		VduFontGenerator() = delete;

	public:
		static bool generationVduFonts(Context& context);
		static bool generateVduFont(const VduFontInfo& fontInfo,
									const std::vector<VduSymbolSubset>& subsets,
									const QString& dir,
									bool generateDebugFiles,
									Context& context);
	};

} // namespace Builder