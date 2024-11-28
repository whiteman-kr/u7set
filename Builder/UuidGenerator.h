#pragma once

namespace Builder
{
	struct UuidGenerator
	{
		Hash hash{};
		uint64_t counter{};

		// Set new hash and reset counter
		//
		void init(const QString& id)
		{
			hash = ::calcHash(id);
			counter = 0;
		}

		// Combine hash and counter into QUuid
		//
		QUuid next()
		{
			Q_ASSERT(hash != 0ull);

			std::array<uint8_t, 16> uuidData;
			std::memcpy(uuidData.data(), &hash, sizeof(hash));

			counter++;
			std::memcpy(uuidData.data() + sizeof(hash), &counter, sizeof(counter));

			return QUuid::fromRfc4122(QByteArray::fromRawData(reinterpret_cast<const char*>(uuidData.data()), 16));
		}
	};
} // namespace Builder