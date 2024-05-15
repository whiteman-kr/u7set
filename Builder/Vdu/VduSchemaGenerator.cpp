#include "VduSchemaGenerator.h"

#include "VduSchemaFile.h"
#include "../Context.h"

#include "../../UtilsLib/Crc.h"

#include <HardwareLib/DeviceModule.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/SchemaItemVduLine.h>
#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/SchemaView.h>
#include <VFrame30/VduSchema.h>

// #define VDU_DEBUG

namespace Builder
{
	bool VduSchemaGenerator::generateVduSchemas(const std::vector<VFrame30::VduSchema*>& schemas, Context& context)
	{
		IssueLogger* log = context.m_log;
		Q_ASSERT(log);

		context.m_vduSchemas.clear();

		bool result = true;

		for (const Hardware::DeviceModule* vdu : context.m_vduModules)
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
				QString nativeVduSchemaFileName = QString("%1.%2").arg(schema->schemaId()).arg(Db::File::VduNativeFileExtension);

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
						log->errINT1001(
							QString("vdu::VduSchemaGenerator::generateVduBackgroundBitmap internal error, schema:").arg(schema->schemaId()));
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
											   Context& context)
	{
		IssueLogger& log = *context.m_log;

		bool result = true;

		std::multimap<QString, size_t> strings; // string -> referenceOffset

		auto addStringRef = [&strings](const QString& str, size_t offset) -> vdu_string_ref
		{
			strings.insert(std::make_pair(str, offset));
			return StringRefStub;               // "STRR" - for debug, easy to find in hex editor.
		};

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
			schemaProperties.schemaId = addStringRef(schema.schemaId(), offsetof(VduSchemaFile, schemaProperties.schemaId));
			schemaProperties.caption = addStringRef(schema.caption(), offsetof(VduSchemaFile, schemaProperties.caption));
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
													   return sum + static_cast<int>(layer->items().size());
												   });

		file.schemaItemCount = static_cast<uint16_t>(totalSchemaItemCount);

		// Save data to output buffer.
		//
		out.clear();
		out.append(reinterpret_cast<const char*>(&file), sizeof(file));

#ifdef VDU_DEBUG
		qDebug() << "out.append(reinterpret_cast<const char*>(&file), sizeof(file)); out.size() == " << out.size();
