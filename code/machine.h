/** \mainpage machine
 * Cette documentation décrit l'interface fournie par le fichier machine.h.
 */

/** \file machine.h
 *  \author YTA
 *  \date 05.02.2021
 * \version 1.0.0
 * \section Description
 * Ce fichier définit l'interface permettant de gérer un machine.
 * Plusieurs des fonctions sont bloquantes. Il est dès lors nécessaire de
 * gérer le machine via plusieurs tâches, étant donné que l'utilisateur
 * peut effectuer les opérations dans un ordre quelconque.
 */

#ifndef MACHINE_H
#define MACHINE_H

#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcosemaphore.h>
#include <pcosynchro/pcoconditionvariable.h>

#include "machineinterface.h"


class Machine : public MachineInterface
{

public:
    Machine();
    virtual ~Machine();

    /** Réalise toutes les initialisations nécessaire. Cette fonction
    * doit être appelée avant d'utiliser l'une des fonctions de cet interface.
    * \return 1 si tout est en ordre et 0 en cas d'erreur.
    */
    int initialize() override;

    /** getCoin: Lecture bloquante jusqu'à l'obtention de la prochaine
    * pièce de monnaie insérée par l'usager.
    * Valeur retournée: une pièce de type COIN.
    */
    COIN getCoin() override;

    /** getArticle: Lecture bloquante jusqu'à ce que l'usager presse l'un
    * des boutons correspondant au choix de son article.
    * Valeur retournée: Un article de type ITEM.
    */
    ARTICLE getArticle() override;

    /** ejectCoin: Actionne la sortie de la pièce de monnaie passée en
    * paramètre si cette pièce est disponible, sinon l'action est idempotente.
    * Effet de bord: Si la pièce est rendue, l'inventaire des pièces est mis a
    * jour.
    * Parametre: la coin de type COIN a rendre.
    */
    void ejectCoin(COIN coin) override;

    /** ejectArticle: Actionne la sortie d'un item distribuable par cette
    * machine. Si l'article n'est plus disponible l'action est idempotente.
    * Effet de bord: Si un item est rendu, l'inventaire des articles est mis a
    * jour.
    * Paramètre: l'item de type ITEM a rendre.
    */
    void ejectArticle(ARTICLE article) override;

    /** getInventoryArticle: Retourne le nombre d'items d'un article donné
    * encore disponibles dans la machine.
    * Paramètre: l'article dont on souhaite connaître le nombre restant.
    * Valeur retournée: le nombre restant de l'item identifié par le paramètre.
    */
    unsigned getInventoryArticle(ARTICLE article) override;

    /** getInventoryCoin: Retourne le namebre de coins d'un certain type
    * encore disponibles dans la machine.
    * Paramètre: l'identifiant de la pièce dont on souhaite connaître le nombre restant.
    * Valeur retournée: le nombre restant de la pièce identifiée par le paramètre.
    */
    unsigned getInventoryCoin(COIN coin) override;

    /** isOpenAccount: Retourne true si un compte est ouvert, false sinon */
    bool isOpenAccount() override;

    /** getCreditOpenAccount: Retourne la somme restant dans le compte.
    * Cette somme retournée peut être quelconque si aucun compte n'est ouvert.
    * Valeur retournee: la somme restant sur le compte.
    */
    int getCreditOpenAccount() override;

    /** updateOpenAccount: Permet de débiter ou de créditer un compte ouvert.
    * Si aucun compte n'est ouvert cette fonction n'a aucun effet. Sinon la
    * somme (positive ou négative) est additionnée au compte. Si la somme à
    * déduire est supérieure au solde du compte, le compte est mis à zéro.
    * Valeur retournée: la somme restant sur le compte après sa modification.
    */
    int updateOpenAccount(int amount) override;

    /** getKeyState: Retourne la derniere touche lue parmi les touches '&'
    * et '/'.
    * Valeur retournee: une valeur dans l'ensemble {KEY_YES,KEY_NO,
    * KEY_UNDEFINED}.
    */
    KEY_STATE getKeyState() override;

