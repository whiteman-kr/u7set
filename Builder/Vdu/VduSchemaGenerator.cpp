#include "VduSchemaGenerator.h"

#include "VduLuaScript.h"
#include "VduSchemaFile.h"
#include "VduTrendConfigGenerator.h"
#include "VduTrendSignalsFile.h"

#include "../Context.h"
#include "../UtilsLib/Crc.h"

#include <HardwareLib/DeviceModule.h>
#include <VFrame30/Context.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/ImageItem.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemVduImage.h>
#include <VFrame30/SchemaItemVduImageValue.h>
#include <VFrame30/SchemaItemVduLine.h>
#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/SchemaItemVduTrend.h>
#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/SchemaView.h>
#include <VFrame30/VduSchema.h>

#include <QImageWriter>


// #define VDU_DEBUG

namespace
{
	VduValue toVduValue(float value)
	{
		VduValue result{};
		result.type = VduValue::Type::f32;
		result.data.f32 = value;
		return result;
	}

	VduValue toVduValue(int32_t value)
	{
		VduValue result{};
		result.type = VduValue::Type::i32;
		result.data.i32 = value;
		return result;
	}

	template<typename T>
	VduValue toVduValue(T) = delete;


	std::optional<QByteArray> compileLuaScript(const VFrame30::Schema& schema, QString scriptProperty, Builder::IssueLogger& log)
	{
		std::optional<QByteArray> result;

		auto property = schema.propertyByCaption(scriptProperty);
		if (property == nullptr)
		{
			log.errINT1001(
				QString("compileLuaScript(), Property %1 for schema %2 is not found.").arg(scriptProperty).arg(schema.schemaId()),
				schema.schemaId());
			return result;
		}

		QString script = property->value().toString().trimmed();

		QString errorMessage;
		QByteArray bytecode = Builder::VduLuaScript::compile(script, errorMessage);

		if (errorMessage.isEmpty() == false)
		{
			log.errEQP6302(schema.schemaId(), scriptProperty, -1, errorMessage);
			result = std::nullopt;
		}
		else
		{
			result = bytecode; // Even empty bytecode will init std::optional;
		}

		return result;
	}

	std::optional<QByteArray> compileLuaScript(const VFrame30::SchemaItem& item, QString scriptProperty, Builder::IssueLogger& log)
	{
		std::optional<QByteArray> result;

		auto property = item.propertyByCaption(scriptProperty);
		if (property == nullptr)
		{
			log.errINT1001(QString("compileLuaScript(), Property %1 not found.").arg(scriptProperty),
						   item.parentSchema()->schemaId(),
						   item.label(),
						   item.guid());
			return result;
		}

		QString script = property->value().toString();

		QString errorMessage;
		QByteArray bytecode = Builder::VduLuaScript::compile(script, errorMessage);

		if (errorMessage.isEmpty() == false)
		{
			log.errEQP6303(item.parentSchema()->schemaId(), item.label(), item.guid(), scriptProperty, -1, errorMessage);
			result = std::nullopt;
		}
		else
		{
			result = bytecode; // Even empty bytecode will init std::optional;
		}

		return bytecode;
	}

#if 0
	bool checkLuaScript(const VFrame30::Schema& schema, QString scriptProperty, Builder::IssueLogger& log)
	{
		auto property = schema.propertyByCaption(scriptProperty);
		if (property == nullptr)
		{
			log.errINT1001(QString("checkLuaScript(), Property %1 for schema %2 is not found.").arg(scriptProperty).arg(schema.schemaId()),
						   schema.schemaId());
			return false;
		}

		QString script = property->value().toString();

		QString errorMessage;
		bool scriptIsOk = Builder::VduLuaScript::checkLuaScript(script, errorMessage);

		if (scriptIsOk == false)
		{
			log.errEQP6302(schema.schemaId(), scriptProperty, -1, errorMessage);
		}

		return scriptIsOk;
	}

	bool checkLuaScript(const VFrame30::SchemaItem& item, QString scriptProperty, Builder::IssueLogger& log)
	{
		auto property = item.propertyByCaption(scriptProperty);
		if (property == nullptr)
		{
			log.errINT1001(QString("checkLuaScript(), Property %1 not found.").arg(scriptProperty),
						   item.parentSchema()->schemaId(),
						   item.label(),
						   item.guid());
			return false;
		}

		QString script = property->value().toString();

		QString errorMessage;
		bool scriptIsOk = Builder::VduLuaScript::checkLuaScript(script, errorMessage);

		if (scriptIsOk == false)
		{
			log.errEQP6303(item.parentSchema()->schemaId(), item.label(), item.guid(), scriptProperty, -1, errorMessage);
		}

		return scriptIsOk;
	}
#endif

	struct VduFileString
	{
		QString string;
		uint32_t stringRefOffset = 0; // Offset to the string reference in the file.

		static const uint32_t stub = StringRefStub;

		static VduFileString createUtf8(const QString& string, uint32_t stringRefOffset)
		{
			return VduFileString{.string = string.trimmed(), .stringRefOffset = stringRefOffset};
		}

		static VduFileString createUtf8(const QString& string, size_t stringRefOffset)
		{
			return VduFileString{.string = string.trimmed(), .stringRefOffset = static_cast<uint32_t>(stringRefOffset)};
		}
	};

	struct VduFileLuaBytecode
	{
		QByteArray bytecode;
		uint32_t refOffset = 0; // Offset to the bytecode reference in the file.

		static const uint32_t stub = LuaBytecodeRefStub;

