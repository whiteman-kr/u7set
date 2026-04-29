//#include <GatewayClientLib/../../src/TuningSources.hpp>
//
//#include <fstream>
//
//#include <gtest/gtest.h>
//
//TEST(TuningSourcesTests, ParsesDefaultProfileAndSignals)
//{
//	// Read TuningSource.xml content
//	//
//	std::ifstream file{"TestTuningSource.xml", std::ios::binary};
//	ASSERT_TRUE(file.is_open()) << "Failed to open TestTuningSource.xml";
//
//	std::stringstream buffer;
//	buffer << file.rdbuf();
//	std::string tuningSourceXmlContent = buffer.str();
//	file.close();
//	EXPECT_FALSE(tuningSourceXmlContent.empty());
//
//	// Parse TuningSource.xml
//	//
//	const auto bytes = std::as_bytes(std::span{tuningSourceXmlContent});
//	const auto result = GatewayClientLib::parseTuningSourcesXml(bytes);
//	ASSERT_TRUE(result.errors.empty());
//
//	// Check BuildInfo
//	//
//	EXPECT_EQ(result.project.name, "test_simulator_v430");
//	EXPECT_EQ(result.project.buildNo, 2713);
//	EXPECT_EQ(result.project.buildDate, "31-Mar-2026 10:27");
//	EXPECT_EQ(result.project.buildUser, "Administrator");
//
//	// Check tuning sources
//	//
//	//ASSERT_EQ(result.tuningSources.size(), 5U);
//
//	//{
//	//	const auto& source5th = result.tuningSources[4];
//
//	//	EXPECT_EQ(source5th.moduleEquipmentId, "SYSTEMID_RACKID_FSCC02_MD00");
//	//	EXPECT_EQ(source5th.moduleCaption, "LM1-SR04");
//	//	EXPECT_EQ(source5th.subsystemId, "SUBSYSID00");
//	//	EXPECT_EQ(source5th.channel, GatewayClientLib::Channel::A);
//	//}
//
//	// Check signals
//	//
//	{
//		const auto& source4th = result.tuningSources[3];
//
//		EXPECT_EQ(source4th.signalIds.size(), 96);
//		EXPECT_EQ(source4th.signals.size(), 96);
//
//		ASSERT_TRUE(source4th.signals.contains(Radiy::calcHash("#TEST_TUNING_LIMITS_INT32")));
//
//		const auto& sp = source4th.signals.at(Radiy::calcHash("#TEST_TUNING_LIMITS_INT32"));
//
//		EXPECT_STREQ(sp.appSignalId, "#TEST_TUNING_LIMITS_INT32");
//		//EXPECT_STREQ(sp.customSignalId, "TEST_TUNING_LIMITS_INT32");
//		//EXPECT_STREQ(sp.caption, "App signal #TEST_TUNING_LIMITS_INT32 in schema SYSTEMID_CLIENTTEST_CH12_MD00");
//		//EXPECT_STREQ(sp.equipmentId, "SYSTEMID_CLIENTTEST_CH12_MD00");
//		///EXPECT_STREQ(sp.lmEquipmentId, "SYSTEMID_CLIENTTEST_CH12_MD00");
//		//EXPECT_STREQ(sp.units, "");
//		//EXPECT_STREQ(sp.tags, "attention critical");
//
//		//EXPECT_EQ(sp.channel, GatewayClientLib::Channel::A);
//		//EXPECT_EQ(sp.inOutType, GatewayClientLib::InOutType::Internal);
//		//EXPECT_EQ(sp.type, GatewayClientLib::SignalType::Float32);
//		//EXPECT_EQ(sp.decimalPlaces, 12);
//		//EXPECT_TRUE(sp.tuning);
//
//		EXPECT_DOUBLE_EQ(sp.lowValidRange, 0.0);
//		EXPECT_DOUBLE_EQ(sp.highValidRange, 100.0);
//
//		EXPECT_DOUBLE_EQ(sp.tuningDefaultValue, 1.0);
//		EXPECT_DOUBLE_EQ(sp.tuningLowBound, 0.0);
//		EXPECT_DOUBLE_EQ(sp.tuningHighBound, 100.0);
//	}
//}
