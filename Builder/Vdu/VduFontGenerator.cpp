#include "VduFontGenerator.h"
#include "VduUnicodeSubsets.h"
#include "../Context.h"
#include "../IssueLogger.h"

#include <HardwareLib/DeviceModule.h>

namespace Builder
{
	VduSymbol::VduSymbol(quint16 width, quint16 height, const QChar& c, const QFontMetrics& fm, const QFont& font) :
		m_header{0, width, height, 0},
		m_image(QSize(width, height), QImage::Format_Grayscale8)
	{
		QByteArray ba = QString(c).toUtf8();
		for (int i = 0; i < ba.size() && i < sizeof(unsigned long); ++i)
		{
			m_header.code |= static_cast<unsigned char>(ba[i]) << (8 * i);
		}

#if 0   // NoAntialiasing
		//
		QImage monoImage{QSize{width, height}, QImage::Format_Mono};
		QPainter painter(&monoImage);

		painter.setRenderHint(QPainter::TextAntialiasing, false);
		painter.setRenderHint(QPainter::Antialiasing, false);

		painter.setFont(font);
		painter.fillRect(0, 0, width, height, Qt::white);
		painter.setPen(Qt::black);
		painter.drawText(0, fm.ascent(), c);
		painter.end();

		m_image = monoImage.convertedTo(QImage::Format_Grayscale8);
#else
		// Antialiasing
		//
		QPainter painter(&m_image);
		painter.setFont(font);
		painter.fillRect(0, 0, width, height, Qt::white);
		painter.setPen(Qt::black);
		painter.drawText(0, fm.ascent(), c);
		painter.end();
#endif

		// Render to m_data
		//
		m_data.clear();
		m_data.reserve(imageSize());

		const char* imageData = reinterpret_cast<const char*>(m_image.bits());

		int repeatCount = 0;
		char pixelColor = 0x55;

		for (int i = 0; i < m_image.height(); i++)
		{
			QString s;
			for (int j = 0; j < m_image.bytesPerLine(); j++)
			{
				if (j < m_image.width())
				{
					char nextPixelColor = ((0xff - *imageData) & 0xc0) >> 6; // Bits 0 and 1 mean color, 00, 01, 10, 11
					if (pixelColor == 0x55)
					{
						// First pixel initialization
						pixelColor = nextPixelColor;
					}

					if (nextPixelColor == pixelColor && repeatCount < 63)
					{
						repeatCount++;
					}
					else
					{
						m_data.append((repeatCount << 2) | pixelColor); // Bits 7..2 mean count of repeats (0..63)
						repeatCount = 1;
						pixelColor = nextPixelColor;
					}
				}
				imageData++;
			}
		}

		// Write the last value
		//
		if (repeatCount > 1)
		{
			m_data.append((repeatCount << 2) | pixelColor); // Bits 7..2 mean count of repeats (0..63)
		}

		/* left for testing
		if (m_header.code == 65)
		{
			qDebug() << "------------";
			QString s;

			for (unsigned char c : m_data)
			{
				int symbol = (unsigned int)c & 0x3;
				int repeats = (unsigned int)c >> 2;

				for (int i = 0; i < repeats; i++)
				{
					s.append(QString::number(symbol));
					int l = s.length();
					if (l >= m_header.width)
					{
						qDebug() << s;
						s.clear();
					}
				}
			}
			if (s.isEmpty() == false)
			{
				qDebug() << s;
			}
		}*/
	}

	void VduSymbol::saveToBmp(QByteArray& out) const
	{
		QBuffer buffer(&out);
		buffer.open(QIODevice::WriteOnly);
		m_image.save(&buffer, "bmp"); // writes image into ba in PNG format
	}

	void VduSymbol::saveToVdut(QByteArray& out) const
	{
		QBuffer buffer(&out);
		buffer.open(QIODevice::WriteOnly);

		const char* data = m_data.data();

		for (int i = 0; i < m_image.height(); i++)
		{
			QString s;
			for (int j = 0; j < m_image.width(); j++)
			{
				QString c = QString::number((unsigned char)(*data++), 16).rightJustified(2, '0');
				s += c + " ";
			}
			buffer.write(s.toLocal8Bit());
			buffer.write("\n");
		}
	}

