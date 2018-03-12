#include <stdio.h>
#include <string.h>
#include <string>
#include <algorithm>

static void do_parent(std::string &path)
{
    size_t slashIndex = path.find_last_of("/\\");
    path = path.substr(0, slashIndex);
}

// TODO: I need to rename this file. It is no longer
// just a "home dir" utility.
static bool is_fast(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
	if (strcmp(argv[i], "-fast") == 0) {
	    return true;
	}
    }
    return false;
}

static const std::string & find_home_dir(const char *selfpath = nullptr)
{
    static std::string home_dir;

    // Avoid warning to unused function is_fast.
    (void)&is_fast;

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