#endif

		{
			// Forming file.items in the output buffer.
			//
			for (const auto& layer : schema.layers())
			{
				for (const auto& item : layer->items())
				{
					QByteArray outSchemaItem{};
					std::list<std::pair<QString, size_t>> addedStringReferences;

					saveSchemaItem1(vduEquipmentId, *item, appSignalHashToSignalIndex, outSchemaItem, addedStringReferences, context);

					// Add added string references to the main string ref container.
					//
					for (const auto& [str, referenceOffset] : addedStringReferences)
					{
						addStringRef(str, referenceOffset + out.size());
					}

					// Save item's data to the output buffer.
					//
					out.append(outSchemaItem);
				}
			}
		}

		// Resolve strings:
		// String consist of 16 bit size of string in symbols, followed with string data. Padding to 4 bytes.
		// The string is a null terminated QChar string.
		// (In Qt, Unicode characters are 16-bit entities without any markup or structure).
		// Note: String in file must be aligned to 4 bytes.
		//

		// Align to 4 bytes the beginning of string area.
		//
		for (size_t ps = 0, rest = 4 - (out.size() % 4); ps < rest; ps++)
		{
			out.push_back(char{0});
		}

		for (const auto& [str, offset] : strings)
		{
			uint32_t stringOffset = out.size();

			// Write string size.
			//
			uint16_t stringSize = static_cast<uint16_t>(str.size());
			out.append(reinterpret_cast<const char*>(&stringSize), sizeof(stringSize));

			// Write string data.
			//
			out.append(reinterpret_cast<const char*>(str.constData()), (str.size() + 1) * sizeof(QChar)); // +1 for null terminator

			// Replace string_ref with offset to the string.
			//
			out.replace(offset, sizeof(vdu_string_ref), reinterpret_cast<const char*>(&stringOffset), sizeof(stringOffset));

			// Add padding bytes to strings (aligned to 4 bytes).
			//
			for (size_t ps = 0, rest = 4 - (out.size() % 4); ps < rest; ps++)
			{
				out.push_back(char{0});
			}
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

	bool VduSchemaGenerator::saveSchemaItem1(QString vduEquipmentId,
											 const VFrame30::SchemaItem& schemaItem,
											 const std::map<Hash, int>& appSignalHashToSignalIndex,
											 QByteArray& out,
											 std::list<std::pair<QString, size_t>>& addedStringReferences,
											 Context& context)
	{
		IssueLogger& log = *context.m_log;

		// Get item type id and size of the specific item structure.
		//
		using ItemTypeId = decltype(VduSchemaFileSchemaItem1::itemType);
		ItemTypeId itemType = 0;

		using TotalItemSizeType = decltype(VduSchemaFileSchemaItem1::totalItemSize);
		TotalItemSizeType specificItemSize = 0;

		if (dynamic_cast<const VFrame30::SchemaItemVduLine*>(&schemaItem) != nullptr)
		{
			itemType = VduFileSchemaItemLineId;
			specificItemSize = sizeof(VduSchemaFileSchemaItemLine1);
		}

		if (dynamic_cast<const VFrame30::SchemaItemVduRect*>(&schemaItem) != nullptr)
		{
			itemType = VduFileSchemaItemRectId;
			specificItemSize = sizeof(VduSchemaFileSchemaItemRect1);
		}

		if (dynamic_cast<const VFrame30::SchemaItemVduValue*>(&schemaItem) != nullptr)
		{
			itemType = VduFileSchemaItemValueId;
			specificItemSize = sizeof(VduSchemaFileSchemaItemValue1);
		}

		if (itemType == 0 || specificItemSize == 0)
		{
			Q_ASSERT(itemType != 0);
			Q_ASSERT(specificItemSize != 0);
			return false;
		}

		// --
		//
		VduSchemaFileSchemaItem1 fileSchemaItem;
		std::memset(&fileSchemaItem, 0, sizeof(fileSchemaItem));

		fileSchemaItem.version = 1;
		fileSchemaItem.size = sizeof(fileSchemaItem);
		fileSchemaItem.itemType = itemType;
		fileSchemaItem.totalItemSize = static_cast<TotalItemSizeType>(sizeof(VduSchemaFileSchemaItem1) + specificItemSize);
		fileSchemaItem.isStatic = schemaItem.IsStatic();

		// Save specific item, depending on itemType.
		//
		QByteArray specificData;

		switch (itemType)
		{
		case VduFileSchemaItemLineId:
			{
				const auto& schemaItemVduLine = static_cast<const VFrame30::SchemaItemVduLine&>(schemaItem);

				VduSchemaFileSchemaItemLine1 structLine{};

				structLine.version = 1;
				structLine.itemType = itemType;

				structLine.x1 = static_cast<decltype(structLine.x1)>(schemaItemVduLine.startXDocPt());
				structLine.y1 = static_cast<decltype(structLine.y1)>(schemaItemVduLine.startYDocPt());
				structLine.x2 = static_cast<decltype(structLine.x2)>(schemaItemVduLine.endXDocPt());
				structLine.y2 = static_cast<decltype(structLine.y2)>(schemaItemVduLine.endYDocPt());

				structLine.weight = static_cast<decltype(structLine.weight)>(schemaItemVduLine.weight());

				structLine.color = schemaItemVduLine.lineColor().rgba();

				specificData.append(reinterpret_cast<const char*>(&structLine), sizeof(structLine));
			}
			break;
		case VduFileSchemaItemRectId:
			{
				const auto& schemaItemVduRect = static_cast<const VFrame30::SchemaItemVduRect&>(schemaItem);

				VduSchemaFileSchemaItemRect1 structRect{};

				structRect.version = 1;
				structRect.itemType = itemType;

				using PosType = decltype(VduSchemaFileSchemaItemRect1::left);
				using SizeType = decltype(VduSchemaFileSchemaItemRect1::width);

				structRect.left = static_cast<PosType>(schemaItemVduRect.leftDocPt());
				structRect.top = static_cast<PosType>(schemaItemVduRect.topDocPt());
				structRect.width = static_cast<SizeType>(schemaItemVduRect.widthDocPt());
				structRect.height = static_cast<SizeType>(schemaItemVduRect.heightDocPt());

				structRect.weight = static_cast<decltype(structRect.weight)>(schemaItemVduRect.weight());
				structRect.fill = schemaItemVduRect.fill();
				structRect.drawRect = schemaItemVduRect.drawRect();

				structRect.lineColor = schemaItemVduRect.lineColor().rgba();
				structRect.fillColor = schemaItemVduRect.fillColor().rgba();
				structRect.textColor = schemaItemVduRect.textColor().rgba();

				// Set font index.
				//
				int fontIndex = context.m_vduFontProvider
					.getFontIndex(vduEquipmentId,
								  schemaItemVduRect.getFontName(),
								  schemaItemVduRect.getFontSize(),
								  schemaItemVduRect.getFontBold(),
								  schemaItemVduRect.getFontItalic(),
								  false);

				if (fontIndex == -1)
				{
					// Font not found.
					//
					QString font = QString{"'%1, %2%3%4'"}
						.arg(schemaItemVduRect.getFontName())
						.arg(schemaItemVduRect.getFontSize())
						.arg(schemaItemVduRect.getFontBold() ? ", bold" : "")
						.arg(schemaItemVduRect.getFontItalic() ? ", italic" : "");

					log.errEQP6401(vduEquipmentId,
								   schemaItem.parentSchema()->schemaId(),
								   schemaItem.label(),
								   schemaItem.guid(),
								   font);

					return false;
				}

				structRect.fontIndex = fontIndex;

				addedStringReferences.emplace_back(schemaItemVduRect.text(),
												   sizeof(fileSchemaItem) + offsetof(VduSchemaFileSchemaItemRect1, text));

				structRect.text = StringRefStub;

				specificData.append(reinterpret_cast<const char*>(&structRect), sizeof(structRect));
			}
			break;
		case VduFileSchemaItemValueId:
			{
				const auto& schemaItemVduValue = static_cast<const VFrame30::SchemaItemVduValue&>(schemaItem);

				VduSchemaFileSchemaItemValue1 structValue{};

				structValue.version = 1;
				structValue.itemType = itemType;

				using PosType = decltype(structValue.left);
				using SizeType = decltype(structValue.width);

				structValue.left = static_cast<PosType>(schemaItemVduValue.leftDocPt());
				structValue.top = static_cast<PosType>(schemaItemVduValue.topDocPt());
				structValue.width = static_cast<SizeType>(schemaItemVduValue.widthDocPt());
				structValue.height = static_cast<SizeType>(schemaItemVduValue.heightDocPt());

				structValue.weight = static_cast<decltype(structValue.weight)>(schemaItemVduValue.weight());
				structValue.drawRect = schemaItemVduValue.drawRect();

				structValue.lineColor = schemaItemVduValue.lineColor().rgba();
				structValue.fillColor = schemaItemVduValue.fillColor().rgba();
				structValue.textColor = schemaItemVduValue.textColor().rgba();

				// TODO: Set font index.
				//
				int fontIndex = context.m_vduFontProvider
									.getFontIndex(vduEquipmentId,
												  schemaItemVduValue.getFontName(),
												  schemaItemVduValue.getFontSize(),
												  schemaItemVduValue.getFontBold(),
												  schemaItemVduValue.getFontItalic(),
												  false);

				if (fontIndex == -1)
				{
					// Font not found.
					//
					QString font = QString{"'%1, %2%3%4'"}
						.arg(schemaItemVduValue.getFontName())
						.arg(schemaItemVduValue.getFontSize())
						.arg(schemaItemVduValue.getFontBold() ? ", bold" : "")
						.arg(schemaItemVduValue.getFontItalic() ? ", italic" : "");

					log.errEQP6401(vduEquipmentId,
								   schemaItem.parentSchema()->schemaId(),
								   schemaItem.label(),
								   schemaItem.guid(),
								   font);

					return false;
				}

				structValue.fontIndex = fontIndex;

				structValue.decimalPlaces = schemaItemVduValue.precision();

				// Set app signal index.
				//
				{
					QString appSignalId = schemaItemVduValue.appSignalId();

					auto sit = appSignalHashToSignalIndex.find(::calcHash(appSignalId));
					if (sit == appSignalHashToSignalIndex.end())
					{
						// Signal not found.
						//
						log.errEQP6400(vduEquipmentId,
									   appSignalId,
									   schemaItem.parentSchema()->schemaId(),
									   schemaItem.label(),
									   schemaItem.guid());
						return false;
					}

					structValue.appSignalIndex = sit->second;
				}

				specificData.append(reinterpret_cast<const char*>(&structValue), sizeof(structValue));
			}
			break;
		}


		// Forming output buffer.
		//
		if (specificData.isEmpty() == true)
		{
			Q_ASSERT(specificData.isEmpty() == false);
			return false;
		}

		out.clear();
		out.append(reinterpret_cast<const char*>(&fileSchemaItem), sizeof(fileSchemaItem));
		out.append(specificData);

		return true;
	}
} // namespace Builder