    /** resetKeyFunction: Remise a zero de la memoire associee aux tou-
    * ches '&' et '/'. Suite a l'appel de cette procedure, la fonction
    * Machine_GetEtatTouche renvoie la valeur KEY_UNDEFINED.
    */
    void resetKeyFunction() override;

    /** Fonction qui retourne vrai lorsque le programme doit se terminer,
     * donc quand l'utilisateur à tapé 'q'.
     *  Permet aux différents threads de savoir s'i 'ils doivent s'arrêter.
     *  \return vrai lorsque le programme doit se terminer.
     */
    bool shouldQuit() override;





protected:

    bool openAccount();

    void closeAccount();

    void openNewAccount();

    void openOldAccount();

    void closeCurrentAccount();

    void openAction(bool newAcc);

    void closeAction();

    void processKey();

    void quit();

    template<typename T> class BufferN {
    protected:
        std::vector<T> elements;
        int writePointer, readPointer, nbElements, bufferSize;
        PcoMutex mutex;
        PcoConditionVariable waitProd, waitConso;
        unsigned nbWaitingProd, nbWaitingConso;

    public:

        BufferN(unsigned int size) : elements(size), writePointer(0),
                                        readPointer(0), nbElements(0),
                                        bufferSize(size),
                                        nbWaitingProd(0), nbWaitingConso(0) {
           }

        void put(T item) {
            mutex.lock();
            if (nbElements == bufferSize) {
                nbWaitingProd += 1;
                waitProd.wait(&mutex);
            }
            elements[writePointer] = item;
            writePointer = (writePointer + 1)
                           % bufferSize;
            nbElements ++;
            if (nbWaitingConso > 0) {
                nbWaitingConso -= 1;
                waitConso.notifyOne();
            }
            mutex.unlock();
        }

        T get(void) {
            T item;
            mutex.lock();
            if (nbElements == 0) {
                nbWaitingConso += 1;
                waitConso.wait(&mutex);
            }
            item = elements[readPointer];
            readPointer = (readPointer + 1)
                          % bufferSize;
            nbElements --;
            if (nbWaitingProd > 0) {
                nbWaitingProd -= 1;
                waitProd.notifyOne();
            }
            mutex.unlock();
            return item;
        }
    };


    template<typename T> class BufferNTime {
    protected:
        std::vector<T> elements;
        int writePointer, readPointer, nbElements, bufferSize;
        PcoMutex mutex;
        PcoConditionVariable waitProd, waitConso;

    public:

        BufferNTime(unsigned int size) : elements(size), writePointer(0),
                                        readPointer(0), nbElements(0),
                                        bufferSize(size) {
           }

        int put(T item) {
            mutex.lock();
            auto nextPointer = (writePointer + 1) % bufferSize;
            if (nbElements == bufferSize) {
                waitProd.waitForSeconds(&mutex, 1);
            }
            if (nextPointer != readPointer) {
                elements[writePointer] = item;
                writePointer = (writePointer + 1)
                               % bufferSize;
                nbElements ++;
                waitConso.notifyOne();
                nextPointer = 1;
            }
            else {
                nextPointer = 0;
            }
            mutex.unlock();
            return nextPointer;
        }

        T get(void) {
            T item;
            mutex.lock();
            if (nbElements == 0) {
                waitConso.wait(&mutex);
            }
            item = elements[readPointer];
            readPointer = (readPointer + 1)
                          % bufferSize;
            nbElements --;
            waitProd.notifyOne();
            mutex.unlock();
            return item;
        }
    };


    static constexpr size_t MAXCOINS = 4;
    BufferN<ARTICLE> bufferArticleIntroduction{1};
    BufferNTime<COIN> bufferCoinsIntroduction{MAXCOINS};

    std::unique_ptr<PcoThread> threadButton;

    PcoSemaphore m_openAccount;
    PcoSemaphore m_closeAccount;
    PcoSemaphore m_key;
    PcoSemaphore m_coin;
    PcoSemaphore m_article;

    PcoMutex m_mutex;

    int m_accStatus{0};
    bool isNewAcc;

    bool m_shouldQuit{false};
};

#endif // MACHINE_H
