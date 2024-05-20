#include "VduConfigFile.h"
#include "../Context.h"

#include "../../UtilsLib/Crc.h"
#include "../../lib/ConstStrings.h"

#include <HardwareLib/DeviceModule.h>
#include <VFrame30/Schema.h>

namespace
{
	const QString VduConfigFileName = "VduConfig.bin";
	const QString VduConfigDumpFileName = "VduConfig.dump";

	/// \brief Generates the VDU config file for a specific VDU device.
	/// \param vdu The VDU device module.
	/// \param context The builder context.
	/// \param out The generated file data.
	/// \return True if the generation is successful, false otherwise.
	bool generateVduConfig(const Hardware::DeviceModule& vdu, Builder::Context& context, QByteArray& out)
	{
		using namespace Builder;

		auto log = context.m_log;

		std::multimap<QString, size_t> strings; // string -> referenceOffset

		auto addStringRef = [&strings](const QString& str, size_t offset) -> vdu_string_ref
		{
			strings.insert(std::make_pair(str, offset));
			return StringRefStub;               // "STRR" - for debug, easy to find in hex editor.
		};

		// --
		//
		out.clear();
		out.reserve(sizeof(VduConfigFile1) + 8);

		// --
		//
		{
			VduConfigFile1 header{};

			header.magic[0] = 'V'; // "VDUCFG1";
			header.magic[1] = 'D';
			header.magic[2] = 'U';
			header.magic[3] = 'C';
			header.magic[4] = 'F';
			header.magic[5] = 'G';
			header.magic[6] = '1';
			header.magic[7] = 0;

			header.version = 1;
			header.size = sizeof(VduConfigFile1);

			header.project = addStringRef(context.m_db.currentProject().projectName(), offsetof(VduConfigFile1, project));
			header.buildNo = context.m_buildResultWriter->buildInfo().id;

			header.equipmentId = addStringRef(vdu.equipmentId(), offsetof(VduConfigFile1, equipmentId));
			header.caption = addStringRef(vdu.caption(), offsetof(VduConfigFile1, caption));

			// For now displayCount is always 1, 1280x720, mode 0.
			header.displayCount = 1;

			// Display properties are taken from vdu module.
			//
			auto propertyByCaption = [&vdu, &log]<typename TYPE>(const QString& caption, const TYPE& defaultValue) -> TYPE
			{
				auto p = vdu.propertyByCaption(caption);
				if (p == nullptr)
				{
					log->errCFG3000(caption, vdu.equipmentId());
					return defaultValue;
				}

				return p->value().value<TYPE>();
			};

			header.display0.width = propertyByCaption("Display0_Width", 0);
			header.display0.height = propertyByCaption("Display0_Height", 0);
			header.display0.mode = 0;

			header.display0.startSchemaId = addStringRef(propertyByCaption("Display0_StartSchemaID", QString()),
														 offsetof(VduConfigFile1, VduConfigFile1::display0.startSchemaId));
			header.display0.displayName =
				addStringRef(propertyByCaption("Display0_Name", QString()), offsetof(VduConfigFile1, VduConfigFile1::display0.displayName));

			// Write font count.
			//
			header.fontCount = context.m_vduFontProvider.getFontCount(vdu.equipmentId());

			// Write schema count.
			//
			header.schemaCount = static_cast<decltype(header.schemaCount)>(context.m_vduSchemas[vdu.equipmentId()].size());

			// Write header struct to the output buffer.
			//
			out.append(reinterpret_cast<const char*>(&header), sizeof(header));
		}

		// Write schemas short description.
		//
		{
			auto vduSchemas = context.m_vduSchemas[vdu.equipmentId()];

			vduSchemas.sort(
				[](const Context::GeneratedVduSchema& a, const Context::GeneratedVduSchema& b) -> bool
				{
					Q_ASSERT(a.schema);
					Q_ASSERT(b.schema);
					return a.schema->caption() < b.schema->caption();
				});

			for (const Context::GeneratedVduSchema& generatedSchema : vduSchemas)
			{
				Q_ASSERT(generatedSchema.schema);

				VduConfigSchema1 schema1{};

				schema1.version = 1; // Schema struct version.
				schema1.size = sizeof(VduConfigSchema1);

				schema1.schemaId = addStringRef(generatedSchema.schema->schemaId(), out.size() + offsetof(VduConfigSchema1, schemaId));
				schema1.caption = addStringRef(generatedSchema.schema->caption(), out.size() + offsetof(VduConfigSchema1, caption));

				schema1.crc64 = generatedSchema.crc64;

				// Write schema struct to the output buffer.
				//
				out.append(reinterpret_cast<const char*>(&schema1), sizeof(schema1));
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
			log->errINT1000("Internal error: VduConfigFileWriter::generate(...) CRC64 check failed!");
			return false;
		}

		return true;
	}

	/// \brief Dumps the binary data to a text format.
	/// \param data The data to be dumped.
	/// \return The dumped data as a string.
	///
	/// This function receives a QByteArray with the content of the file and returns a QString with the dump of the file.
	QString dumpVduConfig(QByteArray& data)
	{
		using namespace Builder;

		if (data.size() < sizeof(VduConfigFile1) + 8)
		{
			return QString("Invalid file size");
		}

		auto getStringRefValue = [](const QByteArray& data, vdu_string_ref ref) -> QString
		{
			// string_ref is an offset in a file to a string.
			// String consist of 16 bit size of string in symbols, followed with string data. Padding to 4 bytes.
			// The string is a null terminated QChar string.
			//
			QString result;

			if (ref == StringRefStub)
			{
				Q_ASSERT(ref != StringRefStub);
				result = QString("Error: StringRefStub detected, offset %1").arg(ref);
				return result;
			}

			uint32_t offset = ref;
			if (offset == 0)
			{
				result = "NULL";
				return result;
			}

			if (offset >= data.size())
			{
				result = QString("Error: Offset %1 is out of file size %2").arg(offset).arg(data.size());
				return result;
			}

			uint16_t size = *reinterpret_cast<const uint16_t*>(data.constData() + offset);
			result = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData() + offset + 2), size);

			return result;
		};

