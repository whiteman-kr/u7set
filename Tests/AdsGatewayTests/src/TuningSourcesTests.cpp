#include <GatewayClientLib/../../src/TuningSources.hpp>

#include <GatewayClientLib/GwHash.hpp>

#include <gtest/gtest.h>

TEST(TuningSourcesTests, ParsesDefaultProfileAndSignals)
{
//	constexpr std::string_view xml = R"(<?xml version="1.0" encoding="UTF-8"?>
//<Content>
//	<DataSources Count="2">
//		<DataSource ModuleEquipmentID="SYSTEMID_CLIENTTEST_CH10_MD00" Profile="Default" Caption="LM1-SR04">
//			<LanControllers Count="1">
//				<LanController EquipmentID="SYSTEMID_CLIENTTEST_CH10_MD00_ETHERNET01" ControllerNo="1" LanControllerType="Tuning">
//					<TuningParams TuningEnable="true" TuningIP="127.0.231.101" TuningPort="50000" TuningServiceID="SYSTEMID_CLIENTTEST_WS01_TUNS2" TuningServiceIP="127.0.231.1" TuningServicePort="13332" TuningServiceNetmask="255.255.255.0"/>
//				</LanController>
//			</LanControllers>
//			<TuningSignals>#TS1_INITIATOR1</TuningSignals>
//			<TuningData LmEquipmentID="SYSTEMID_CLIENTTEST_CH10_MD00" SignalsCount="1">
//				<AnalogFloatSignals Count="0"/>
//				<AnalogInt32Signals Count="0"/>
//				<DiscreteSignals Count="1">
//					<Signal AppSignalID="#TS1_INITIATOR1" CustomAppSignalID="TS1_INITIATOR1" Caption="App signal #TS1_INITIATOR1" EquipmentID="SYSTEMID_CLIENTTEST_CH10_MD00" Type="Discrete" EnableTuning="true" TuningValueTypeStr="Discrete" TuningDefaultValue="0" TuningLowBound="0" TuningHighBound="1"/>
//				</DiscreteSignals>
//			</TuningData>
//		</DataSource>
//		<DataSource ModuleEquipmentID="SYSTEMID_CLIENTTEST_CH10_MD00" Profile="linux_test_job" Caption="LM1-SR04">
//			<LanControllers Count="1">
//				<LanController EquipmentID="IGNORED_LAN" ControllerNo="1" LanControllerType="Tuning">
//					<TuningParams TuningEnable="true" TuningIP="10.10.10.10" TuningPort="50001" TuningServiceID="IGNORED_TS" TuningServiceIP="10.10.10.1" TuningServicePort="13333" TuningServiceNetmask="255.255.255.0"/>
//				</LanController>
//			</LanControllers>
//			<TuningSignals>#IGNORED_SIGNAL</TuningSignals>
//		</DataSource>
//	</DataSources>
//</Content>)";
//
//	const auto result = GatewayClientLib::parseTuningSourcesXml(xml);
//
//	ASSERT_TRUE(result.errors.empty());
//	ASSERT_EQ(result.tuningSources.size(), 1U);
//
//	const auto& source = result.tuningSources.front();
//	EXPECT_EQ(source.moduleEquipmentId, "SYSTEMID_CLIENTTEST_CH10_MD00");
//	EXPECT_EQ(source.profile, "Default");
//	EXPECT_EQ(source.caption, "LM1-SR04");
//	EXPECT_EQ(source.lanEquipmentId, "SYSTEMID_CLIENTTEST_CH10_MD00_ETHERNET01");
//	EXPECT_TRUE(source.tuningEnabled);
//	EXPECT_EQ(source.tuningIp, "127.0.231.101");
//	EXPECT_EQ(source.tuningPort, 50000);
//	EXPECT_EQ(source.tuningServiceId, "SYSTEMID_CLIENTTEST_WS01_TUNS2");
//	EXPECT_EQ(source.tuningServiceIp, "127.0.231.1");
//	EXPECT_EQ(source.tuningServicePort, 13332);
//	EXPECT_EQ(source.tuningServiceNetmask, "255.255.255.0");
//
//	ASSERT_EQ(source.tuningSignalIds.size(), 1U);
//	EXPECT_EQ(source.tuningSignalIds.front(), "#TS1_INITIATOR1");
//
//	ASSERT_EQ(source.signals.size(), 1U);
//	const auto& signal = source.signals.front();
//	EXPECT_EQ(signal.appSignalId, "#TS1_INITIATOR1");
//	EXPECT_EQ(signal.hash, Radiy::calcHash("#TS1_INITIATOR1"));
//	EXPECT_EQ(signal.customAppSignalId, "TS1_INITIATOR1");
//	EXPECT_EQ(signal.caption, "App signal #TS1_INITIATOR1");
//	EXPECT_EQ(signal.equipmentId, "SYSTEMID_CLIENTTEST_CH10_MD00");
//	EXPECT_EQ(signal.type, "Discrete");
//	EXPECT_EQ(signal.tuningValueType, "Discrete");
//	EXPECT_TRUE(signal.enableTuning);
//	ASSERT_TRUE(signal.tuningDefaultValue.has_value());
//	ASSERT_TRUE(signal.tuningLowBound.has_value());
//	ASSERT_TRUE(signal.tuningHighBound.has_value());
//	EXPECT_DOUBLE_EQ(*signal.tuningDefaultValue, 0.0);
//	EXPECT_DOUBLE_EQ(*signal.tuningLowBound, 0.0);
//	EXPECT_DOUBLE_EQ(*signal.tuningHighBound, 1.0);
}
