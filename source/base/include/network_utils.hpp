
#pragma once

#include <string>

#include "glsm_baseheaders.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;

namespace cfx {
   
   using HostInetInfo = tcp::resolver::iterator;

   class NetworkUtils {
   private:
      static HostInetInfo queryHostInetInfo();
   public:
      static std::string hostIP(unsigned short family, int port_num);
      // gets the host IP4 string formatted
      static std::string hostIP4(int port_num) {
         return hostIP(AF_INET,port_num);
      }

      // gets the host IP6 string formatted
      static std::string hostIP6(int port_num) {

         return hostIP(AF_INET6,port_num);
      }
      static std::string hostName() {
         return ip::host_name();
      }
   };
      
}
