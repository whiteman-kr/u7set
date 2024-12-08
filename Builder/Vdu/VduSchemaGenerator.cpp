#include "VduSchemaGenerator.h"

#include "../Context.h"
#include "VduSchemaFile.h"

#include "../../UtilsLib/Crc.h"

#include <HardwareLib/DeviceModule.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/SchemaItemVduLine.h>
#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/SchemaView.h>
#include <VFrame30/VduSchema.h>

// #define VDU_DEBUG

namespace
{
	struct VduFileString
	{
		QString string;
		uint32_t stringRefOffset = 0; // Offset to the string reference in the file.

		static const uint32_t stub = StringRefStub;

		static VduFileString createUtf8(const QString& string, uint32_t stringRefOffset)
		{
			return VduFileString{.string = string.trimmed(), .stringRefOffset = stringRefOffset};
		}
	};

	// --
	//
	class SaveVduItemVisitor : public VFrame30::VduItemVisitor
	{
		QString m_vduEquipmentId;
		Builder::IssueLogger& m_log;
		const Builder::VduFontProvider& m_vduFontProvider;
		const std::map<Hash, int>& m_appSignalHashToSignalIndex;

	public:
		QByteArray outData;
		std::list<VduFileString> addedStrings;
		uint16_t itemType = 0;

		void reset()
		{
			outData.clear();
			addedStrings.clear();
			itemType = 0;
		}

		SaveVduItemVisitor(QString vduEquipmentId,
						   Builder::IssueLogger& m_log,
						   const Builder::VduFontProvider& m_vduFontProvider,
						   const std::map<Hash, int>& appSignalHashToSignalIndex) :
			m_vduEquipmentId(vduEquipmentId),
			m_log(m_log),
			m_vduFontProvider(m_vduFontProvider),
			m_appSignalHashToSignalIndex(appSignalHashToSignalIndex)
		{
		}

		// SchemaItemVduLine
		//
		void visit(const VFrame30::SchemaItemVduLine& schemaItem) override
		{
			reset();

			VduSchemaFileSchemaItemLine1 structLine{};

			structLine.version = 1;
			structLine.itemType = VduFileSchemaItemLineId; // ! Do not forget to set itemType.

			structLine.x1 = static_cast<decltype(structLine.x1)>(schemaItem.startXDocPt());
			structLine.y1 = static_cast<decltype(structLine.y1)>(schemaItem.startYDocPt());
			structLine.x2 = static_cast<decltype(structLine.x2)>(schemaItem.endXDocPt());
			structLine.y2 = static_cast<decltype(structLine.y2)>(schemaItem.endYDocPt());

			structLine.weight = static_cast<decltype(structLine.weight)>(schemaItem.weight());

			structLine.color = schemaItem.lineColor().rgba();

			itemType = structLine.itemType;
			outData = QByteArray(reinterpret_cast<const char*>(&structLine), sizeof(structLine));
		}

