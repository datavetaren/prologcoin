#include <stdio.h>
#include <string>
#include <algorithm>

static void do_parent(std::string &path)
{
    size_t slashIndex = path.find_last_of("/\\");
    path = path.substr(0, slashIndex);
}

static const std::string & find_home_dir(const char *selfpath = nullptr)
{
    static std::string home_dir;

    if (home_dir.size() > 0) {
	return home_dir;
    }

    // Current path
    home_dir = selfpath;
    std::replace( home_dir.begin(), home_dir.end(), '\\', '/');

    bool found = false;
    do {
      do_parent(home_dir);
      std::string checkfile = home_dir + "/env/Makefile.main";
      if (auto f = fopen(checkfile.c_str(), "r")) {
   	  fclose(f);
          found = true;
      }
    } while (!found);

    return home_dir;
}