		static VduFileLuaBytecode create(const QByteArray& bytecode, uint32_t stringRefOffset)
		{
			return VduFileLuaBytecode{.bytecode = bytecode, .refOffset = stringRefOffset};
		}

		static VduFileLuaBytecode create(const QByteArray& bytecode, size_t stringRefOffset)
		{
			return VduFileLuaBytecode{.bytecode = bytecode, .refOffset = static_cast<uint32_t>(stringRefOffset)};
		}
	};

	quint32 VduImageHash(const QImage& image)
	{
		const QImage converted = image.convertToFormat(QImage::Format_ARGB32);

		QCryptographicHash hash{QCryptographicHash::Md5};

		for (int y = 0; y < converted.height(); ++y)
		{
			const uchar* line = converted.constScanLine(y);

			QByteArray lineData = QByteArray::fromRawData(reinterpret_cast<const char*>(line), static_cast<int>(converted.bytesPerLine()));
			hash.addData(lineData);
		}

		return ::calcHash32(hash.resultView().constData(), hash.resultView().size());
	}

	// --
	//
	class SaveVduItemVisitor : public VFrame30::VduItemVisitor
	{
		Builder::Context& m_context;
		QString m_vduEquipmentId;
		QString m_subsystemId;
		Builder::IssueLogger& m_log;
		const Builder::VduFontProvider& m_vduFontProvider;
		const std::map<Hash, int>& m_appSignalHashToSignalIndex;
		const std::set<TrendItemSignal>& m_vduTrendSignals;

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

		SaveVduItemVisitor(Builder::Context& context,
						   QString vduEquipmentId,
						   QString subsystemId,
						   const Builder::VduFontProvider& m_vduFontProvider,
						   const std::map<Hash, int>& appSignalHashToSignalIndex,
						   const std::set<TrendItemSignal>& vduTrendSignals) :
			m_context{context},
			m_vduEquipmentId{vduEquipmentId},
			m_subsystemId{subsystemId},
			m_log{*context.m_log},
			m_vduFontProvider{m_vduFontProvider},
			m_appSignalHashToSignalIndex{appSignalHashToSignalIndex},
			m_vduTrendSignals{vduTrendSignals}
		{
		}

		// SchemaItemVduLine
		//
		bool visit(const VFrame30::SchemaItemVduLine& schemaItem) override
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

			return true;
		}

		// SchemaItemVduRect
		//
		bool visit(const VFrame30::SchemaItemVduRect& schemaItem) override
		{
			reset();

			VduSchemaFileSchemaItemRect1 structRect{};

			structRect.version = 1;
			structRect.itemType = VduFileSchemaItemRectId; // ! Do not forget to set itemType.

			using PosType = decltype(structRect.left);
			using SizeType = decltype(structRect.width);

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
				return false;
			}

			structRect.fontIndex = fontIndex;
			structRect.wordWrap = schemaItem.wordWrap();

			structRect.text = VduFileString::stub;

			auto text = VduFileString::createUtf8(schemaItem.text(),
												  sizeof(VduSchemaFileSchemaItem1) + offsetof(VduSchemaFileSchemaItemRect1, text));
			addedStrings.push_back(std::move(text));

			structRect.align = static_cast<int32_t>(schemaItem.horzAlign()) | static_cast<int32_t>(schemaItem.vertAlign());

			itemType = structRect.itemType;
			outData = QByteArray(reinterpret_cast<const char*>(&structRect), sizeof(structRect));