		// SchemaItemVduRect
		//
		void visit(const VFrame30::SchemaItemVduRect& schemaItem) override
		{
			reset();

			VduSchemaFileSchemaItemRect1 structRect{};

			structRect.version = 1;
			structRect.itemType = VduFileSchemaItemRectId; // ! Do not forget to set itemType.

			using PosType = decltype(VduSchemaFileSchemaItemRect1::left);
			using SizeType = decltype(VduSchemaFileSchemaItemRect1::width);

			structRect.left = static_cast<PosType>(schemaItem.leftDocPt());
			structRect.top = static_cast<PosType>(schemaItem.topDocPt());
			structRect.width = static_cast<SizeType>(schemaItem.widthDocPt());
			structRect.height = static_cast<SizeType>(schemaItem.heightDocPt());

			structRect.weight = static_cast<decltype(structRect.weight)>(schemaItem.weight());
			structRect.fill = schemaItem.fill();
			structRect.drawRect = schemaItem.drawRect();

			structRect.lineColor = schemaItem.lineColor().rgba();
			structRect.fillColor = schemaItem.fillColor().rgba();
			structRect.textColor = schemaItem.textColor().rgba();

			// Set font index.
			//
			int fontIndex = m_vduFontProvider.getFontIndex(m_vduEquipmentId,
														   schemaItem.getFontName(),
														   schemaItem.getFontSize(),
														   schemaItem.getFontBold(),
														   schemaItem.getFontItalic(),
														   false);

			if (fontIndex == -1)
			{
				// Font not found.
				//
				QString font = QString{"'%1, %2%3%4'"}
								   .arg(schemaItem.getFontName())
								   .arg(schemaItem.getFontSize())
								   .arg(schemaItem.getFontBold() ? ", bold" : "")
								   .arg(schemaItem.getFontItalic() ? ", italic" : "");

				m_log.errEQP6401(m_vduEquipmentId, schemaItem.parentSchema()->schemaId(), schemaItem.label(), schemaItem.guid(), font);
				return;
			}

			structRect.fontIndex = fontIndex;

			structRect.text = VduFileString::stub;

			auto text = VduFileString::createUtf8(schemaItem.text(),
												  sizeof(VduSchemaFileSchemaItem1) + offsetof(VduSchemaFileSchemaItemRect1, text));
			addedStrings.push_back(std::move(text));

			itemType = structRect.itemType;
			outData = QByteArray(reinterpret_cast<const char*>(&structRect), sizeof(structRect));
		}

		// SchemaItemVduValue
		//
		void visit(const VFrame30::SchemaItemVduValue& schemaItem) override
		{
			reset();

			VduSchemaFileSchemaItemValue1 structValue{};

			structValue.version = 1;
			structValue.itemType = VduFileSchemaItemValueId;

			using PosType = decltype(structValue.left);
			using SizeType = decltype(structValue.width);

			structValue.left = static_cast<PosType>(schemaItem.leftDocPt());
			structValue.top = static_cast<PosType>(schemaItem.topDocPt());
			structValue.width = static_cast<SizeType>(schemaItem.widthDocPt());
			structValue.height = static_cast<SizeType>(schemaItem.heightDocPt());

			structValue.weight = static_cast<decltype(structValue.weight)>(schemaItem.weight());
			structValue.drawRect = schemaItem.drawRect();

			structValue.lineColor = schemaItem.lineColor().rgba();
			structValue.fillColor = schemaItem.fillColor().rgba();
			structValue.textColor = schemaItem.textColor().rgba();

			int fontIndex = m_vduFontProvider.getFontIndex(m_vduEquipmentId,
														   schemaItem.getFontName(),
														   schemaItem.getFontSize(),
														   schemaItem.getFontBold(),
														   schemaItem.getFontItalic(),
														   false);

			if (fontIndex == -1)
			{
				// Font not found.
				//
				QString font = QString{"'%1, %2%3%4'"}
								   .arg(schemaItem.getFontName())
								   .arg(schemaItem.getFontSize())
								   .arg(schemaItem.getFontBold() ? ", bold" : "")
								   .arg(schemaItem.getFontItalic() ? ", italic" : "");

				m_log.errEQP6401(m_vduEquipmentId, schemaItem.parentSchema()->schemaId(), schemaItem.label(), schemaItem.guid(), font);
				return;
			}

			structValue.fontIndex = fontIndex;

			structValue.decimalPlaces = schemaItem.precision();

			// Save text in UTF-8, this text is embedded to the structure, as it can be used in the script.
			//
			auto text = schemaItem.text().toUtf8();
			text.truncate(sizeof(structValue.text) - 2);

			std::memcpy(structValue.text, text.constData(), text.size());
			structValue.text[sizeof(structValue.text) - 1] = 0;
			structValue.text[sizeof(structValue.text) - 2] = 0;

			// Set app signal indexes.
			//
			QStringList appSignalIds = schemaItem.appSignalIds();
			structValue.appSignalCount = static_cast<decltype(structValue.appSignalCount)>(appSignalIds.size());

			outData = QByteArray(reinterpret_cast<const char*>(&structValue), sizeof(structValue));

			for (const QString& appSignalId : appSignalIds)
			{
				auto sit = m_appSignalHashToSignalIndex.find(::calcHash(appSignalId));

				if (sit == m_appSignalHashToSignalIndex.end())
				{
					// Signal not found.
					//
					m_log.errEQP6400(m_vduEquipmentId,
									 appSignalId,
									 schemaItem.parentSchema()->schemaId(),
									 schemaItem.label(),
									 schemaItem.guid());

					reset();
					return;
				}

				// Signal index follows the structValue.
				//
				uint32_t signalIndex = static_cast<uint32_t>(sit->second);
				outData.append(reinterpret_cast<const char*>(&signalIndex), sizeof(signalIndex));
			}

			// OutData already set.
			//
			itemType = structValue.itemType;
			return;
		}
	};

