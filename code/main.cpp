#include <iostream>
#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcologger.h>

#include "machine.h"
#include "machinemanager.h"

int main()
{
    logger().setVerbosity(1);

    logger() << "Simulateur de machine\n"
                 " + , \" , * , % : Choix de l'article\n"
                 " ?, ( , ) : Respectivement créer, ouvrir et fermer un compte\n"
                 " chiffre [1..9]: Pièce de monnaie avec une valeur allant de 1 a 9\n"
                 " & , / : Touches spéciales pour valider ou non un choix" << std::endl;

    Machine machine;
    if (machine.initialize()) {
        MachineManager manager(machine);
        PcoThread ThreadMoney(&MachineManager::Money, &manager);
        PcoThread ThreadMerchandise(&MachineManager::Merchandise, &manager);
        ThreadMoney.join();
        ThreadMerchandise.join();
    }
    return EXIT_FAILURE;
}
