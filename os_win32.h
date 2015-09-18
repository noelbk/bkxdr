#include <io.h>

// who knows why Windows loves the underscore prefix?
#define vsnprintf  _vsnprintf
#define snprintf   _snprintf
#define popen      _popen
#define mkdir      _mkdir
#define strcasecmp _stricmp
#define timezone   _timezone
#define lseek   _lseek
#define inline     __inline
