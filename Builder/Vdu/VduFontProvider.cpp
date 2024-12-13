#include "VduFontProvider.h"
#include "VduUnicodeSubsets.h"


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

	int VduFontProvider::getFontIndex(QString vduEquipmentId, QString fontName, int pixelSize, bool bold, bool italic, bool underlined) const
	{
		auto it = m_fontsInfo.find(vduEquipmentId);
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

	int VduFontProvider::getFontCount(QString vduEquipmentId) const
	{
		int fontCount = 0;

		auto it = m_fontsInfo.find(vduEquipmentId);
		if (it != m_fontsInfo.end())
		{
			fontCount = static_cast<int>(it->second.size());
		}

		return fontCount;
	}

	void VduFontProvider::setUnicodeSubsets(const QString& vduEquipmentId, const std::vector<VduSymbolSubset>& vduSubsets)
	{
		m_symbolSubsets[vduEquipmentId]  = vduSubsets;
	}

	bool VduFontProvider::checkStringForUnicodeSubsets(const QString& vduEquipmentId, const QString& str, QString& errorMsg) const
	{
		const auto& subsetsIt = m_symbolSubsets.find(vduEquipmentId);
		if (subsetsIt == m_symbolSubsets.end())
		{
			errorMsg = QObject::tr("The VDU %1 does not exist").arg(vduEquipmentId);
			return false;
		}

		const auto& vduSubsets = subsetsIt->second;

		for (const QChar& c : str)
		{
			// Check if the symbol exists in the used subsets
			//
			bool symbolFound = std::any_of(vduSubsets.begin(),
										   vduSubsets.end(),
										   [c](const auto& subset)
										   {
											   return c >= subset.start && c <= subset.finish;
										   });

			if (symbolFound == false)
			{
				// Symbol was not found - look up the required subset
				//
				auto notIncludedIt = std::find_if(AllUnicodeSubsets.begin(),
												  AllUnicodeSubsets.end(),
												  [c](const auto& subset)
												  {
													  return c >= subset.second.first && c <= subset.second.second;
												  });

				if (notIncludedIt == AllUnicodeSubsets.end())
				{
					errorMsg = QObject::tr("The string '%1' processed by the VDU '%2' contains symbols from the unknown Unicode subset.")
								   .arg(str)
								   .arg(vduEquipmentId);
				}
				else
				{
					errorMsg =
						QObject::tr("The string '%1' processed by the VDU '%2' contains symbols from the '%3' Unicode subset, which is "
									"not included to the VDU")
							.arg(str)
							.arg(vduEquipmentId)
							.arg(notIncludedIt->first);
				}

				return false;
			}
		}

		return true;
	}

} // namespace Builder