	const QByteArray& VduSymbol::data() const
	{
		return m_data;
	}

	const QImage& VduSymbol::image() const
	{
		return m_image;
	}

	const VduSymbolHeader& VduSymbol::header() const
	{
		return m_header;
	}

	int VduSymbol::imageSize() const
	{
		int result = m_image.height() * m_image.width(); // NOT sizeInBytes()!
		return result;
	}

	void VduSymbol::setHeaderOffset(quint32 offset)
	{
		m_header.offset = offset;
	}

	// ----------------------------------------------------------------------------------------
	//									VduFontGenerator
	// ----------------------------------------------------------------------------------------

	bool VduFontGenerator::generationVduFonts(Context& context)
	{
		IssueLogger* log = context.m_log;
		Q_ASSERT(log);

		bool result = true;

		auto isVduModule = [](Hardware::DeviceModule* module)
			{
				return module->isVdu();
			};

		for (const Hardware::DeviceModule* vdu : context.m_fscModules | std::views::filter(isVduModule))
		{
			Q_ASSERT(vdu);

			LOG_MESSAGE(log, QString("Generating fonts for VDU %1.").arg(vdu->equipmentId()));

			if (vdu->propertyExists("SubsystemID") == false)
			{
				log->errCFG3000("SubsystemID", vdu->equipmentId());
				return false;
			}
			QString subsystemID = vdu->propertyValue("SubsystemID").toString();

			auto fontsProperty = vdu->propertyByCaption(EquipmentPropNames::FONTS);
			if (fontsProperty == nullptr)
			{
				// Property '%1.%2' is not found.
				//
				log->errCFG3020(vdu->equipmentId(), EquipmentPropNames::FONTS);
				result = false;
				continue;
			}

			// Load Unicode symbols subsets
			//
			std::vector<VduSymbolSubset> vduSubsets;
			vduSubsets.push_back({"BasicLatin", 0x0020, 0x007F});
			vduSubsets.push_back({"Latin1Supplement", 0x0080, 0x00FF});

			for (const auto& specProp: vdu->specificProperties())
			{
				if (specProp->category() == EquipmentPropNames::UNICODDE_SUBSETS) 
				{
					auto subsetIt = AllUnicodeSubsets.find(specProp->caption());
					if (subsetIt == AllUnicodeSubsets.end())
					{
						context.m_log->errINT1000(QObject::tr("VDU object %1 has incorrect Unicode symbol subset property: %2")
													  .arg(vdu->equipmentId())
													  .arg(specProp->caption()));
						return false;
					}
					if (specProp->value().toBool() == true)
					{
						vduSubsets.push_back({subsetIt->first, subsetIt->second.first, subsetIt->second.second});
					}
				}
			}
			context.m_vduFontProvider.setUnicodeSubsets(vdu->equipmentId(), vduSubsets);

			// Parse fonts info from Fonts property
			//
			auto fontsValue = fontsProperty->value().toString();
			if (fontsValue.isEmpty() == true)
			{
				continue;
			}
			std::vector<VduFontInfo> fontsInfo;

			QXmlStreamReader reader(fontsValue);

			if (reader.readNextStartElement() == false)
			{
				reader.raiseError(QObject::tr("Internal error: Failed to load root element in Fonts property in VDU %1.").arg(vdu->equipmentId()));
				log->errINT1000(reader.errorString());
				continue;
			}

			if (reader.name() != QLatin1String("VduFonts"))
			{
				reader.raiseError(QObject::tr("Internal error: Error loading fonts for VDU %1: unknown tag %2.")
								  .arg(vdu->equipmentId())
								  .arg(reader.name()));
				log->errINT1000(reader.errorString());
				continue;
			}

			// Read signals
			//
			while (reader.readNextStartElement())
			{
				if (reader.name() == QLatin1String("VduFont"))
				{
					QString errorMessage;
					VduFontInfo fi;

					if (fi.load(reader, &errorMessage) == true)
					{
						fontsInfo.push_back(fi);
					}
					else
					{
						reader.raiseError(QObject::tr("Internal error: Error loading font for VDU %1: %2.").arg(vdu->equipmentId()).arg(errorMessage));
						log->errINT1000(reader.errorString());
					}
				}
				else
				{
					reader.raiseError(QObject::tr("Internal error: Error loading fonts for VDU %1: unknown tag %2.")
									  .arg(vdu->equipmentId())
									  .arg(reader.name()));
					log->errINT1000(reader.errorString());
				}
				reader.skipCurrentElement();
			}

			result &= (reader.hasError() == false);

			// Add parsed fonts info to VduFontProvider
			//
			context.m_vduFontProvider.setFontsInfo(vdu->equipmentId(), fontsInfo);

			// Generate font files for loaded fonts
			//
			int fontIndex = 0;
			for (const VduFontInfo& fi : fontsInfo)
			{
				QString vduDir = Directory::SUBSYSTEMS + Separator::DIR + subsystemID + Separator::DIR + vdu->equipmentId() +
								 QString("/Fonts/%1/").arg(fontIndex++);
				result &= Builder::VduFontGenerator::generateVduFont(fi, vduSubsets, vduDir, context.generateExtraDebugInfo(), context);
			}
		}

		return result;
	}

