#pragma once

namespace Hardware
{
	void init();
	void shutdown();

	bool canCreateDevice(quint32 classNameHash);
} // namespace Hardware