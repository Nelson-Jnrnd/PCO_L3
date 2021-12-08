

#ifndef MACHINEMANAGER_H
#define MACHINEMANAGER_H

#include <iostream>

#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcologger.h>
#include <pcosynchro/pcomutex.h>

#include "machineinterface.h"



class MachineManager
{
    /** Prix des differents articles vendus par cette machine */
    const std::array<int, MAX_ARTICLES> prixArticles = {1,2,3,4};
    PcoMutex* mutexCredit = new PcoMutex();
    PcoMutex* mutexUpdate = new PcoMutex();
    int credit = 0;
public:

    MachineManager(MachineInterface &machine) : machine(machine)
    {
    }

    ~MachineManager() {
        delete(mutexCredit);
        delete(mutexUpdate);
    }


    /**
     * Thread pour la gestion de la réception de la monnaie. Scrute la
     * réception de pièces en boucle sans bloquer le reste de l'appli.
     * Pour chaque pièce reçu, la comptabilise dans le compte et affiche
     * le nouveau solde à l'utilisateur.
     */
    void Money()
    {
        COIN coin;

        if (machine.shouldQuit()) {
            return;
        }

        while (1) {
            // Quand l'utilisateur a insérer une pièce
            coin = machine.getCoin();

            // Si un compte est déjà ouvert
            if(machine.isOpenAccount()){
                // On accrédite le compte
                mutexUpdate->lock();
                machine.updateOpenAccount(coin);
                mutexUpdate->unlock();
            }else{
                // On met l'argent dans la session
                mutexCredit->lock();
                credit += coin;
                mutexCredit->unlock();
            }
            logger() << " | ";
            afficherSolde();
        }
    }

    /**
     * Ce thread s'occupe de la gestion de la marchandise. Il atteint
     * le choix du client pour l'article et si il est disponible,
     * vérifie que le amount ouvert est suffisant. Il s'occupe ensuite
     * de rendre la monnaie, l'article et de mettre à jour le solde
     * ouvert du client.
     * Ce thread demande aussi au client s'il confirme son achat lorsque
     * la monnaie ne peut etre rendue de façon optimale.
     */
    void Merchandise()
    {
        ARTICLE article;     // Article voulu par le client

        if (machine.shouldQuit()) {
            return;
        }

        while (1) {

            article = machine.getArticle();   // lecture du souhait du client

            // Récupère le nombre d'articles encore disponibles en stock
            int articleCount = machine.getInventoryArticle(article);

            // S'il reste du stock
            if (articleCount > 0) {
                logger() << "[Marchandise]" << std::endl;
                logger() << "Article selectionne : ";
                displayArticle(article);
                acheterArticle(article);
                afficherSolde();
            }
            //
            // Sinon, on affiche un message d'erreur
            else{
                logger() << "Cet article n'est plus en stock" << std::endl;
            }
        }
    }

    void afficherSolde(){
        mutexCredit->lock();
        logger() << "Le solde disponible est : "
                << ((machine.isOpenAccount() ? machine.getCreditOpenAccount() : 0) + credit)
                << std::endl;
        mutexCredit->unlock();
    }

    void acheterArticle(ARTICLE article){
        // Récupère le prix de l'article en question
        int prix = prixArticles[article];
        int totalCredit = credit;

        // Vérifie si un compte est ouvert
        if (machine.isOpenAccount()) {
            totalCredit += machine.getCreditOpenAccount();
        }
        // Si le crédit total est suffisant
        if (totalCredit >= prix) {

            // On regarde si les pièces insérées suffisent
            if (credit >= prix) {
                // On retourne la monnaie, seulement si l'user n'est pas connecté
                if(!machine.isOpenAccount()){
                    // Retour de monnaie
                    int montantARendre = credit - prix;
                    if(montantARendre > 0){
                        std::array<int, 9> rendu;
                        int montantRendable = 0;

                        // Le compte est exact, pas besoin de demander à l'utilisateur
                        if(amountToReturn(montantARendre, rendu, montantRendable)){
                            logger() << "montant exact doit rendre " << montantARendre << " , rend " << montantRendable << std::endl;
                            rendreMonnaie(rendu);
                        } else{
                            // demander l'avis à l'utilisateur ça marche pas là...
                            logger() << "Seul le montant de " << montantRendable << " peut être rendu. Continuer ? [& ou /]" << std::endl;
                            KEY_STATE key = KEY_UNDEFINED;
                            machine.resetKeyFunction();
                            while(key == KEY_UNDEFINED) {
                                key = machine.getKeyState();
                            }
                            switch (key) {
                                case KEY_YES:
                                    logger() << "Ok cétiparrr" << std::endl;
                                    rendreMonnaie(rendu);
                                    break;
                                case KEY_NO:
                                    logger() << "Achat annulé !" << std::endl;
                                    return;
                                case KEY_UNDEFINED:
                                    logger() << "pas la bonne touche woulla" << std::endl;
                                    //key = machine.getKeyState();
                             }
                            machine.resetKeyFunction();
                        }
                    }else{
                        mutexCredit->lock();
                        credit = 0;
                        mutexCredit->unlock();
                    }
                } else{
                    // On enlève le montant du prix seulement si on a pas deja enlever de monnaie
                    mutexCredit->lock();
                    credit -= prix;
                    mutexCredit->unlock();
                }
            }
            // Sinon, on met le crédit à zéro et on débite du compte la différende
            else {
                mutexUpdate->lock();
                machine.updateOpenAccount(-(prix - credit));
                mutexUpdate->unlock();
                mutexCredit->lock();
                credit = 0;
                mutexCredit->unlock();
            }
            // Ensuite on éjecte l'article
            machine.ejectArticle(article);
        }else{
        logger() << "Votre crédit est insuffisant !" << std::endl;
        }

    }