	bool VduFontGenerator::generateVduFont(const VduFontInfo& fontInfo,
										   const std::vector<VduSymbolSubset>& vduSubsets,
										   const QString& dir,
										   bool generateDebugFiles,
										   Context& context)
	{
		bool result = true;

		const QString genericExtension = ".vdf";
		const QString symbolExtension = ".vds";
		const QString textSymbolExtension = ".vdst";

		QFont font;
		font.setFamily(fontInfo.family);
		font.setPixelSize(fontInfo.pixelSize);
		font.setBold(fontInfo.bold);
		font.setItalic(fontInfo.italic);
		font.setUnderline(fontInfo.underlined);
		QFontMetrics fm(font);

		std::vector<VduSymbol> symbols;

		if (vduSubsets.empty() == true)
		{
			context.m_log->errINT1000(QObject::tr("VDU Font %1 has no Unicode symbol subsets!").arg(fontInfo.family));
			return false;
		}

		for (const VduSymbolSubset& vduSubset : vduSubsets)
		{
			for (char16_t i = vduSubset.start; i <= vduSubset.finish; i++)
			{
				QChar c(i);

				quint16 height = fm.height();
				quint16 width = fm.horizontalAdvance(c);

				VduSymbol s(width, height, c, fm, font);
				symbols.push_back(s);

				QString fileNameSuffix = QString::number(i, 16).rightJustified(4, '0');

#if 0 // This code is left for debugging
	  // result &= context.m_buildResultWriter->addFile(dir, fileNameSuffix + symbolExtension, s.data()) != nullptr;
#endif


				if (generateDebugFiles == true)
				{
					// Save debug files
					//
					{
						QByteArray ba;
						s.saveToBmp(ba);
						result &= context.m_buildResultWriter->addFile(dir, fileNameSuffix + ".bmp", ba) != nullptr;
					}
#if 0 // This code is left for debugging
	  {
	   QByteArray ba;
	   s.saveToVdut(ba);
	   result &= context.m_buildResultWriter->addFile(dir, fileNameSuffix + textSymbolExtension, ba) != nullptr;
	  }
#endif
				}
			}
		}

		quint32 offset = static_cast<quint32>(symbols.size() * sizeof(VduSymbolHeader));

		for (VduSymbol& s : symbols)
		{
			s.setHeaderOffset(offset);
			offset += s.data().size();
		}

		// Save font to the file
		//
		QByteArray ba;
		{
			QBuffer buffer(&ba);
			buffer.open(QIODevice::WriteOnly);
			for (VduSymbol& s : symbols)
			{
				buffer.write(reinterpret_cast<const char*>(&s.header()), sizeof(VduSymbolHeader));
			}

			for (VduSymbol& s : symbols)
			{
				const QByteArray& data = s.data();
				buffer.write(data);
			}
		}
		
		QString fontFileName = "font";
		result &= context.m_buildResultWriter->addFile(dir, fontFileName + genericExtension, ba) != nullptr;

		return result;
	}

} // namespace Builder