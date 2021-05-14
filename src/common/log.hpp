#pragma once

#ifndef _common_log_hpp
#define _common_log_hpp

#include <string>
#include <boost/thread/mutex.hpp>

namespace prologcoin { namespace common {

class log {
public:
   static void set_directory(const std::string &dir);
   static void printf(const std::string &section, const char *format, ...);

private:
   static boost::mutex lock_;
   static std::string dir_;
};
   
}}


	

#endif
