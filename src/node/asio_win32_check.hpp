#ifndef _node_asio_win32_check_hpp
#define _node_asio_win32_check_hpp

//
// We get a warning on Windows compiler because BOOST library does
// do a redefinition of that macro. Seems to be a bug in BOOST 1_59 which
// I used as the current reference. I'll have this workaround to avoid that
// problem.
//

#if _WIN32

#if !defined(BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT)
#define BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT noexcept(true)
#endif

#endif

#endif