			return true;
		}

		// SchemaItemVduImage
		//
		bool visit(const VFrame30::SchemaItemVduImage& schemaItem) override
		{
			reset();

			VduSchemaFileSchemaItemImage1 structImage{};

			structImage.version = 1;
			structImage.itemType = VduFileSchemaItemImageId; // ! Do not forget to set itemType.

			using PosType = decltype(structImage.left);
			using SizeType = decltype(structImage.width);

			structImage.left = static_cast<PosType>(schemaItem.leftDocPt());
			structImage.top = static_cast<PosType>(schemaItem.topDocPt());
			structImage.width = static_cast<SizeType>(schemaItem.widthDocPt());
			structImage.height = static_cast<SizeType>(schemaItem.heightDocPt());

			// Same image.
			//
			auto image = schemaItem.toQImage(QRectF{0, 0, static_cast<qreal>(structImage.width), static_cast<qreal>(structImage.height)});

			if (image.isNull() == true)
			{
				// SchemaItem %1 has no assigned image in VduSchema %2.
				//
				m_log.errALP4400(schemaItem.parentSchema()->schemaId(), schemaItem.label(), schemaItem.guid());
				return false;
			}

			auto imageHash = quint32{VduImageHash(image)}; // {} Just in case, to prevent narrowing conversion.
			structImage.imageHash = imageHash;

			// If file is not exists then save file to output.
			// '/' in the fron is required for correct work m_buildResultWriter->isBuildFileExists.
			//
			QString subsystemDir = m_context.m_buildResultWriter->subsystemDirectory(m_subsystemId);
			QString vduSchemaImageDir = QString{'/'} + subsystemDir + "/" + m_vduEquipmentId + "/Schemas/Images";

			QString fileName = QString{"%1"}.arg(imageHash, 8, 16, QChar{'0'}).toUpper() + QString{".bmp"};

			bool fileAlreadyExists = m_context.m_buildResultWriter->isBuildFileExists(vduSchemaImageDir + '/' + fileName);
			if (fileAlreadyExists == false)
			{
				QByteArray data;
				QBuffer buffer(&data);
				buffer.open(QIODevice::WriteOnly);

				image.save(&buffer, "bmp");

				m_context.m_buildResultWriter->addFile(vduSchemaImageDir, fileName, data, false);
			}

			// --
			//
			itemType = structImage.itemType;
			outData = QByteArray(reinterpret_cast<const char*>(&structImage), sizeof(structImage));

			return true;
		}

		// SchemaItemVduValue
		//
		bool visit(const VFrame30::SchemaItemVduValue& schemaItem) override
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
				return false;
			}

			structValue.fontIndex = fontIndex;

			structValue.decimalPlaces = schemaItem.precision();

			structValue.align = static_cast<int32_t>(schemaItem.horzAlign()) | static_cast<int32_t>(schemaItem.vertAlign());

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
					return false;
				}

				// Signal index follows the structValue.
				//
				uint32_t signalIndex = static_cast<uint32_t>(sit->second);
				outData.append(reinterpret_cast<const char*>(&signalIndex), sizeof(signalIndex));
			}

			// OutData already set.
			//
			itemType = structValue.itemType;

			return true;
		}

		bool visit(const VFrame30::SchemaItemVduImageValue& schemaItem) override
		{
			reset();

			VduSchemaFileSchemaItemImageValue1 structImageValue{};

			structImageValue.version = 1;
			structImageValue.itemType = VduFileSchemaItemImageValueId;

			using PosType = decltype(structImageValue.left);
			using SizeType = decltype(structImageValue.width);

			structImageValue.left = static_cast<PosType>(schemaItem.leftDocPt());
			structImageValue.top = static_cast<PosType>(schemaItem.topDocPt());
			structImageValue.width = static_cast<SizeType>(schemaItem.widthDocPt());
			structImageValue.height = static_cast<SizeType>(schemaItem.heightDocPt());

			// Images
			//
			structImageValue.imageCount = static_cast<decltype(structImageValue.imageCount)>(schemaItem.images().size());

			// Chech if image count is not greater than size of VduSchemaFileSchemaItemImageValue1::images[]
			//
			if (const auto maxImageCount = sizeof(structImageValue.images) / sizeof(structImageValue.images[0]);
				structImageValue.imageCount > maxImageCount)
			{
				QString schemaId = schemaItem.parentSchema() ? schemaItem.parentSchema()->schemaId() : QString{};
				m_log.errINT1001(QString("Item has more than %1 images.").arg(maxImageCount),
								 schemaId,
								 schemaItem.label(),
								 schemaItem.guid());
				return false;
			}

			const auto& imageItems = schemaItem.images();

			for (size_t i = 0; i < structImageValue.imageCount; i++)
			{
				std::shared_ptr<VFrame30::ImageItem> imageItem = imageItems[i];

				auto& is = structImageValue.images[i];
				is.version = 1;
				is.imageId = VduFileString::stub;

				// imageId
				//
				{
					size_t imageIdOffest = sizeof(VduSchemaFileSchemaItem1) + offsetof(VduSchemaFileSchemaItemImageValue1, images) +
										   sizeof(is) * i + offsetof(VduSchemaFileSchemaItemImageValue1::Image, imageId);

					addedStrings.push_back(VduFileString::createUtf8(imageItem->imageId(), imageIdOffest));
				}

				// imageHash
				//
				QRectF imageRect{0, 0, static_cast<qreal>(structImageValue.width), static_cast<qreal>(structImageValue.height)};
				auto image = imageItem->toQImage(imageRect, schemaItem.fillColor());

				if (image.isNull() == true)
				{
					// SchemaItem %1 has no assigned image in VduSchema %2.
					//
					m_log.errALP4400(schemaItem.parentSchema()->schemaId(), schemaItem.label(), schemaItem.guid());
					return false;
				}

				auto imageHash = quint32{VduImageHash(image)}; // {} Just in case, to prevent narrowing conversion.
				is.imageHash = imageHash;

				// imageFile
				//
				{
					QString fileName = QString{"%1"}.arg(is.imageHash, 8, 16, QChar{'0'}).toUpper() + QString{".bmp"};

					// If file is not exists then save file to output.
					// '/' in the fron is required for correct work m_buildResultWriter->isBuildFileExists.
					//
					QString subsystemDir = m_context.m_buildResultWriter->subsystemDirectory(m_subsystemId);
					QString vduSchemaImageDir =
						QStringLiteral("/") + subsystemDir + QStringLiteral("/") + m_vduEquipmentId + "/Schemas/Images";

					bool fileAlreadyExists = m_context.m_buildResultWriter->isBuildFileExists(vduSchemaImageDir + '/' + fileName);
					if (fileAlreadyExists == false)
					{
						QByteArray data;
						QBuffer buffer(&data);
						buffer.open(QIODevice::WriteOnly);

						image.save(&buffer, "bmp");

						m_context.m_buildResultWriter->addFile(vduSchemaImageDir, fileName, data, false);
					}
				}
			}

			// Set app signal indexes.
			//
			QStringList appSignalIds = schemaItem.signalIds();
			structImageValue.appSignalCount = static_cast<decltype(structImageValue.appSignalCount)>(appSignalIds.size());

			outData = QByteArray(reinterpret_cast<const char*>(&structImageValue), sizeof(structImageValue));

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
					return false;
				}

				// Signal index follows the structValue.
				//
				uint32_t signalIndex = static_cast<uint32_t>(sit->second);
				outData.append(reinterpret_cast<const char*>(&signalIndex), sizeof(signalIndex));
			}

			// OutData already set.
			//
			itemType = structImageValue.itemType;
			return true;
		}

		// SchemaItemVduTrend
		//
		bool visit(const VFrame30::SchemaItemVduTrend& schemaItem) override
		{
			reset();

			VduSchemaFileSchemaItemTrend1 structTrend{};

			structTrend.version = 2;
			structTrend.itemType = VduFileSchemaItemTrendId;

			using PosType = decltype(structTrend.left);
			using SizeType = decltype(structTrend.width);

			structTrend.left = static_cast<PosType>(schemaItem.leftDocPt());
			structTrend.top = static_cast<PosType>(schemaItem.topDocPt());
			structTrend.width = static_cast<SizeType>(schemaItem.widthDocPt());
			structTrend.height = static_cast<SizeType>(schemaItem.heightDocPt());

			using IndentType = decltype(structTrend.indentLeft);
			structTrend.indentLeft = static_cast<IndentType>(schemaItem.indentLeft());
			structTrend.indentRight = static_cast<IndentType>(schemaItem.indentRight());
			structTrend.indentTop = static_cast<IndentType>(schemaItem.indentTop());
			structTrend.indentBottom = static_cast<IndentType>(schemaItem.indentBottom());

			structTrend.columnCount = schemaItem.columnCount();

			const std::size_t MaxExtraDuration =
				sizeof(VduSchemaFileSchemaItemTrend1::durationsSecs) / sizeof(VduSchemaFileSchemaItemTrend1::durationsSecs[0]);

			std::vector<uint32_t> durations = schemaItem.durationsSeconds();
			if (durations.size() == 0) 
			{
				m_log.errINT1001(QString("Internal error, no duration values are set for SchemaItemVduTrend {%1, %2, %3}"),
								 schemaItem.parentSchema()->schemaId(),
								 schemaItem.guid());
				
				reset();
				return false;
			}

			for (std::size_t i = 0; i < durations.size() && i < MaxExtraDuration; i++)
			{
				structTrend.durationsSecs[i] = durations[i];
			}
			
			structTrend.viewMode = static_cast<uint16_t>(schemaItem.viewMode());
			structTrend.scaleType = static_cast<uint16_t>(schemaItem.scaleType());

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
				return false;
			}

			structTrend.fontIndex = fontIndex;

			structTrend.lineColor = schemaItem.lineColor().rgba();
			structTrend.backColor = schemaItem.backColor().rgba();
			structTrend.backColor1st = schemaItem.backColor1st().rgba();
			structTrend.backColor2nd = schemaItem.backColor2nd().rgba();

			structTrend.showSignalIds = schemaItem.showSignalIds();
			structTrend.showSignalCaptions = schemaItem.showSignalCaptions();
			structTrend.showSignalScales = schemaItem.showSignalScales();
			structTrend.showTimeLabels = schemaItem.showTimeLabels();
			structTrend.showDateLabels = schemaItem.showDateLabels();

			// Saving signals
			//
			const std::size_t MaxSignalCount =
				sizeof(VduSchemaFileSchemaItemTrend1::trendSignals) / sizeof(VduSchemaFileSchemaItemTrend1::trendSignals[0]);

			auto trendSignals = schemaItem.signalParams();
			while (trendSignals.size() > MaxSignalCount)
			{
				trendSignals.pop_back();
			}

			bool signalNotFound = false;
			for (const auto& trendSignal : trendSignals)
			{
				// Check that AppSignalID exists.
				//
				{
					Hash signalHash = ::calcHash(trendSignal->appSignalId());
					auto sit = m_appSignalHashToSignalIndex.find(signalHash);

					if (sit == m_appSignalHashToSignalIndex.end())
					{
						// Signal not found.
						//
						m_log.errEQP6400(m_vduEquipmentId,
										 trendSignal->appSignalId(),
										 schemaItem.parentSchema()->schemaId(),
										 schemaItem.label(),
										 schemaItem.guid());
						signalNotFound = true;
					}
				}

				// If Validity AppSignalID is not empty, then it must exist and be a discrete signal.
				//
				if (trendSignal->validityAppSignalId().trimmed().isEmpty() == false)
				{
					Hash signalHash = ::calcHash(trendSignal->validityAppSignalId());
					auto sit = m_appSignalHashToSignalIndex.find(signalHash);

					if (sit == m_appSignalHashToSignalIndex.end())
					{
						// Signal not found.
						//
						m_log.errEQP6400(m_vduEquipmentId,
										 trendSignal->validityAppSignalId(),
										 schemaItem.parentSchema()->schemaId(),
										 schemaItem.label(),
										 schemaItem.guid());
						signalNotFound = true;
					}
					else
					{
						AppSignal* validityAppSignal = m_context.m_signalSet->getSignal(trendSignal->validityAppSignalId());
						if (validityAppSignal == nullptr)
						{
							// Signal not found.
							//
							m_log.errEQP6400(m_vduEquipmentId,
											 trendSignal->validityAppSignalId(),
											 schemaItem.parentSchema()->schemaId(),
											 schemaItem.label(),
											 schemaItem.guid());
							signalNotFound = true;
						}
						else if (validityAppSignal->isDiscrete() == false)
						{
							// Validity AppSignal must be discrete.
							//
							m_log.errEQP6402(m_vduEquipmentId,
											 trendSignal->validityAppSignalId(),
											 schemaItem.parentSchema()->schemaId(),
											 schemaItem.label(),
											 schemaItem.guid());
							signalNotFound = true;
						}
					}
				}
			}

			if (signalNotFound == true)
			{
				reset();
				return false;
			}

			structTrend.maxSignalCount = static_cast<uint16_t>(MaxSignalCount);
			structTrend.signalCount = std::clamp<uint16_t>(static_cast<uint16_t>(trendSignals.size()), 0, structTrend.maxSignalCount);

			structTrend.discreteSignalCount = std::count_if(trendSignals.begin(),
															trendSignals.end(),
															[this](const auto& trendSignal)
															{
																AppSignal* s = m_context.m_signalSet->getSignal(trendSignal->appSignalId());
																return s ? s->isDiscrete() : false;
															});

			size_t signalIndex = 0;
			for (const auto& trendSignal : trendSignals)
			{
				int appSignalIndex = -1;
				int validityAppSignalIndex = -1;
				uint16_t trendItemSignalIndex = 0xFFFF;

				if (auto sit = m_appSignalHashToSignalIndex.find(::calcHash(trendSignal->appSignalId())); //
					sit == m_appSignalHashToSignalIndex.end())
				{
					// We already checked that all signals exist, so this should not happen.
					//
					assert(sit != m_appSignalHashToSignalIndex.end());
					reset();
					return false;
				}
				else
				{
					appSignalIndex = sit->second;
				}

				if (trendSignal->validityAppSignalId().trimmed().isEmpty() == false)
				{
					Hash signalHash = ::calcHash(trendSignal->validityAppSignalId());

					if (auto sit = m_appSignalHashToSignalIndex.find(signalHash); //
						sit == m_appSignalHashToSignalIndex.end())
					{
						// We already checked that all signals exist, so this should not happen.
						//
						assert(sit != m_appSignalHashToSignalIndex.end());
						reset();
						return false;
					}
					else
					{
						validityAppSignalIndex = sit->second;
					}
				}

				// Find index from the set.
				//
				{
					TrendItemSignal tis{};
					tis.appSignalIndex = static_cast<uint32_t>(appSignalIndex);
					tis.validityAppSignalIndex = static_cast<uint32_t>(validityAppSignalIndex);
					tis.durationSecs = static_cast<uint32_t>(schemaItem.durationsSeconds()[0]);
					tis.columnCount = static_cast<uint32_t>(schemaItem.columnCount());

					auto it = m_vduTrendSignals.find(tis);
					if (it == m_vduTrendSignals.end())
					{
						// Internal error, this should not happen, we generated the set from the same signals.
						//
						assert(it != m_vduTrendSignals.end());
						m_log.errINT1001(QString("Internal error, trend signal not found in the set, TrendItemSignal {%1, %2, %3}")
											 .arg(tis.appSignalIndex)
											 .arg(tis.validityAppSignalIndex)
											 .arg(tis.durationSecs),
										 schemaItem.parentSchema()->schemaId(),
										 schemaItem.guid());
						reset();
						return false;
					}

					if (m_vduTrendSignals.size() > 65535)
					{
						// Internal error, too many trend signals.
						//
						m_log.errINT1000(QString("Internal error, too many trend signals, count = %1, vdu = %2")
											 .arg(m_vduTrendSignals.size())
											 .arg(m_vduEquipmentId));
						reset();
						return false;
					}

					trendItemSignalIndex = static_cast<uint16_t>(std::distance(m_vduTrendSignals.begin(), it));
				}

				VduSchemaFileSchemaItemTrend1::TrendSignal& trendSignalStruct = structTrend.trendSignals[signalIndex++];
				trendSignalStruct.version = 1;
				trendSignalStruct.reserve0 = 0;

				trendSignalStruct.appSignalIndex = static_cast<uint32_t>(appSignalIndex);
				trendSignalStruct.validityAppSignalIndex =
					static_cast<uint32_t>(validityAppSignalIndex); // 0xFFFFFFFF if no validity signal is used.
				trendSignalStruct.trendSignalFileRecordIndex = trendItemSignalIndex;

				trendSignalStruct.decimalPlaces = trendSignal->precision();
				trendSignalStruct.valueFormat = static_cast<uint16_t>(trendSignal->valueFormat());

				AppSignal* appSignal = m_context.m_signalSet->getSignal(trendSignal->appSignalId());
				if (appSignal == nullptr)
				{
					// Signal not found.
					//
					m_log.errEQP6400(m_vduEquipmentId,
									 trendSignal->appSignalId(),
									 schemaItem.parentSchema()->schemaId(),
									 schemaItem.label(),
									 schemaItem.guid());
					reset();
					return false;
				}

				if (appSignal->isAnalog() && appSignal->dataFormat() == E::DataFormat::Float)
				{
					trendSignalStruct.highViewLimit = toVduValue(static_cast<float>(trendSignal->highViewLimit()));
					trendSignalStruct.lowViewLimit = toVduValue(static_cast<float>(trendSignal->lowViewLimit()));
				}
				else
				{
					trendSignalStruct.highViewLimit = toVduValue(static_cast<int32_t>(trendSignal->highViewLimit()));
					trendSignalStruct.lowViewLimit = toVduValue(static_cast<int32_t>(trendSignal->lowViewLimit()));
				}

				trendSignalStruct.color = trendSignal->color().rgba();
				trendSignalStruct.lineWeight = static_cast<uint16_t>(trendSignal->lineWeight());
				trendSignalStruct.reserve1 = 0;
			}

			// OutData already set.
			//
			itemType = structTrend.itemType;
			outData = QByteArray(reinterpret_cast<const char*>(&structTrend), sizeof(structTrend));
			return true;
		}
	};

	// Save schema item to the file - actually to the QByteArray.
	//
	bool saveSchemaItem1(QString vduEquipmentId,
						 QString subsystemId,
						 const VFrame30::SchemaItem& schemaItem,
						 const std::map<Hash, int>& appSignalHashToSignalIndex,
						 const std::set<TrendItemSignal>& vduTrendSignals,
						 QByteArray& out,
						 std::list<VduFileString>& addedStrings,
						 std::list<VduFileLuaBytecode>& addedLuaBytecodes,
						 Builder::Context& context)
	{
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
			auto bytecode = compileLuaScript(schemaItem, VFrame30::PropertyNames::clickScript, *context.m_log);
			if (bytecode.has_value() == false)
			{
				return false;
			}

			fileSchemaItem.clickScript = VduFileString::stub;
			fileSchemaItem.clickScriptBytecode = VduFileLuaBytecode::stub;

			auto clickScript = VduFileString::createUtf8(schemaItem.clickScript(), offsetof(VduSchemaFileSchemaItem1, clickScript));
			addedStrings.push_back(std::move(clickScript));

			auto clickScriptBytecode =
				VduFileLuaBytecode::create(bytecode.value(), offsetof(VduSchemaFileSchemaItem1, clickScriptBytecode));
			addedLuaBytecodes.push_back(std::move(clickScriptBytecode));
		}

		// preDrawScript
		//
		{
			auto bytecode = compileLuaScript(schemaItem, VFrame30::PropertyNames::preDrawScript, *context.m_log);
			if (bytecode.has_value() == false)
			{
				return false;
			}

			fileSchemaItem.preDrawScript = VduFileString::stub;
			fileSchemaItem.preDrawScriptBytecode = VduFileLuaBytecode::stub;

			auto preDrawScript = VduFileString::createUtf8(schemaItem.preDrawScript(), offsetof(VduSchemaFileSchemaItem1, preDrawScript));
			addedStrings.push_back(std::move(preDrawScript));

			auto preDrawScriptBytecode =
				VduFileLuaBytecode::create(bytecode.value(), offsetof(VduSchemaFileSchemaItem1, preDrawScriptBytecode));
			addedLuaBytecodes.push_back(std::move(preDrawScriptBytecode));
		}

		// Save specific item struct, depending on itemType.
		//
		SaveVduItemVisitor saveVduItemVisitor(context,
											  vduEquipmentId,
											  subsystemId,
											  context.m_vduFontProvider,
											  appSignalHashToSignalIndex,
											  vduTrendSignals);

		bool saveOk = dynamic_cast<const VFrame30::SchemaItemVdu&>(schemaItem).accept(saveVduItemVisitor);
		if (saveOk == false)
		{
			return false;
		}

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

		bool writeToOut(QByteArray& out) const
		{
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
				while ((container.size() % padding) != 0)
				{
					container.push_back(char{0});
				}
			};

			addPadding(out, 4);

			for (const auto& str : m_strings)
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
				for (const auto& stringData : offsets(str))
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

			return true;
		}

	private:
		using Key = QString;

		std::multimap<Key, VduFileString> m_offsetToString;
		std::set<Key> m_strings;
	};

	// Accumulate string references and strings, then write them to the file.
	//
	class VduLuaBytecodeWriter
	{
	public:
		using Offset = uint32_t;

		void clear() { *this = {}; }

		Offset add(const QByteArray& data, Offset offset)
		{
			auto record = VduFileLuaBytecode::create(data, offset);

			m_offsetToBytecode.insert(std::pair{Key{data}, record});
			m_bytecodes.insert(data);

			return VduFileLuaBytecode::stub;
		}

		std::vector<QByteArray> allData() const { return {m_bytecodes.begin(), m_bytecodes.end()}; }

		std::vector<VduFileLuaBytecode> offsets(const QByteArray& bytecode) const
		{
			Key key{bytecode};

			auto [beginIt, endIt] = m_offsetToBytecode.equal_range(key);

			std::vector<VduFileLuaBytecode> result(std::distance(beginIt, endIt));

			std::transform(beginIt,
						   endIt,
						   result.begin(),
						   [](const auto& p)
						   {
							   return p.second;
						   });

			return result;
		}

		bool writeToOut(QByteArray& out) const
		{
			// Resolve bytecodes:
			// Bytecode consist of 32 bit size, followed with data. Padding to 4 bytes.
			// Aligned to 4 bytes.
			//

			// Align to 4 bytes the beginning of string area.
			//
			auto addPadding = [](auto& container, size_t padding)
			{
				while ((container.size() % padding) != 0)
				{
					container.push_back(char{0});
				}
			};

			addPadding(out, 4);

			for (const auto& bc : m_bytecodes)
			{
				auto bytecodeOffset = static_cast<vdu_scriptbc>(out.size());

				// Write size.
				//
				uint32_t bytecodeSize = static_cast<uint32_t>(bc.size());
				out.append(reinterpret_cast<const char*>(&bytecodeSize), sizeof(bytecodeSize));

				// Write data.
				//
				out.append(bc);

				// Replace refs with offset to the data.
				//
				for (const auto& bytecodeRecord : offsets(bc))
				{
					out.replace(bytecodeRecord.refOffset,
								sizeof(vdu_scriptbc),
								reinterpret_cast<const char*>(&bytecodeOffset),
								sizeof(bytecodeOffset));
				}

				addPadding(out, 4);
			}

			return true;
		}

	private:
		using Key = QByteArray;

		std::multimap<Key, VduFileLuaBytecode> m_offsetToBytecode;
		std::set<Key> m_bytecodes;
	};

} // namespace

