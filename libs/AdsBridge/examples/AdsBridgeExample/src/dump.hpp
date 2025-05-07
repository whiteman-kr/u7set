#pragma once

#include <string_view>

struct AdsConnectionStatus;
struct MatsAppSignalParam;
struct MatsAppSignalState;

void dumpConnectionStatus(const AdsConnectionStatus& status);
void dumpConnectionStatus();

void dumpAppSignalParam(const MatsAppSignalParam& signalParam);
void dumpAppSignalState(const MatsAppSignalState& state, std::string_view appSignalId);
