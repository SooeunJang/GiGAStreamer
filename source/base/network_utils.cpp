
#include "network_utils.hpp"
#include <iostream>

namespace cfx {

   HostInetInfo NetworkUtils::queryHostInetInfo()
   {
       io_service ios;
       tcp::resolver resolver(ios);
       tcp::resolver::query query(host_name(), "");
       return resolver.resolve(query);
   }

   std::string NetworkUtils::hostIP(unsigned short family, int port_num)
   {
		if(family == AF_INET)
		{
			auto hostInetInfo = queryHostInetInfo();
			tcp::resolver::iterator end;
			while(hostInetInfo != end) {
			  tcp::endpoint ep = *hostInetInfo++;
			  sockaddr sa = *ep.data();

			  if (sa.sa_family == family) {
				  return ep.address().to_string();
			  }
			}
			return nullptr;
		}
		else
		{
			tcp::endpoint ep(boost::asio::ip::tcp::v4(), port_num);
			return ep.address().to_string();
		}
   }

}
