#ifndef DEFUTIL_H_INCLUDED
#define DEFUTIL_H_INCLUDED

#define FREE_ZERO(ptr) if( ptr ) { free(ptr); ptr = 0; }

#ifndef ABS
#define ABS(a) (((a)<0) ? -(a) : (a))
#endif // ABS

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif // MIN

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif // MAX

#ifndef OFFSETOF
#define OFFSETOF(struct, memb) ((int)(&(((struct*)0)->memb)))
#endif // OFFSETOF

#define NELTS(x) (sizeof(x)/sizeof(*(x)))

#define SWAP(a, b, tmp) ((tmp)=(a), (a)=(b), (b)=(tmp))

#define BUF_ADD(buf, len, off) \
    assertb((off)>=0);	       \
    (buf) += (off);	       \
    (len) -= (off);



#endif // DEFUTIL_H_INCLUDED
