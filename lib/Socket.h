#pragma once

namespace Socket
{
	const char* const IP_NULL = "0.0.0.0";
	const char* const IP_ANY = "0.0.0.0";
	const char* const IP_LOCALHOST = "127.0.0.1";
	const char* const IP_BROADCAST = "255.255.255.255";

	const char* const NETMASK_DEFAULT = "255.255.255.0";

	const int PORT_LOWEST = 0;
	const int PORT_HIGHEST = 65535;

	const int ENTIRE_UDP_SIZE = 1472;
}


