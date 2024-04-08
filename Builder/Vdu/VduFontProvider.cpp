#include "VduFontProvider.h"


namespace Builder
{
	bool VduFontInfo::load(QXmlStreamReader& reader, QString* errorMessage) 
	{
		static const QString attrFamily = "Family";
		static const QString attrPixelSize = "PixelSize";
		static const QString attrBold = "Bold";
		static const QString attrItalic = "Italic";
		static const QString attrUnderlined = "Underlined";

		if (reader.attributes().hasAttribute(attrFamily))
		{
			family = reader.attributes().value(attrFamily).toString();
		}
		else 
		{
			*errorMessage = QObject::tr("VduFontInfo: %1 attribute is missing.").arg(attrFamily);
			return false;
		}

		if (reader.attributes().hasAttribute(attrPixelSize))
		{
			pixelSize = reader.attributes().value(attrPixelSize).toInt();
		}
		else 
		{
			*errorMessage = QObject::tr("VduFontInfo: %1 attribute is missing.").arg(attrPixelSize);
			return false;
		}

		if (reader.attributes().hasAttribute(attrBold))
		{
			bold = reader.attributes().value(attrBold).toString().compare("true", Qt::CaseInsensitive) == 0;
		}
		else 
		{
			*errorMessage = QObject::tr("VduFontInfo: %1 attribute is missing.").arg(attrBold);
			return false;
		}

		if (reader.attributes().hasAttribute(attrItalic))
		{
			italic = reader.attributes().value(attrItalic).toString().compare("true", Qt::CaseInsensitive) == 0;
		}
		else 
		{
			*errorMessage = QObject::tr("VduFontInfo: %1 attribute is missing.").arg(attrItalic);
			return false;
		}

		if (reader.attributes().hasAttribute(attrUnderlined))
		{
			underlined = reader.attributes().value(attrUnderlined).toString().compare("true", Qt::CaseInsensitive) == 0;
		}
		else 
		{
			*errorMessage = QObject::tr("VduFontInfo: %1 attribute is missing.").arg(attrUnderlined);
			return false;
		}
		return true;
	}

	void VduFontProvider::setFontsInfo(const QString& equipmentId, std::vector<VduFontInfo>& info) 
	{
		m_fontsInfo[equipmentId] = info;
	}

	int VduFontProvider::getFontIndex(QString vduEquipmnentId, QString fontName, int pixelSize, bool bold, bool italic, bool underlined) const 
	{
		auto it = m_fontsInfo.find(vduEquipmnentId);
		if (it == m_fontsInfo.end()) 
		{
			return -1;
		}

		const std::vector<VduFontInfo>& info = it->second;

		auto foundIt = std::find_if(info.cbegin(),
									info.cend(),
									[fontName, pixelSize, bold, italic, underlined](const VduFontInfo& fi)
									{
										return fi.family == fontName && fi.pixelSize == pixelSize && fi.bold == bold &&
												   fi.italic == italic && fi.underlined == underlined;
									});

		if (foundIt == info.end())
		{
			return -1;
		}

		return std::distance(info.begin(), foundIt);
	}
	
} // namespace Builder