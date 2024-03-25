#include "userlib/libnachos.h"
#include "userlib/syscall.h"

int main()
{
    LockId im_Lock1 = LockCreate("Lock1");
    im_Lock1 = LockCreate("Lock1");
    PError("Success error (Lock was already created)");
    LockAcquire(im_Lock1);
    LockAcquire(im_Lock1);
    PError("Success error (Lock was already Acquired)");
    LockRelease(im_Lock1);
    LockRelease(im_Lock1);
    PError("Success error (Lock was already Released)");
    LockDestroy(im_Lock1);

    LockId im_Lock2 = LockCreate("Lock2");
    LockDestroy(im_Lock2);
    LockDestroy(im_Lock2);
    PError("Success error (Lock was already destroy)");
    return 0;
}