namespace Builder
{
	bool VduSchemaGenerator::generateVduSchemas(const std::vector<VFrame30::VduSchema*>& schemas, Context& context)
	{
		IssueLogger* log = context.m_log;
		Q_ASSERT(log);

		context.m_vduSchemas.clear();
		auto& buildResultWriter = *context.m_buildResultWriter;

		bool result = true;

		auto isVduModule = [](Hardware::DeviceModule* module)
		{
			return module->isVdu();
		};

		struct VduInfo
		{
			const Hardware::DeviceModule* vdu = nullptr;
			QString subsystemId;

			QString vduDir;
			QString vduSchemaDir;
			QString vduImageDir;

			QStringList vduSchemaTagList;
			std::map<Hash, int> vduSignals;
			std::set<TrendItemSignal> vduTrendSignals;

			std::vector<VFrame30::VduSchema*> vduSchemas;
		};

		std::list<VduInfo> vdus;

		for (const Hardware::DeviceModule* vdu : context.m_fscModules | std::views::filter(isVduModule))
		{
			Q_ASSERT(vdu);

			VduInfo vduInfo;
			vduInfo.vdu = vdu;

			// Get VDU subsystemId.
			//
			if (auto subsystemIdProp = vdu->propertyByCaption(EquipmentPropNames::SUBSYSTEM_ID); //
				subsystemIdProp == nullptr)
			{
				// Property '%1.%2' is not found.
				//
				log->errCFG3020(vdu->equipmentId(), EquipmentPropNames::SUBSYSTEM_ID);
				result = false;
				continue;
			}
			else
			{
				vduInfo.subsystemId = subsystemIdProp->value().toString();
			}

			vduInfo.vduDir = buildResultWriter.subsystemDirectory(vduInfo.subsystemId) + '/' + vdu->equipmentId();
			vduInfo.vduSchemaDir = vduInfo.vduDir + '/' + "Schemas";
			vduInfo.vduImageDir = vduInfo.vduSchemaDir + '/' + "Images";

			// Get device tags
			//
			auto schemaTagsProperty = vdu->propertyByCaption(EquipmentPropNames::SCHEMA_TAGS);
			if (schemaTagsProperty == nullptr)
			{
				// Property '%1.%2' is not found.
				//
				log->errCFG3020(vdu->equipmentId(), EquipmentPropNames::SCHEMA_TAGS);
				result = false;
				continue;
			}

			vduInfo.vduSchemaTagList = schemaTagsProperty->value().toString().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);

			auto vduSignalsIt = context.m_vduSignals.find(vdu->equipmentId());
			if (vduSignalsIt == context.m_vduSignals.end())
			{
				// Signals for VDU %1 are not found.
				//
				log->errINT1000(QString("Internal error: VduSignals structure is not found for VDU %1").arg(vdu->equipmentId()));
				result = false;
				continue;
			}

			vduInfo.vduSignals = vduSignalsIt->second; // Deep copy.

			auto schemaFilterFunc = [&vduInfo](VFrame30::VduSchema* schema)
			{
				if (schema == nullptr)
				{
					assert(schema);
					return false;
				}

				// If schemaTags is empty, then all schemas are for this VDU
				//
				bool schemaHasTag = vduInfo.vduSchemaTagList.isEmpty();
				schemaHasTag |= std::any_of(vduInfo.vduSchemaTagList.begin(),
											vduInfo.vduSchemaTagList.end(),
											[&schema](const QString& tag)
											{
												return schema->tagsAsList().contains(tag.toLower());
											});
				return schemaHasTag;
			};

			// Get all schemas.
			//
			vduInfo.vduSchemas.reserve(schemas.size());
			std::copy_if(schemas.begin(), schemas.end(), std::back_inserter(vduInfo.vduSchemas), schemaFilterFunc);

			// Add to the list of VDU info.
			//
			vdus.push_back(std::move(vduInfo));
		}