	bool saveSchemaItem1(QString vduEquipmentId,
						 const VFrame30::SchemaItem& schemaItem,
						 const std::map<Hash, int>& appSignalHashToSignalIndex,
						 QByteArray& out,
						 std::list<VduFileString>& addedStrings,
						 Builder::Context& context)
	{
		Builder::IssueLogger& log = *context.m_log;

		// --
		//
		VduSchemaFileSchemaItem1 fileSchemaItem{};
		std::memset(&fileSchemaItem, 0, sizeof(fileSchemaItem));

		fileSchemaItem.version = 1;
		fileSchemaItem.size = sizeof(fileSchemaItem);
		// fileSchemaItem.itemType = filled in the end, when specific item data is saved.
		// fileSchemaItem.totalItemSize = filled in the end, when specific item data is saved.
		fileSchemaItem.isStatic = schemaItem.IsStatic();
		fileSchemaItem.acceptClick = schemaItem.acceptClick();

		{
			fileSchemaItem.objectName = VduFileString::stub;

			auto objectName = VduFileString::createUtf8(schemaItem.objectName(), offsetof(VduSchemaFileSchemaItem1, objectName));
			addedStrings.push_back(std::move(objectName));
		}

		// onClickScript
		//
		{
			fileSchemaItem.clickScript = VduFileString::stub;

			auto clickScript = VduFileString::createUtf8(schemaItem.clickScript(), offsetof(VduSchemaFileSchemaItem1, clickScript));
			addedStrings.push_back(std::move(clickScript));
		}

		// preDrawScript
		//
		{
			fileSchemaItem.preDrawScript = VduFileString::stub;

			auto preDrawScript = VduFileString::createUtf8(schemaItem.preDrawScript(), offsetof(VduSchemaFileSchemaItem1, preDrawScript));
			addedStrings.push_back(std::move(preDrawScript));
		}

		// Save specific item struct, depending on itemType.
		//
		SaveVduItemVisitor saveVduItemVisitor(vduEquipmentId, log, context.m_vduFontProvider, appSignalHashToSignalIndex);

		dynamic_cast<const VFrame30::SchemaItemVdu&>(schemaItem).accept(saveVduItemVisitor);

		// Save result after visiting specific item. SaveVduItemVisitor
		//
		QByteArray specificData = saveVduItemVisitor.outData;

		std::copy(saveVduItemVisitor.addedStrings.begin(), saveVduItemVisitor.addedStrings.end(), std::back_inserter(addedStrings));

		if (specificData.isEmpty() == true)
		{
			// Error occurred during saving specific item.
			//
			return false;
		}

		Q_ASSERT(saveVduItemVisitor.itemType != 0);
		fileSchemaItem.itemType = saveVduItemVisitor.itemType;

		using TotalItemSizeType = decltype(VduSchemaFileSchemaItem1::totalItemSize);
		fileSchemaItem.totalItemSize = static_cast<TotalItemSizeType>(sizeof(VduSchemaFileSchemaItem1) + specificData.size());

		out.clear();
		out.append(reinterpret_cast<const char*>(&fileSchemaItem), sizeof(fileSchemaItem));
		out.append(specificData);

		return true;
	}

