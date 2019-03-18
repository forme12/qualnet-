#ifdef _WIN32

// Windows
#  define _DEFINE_DEPRECATED_HASH_CLASSES 0
#  include <hash_set>
// for vs 2003.net
  using namespace stdext;
  using namespace std;

// In msvc - it's hash_compare
#  define HASH_VALUE_FUNCTION hash_compare

#else
// # NON - WINDOWS

# ifndef HASH_NAMESPACE
#  define HASH_NAMESPACE __gnu_cxx
# endif

// in STL port/sgi/gcc it is simply hash
#  define HASH_VALUE_FUNCTION hash


// with gcc, the name of hash_map include files moved around.
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#     if __GNUC_MINOR__ >= 3
#      include <backward/hash_fun.h>
#     else
#      include<ext/hash_fun.h> 
#     endif
#    else
#        if __GNUC__ == 3
#           if __GNUC_MINOR__ >= 4
#               include<ext/hash_fun.h> 
#           else
#               include <ext/stl_hash_fun.h>
#           endif
#       else
#           include <ext/stl_hash_fun.h>
#      endif
#    endif 
#  endif 

#endif
