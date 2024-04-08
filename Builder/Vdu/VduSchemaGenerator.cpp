#include "VduSchemaGenerator.h"
#include "../../VFrame30/DrawParam.h"
#include "../../VFrame30/SchemaItems/SchemaItemVduLine.h"
#include "../../VFrame30/SchemaItems/SchemaItemVduRect.h"
#include "../../VFrame30/SchemaItems/SchemaItemVduValue.h"
#include "../../VFrame30/SchemaView.h"
#include "../../VFrame30/VduSchema.h"
#include "../Context.h"
#include "VduSchemaFile.h"

#include <QPageSize>
#include <QPainter>

// #define VDU_DEBUG

namespace Builder
{
	static const vdu_string_ref StringRefStub = 0x52525453; // "STRR" - for debug, easy to find in hex editor.

	bool VduSchemaGenerator::generateVduSchema(QString vduEquipmentId, 
											   const VFrame30::VduSchema& schema,
											   const std::map<Hash, int>& appSignalHashToSignalIndex,
											   QByteArray& out,
											   IssueLogger& log)
	{
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

					saveSchemaItem1(vduEquipmentId, *item, appSignalHashToSignalIndex, outSchemaItem, addedStringReferences, log);

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

		// TODO: Calculate and write CRC64
		// Temporary just write "TODO:CRC"
		// Before wring CRC, align out buffer to 8 bytes.
		//
		for (size_t ps = 0, rest = 8 - (out.size() % 8); ps < rest; ps++)
		{
			out.push_back(char{0});
		}

		const char crc[] = "TODO:CRC";
		out.append(crc, sizeof(crc) - 1);

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
											 IssueLogger& log)
	{
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

				structRect.left = static_cast<PosType>(schemaItemVduRect.leftDocPt());
				structRect.top = static_cast<PosType>(schemaItemVduRect.topDocPt());
				structRect.width = static_cast<PosType>(schemaItemVduRect.widthDocPt());
				structRect.height = static_cast<PosType>(schemaItemVduRect.heightDocPt());

				structRect.weight = static_cast<decltype(structRect.weight)>(schemaItemVduRect.weight());
				structRect.fill = schemaItemVduRect.fill();
				structRect.drawRect = schemaItemVduRect.drawRect();

				structRect.lineColor = schemaItemVduRect.lineColor().rgba();
				structRect.fillColor = schemaItemVduRect.fillColor().rgba();
				structRect.textColor = schemaItemVduRect.textColor().rgba();

				// TODO: Set font index.
				//
				structRect.fontIndex = 0;

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

				structValue.left = static_cast<decltype(structValue.left)>(schemaItemVduValue.leftDocPt());
				structValue.top = static_cast<decltype(structValue.top)>(schemaItemVduValue.topDocPt());
				structValue.width = static_cast<decltype(structValue.width)>(schemaItemVduValue.widthDocPt());
				structValue.height = static_cast<decltype(structValue.height)>(schemaItemVduValue.heightDocPt());

				structValue.weight = static_cast<decltype(structValue.weight)>(schemaItemVduValue.weight());
				structValue.drawRect = schemaItemVduValue.drawRect();

				structValue.lineColor = schemaItemVduValue.lineColor().rgba();
				structValue.fillColor = schemaItemVduValue.fillColor().rgba();
				structValue.textColor = schemaItemVduValue.textColor().rgba();

				// TODO: Set font index.
				//
				structValue.fontIndex = 0;

				// TODO: Set app signal index.
				//
				{
					QString appSignalId = schemaItemVduValue.appSignalId();

					auto sit = appSignalHashToSignalIndex.find(::calcHash(appSignalId));
					if (sit == appSignalHashToSignalIndex.end())
					{
						// Signal not found.
						//
						log.errEQP6400(vduEquipmentId, appSignalId, schemaItem.parentSchema()->schemaId(), schemaItem.label(), schemaItem.guid());
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