	// Accumulate string references and strings, then write them to the file.
	//
	class VduStringWriter
	{
	public:
		using Offset = uint32_t;

		void clear() { *this = {}; }

		Offset addString(const QString& str, Offset offset)
		{
			auto vduString = VduFileString::createUtf8(str, offset);

			m_offsetToString.insert(std::pair{Key{str}, vduString});
			m_strings.insert(str);

			return VduFileString::stub;
		}

		std::vector<QString> strings() const { return {m_strings.begin(), m_strings.end()}; }

		std::vector<VduFileString> offsets(const QString& str) const
		{
			Key key{str};

			auto [beginIt, endIt] = m_offsetToString.equal_range(key);

			std::vector<VduFileString> result(std::distance(beginIt, endIt));

			std::transform(beginIt,
						   endIt,
						   result.begin(),
						   [](const auto& p)
						   {
							   return p.second;
						   });

			return result;
		}

	private:
		using Key = QString;

		std::multimap<Key, VduFileString> m_offsetToString;
		std::set<Key> m_strings;
	};
} // namespace

namespace Builder
{
	bool VduSchemaGenerator::generateVduSchemas(const std::vector<VFrame30::VduSchema*>& schemas, Context& context)
	{
		IssueLogger* log = context.m_log;
		Q_ASSERT(log);

		context.m_vduSchemas.clear();

		bool result = true;

		auto isVduModule = [](Hardware::DeviceModule* module)
		{
			return module->isVdu();
		};

		for (const Hardware::DeviceModule* vdu : context.m_fscModules | std::views::filter(isVduModule))
		{
			Q_ASSERT(vdu);

			LOG_MESSAGE(log, QString("Generating schemas for VDU %1.").arg(vdu->equipmentId()));

			auto schemaTagsProperty = vdu->propertyByCaption(EquipmentPropNames::SCHEMA_TAGS);
			if (schemaTagsProperty == nullptr)
			{
				// Property '%1.%2' is not found.
				//
				log->errCFG3020(vdu->equipmentId(), EquipmentPropNames::SCHEMA_TAGS);
				result = false;
				continue;
			}

			auto vduSignalsIt = context.m_vduSignals.find(vdu->equipmentId());
			if (vduSignalsIt == context.m_vduSignals.end())
			{
				// Signals for VDU %1 are not found.
				//
				log->errINT1000(QString("Internal error: VduSignals structure is not found for VDU %1").arg(vdu->equipmentId()));
				result = false;
				continue;
			}
			const auto& vduSignals = vduSignalsIt->second;

			auto vduSchemaTagList = schemaTagsProperty->value().toString().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);

			for (auto schema : schemas)
			{
				Q_ASSERT(schema);

				// If schemaTags is empty, then all schemas are for this VDU
				//
				bool schemaHasTag = vduSchemaTagList.isEmpty();
				schemaHasTag |= std::ranges::any_of(vduSchemaTagList,
													[&schema](QString& tag)
													{
														return schema->tagsAsList().contains(tag.toLower());
													});

				if (schemaHasTag == false)
				{
					continue;
				}

				// Generate VDU schema.
				//
				LOG_MESSAGE(log, QString("Converting schema %1 to VDU format.").arg(schema->schemaId()));

				QStringList errorMessages;
				QByteArray nativeVduData;

				bool genSchemaOk =
					Builder::VduSchemaGenerator::generateVduSchema(vdu->equipmentId(), *schema, vduSignals, nativeVduData, context);

				if (genSchemaOk == false)
				{
					result = false;
					continue;
				}

				// Save result.
				//
				QString nativeVduSchemaFileName = QString("%1.%2").arg(schema->schemaId()).arg(File::VduNativeFileExtension);

				QString vduDir = Directory::VDUs + "/" + vdu->equipmentId() + "/Schemas";

				context.m_buildResultWriter->addFile(vduDir, nativeVduSchemaFileName, nativeVduData);

#if 1
				// Generate background bitmap from the static data.
				//
				{
					QByteArray backgroundImageData;
					QString backgroundBitmapFileName = QString("%1.bmp").arg(schema->schemaId());

					QImage backgroundImage;

					bool genBitmapOk =
						Builder::VduSchemaGenerator::generateVduBackgroundBitmap(schema->shared_from_this(), backgroundImage);
					if (genBitmapOk == false)
					{
						log->errINT1001(QString("vdu::VduSchemaGenerator::generateVduBackgroundBitmap internal error, schema:")
											.arg(schema->schemaId()));
						result = false;
						continue;
					}

					QBuffer buffer(&backgroundImageData);
					buffer.open(QIODevice::WriteOnly);
					backgroundImage.save(&buffer, "BMP");

					context.m_buildResultWriter->addFile(vduDir, backgroundBitmapFileName, backgroundImageData);
				}
#endif
				// Add schema to the global build context.
				//
				{
					// Crc64 is the 8 last bytes from nativeVduData.
					//
					uint64_t crc64 = 0;

					if (nativeVduData.size() >= 8)
					{
						crc64 = *reinterpret_cast<const uint64_t*>(nativeVduData.constData() + nativeVduData.size() - 8);
					}
					else
					{
						Q_ASSERT(nativeVduData.size() >= 8);
					}

					Context::GeneratedVduSchema generatedSchema{.schema = schema->shared_from_this(), .crc64 = crc64};
					context.m_vduSchemas[vdu->equipmentId()].push_back(generatedSchema);
				}
			}
		}

