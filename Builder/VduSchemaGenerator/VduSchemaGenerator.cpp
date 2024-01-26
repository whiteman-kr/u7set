#include "VduSchemaGenerator.h"
#include "../../VFrame30/DrawParam.h"
#include "../../VFrame30/SchemaItemVduLine.h"
#include "../../VFrame30/SchemaItemVduRect.h"
#include "../../VFrame30/SchemaView.h"
#include "../../VFrame30/VduSchema.h"
#include "VduSchemaFile.h"

namespace vdu
{
	static const vdu_string_ref StringRefStub = 0x52525453;          // "STRR" - for debug, easy to find in hex editor.
	static const vdu_schema_item_ref SchemaItemRefStub = 0x29495328; // "(SI)" - for debug, easy to find in hex editor.


	bool VduSchemaGenerator::generateVduSchema(const VFrame30::VduSchema& schema, QByteArray& out, QStringList& outErrorMessages)
	{
		bool result = true;
		outErrorMessages.clear();

		std::multimap<QString, size_t> strings; // string -> referenceOffset

		auto addStringRef = [&strings](const QString& str, size_t offset) -> vdu_string_ref
		{
			strings.insert(std::make_pair(str, offset));
			return StringRefStub;               // "STRR" - for debug, easy to find in hex editor.
		};

		struct VduSchemaFile file
		{
		};

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
			schemaProperties.width = static_cast<uint16_t>(schema.docWidth());
			schemaProperties.height = static_cast<uint16_t>(schema.docHeight());
			schemaProperties.reserve0 = 0;
			schemaProperties.backgroundColor = schema.backgroundColor().rgba();
			schemaProperties.schemaId = addStringRef(schema.schemaId(), offsetof(VduSchemaFile, schemaProperties.schemaId));
			schemaProperties.caption = addStringRef(schema.caption(), offsetof(VduSchemaFile, schemaProperties.caption));
			schemaProperties.reserve1 = 0;
			schemaProperties.reserve2 = 0;
		};

		// Save data to output buffer.
		//
		out.clear();
		out.append(reinterpret_cast<const char*>(&file), sizeof(file));
		qDebug() << "out.append(reinterpret_cast<const char*>(&file), sizeof(file)); out.size() == " << out.size();

		// Forming file.items
		//
		{
			auto schemaItemLess = [](const VFrame30::SchemaItem* lhs, const VFrame30::SchemaItem* rhs) -> bool
			{
				return lhs->guid() < rhs->guid();
			};

			std::map<const VFrame30::SchemaItem*, size_t, decltype(schemaItemLess)> itemRefs; // SchemaItem.guid() -> referenceOffset

			auto addSchemaItemRef = [&itemRefs](const VFrame30::SchemaItem* item, size_t offset) -> vdu_schema_item_ref
			{
				itemRefs.insert(std::make_pair(item, offset));
				return SchemaItemRefStub;
			};

			// Adding item references to the output buffer.
			//
			decltype(VduSchemaFile::count) itemCount = 0;
			for (const auto& layer : schema.layers())
			{
				itemCount += static_cast<decltype(itemCount)>(layer->items().size());

				for (const auto& item : layer->items())
				{
					vdu_schema_item_ref itemRef = addSchemaItemRef(item.get(), out.size());
					out.append(reinterpret_cast<const char*>(&itemRef), sizeof(itemRef));
				}
			}

			// Forming file.count in the output buffer.
			//
			out.replace(offsetof(VduSchemaFile, count), sizeof(file.count), reinterpret_cast<const char*>(&itemCount), sizeof(itemCount));

			// Forming file.items in the output buffer.
			//
			for (const auto& layer : schema.layers())
			{
				for (const auto& item : layer->items())
				{
					QByteArray outSchemaItem{};
					std::list<std::pair<QString, size_t>> addedStringReferences;

					saveSchemaItem1(*item, outSchemaItem, addedStringReferences);

					// Add added string references to the main string ref container.
					//
					for (const auto& [str, referenceOffset] : addedStringReferences)
					{
						addStringRef(str, referenceOffset + out.size());
					}

					// Save item's offset
					//
					uint32_t itemOffset = static_cast<uint32_t>(out.size());
					auto offsetOfTheItemReference = itemRefs[item.get()];

					out.append(outSchemaItem);
					out.replace(offsetOfTheItemReference, sizeof(vdu_schema_item_ref), reinterpret_cast<const char*>(&itemOffset), sizeof(vdu_schema_item_ref));
				}
			}
		}

		// Resolve strings:
		// Append strings to the end of the file, the string is null terminated QChar array.
		// Replace string_ref with offset to the string.
		//
		for (const auto& [str, offset] : strings)
		{
			uint32_t stringOffset = out.size();
			out.append(reinterpret_cast<const char*>(str.constData()), (str.size() + 1) * sizeof(QChar)); // +1 for null terminator

			// Replace string_ref with offset to the string.
			//
			out.replace(offset, sizeof(vdu_string_ref), reinterpret_cast<const char*>(&stringOffset), sizeof(stringOffset));
		}

#if 0
		QFile fileOut("D:/Temp/1/" + schema.schemaId() + ".sss");
		fileOut.open(QIODevice::WriteOnly);
		fileOut.write(out);
		fileOut.close();
#endif

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

