#include "userlib/libnachos.h"
#include "userlib/syscall.h"

const int N = 10;

SemId Paul;
SemId Jean;
SemId Bernard;
SemId Micheline;
int i_plein = 1;
int i_vide = 1;

void prog_1()
{
    P(Paul);
    P(Jean);
    V(Bernard);
    P(Micheline);
}

void prog_2()
{
    V(Paul);
    V(Jean);
    P(Bernard);
    V(Micheline);
    n_printf("We passed !\n");
}

int main()
{
    Paul = SemCreate("Paul", 0);
    Jean = SemCreate("Jean", 0);
    Bernard = SemCreate("Bernard", 0);
    Micheline = SemCreate("Micheline", 0);

    ThreadId prog_1_th = threadCreate("prod thread", &prog_1);
    ThreadId prog_2_th = threadCreate("conso thread", &prog_2);

    Join(prog_1_th);
    Join(prog_2_th);

    // Si un output de info : 1 , c'est bon ! car le P() a bien marché et le V() également
    return 0;
}
