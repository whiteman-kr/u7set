#include "VduFontGenerator.h"
#include "../Context.h"
#include "../IssueLogger.h"

#include <HardwareLib/DeviceModule.h>

namespace Builder
{
	VduSymbol::VduSymbol(quint16 width, quint16 height, char c, const QFontMetrics& fm, const QFont& font) :
		m_header{static_cast<quint32>(c), width, height, 0},
		m_image(QSize(width, height), QImage::Format_Grayscale8)
	{

#if 1   // NoAntialiasing
		//
		QImage monoImage{QSize{width, height}, QImage::Format_Mono};
		QPainter painter(&monoImage);

		painter.setRenderHint(QPainter::TextAntialiasing, false);
		painter.setRenderHint(QPainter::Antialiasing, false);

		painter.setFont(font);
		painter.fillRect(0, 0, width, height, Qt::white);
		painter.setPen(Qt::black);
		painter.drawText(0, fm.ascent(), QChar(c));
		painter.end();

		m_image = monoImage.convertedTo(QImage::Format_Grayscale8);
#else
		QPainter painter(&m_image);
		painter.setFont(font);
		painter.fillRect(0, 0, width, height, Qt::white);
		painter.setPen(Qt::black);
		painter.drawText(0, fm.ascent(), QChar(c));
		painter.end();
#endif

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

		for (const Hardware::DeviceModule* vdu : context.m_vduModules)
		{
			Q_ASSERT(vdu);

			LOG_MESSAGE(log, QString("Generating fonts for VDU %1.").arg(vdu->equipmentId()));

			auto fontsProperty = vdu->propertyByCaption(EquipmentPropNames::FONTS);
			if (fontsProperty == nullptr)
			{
				// Property '%1.%2' is not found.
				//
				log->errCFG3020(vdu->equipmentId(), EquipmentPropNames::FONTS);
				result = false;
				continue;
			}

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
				QString vduDir = Directory::VDUs + "/" + vdu->equipmentId() + QString("/Fonts/%1/").arg(fontIndex++);
				result &= Builder::VduFontGenerator::generateVduFont(fi, vduDir, context.generateExtraDebugInfo(), context);
			}
		}

		return result;
	}

	bool VduFontGenerator::generateVduFont(const VduFontInfo& fontInfo, const QString& dir, bool generateDebugFiles, Context& context)
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
		const int start = 32;
		const int end = 127;

		symbols.reserve(end - start);

		for (int i = start; i <= end; i++)
		{
			char c = (char)i;

			quint16 height = fm.height();
			quint16 width = fm.horizontalAdvance(c);

			VduSymbol s(width, height, c, fm, font);
			symbols.push_back(s);

			QString fileNameSuffix = QString::number(i).rightJustified(3, '0');
			
			result &= context.m_buildResultWriter->addFile(dir, fileNameSuffix + symbolExtension, s.data()) != nullptr;

			if (generateDebugFiles == true)
			{
				// Save debug files
				//
				{
					QByteArray ba;
					s.saveToBmp(ba);
					result &= context.m_buildResultWriter->addFile(dir, fileNameSuffix + ".bmp", ba) != nullptr;
				}

				{
					QByteArray ba;
					s.saveToVdut(ba);
					result &= context.m_buildResultWriter->addFile(dir, fileNameSuffix + textSymbolExtension, ba) != nullptr;
				}
			}
		}

		quint32 offset = static_cast<quint32>(symbols.size() * sizeof(VduSymbolHeader));

		for (VduSymbol& s : symbols)
		{
			s.setHeaderOffset(offset);
			offset += s.imageSize();
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

		/*QString fontFileName = QObject::tr("%1_%2").arg(fontInfo.family).arg(fontInfo.pixelSize);
		if (fontInfo.bold == true)
		{
			fontFileName += "b";
		}
		if (fontInfo.italic == true)
		{
			fontFileName += "i";
		}
		if (fontInfo.underlined == true)
		{
			fontFileName += "u";
		}*/
		
		QString fontFileName = "font";
		result &= context.m_buildResultWriter->addFile(dir, fontFileName + genericExtension, ba) != nullptr;

		return result;
	}

} // namespace Builder