#include "VduTrendConfigGenerator.h"
#include "Context.h"
#include "VduTrendSignalsFile.h"

#include <VFrame30/SchemaItemVduTrend.h>
#include <VFrame30/VduSchema.h>

#include "../UtilsLib/Crc.h"

#include <cstring>

namespace Builder
{
	bool VduTrendConfigGenerator::generate(QString vduEquipmentId,
										   QString vduDir,
										   const std::vector<VFrame30::VduSchema*>& schemas,
										   const std::map<Hash, int>& appSignalHashToSignalIndex,
										   std::set<TrendItemSignal>& outTrendSignals,
										   Builder::Context& context)
	{
		assert(vduEquipmentId.isEmpty() == false);
		assert(vduDir.isEmpty() == false);

		bool result = true;
		std::set<TrendItemSignal> vduTrendRecords;

		// Find all VDU trend items and save their parameters to vduTrendRecords
		//
		for (auto schema : schemas)
		{
			assert(schema);

			for (auto layer : schema->layers())
			{
				assert(layer);

				for (auto schemaItem : layer->items())
				{
					assert(schemaItem);

					auto trendItem = schemaItem->toType<VFrame30::SchemaItemVduTrend>();
					if (trendItem == nullptr)
					{
						continue;
					}

					for (const auto& trendSignal : trendItem->signalParams())
					{
						TrendItemSignal record{};

						// Find AppSignalIndex
						//
						if (auto sit = appSignalHashToSignalIndex.find(::calcHash(trendSignal->appSignalId()));
							sit == appSignalHashToSignalIndex.end())
						{
							// Signal not found.
							//
							context.m_log->errEQP6400(vduEquipmentId,
													  trendSignal->appSignalId(),
													  schemaItem->parentSchema()->schemaId(),
													  schemaItem->label(),
													  schemaItem->guid());

							result = false;
							continue;
						}
						else
						{
							record.appSignalIndex = static_cast<uint32_t>(sit->second);
						}

						// Find ValidityAppSignalIndex
						//
						if (trendSignal->validityAppSignalId().isEmpty() == false)
						{
							auto sit = appSignalHashToSignalIndex.find(::calcHash(trendSignal->validityAppSignalId()));
							if (sit == appSignalHashToSignalIndex.end())
							{
								// Signal not found.
								//
								context.m_log->errEQP6400(vduEquipmentId,
														  trendSignal->validityAppSignalId(),
														  schemaItem->parentSchema()->schemaId(),
														  schemaItem->label(),
														  schemaItem->guid());
								result = false;
								continue;
							}
							else
							{
								record.validityAppSignalIndex = static_cast<uint32_t>(sit->second);
							}
						}
						else
						{
							record.validityAppSignalIndex = 0xFFFFFFFF; // No validity signal
						}

						// Set DurationSecs
						//
						record.durationSecs = static_cast<uint32_t>(trendItem->durationSeconds());
						record.columnCount = static_cast<uint32_t>(trendItem->columnCount());
						record.reserve = 0;

						vduTrendRecords.insert(record);
					}
				}
			}
		}

		if (result == false)
		{
			return false;
		}

		// Save all VDU trends to a file in build result to file File::VDU_TREND_SIGNALS
		//
		QByteArray data;
		data.resize(sizeof(TrendItemSignalsHeader) + vduTrendRecords.size() * sizeof(TrendItemSignal) + sizeof(uint64_t));
		data.fill(0);

		TrendItemSignalsHeader header{};
		header.version = 2;
		header.recordSize = sizeof(TrendItemSignal);
		header.count = static_cast<uint32_t>(vduTrendRecords.size());
		header.reserve = 0;

		std::memcpy(data.data(), &header, sizeof(header));

		auto* out = data.data() + sizeof(TrendItemSignalsHeader);
		for (const auto& record : vduTrendRecords)
		{
			std::memcpy(out, &record, sizeof(record));
			out += sizeof(record);
		}

		// Write CRC64 of the file
		//
		quint64 crc = qToBigEndian(Crc::crc64(data.constData(), data.size() - sizeof(quint64)));
		data.replace(data.size() - sizeof(quint64), sizeof(quint64), reinterpret_cast<const char*>(&crc), sizeof(quint64));

		// Check crc, crc on data with crc field must be 0.
		//
		quint64 checkCrc = Crc::crc64(data.constData(), data.size());
		if (checkCrc != 0)
		{
			Q_ASSERT(checkCrc == 0);
			context.m_log->errINT1000("Internal error: VduTrendConfigGenerator::generate(...) CRC64 check failed!");
			return false;
		}

		// Save file to build output.
		//
		context.m_buildResultWriter->addFile(vduDir, File::VDU_TREND_SIGNALS, data);

		// Save all VDU trend signals
		//
		outTrendSignals = std::move(vduTrendRecords);

		return result;
	}
} // namespace Builder