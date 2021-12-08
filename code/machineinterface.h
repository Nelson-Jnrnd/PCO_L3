#ifndef MACHINEINTERFACE_H
#define MACHINEINTERFACE_H

#include <vector>

/** Type des différentes pièces de monnaie, une valeur dans 1..9. */
typedef int COIN;

/** nombre d'articles différents proposées par la machine */
constexpr size_t MAX_ARTICLES = 4;

/** ARTICLE: type des differents items distribuables par ce machine. */
typedef enum {CHOCOLATE,CANDYCANE,GUMMYBEAR,LOLLIPOP} ARTICLE;

/** ETAT_TOUCHE: type des valeurs des touches de fonction '%' et '&' ayant les
* inscription OUI et NON.
*/
typedef enum {KEY_YES,KEY_NO,KEY_UNDEFINED} KEY_STATE;




class MachineInterface
{
public:


    /** Réalise toutes les initialisations nécessaire. Cette fonction
    * doit être appelée avant d'utiliser l'une des fonctions de cet interface.
    * \return 1 si tout est en ordre et 0 en cas d'erreur.
    */
    virtual int initialize() = 0;

    /** getCoin: Lecture bloquante jusqu'à l'obtention de la prochaine
    * pièce de monnaie insérée par l'usager.
    * Valeur retournée: une pièce de type COIN.
    */
    virtual COIN getCoin() = 0;

    /** getArticle: Lecture bloquante jusqu'à ce que l'usager presse l'un
    * des boutons correspondant au choix de son article.
    * Valeur retournée: Un article de type ITEM.
    */
    virtual ARTICLE getArticle() = 0;

    /** ejectCoin: Actionne la sortie de la pièce de monnaie passée en
    * paramètre si cette pièce est disponible, sinon l'action est idempotente.
    * Effet de bord: Si la pièce est rendue, l'inventaire des pièces est mis a
    * jour.
    * Paramètre: la pièce de type COIN a rendre.
    */
    virtual void ejectCoin(COIN coin) = 0;

    /** ejectArticle: Actionne la sortie d'un item distribuable par cette
    * machine. Si l'article n'est plus disponible l'action est idempotente.
    * Effet de bord: Si un item est rendu, l'inventaire des articles est mis a
    * jour.
    * Paramètre: l'item de type ITEM a rendre.
    */
    virtual void ejectArticle(ARTICLE article) = 0;

    /** getInventoryArticle: Retourne le nombre d'items d'un article donné
    * encore disponibles dans la machine.
    * Paramètre: l'article dont on souhaite connaître le nombre restant.
    * Valeur retournée: le nombre restant de l'item identifié par le paramètre.
    */
    virtual unsigned getInventoryArticle(ARTICLE article) = 0;

    /** getInventoryCoin: Retourne le nombre de pièces d'un certain type
    * encore disponibles dans la machine.
    * Paramètre: l'identifiant de la pièce dont on souhaite connaître le nombre restant.
    * Valeur retournée: le nombre restant de la pièce identifiée par le paramètre.
    */
    virtual unsigned getInventoryCoin(COIN coin) = 0;

    /** isOpenAccount: Retourne true si un compte est ouvert, false sinon */
    virtual bool isOpenAccount() = 0;

    /** getCreditOpenAccount: Retourne la somme restant dans le compte.
    * Cette somme retournée peut être quelconque si aucun compte n'est ouvert.
    * Valeur retournee: la somme restant sur le compte.
    */
    virtual int getCreditOpenAccount() = 0;

    /** updateOpenAccount: Permet de débiter ou de créditer un compte ouvert.
    * Si aucun compte n'est ouvert cette fonction n'a aucun effet. Sinon la
    * somme (positive ou négative) est additionnée au compte. Si la somme à
    * déduire est supérieure au solde du compte, le compte est mis à zéro.
    * Valeur retournée: la somme restant sur le compte après sa modification.
    */
    virtual int updateOpenAccount(int amount) = 0;

    /** getKeyState: Retourne la derniere touche lue parmi les touches '&'
    * et '/'.
    * Valeur retournee: une valeur dans l'ensemble {KEY_YES,KEY_NO,
    * KEY_UNDEFINED}.
    */
    virtual KEY_STATE getKeyState() = 0;

    /** resetKeyFunction: Remise a zero de la memoire associee aux tou-
    * ches '&' et '/'. Suite a l'appel de cette procedure, la fonction
    * Machine_GetEtatTouche renvoie la valeur KEY_UNDEFINED.
    */
    virtual void resetKeyFunction() = 0;

    /** Fonction qui retourne vrai lorsque le programme doit se terminer,
     * donc quand l'utilisateur à tapé 'q'.
     *  Permet aux différents threads de savoir s'i 'ils doivent s'arrêter.
     *  \return vrai lorsque le programme doit se terminer.
     */
    virtual bool shouldQuit() = 0;

};

#endif // MACHINEINTERFACE_H
