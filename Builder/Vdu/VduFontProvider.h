#pragma once

namespace Builder
{
	struct VduFontInfo
	{
		QString family;
		int pixelSize = 12;
		bool bold = false;
		bool italic = false;
		bool underlined = false;


		bool load(QXmlStreamReader& reader, QString* errorMessage);
	};

	class VduFontProvider
	{
	public:
		VduFontProvider() = default;

	public:
		void setFontsInfo(const QString& equipmentId, std::vector<VduFontInfo>& info);
		int getFontIndex(QString vduEquipmnentId, QString fontName, int pixelSize, bool bold, bool italic, bool underlined) const; 

	private:
		std::map<QString, std::vector<VduFontInfo>> m_fontsInfo;
	};

} // namespace Builder