		if (result == false)
		{
			return false;
		}

		// Generate trends for VDU.
		//
		for (VduInfo& vduInfo : vdus)
		{
			bool trendGenOk = Builder::VduTrendConfigGenerator::generate(vduInfo.vdu->equipmentId(),
																		 vduInfo.vduDir,
																		 vduInfo.vduSchemas,
																		 vduInfo.vduSignals,
																		 vduInfo.vduTrendSignals, // Out
																		 context);
			if (trendGenOk == false)
			{
				result = false;
			}
		}

		// Generate schemas for VDU.
		//
		for (const VduInfo& vduInfo : vdus)
		{
			Q_ASSERT(vduInfo.vdu);
			LOG_MESSAGE(log, QString("Generating schemas for VDU %1.").arg(vduInfo.vdu->equipmentId()));

			// Generate schemas for VDU.
			//
			for (auto schema : vduInfo.vduSchemas)
			{
				auto schemaPtr = std::dynamic_pointer_cast<VFrame30::Schema>(schema->shared_from_this());

				// Generate VDU schema.
				//
				LOG_MESSAGE(log, QString("Converting schema %1 to VDU format.").arg(schema->schemaId()));

				QStringList errorMessages;
				QByteArray nativeVduData;

				bool genSchemaOk = Builder::VduSchemaGenerator::generateVduSchema(vduInfo.vdu->equipmentId(),
																				  vduInfo.subsystemId,
																				  *schema,
																				  vduInfo.vduSignals,
																				  vduInfo.vduTrendSignals,
																				  nativeVduData,
																				  context);

				if (genSchemaOk == false)
				{
					result = false;
					continue;
				}

				// Save result.
				//
				QString nativeVduSchemaFileName = QString("%1.%2").arg(schema->schemaId()).arg(File::VduNativeFileExtension);

				buildResultWriter.addFile(vduInfo.vduSchemaDir, nativeVduSchemaFileName, nativeVduData);

#if 1
				// Generate background bitmap from the static data.
				//
				{
					QByteArray backgroundImageData;
					QString backgroundBitmapFileName = QString("%1.bmp").arg(schema->schemaId());

					QImage backgroundImage;
					bool genBitmapOk = Builder::VduSchemaGenerator::generateVduBackgroundBitmap(schemaPtr, backgroundImage);

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

					buildResultWriter.addFile(vduInfo.vduSchemaDir, backgroundBitmapFileName, backgroundImageData);
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

					Context::GeneratedVduSchema generatedSchema{.schema = schemaPtr, .crc64 = crc64};

					context.m_vduSchemas[vduInfo.vdu->equipmentId()].push_back(generatedSchema);
				}
			}
		}

