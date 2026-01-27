#include <AdsGatewayLib/GwCrc32.hpp>

#include <gtest/gtest.h>

#include <array>


TEST(GwCrc32Tests, SimpleCrc10)
{
	std::array<std::byte, 9> data{std::byte{'1'},
								  std::byte{'2'},
								  std::byte{'3'},
								  std::byte{'4'},
								  std::byte{'5'},
								  std::byte{'6'},
								  std::byte{'7'},
								  std::byte{'8'},
								  std::byte{'9'}};
	EXPECT_EQ(Radiy::CRC32(std::span{data}), 0xCBF43926);

	const char text[] = "123456789";
	EXPECT_EQ(Radiy::CRC32(std::span{text, 9}), 0xCBF43926);
	EXPECT_EQ(Radiy::CRC32(text, 9), 0xCBF43926);
}

TEST(GwCrc32Tests, CrcPartition)
{
	std::array<std::byte, 5> data1{std::byte{'1'}, std::byte{'2'}, std::byte{'3'}, std::byte{'4'}, std::byte{'5'}};
	std::array<std::byte, 4> data2{std::byte{'6'}, std::byte{'7'}, std::byte{'8'}, std::byte{'9'}};

	auto crcPart1 = Radiy::CRC32(std::span{data1}, false);
	auto crcFinal = Radiy::CRC32(std::span{data2}, true, crcPart1);
	EXPECT_EQ(crcFinal, 0xCBF43926);

	const char text1[] = "12345";
	const char text2[] = "6789";
	auto crcPart1Text = Radiy::CRC32(std::span{text1, 5}, false);
	auto crcFinalText = Radiy::CRC32(std::span{text2, 4}, true, crcPart1Text);
	EXPECT_EQ(crcFinalText, 0xCBF43926);
}

TEST(GwCrc32Tests, CrcOverResude)
{
	std::array<std::byte, 13> data{std::byte{'1'},
								   std::byte{'2'},
								   std::byte{'3'},
								   std::byte{'4'},
								   std::byte{'5'},
								   std::byte{'6'},
								   std::byte{'7'},
								   std::byte{'8'},
								   std::byte{'9'},
								   std::byte{0x26}, // Calculated CRC appended in little-endian
								   std::byte{0x39},
								   std::byte{0xF4},
								   std::byte{0xCB}};

	// Crc32Residue = 0x2144DF1C
	//
	EXPECT_EQ(Radiy::CRC32(std::span{data}), Radiy::Crc32Residue);

	const char text[] = "123456789\x26\x39\xF4\xCB";
	EXPECT_EQ(Radiy::CRC32(std::span{text, 13}), Radiy::Crc32Residue);
}