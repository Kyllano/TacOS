#include "userlib/libnachos.h"
#include "userlib/syscall.h"

int main()
{
    SemId sem = SemCreate("je s'appelle SEMAPHORE", 1);
    P(sem);
    V(sem);
    SemDestroy(sem);
    if (SemDestroy(sem) != -1)
    {
        printf("erreur !");
        return;
    }
    SemId sem_snd = SemCreate("je s'appelle SEMAPHORE second !", 0);
    SemCreate("je s'appelle SEMAPHORE second !", 0);
    PError("Success error (semaphore was already created)");
    V(sem_snd);
    P(sem_snd);
    V(sem_snd);
    P(sem_snd);
    SemDestroy(sem_snd);
    SemDestroy(sem_snd);
    PError("Success error (semaphore was already destroyed)");

    int nmb = -3;
    SemId sem_nega = SemCreate("je s'appelle SEMAPHORE negative !", nmb);
    for (int i = -3; i < 0; i++)
    {
        V(sem_nega);
    }
    V(sem_nega);
    P(sem_nega);
    SemDestroy(sem_nega);
    V(sem_nega);
    PError("Success error (semaphore was destroyed before the V())");
    P(sem_nega);
    PError("Success error (semaphore was destroyed before the P())");
    return 0;
}
