#pragma once

namespace Builder
{
	struct VduSymbolSubset
	{
		QString name;
		char16_t start = 0;
		char16_t finish = 0;
	};

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
		int getFontIndex(QString vduEquipmentId, QString fontName, int pixelSize, bool bold, bool italic, bool underlined) const; 

		int getFontCount(QString vduEquipmentId) const;

		void setUnicodeSubsets(const QString& vduEquipmentId, const std::vector<VduSymbolSubset>& vduSubsets);

		// Use this function to check the string if it contains unknown (not contained in m_symbolSubsets) Unicode symbols
		//
		bool checkStringForUnicodeSubsets(const QString& vduEquipmentId, const QString& str, QString& errorMsg) const;	

	private:
		std::map<QString, std::vector<VduSymbolSubset>> m_symbolSubsets;	// Key is VDU equipment Id, value is symbol subsets
		std::map<QString, std::vector<VduFontInfo>> m_fontsInfo;			// Key is VDU equipment Id, value is fonts info
	};

} // namespace Builder