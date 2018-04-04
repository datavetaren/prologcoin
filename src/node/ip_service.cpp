#include "ip_service.hpp"

namespace prologcoin { namespace node {

std::string ip_service::str(size_t maxlen) const
{
    std::string s = ip_address::str();
    s += ":" + boost::lexical_cast<std::string>(port());

    if (maxlen != std::string::npos && s.size() > maxlen) {
	s = s.substr(s.size()-maxlen);
    }
    return s;
    
}

}}