		const VduConfigFile1* header = reinterpret_cast<const VduConfigFile1*>(data.constData());

		auto printAddress = [](int indent, auto offset, auto size) -> QString
		{
			return QString{"%1%2:%3 "}.arg(QString(indent, ' ')).arg(offset, 4, 16, QChar{'0'}).arg(size, 4, 16, QChar{'0'});
		};

		QString result;

		result += printAddress(0, offsetof(VduConfigFile1, magic), sizeof(header->magic));
		result += QString("Magic: %1\\0\n").arg(QString::fromLatin1(header->magic, 7));

		result += printAddress(0, offsetof(VduConfigFile1, version), sizeof(header->version));
		result += QString("StructVersion: %1\n").arg(header->version);

		result += printAddress(0, offsetof(VduConfigFile1, size), sizeof(header->size));
		result += QString("HeaderSize: %1\n").arg(header->size);

		result += printAddress(0, offsetof(VduConfigFile1, project), sizeof(header->project));
		auto project = getStringRefValue(data, header->project);
		result += QString("Project: %1\n").arg(project);

		result += printAddress(0, offsetof(VduConfigFile1, buildNo), sizeof(header->buildNo));
		result += QString("BuildNo: %1\n").arg(header->buildNo);

		result += printAddress(0, offsetof(VduConfigFile1, equipmentId), sizeof(header->equipmentId));
		auto equipmentId = getStringRefValue(data, header->equipmentId);
		result += QString("EquipmentId: %1\n").arg(equipmentId);

		result += printAddress(0, offsetof(VduConfigFile1, caption), sizeof(header->caption));
		auto caption = getStringRefValue(data, header->caption);
		result += QString("Caption: %1\n").arg(caption);

		result += printAddress(0, offsetof(VduConfigFile1, displayCount), sizeof(header->displayCount));
		result += QString("DisplayCount: %1\n").arg(header->displayCount);

		// File contains data for 4 displays, but used ony header->displayCount displays
		//
		for (int i = 0; i < VduConfigFile1::MaxDisplayCount /*header->displayCount*/; i++)
		{
			const VduConfigDisplay1* display = &header->display0 + i;

			result += QString("\nDisplay %1\n").arg(i);

			result += printAddress(0, sizeof(VduConfigDisplay1) * i + offsetof(VduConfigDisplay1, width), sizeof(display->width));
			result += QString("Width: %1\n").arg(display->width);

			result += printAddress(0, sizeof(VduConfigDisplay1) * i + offsetof(VduConfigDisplay1, height), sizeof(display->height));
			result += QString("Height: %1\n").arg(display->height);

			result += printAddress(0, sizeof(VduConfigDisplay1) * i + offsetof(VduConfigDisplay1, mode), sizeof(display->mode));
			result += QString("Mode: %1\n").arg(display->mode);

			result +=
				printAddress(0, sizeof(VduConfigDisplay1) * i + offsetof(VduConfigDisplay1, startSchemaId), sizeof(display->startSchemaId));
			auto startSchemaId = getStringRefValue(data, display->startSchemaId);
			result += QString("StartSchemaID: %1\n").arg(startSchemaId);

			result +=
				printAddress(0, sizeof(VduConfigDisplay1) * i + offsetof(VduConfigDisplay1, displayName), sizeof(display->displayName));
			auto displayName = getStringRefValue(data, display->displayName);
			result += QString("DisplayName: %1\n").arg(displayName);
		}

