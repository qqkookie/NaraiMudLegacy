#if defined(__CYGWIN32__) && !defined(sigmask)

static _Sigprocmask( int how, int mask )
{
    sigset_t newset=mask, oldset;
    int ccode = sigprocmask( how, &newset, &oldset );
    return ( ccode < 0 )? ccode : oldset;
}

#define sigmask(sig)   ( 1UL << ( (sig) % ( 8*sizeof(sigset_t) )))
#define sigblock(mask)    _Sigprocmask( SIG_BLOCK,   (mask) )
#define sigsetmask(mask)  _Sigprocmask( SIG_SETMASK, (mask) )

#endif

