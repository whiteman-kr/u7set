#include "UdpRetranslator.h"

#include "../../libs/asio/include/asio.hpp"

using namespace asio;
using namespace asio::ip;

struct sockaddr_ll
{
	unsigned short sll_family;
	unsigned short sll_protocol;
	int            sll_ifindex;
	unsigned short sll_hatype;
	unsigned char  sll_pkttype;
	unsigned char  sll_halen;
	unsigned char  sll_addr[8];
};

int main(int argc, char *argv[])
{
	asio::io_context ioContext;
	asio::generic::raw_protocol::socket rawSocket(ioContext);

	sockaddr_ll sockaddr;


	return 0;
}