		return result;
	}

	bool VduSchemaGenerator::generateVduSchema(QString vduEquipmentId,
											   QString subsystemId,
											   const VFrame30::VduSchema& schema,
											   const std::map<Hash, int>& appSignalHashToSignalIndex,
											   const std::set<TrendItemSignal>& vduTrendSignals,
											   QByteArray& out,
											   Builder::Context& context)
	{
		Builder::IssueLogger& log = *context.m_log;

		bool result = true;

		VduStringWriter stringWriter;
		VduLuaBytecodeWriter luaBytecodeWriter;

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
			auto showScriptBytecode = compileLuaScript(schema, VFrame30::PropertyNames::onShowScript, *context.m_log);
			auto preDrawScriptBytecode = compileLuaScript(schema, VFrame30::PropertyNames::preDrawScript, *context.m_log);

			if (showScriptBytecode.has_value() == false || preDrawScriptBytecode.has_value() == false)
			{
				return false;
			}

			VduSchemaFileProperties1& schemaProperties = file.schemaProperties;
			schemaProperties.version = 1;
			schemaProperties.headerSize = sizeof(schemaProperties);
			schemaProperties.width = static_cast<uint16_t>(schema.docWidth());
			schemaProperties.height = static_cast<uint16_t>(schema.docHeight());
			schemaProperties.reserve0 = 0;
			schemaProperties.backgroundColor = schema.backgroundColor().rgba();

			const auto schemaPropertiesOffset = offsetof(VduSchemaFile, schemaProperties);

			schemaProperties.schemaId =
				stringWriter.addString(schema.schemaId(), schemaPropertiesOffset + offsetof(VduSchemaFileProperties1, schemaId));

			schemaProperties.caption =
				stringWriter.addString(schema.caption(), schemaPropertiesOffset + offsetof(VduSchemaFileProperties1, caption));

			// onShowScript
			{
				auto fieldOffset = schemaPropertiesOffset + offsetof(VduSchemaFileProperties1, onShowScript);
				schemaProperties.onShowScript = stringWriter.addString(schema.onShowScript(), fieldOffset);
			}

			// onShowScriptBytecode
			{
				auto fieldOffset = schemaPropertiesOffset + offsetof(VduSchemaFileProperties1, onShowScriptBytecode);
				schemaProperties.onShowScriptBytecode = luaBytecodeWriter.add(showScriptBytecode.value(), fieldOffset);
			}

			// preDrawScript
			{
				auto fieldOffset = schemaPropertiesOffset + offsetof(VduSchemaFileProperties1, preDrawScript);
				schemaProperties.preDrawScript = stringWriter.addString(schema.preDrawScript(), fieldOffset);
			}

			// preDrawScriptBytecode
			{
				auto fieldOffset = schemaPropertiesOffset + offsetof(VduSchemaFileProperties1, preDrawScriptBytecode);
				schemaProperties.preDrawScriptBytecode = luaBytecodeWriter.add(preDrawScriptBytecode.value(), fieldOffset);
			}

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
				std::list<VduFileString> addedItemStrings;       // Strings added in the fallowing call of saveSchemaItem1(...).
				std::list<VduFileLuaBytecode> addedLuaBytecodes; // Lua bytecodes added in the fallowing call of saveSchemaItem1(...).

				saveSchemaItem1(vduEquipmentId,
								subsystemId,
								*item,
								appSignalHashToSignalIndex,
								vduTrendSignals,
								outSchemaItem,
								addedItemStrings,
								addedLuaBytecodes,
								context);

				// Add added string references to the main string ref container.
				//
				for (const auto& str : addedItemStrings)
				{
					context.m_vduFontProvider.checkStringForUnicodeSubsets(vduEquipmentId, str.string, log);
					stringWriter.addString(str.string, str.stringRefOffset + out.size());
				}

				// Add added Lua bytecode references to the main bytecode ref container.
				//
				for (const auto& bc : addedLuaBytecodes)
				{
					luaBytecodeWriter.add(bc.bytecode, out.size() + bc.refOffset);
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

		// Add strings to the output buffer.
		//
		stringWriter.writeToOut(out);

		// Add compiled Lua bytecodes to the output buffer.
		//
		luaBytecodeWriter.writeToOut(out);

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