    void rendreMonnaie(const std::array<int, 9> &rendu){
        for(uint iPiece = 0; iPiece < rendu.size(); ++iPiece){
            // Pour chaque type de pièces, on regarde le nombre de pièce a rendre, et on en rend ce nombre
            for(uint nbPieceARendre = rendu.at(iPiece); nbPieceARendre > 0; --nbPieceARendre)
                // +1 car la pièce 1 est à l'index 0
                machine.ejectCoin(iPiece+1);
        }
        mutexCredit->lock();
        credit = 0;
        mutexCredit->unlock();
    }

    /**
     * Calcul la monnaie à rendre en fonction du amount
     * à rendre et des pièces disponibles dans la machine.
     * Paramètres :
     * - const uint aRendre         Valeur du amount à rendre
     * - std::array<int, 9>& rendu  Tableau contenant les pièces à rendre (retour)
     * - uint* renduTot             Valeur de rendu possible (retour)
     * Retourne vrai si un rendu optimal est possible
     */
    bool amountToReturn(int aRendre, std::array<int, 9>& rendu, int& renduTot){

        //On crée un tableau temporaire pour l'algorithme, toutes les cases étant initialisées
        // à 0
        std::vector<uint> tabTemp(aRendre+1, 0);
        //On clean le tableau reçu en paramètre des pièces à rendre
        for (int i = 0; i < 9 ; i++) {
            rendu[i] = 0;
        }

        // Algorithme : remplissage du tableau temporaire

        //On parcourt tout les types de pièces de la plus grande
        //(ou valeur à rendre si plus petit) à la plus petite
        for(int i = aRendre<9?aRendre:9 ; i>0 ; i--) {
            // Pour toutes les pièces en stock d'une valeur donnée
            for (uint j = 0 ; j<machine.getInventoryCoin(i) ; j++) {
                // On parcourt et remplit le tableau temporaire de l'algorithme
                // (on s'arrête lorsqu'on à dépassé l'indice de la valeur de la
                // pièce en cours)
                for(int k = aRendre ; k>=i ; k--) {

                    /* On place la pièce dans le tableau si les conditions sont remplies :
                         - si l'emplacement actuel et vide et
                                - si i cases avant il y a déjà une pièce ou
                            si l'indice courant = la valeur de la pièce */
                    if( tabTemp[k] == 0 && ( tabTemp[k-i] != 0 || k == i )) {
                        tabTemp[k] = i;
                        break;
                    }
                }
            }
        }

        // Préparation du tableau à retourner

        //On part depuis la fin du tableau temporaire
        int indice = aRendre;
        int somme = 0;
        int coin;

        //Tant qu'on est pas arrivé au début du tableau temporaire
        while(indice > 0) {
            /*S'il y a une pièce à l'indice en cours on la prend et on décrèmente
              l'indice de la valeur de la pièce. sinon on descend dans la tableau
                jusqu'à tomber sur une pièce */
            if(tabTemp[indice] != 0) {
                coin = tabTemp[indice];

                somme += coin;
                rendu[coin-1]++;
                indice -= coin;
            }
            else
                indice--;
        }

        // Retourne le tableau des pièces à rendre, on indique si on peut rendre juste
        return (renduTot = somme) == aRendre;
    }


    void displayArticle(ARTICLE article)
    {
        switch (article) {
        case CHOCOLATE: logger() << "Chocolate " << std::endl; break;
        case CANDYCANE: logger() << "Candy cane " << std::endl; break;
        case GUMMYBEAR: logger() << "Gummy bear " << std::endl; break;
        case LOLLIPOP: logger() << "Lollipop " << std::endl; break;
        default: logger() << "Error, unexisting article " << std::endl; break;
        }
    }


private:

    MachineInterface &machine;
};


#endif // MACHINEMANAGER_H