		result += QString("\n");

		result += printAddress(0, offsetof(VduConfigFile1, fontCount), sizeof(header->fontCount));
		result += QString("FontCount: %1\n").arg(header->fontCount);

		result += QString("\n");

		result += printAddress(0, offsetof(VduConfigFile1, schemaCount), sizeof(header->schemaCount));
		result += QString("SchemaCount: %1\n").arg(header->schemaCount);

		// Schemas
		//
		for (int i = 0; i < header->schemaCount; i++)
		{
			const VduConfigSchema1* schema =
				reinterpret_cast<const VduConfigSchema1*>(data.constData() + header->size + i * sizeof(VduConfigSchema1));
			auto schemaOffset = header->size + i * sizeof(VduConfigSchema1);

			result += QString("\nSchema %1\n").arg(i);

			result += printAddress(0, schemaOffset + offsetof(VduConfigSchema1, version), sizeof(schema->version));
			result += QString("  StructVersion: %1\n").arg(schema->version);

			result += printAddress(0, schemaOffset + offsetof(VduConfigSchema1, size), sizeof(schema->size));
			result += QString("  StructSize: %1\n").arg(schema->size);

			result += printAddress(0, schemaOffset + offsetof(VduConfigSchema1, schemaId), sizeof(schema->schemaId));
			auto schemaId = getStringRefValue(data, schema->schemaId);
			result += QString("  SchemaID: %1\n").arg(schemaId);

			result += printAddress(0, schemaOffset + offsetof(VduConfigSchema1, caption), sizeof(schema->caption));
			auto schemaCaption = getStringRefValue(data, schema->caption);
			result += QString("  Caption: %1\n").arg(schemaCaption);

			result += printAddress(0, schemaOffset + offsetof(VduConfigSchema1, crc64), sizeof(schema->crc64));
			result += QString("  CRC64: 0x%1\n").arg(schema->crc64, 16, 16);
		}

		// Check crc, crc on data with crc field must be 0.
		//
		result += QString("\n");

		quint64 crcFromFile = *reinterpret_cast<const quint64*>(data.constData() + data.size() - sizeof(quint64));
		result += printAddress(0, data.size() - sizeof(crcFromFile), sizeof(crcFromFile));
		result += QString("File CRC64: 0x%1\n").arg(crcFromFile, 16, 16, QChar{'0'});

		result += QString("\n");

		quint64 checkCrc = Crc::crc64(data.constData(), data.size());
		if (checkCrc != 0)
		{
			result += QString("CRC64 applied for file (including CRC64), FAILED - 0x%1\n").arg(checkCrc, 16, 16, QChar{'0'});
		}
		else
		{
			result += QString("CRC64 applied for file (including CRC64), Ok - 0x%1\n").arg(checkCrc, 16, 16, QChar{'0'});
		}

		return result;
	}
} // namespace

namespace Builder
{
	bool VduConfigFileWriter::generate(Builder::Context& context)
	{
		IssueLogger* log = context.m_log;
		Q_ASSERT(log);

		bool result = true;

		for (const Hardware::DeviceModule* vdu : context.m_vduModules)
		{
			Q_ASSERT(vdu);

			LOG_MESSAGE(log, QString("Generating configuration for VDU %1.").arg(vdu->equipmentId()));

			QByteArray out;
			bool ok = generateVduConfig(*vdu, context, out);

			if (ok == true)
			{
				auto addedFile = context.m_buildResultWriter->addFile("/VDUs/" + vdu->equipmentId(), VduConfigFileName, out, false);
				ok &= addedFile != nullptr;

				// Write dump file
				//
				QString configDump = dumpVduConfig(out);
				addedFile = context.m_buildResultWriter->addFile("/VDUs/" + vdu->equipmentId(), VduConfigDumpFileName, configDump, false);

				ok &= addedFile != nullptr;
			}

			result &= ok;
		}

		return result;
	}
} // namespace Builder