#include "log.hpp"
#include <stdarg.h>
#include <boost/filesystem/path.hpp>
#include <fstream>

namespace prologcoin { namespace common {

boost::mutex log::lock_;
std::string log::dir_;
   
void log::set_directory(const std::string &dir) {
   dir_ = dir;
}
   
void log::printf(const std::string &section, const char *format, ...) {
   boost::filesystem::path p = dir_;
   p /= section;
   p += ".log";
   
   char buffer[65536];

   boost::unique_lock<boost::mutex> lockit(lock_);
   std::ofstream fos(p.string(), std::fstream::out | std::fstream::app );
   va_list args;
   va_start(args, format);
   vsprintf(buffer, format, args);
   fos << buffer << std::endl;
   fos.flush();
}
     
}}


 