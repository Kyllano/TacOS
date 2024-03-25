#include "userlib/libnachos.h"
#include "userlib/syscall.h"

const int N = 10;

int tab_com[10];

SemId s_vide;
SemId s_plein;
int i_plein = 0;
int i_vide = N;

void prod()
{
    int info = 1;
    P(s_vide);
    i_plein = (i_plein + 1) % N;
    tab_com[i_plein] = info;
    V(s_plein);
}

void conso()
{
    P(s_plein);
    i_vide = (i_vide + 1) % N;
    int info = tab_com[i_vide];
    V(s_vide);
    n_printf("info : %d\n", info);
}

int main()
{
    s_vide = SemCreate("s_vide", N);
    s_plein = SemCreate("s_plein", 0);

    ThreadId prod_th = threadCreate("prod thread", &prod);
    ThreadId conso_th1 = threadCreate("conso thread", &conso);

    Join(conso_th1);
    Join(prod_th);

    // Si un output de info : 1 , c'est bon ! car le P() a bien marché et le V() également
    return 0;
}
