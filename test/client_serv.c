#include "userlib/libnachos.h"
#include "userlib/syscall.h"

const int N = 10;

SemId s_vide;
SemId s_plein;
int i_plein = 0;
int i_vide = N;

void Clients()
{

    /*int count = 0;       // problème avec cette version, seulement 2 clients arrivent puis on boucle dans le vide
    while (count < i_vide)
    {
        count = count + 1;
        P(s_vide);
        n_printf("Client arrive \n");
        V(s_plein);
        i_plein = i_plein + 1;
    }
    i_vide = i_vide - count;*/

    int count = 0; // cette version marche mais on décompose plus que la première
    while (count < i_vide)
    {
        count = count + 1;
        P(s_vide);
        n_printf("Client arrive \n");
    }
    for (int i = 0; i < count; i++)
    {
        V(s_plein);
        i_plein = i_plein + 1;
        i_vide = i_vide - 1;
    }
}

void Serveur()
{

    P(s_plein);
    i_plein = i_plein - 1;
    n_printf("Serveur sert \n");
    V(s_vide);
    i_vide = i_vide + 1;
}

int main()
{
    s_vide = SemCreate("s_vide", N);
    s_plein = SemCreate("s_plein", 0);

    ThreadId Clients_th = threadCreate("prod thread", &Clients);
    ThreadId Serveur_th = threadCreate("conso thread", &Serveur);

    Join(Serveur_th);
    Join(Clients_th);

    return 0;
}