		return result;
	}

	bool VduSchemaGenerator::generateVduSchema(QString vduEquipmentId,
											   const VFrame30::VduSchema& schema,
											   const std::map<Hash, int>& appSignalHashToSignalIndex,
											   QByteArray& out,
											   Builder::Context& context)
	{
		Builder::IssueLogger& log = *context.m_log;

		bool result = true;

		VduStringWriter stringWriter;

		struct VduSchemaFile file;
		std::memset(&file, 0, sizeof(file));

		// Forming file.header
		//
		{
			file.magic[0] = 'V';
			file.magic[1] = 'D';
			file.magic[2] = 'U';
			file.magic[3] = '\0';
			file.fileVersion = 1;
		}

		// Forming file.body
		//
		{
			VduSchemaFileProperties1& schemaProperties = file.schemaProperties;
			schemaProperties.version = 1;
			schemaProperties.headerSize = sizeof(schemaProperties);
			schemaProperties.width = static_cast<uint16_t>(schema.docWidth());
			schemaProperties.height = static_cast<uint16_t>(schema.docHeight());
			schemaProperties.reserve0 = 0;
			schemaProperties.backgroundColor = schema.backgroundColor().rgba();

			schemaProperties.schemaId =
				stringWriter.addString(schema.schemaId(),
									   offsetof(VduSchemaFile, schemaProperties) + offsetof(VduSchemaFileProperties1, schemaId));

			schemaProperties.caption =
				stringWriter.addString(schema.caption(),
									   offsetof(VduSchemaFile, schemaProperties) + offsetof(VduSchemaFileProperties1, caption));

			schemaProperties.onShowScript =
				stringWriter.addString(schema.onShowScript(),
									   offsetof(VduSchemaFile, schemaProperties) + offsetof(VduSchemaFileProperties1, onShowScript));

			schemaProperties.preDrawScript =
				stringWriter.addString(schema.preDrawScript(),
									   offsetof(VduSchemaFile, schemaProperties) + offsetof(VduSchemaFileProperties1, preDrawScript));

			schemaProperties.reserve1 = 0;
			schemaProperties.reserve2 = 0;
		};

		// Forming file.schemaItemCount
		//
		int totalSchemaItemCount = std::accumulate(schema.layers().begin(),
												   schema.layers().end(),
												   0,
												   [](int sum, const auto& layer) -> int
												   {
													   auto vduItems = layer->items() | std::views::filter(
																							[](auto&& item)
																							{
																								return item->isVduItem();
																							});
													   return sum + static_cast<int>(std::ranges::distance(vduItems));
												   });

		file.schemaItemCount = static_cast<uint16_t>(totalSchemaItemCount);

		std::vector<uint32_t> schemaOffsets;
		schemaOffsets.reserve(totalSchemaItemCount);

		// Save data to output buffer.
		//
		out.clear();
		out.append(reinterpret_cast<const char*>(&file), sizeof(file));

		// Reserve space for schemaItemOffsets.
		// Later we will fill it with offsets to the schema items (schemaOffsets).
		//
		qsizetype schemaItemOffsetsTable = out.size();
		out.append(totalSchemaItemCount * sizeof(decltype(schemaOffsets)::value_type), '@'); // @ will be replaced with real data.

#ifdef VDU_DEBUG
		qDebug() << "out.append(reinterpret_cast<const char*>(&file), sizeof(file)); out.size() == " << out.size();
#endif

		// Forming file.items in the output buffer.
		//
		for (const auto& layer : schema.layers())
		{
			for (const auto& item : layer->items())
			{
				if (item->isVduItem() == false)
				{
					log.wrnEQP6405(schema.schemaId(), item->label(), item->guid());
					continue;
				}

				QByteArray outSchemaItem{};
				std::list<VduFileString> addedItemStrings; // Strings added in the fallowing call of saveSchemaItem1(...).

				saveSchemaItem1(vduEquipmentId, *item, appSignalHashToSignalIndex, outSchemaItem, addedItemStrings, context);

				// Add added string references to the main string ref container.
				//
				for (const auto& str : addedItemStrings)
				{
					stringWriter.addString(str.string, str.stringRefOffset + out.size());
				}

				// Save item's data to the output buffer.
				//
				schemaOffsets.push_back(out.size()); // Save offset to the schema item.

				out.append(outSchemaItem);           // Save Item data.
			}
		}

		// Forming file.schemaItemOffsets[totalSchemaItemCount]
		// Write schema item offset table by offset schemaItemOffsetsTable
		//
		Q_ASSERT(schemaOffsets.size() == totalSchemaItemCount);

		QByteArray schemaOffsetsData = QByteArray::fromRawData(reinterpret_cast<const char*>(schemaOffsets.data()),
															   schemaOffsets.size() * sizeof(decltype(schemaOffsets)::value_type));

		out.replace(schemaItemOffsetsTable, schemaOffsetsData.size(), schemaOffsetsData);

		// Resolve strings:
		// String consist of 16 bit size of string in symbols, followed with string data. Padding to 4 bytes.
		// The string is a null terminated QChar string.
		// (In Qt, Unicode characters are 16-bit entities without any markup or structure).
		// Note: String in file must be aligned to 4 bytes.
		//

		// Align to 4 bytes the beginning of string area.
		//
		auto addPadding = [](auto& container, size_t padding)
		{
			for (size_t ps = 0, rest = padding - (container.size() % padding); ps < rest; ps++)
			{
				container.push_back(char{0});
			}
		};

		addPadding(out, 4);

		for (const auto& str : stringWriter.strings())
		{
			auto stringOffset = static_cast<vdu_cstr>(out.size());

			// Write string size.
			//
			std::string utf8Str = str.toUtf8().toStdString();

			// Write string size.
			//
			uint16_t stringSize = static_cast<uint16_t>(utf8Str.size());
			out.append(reinterpret_cast<const char*>(&stringSize), sizeof(stringSize));

			// Write string data.
			//
			out.append(reinterpret_cast<const char*>(utf8Str.data()),
					   (utf8Str.size() + 1) * sizeof(std::string::value_type)); // +1 for null terminator

			// Replace string_ref with offset to the string.
			//
			for (const auto& stringData : stringWriter.offsets(str))
			{
				out.replace(stringData.stringRefOffset,
							sizeof(vdu_string_ref),
							reinterpret_cast<const char*>(&stringOffset),
							sizeof(stringOffset));
			}

			// Add padding bytes to strings (aligned to 4 bytes).
			//
			addPadding(out, 4);
		}

		// Calculate CRC64 for the whole file.
		// Align to 8 bytes the end of string area.
		//
		for (size_t ps = 0, rest = 8 - (out.size() % 8); ps < rest; ps++)
		{
			out.push_back(char{0});
		}

		quint64 crc = qToBigEndian(Crc::crc64(out.constData(), out.size()));
		out.append(reinterpret_cast<const char*>(&crc), sizeof(crc));

		// Check crc, crc on data with crc field must be 0.
		//
		quint64 checkCrc = Crc::crc64(out.constData(), out.size());
		if (checkCrc != 0)
		{
			Q_ASSERT(checkCrc == 0);
			log.errINT1000("Internal error: VduSchemaGenerator::generateVduSchema(...) CRC64 check failed!");
			return false;
		}

		return result;
	}

	bool VduSchemaGenerator::generateVduBackgroundBitmap(std::shared_ptr<VFrame30::Schema> schema, QImage& out)
	{
		// Generate background image in size of the schema.
		// Fill it with schema's background color.
		// Draw only static schema items on it.
		//
		class DrawBackgroundSchemaView : public VFrame30::SchemaView
		{
		public:
			DrawBackgroundSchemaView(std::shared_ptr<VFrame30::Schema> schema) :
				VFrame30::SchemaView(schema)
			{
			}

			virtual VFrame30::DrawMode drawMode() const override { return VFrame30::DrawMode::Monitor; }
		};

		auto oldContext = schema->context();
		schema->setContext(VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr, nullptr));

		DrawBackgroundSchemaView schemaView{schema};

		QPageSize pageSize;
		double pageWidth = schema->docWidth();
		double pageHeight = schema->docHeight();

		if (schema->unit() == SchemaUnit::Inch)
		{
			pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
		}
		else
		{
			assert(schema->unit() == SchemaUnit::Display);
			pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));
		}

		// Calc size
		//
		const int resolution = 300;                                       // Image resolution is 300 dpi

		int widthInPixel = schema->GetDocumentWidth(resolution, 100.0);   // Export 100% zoom
		int heightInPixel = schema->GetDocumentHeight(resolution, 100.0); // Export 100% zoom

		// --
		//
		out = QImage(QSize(widthInPixel, heightInPixel), QImage::Format_ARGB32);

		QPainter p(&out);
		VFrame30::CDrawParam drawParam(&p, &schemaView, schema->gridSize(), schema->pinGridStep(), schema->unit());

		drawParam.setInfoMode(false);
		drawParam.setPdfMode(false);

		// Clear device
		//
		p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), schema->backgroundColor());
		p.setRenderHint(QPainter::Antialiasing);

		// Adjust QPainter
		//
		VFrame30::SchemaView::Ajust(&p, schema->unit(), 0, 0, 100.0); // Export 100% zoom

		// Draw Schema
		//
		QRectF clipRect(0, 0, schema->docWidth(), schema->docHeight());

		schema->Draw(&drawParam, clipRect);

		schema->setContext(oldContext);

		return true;
	}
} // namespace Builder