			virtual VFrame30::DrawMode drawMode() const override
			{
				return VFrame30::DrawMode::Monitor;
			}
		};

		auto oldContext = schema->context();
		schema->setContext(VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr));

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

	bool VduSchemaGenerator::saveSchemaItem1(const VFrame30::SchemaItem& schemaItem, QByteArray& out, std::list<std::pair<QString, size_t>>& addedStringReferences)
	{
		decltype(VduSchemaFileSchemaItemRect1::itemType) itemType = 0;

		do
		{
			if (dynamic_cast<const VFrame30::SchemaItemVduLine*>(&schemaItem) != nullptr)
			{
				itemType = VduFileSchemaItemLineId;
			}

			if (dynamic_cast<const VFrame30::SchemaItemVduRect*>(&schemaItem) != nullptr)
			{
				itemType = VduFileSchemaItemRectId;
			}
		} while (false);

		if (itemType == 0)
		{
			Q_ASSERT(itemType != 0);
			return false;
		}

		VduSchemaFileSchemaItem1 fileSchemaItem{};
		fileSchemaItem.version = 1;
		fileSchemaItem.itemType = itemType;
		fileSchemaItem.size = sizeof(fileSchemaItem);
		fileSchemaItem.reserve0 = 0;
		fileSchemaItem.reserve1 = 0;

		// Save specific item, depending on itemType.
		//
		VduSchemaFileSchemaItemLine1 structLine{};
		VduSchemaFileSchemaItemRect1 structRect{};

		const char* specificItemPtr = nullptr;
		size_t specificItemSize = 0;

		switch (itemType)
		{
		case VduFileSchemaItemLineId:
			{
				const auto& schemaItemVduLine = static_cast<const VFrame30::SchemaItemVduLine&>(schemaItem);

				specificItemPtr = reinterpret_cast<const char*>(&structLine);
				specificItemSize = sizeof(structLine);

				structLine.itemType = itemType;
				structLine.version = 1;
				structLine.size = sizeof(structLine);

				structLine.x1 = static_cast<decltype(structLine.x1)>(schemaItemVduLine.startXDocPt());
				structLine.y1 = static_cast<decltype(structLine.y1)>(schemaItemVduLine.startYDocPt());
				structLine.x2 = static_cast<decltype(structLine.x2)>(schemaItemVduLine.endXDocPt());
				structLine.y2 = static_cast<decltype(structLine.y2)>(schemaItemVduLine.endYDocPt());

				structLine.color = schemaItemVduLine.lineColor().rgba();
			}
			break;
		case VduFileSchemaItemRectId:
			{
				const auto& schemaItemVduRect = static_cast<const VFrame30::SchemaItemVduRect&>(schemaItem);

				specificItemPtr = reinterpret_cast<const char*>(&structRect);
				specificItemSize = sizeof(structRect);

				structRect.itemType = itemType;
				structRect.version = 1;
				structRect.size = sizeof(structRect);

				// --
				//
				structRect.left = static_cast<decltype(structRect.left)>(schemaItemVduRect.leftDocPt());
				structRect.top = static_cast<decltype(structRect.top)>(schemaItemVduRect.topDocPt());
				structRect.width = static_cast<decltype(structRect.width)>(schemaItemVduRect.widthDocPt());
				structRect.height = static_cast<decltype(structRect.height)>(schemaItemVduRect.heightDocPt());

				structRect.weight = static_cast<decltype(structRect.weight)>(schemaItemVduRect.weight());
				structRect.fill = schemaItemVduRect.fill();
				structRect.drawRect = schemaItemVduRect.drawRect();

				structRect.lineColor = schemaItemVduRect.lineColor().rgba();
				structRect.fillColor = schemaItemVduRect.fillColor().rgba();
				structRect.textColor = schemaItemVduRect.textColor().rgba();

				addedStringReferences.emplace_back(schemaItemVduRect.fontName(), sizeof(fileSchemaItem) + offsetof(VduSchemaFileSchemaItemRect1, fontName));
				addedStringReferences.emplace_back(schemaItemVduRect.text(), sizeof(fileSchemaItem) + offsetof(VduSchemaFileSchemaItemRect1, text));

				structRect.fontName = StringRefStub;
				structRect.text = StringRefStub;
			}
			break;
		}

		if (specificItemSize == 0)
		{
			Q_ASSERT(specificItemSize != 0);
			fileSchemaItem.size += static_cast<decltype(fileSchemaItem.size)>(specificItemSize);
			return false;
		}

		out.clear();

		// Forming output buffer.
		//
		out.append(reinterpret_cast<const char*>(&fileSchemaItem), sizeof(fileSchemaItem));
		out.append(specificItemPtr, specificItemSize);

		return true;
	}
} // namespace vdu