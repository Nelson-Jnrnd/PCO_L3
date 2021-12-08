

#include <iostream>
#include <string.h>

#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcologger.h>

#include "machine.h"

/** InventoryCoins: inventaire des coins de monnaies dans la machine. */
//static std::array<unsigned, 9> InventoryCoins = {3,3,3,3,3,3,3,3,3};
static std::array<unsigned, 9> InventoryCoins = {0,0,0,0,0,0,0,0,0};

/** InventoryItem: inventaire des items distribuables dans la machine. */
static std::array<unsigned, MAX_ARTICLES> InventoryItem = {4,4,4,4};

/** Gestion des comptes */
#define MAX_ID_COMPTE 25
typedef struct LISTCOMPTE {
    char id[MAX_ID_COMPTE];
    int amount;
    struct LISTCOMPTE *next;
} LISTCOMPTE;

extern LISTCOMPTE *currentAccount;

Machine::Machine()
{
}

Machine::~Machine()
{
    if (threadButton.get() != nullptr) {
        threadButton->join();
    }
}

int Machine::initialize(){
    threadButton = std::make_unique<PcoThread>(&Machine::processKey, this);
    return 1;
}



bool Machine::openAccount()
{
    m_openAccount.acquire();
    return isNewAcc;
}

void Machine::closeAccount()
{
    m_closeAccount.acquire();
}

void Machine::openAction(bool newAcc)
{
    if (m_accStatus == 0) {
        m_openAccount.release();
        m_accStatus = 1;
        isNewAcc = newAcc;
        m_key.acquire();
    }
}

void Machine::closeAction()
{
    if (m_accStatus == 1) {
        m_closeAccount.release();
        m_accStatus = 0;
    }
}



/** Key_State: derniere touche presse par l'usager. */
static KEY_STATE Key_State = KEY_UNDEFINED;


LISTCOMPTE *currentAccount;

static LISTCOMPTE *listAccounts;

static int GetAccountId(char *name)
{
    int car, i;
    while (getchar() != '\n');
    logger() << "Entrez l'identifiant du compte: " << std::endl;
    for (i = 0; i < MAX_ID_COMPTE - 1; ) {
        car = getchar();
        if (((car >= 'a') && (car <= 'z')) || ((car >= 'A') && (car <= 'Z'))) {
            name[i] = car; i += 1;
        }
        else if (car == '\n')
            break;
    }
    name[i] = '\0';

    return i;
}


static LISTCOMPTE *GetAccount(char *name)
{
    LISTCOMPTE *compte;
    int i;
    for (compte = listAccounts; compte != nullptr; compte = compte->next)
        if ((i = strcmp(name,compte->id)) == 0)
            return compte;
        else if (i < 0) break;
            return nullptr;
}

void Machine::openNewAccount(void)
{
    LISTCOMPTE *compte, *pred;
    char name[MAX_ID_COMPTE];
    if (GetAccountId(name)) {
        if (GetAccount(name) != nullptr)
            logger() << "Le compte " << name << " existe déja. Votre opération est annulée." << std::endl;
        else if ((compte = (LISTCOMPTE *)malloc(sizeof(LISTCOMPTE))) != nullptr) {
            strncpy(compte->id,name,MAX_ID_COMPTE); compte->amount = 0;
        if (listAccounts == nullptr || strcmp(listAccounts->id,compte->id) > 0) {
            compte->next = listAccounts; listAccounts = compte;
        }
        else {
            for (pred = listAccounts; pred->next; pred = pred->next)
                if (strcmp(pred->next->id,compte->id) > 0)
                    break;
            compte->next = pred->next; pred->next = compte;
        }
        currentAccount = compte;
        logger() << "Solde du compte " << compte->id << ": " << currentAccount->amount << std::endl;
        }
    }

    m_key.release();
}


void Machine::openOldAccount(void)
{
    char name[MAX_ID_COMPTE];
    if (GetAccountId(name)) {
        if ((currentAccount = GetAccount(name)) == nullptr)
            logger() << "Le compte " << name << " n'existe pas." << std::endl;
        else
            logger() << "Solde du compte " << name << ": " << currentAccount->amount << std::endl;

    }
    m_key.release();
}


void Machine::closeCurrentAccount(void)
{
    if (currentAccount != nullptr)
        logger() << "Fermeture du compte " << currentAccount->id << " "
"avec solde de " << currentAccount->amount << std::endl;

    currentAccount = nullptr;
}







unsigned Machine::getInventoryArticle(ARTICLE item)
{
    return InventoryItem[item];
}

unsigned Machine::getInventoryCoin(COIN coin)
{
    return InventoryCoins[coin-1];
}


COIN Machine::getCoin()
{
    m_coin.acquire();
    return  bufferCoinsIntroduction.get();
}


ARTICLE Machine::getArticle()
{
    m_article.acquire();
    return bufferArticleIntroduction.get();
}


bool Machine::isOpenAccount()
{
    return currentAccount != nullptr;
}

int Machine::getCreditOpenAccount()
{
    if (currentAccount != nullptr)
        return currentAccount->amount;
    else
        return -1;
}

int Machine::updateOpenAccount(int amount)
{
    if (currentAccount != nullptr)
        if (currentAccount->amount + amount >= 0)
            return currentAccount->amount += amount;
        else
            return currentAccount->amount = 0;
    else
        return -1;
}

KEY_STATE Machine::getKeyState(void)
{
    return Key_State;
}

void Machine::resetKeyFunction(void)
{
    Key_State = KEY_UNDEFINED;
}





void Machine::ejectCoin(COIN coin)
{
    if (coin > 0 && coin <= 9) {
        PcoThread::usleep(1000000);
        m_mutex.lock();
        if (InventoryCoins[coin-1] > 0) {
            InventoryCoins[coin-1] -= 1;
            logger() << "Sortie pièce " << coin << std::endl;

        }
        m_mutex.unlock();
    }
}

void Machine::ejectArticle(ARTICLE item)
{
    if (item >= 0 /* && item < MAX_ARTICLES*/) {
        PcoThread::usleep(2000000);
        m_mutex.lock();
        if (InventoryItem[item] > 0) {
            InventoryItem[item] -= 1;
            logger() << "Sortie " << std::endl;
            switch (item) {
                case CHOCOLATE: logger() << "Chocolate" << std::endl; break;
                case CANDYCANE: logger() << "Candy cane" << std::endl; break;
                case GUMMYBEAR: logger() << "Gummy bear" << std::endl; break;
                default: logger() << "Lollipop" << std::endl;
            }
        }
        m_mutex.unlock();
    }
}






void Machine::processKey()
{
    char car;


    while (!shouldQuit()) {

       // logger() << "Entrez une commande ";

       std::cin >> car;
       // logger() << "Char: " << car << std::endl;
       switch (car) {
           case '&':
               Key_State = KEY_YES;
           break;
           case '/':
               Key_State = KEY_NO;
           break;
           case '1':  case '2':  case '3':  case '4':  case '5':  case '6':
           case '7':  case '8':  case '9':
               car -= '0';
               if (bufferCoinsIntroduction.put(car)) {
                   m_mutex.lock();
                   InventoryCoins[car - 1] ++;
                   m_mutex.unlock();
               }
               m_coin.release();
           break;
           case '+':
               bufferArticleIntroduction.put(CHOCOLATE);m_article.release();
           break;
           case '"':
               bufferArticleIntroduction.put(CANDYCANE);m_article.release();
           break;
           case '*':
               bufferArticleIntroduction.put(GUMMYBEAR);m_article.release();
           break;
           case '%':
               bufferArticleIntroduction.put(LOLLIPOP);m_article.release();
           break;
           case '?':
               openNewAccount();
//               openAction(true);
           break;
           case '(':
            openOldAccount();
//               openAction(false);
           break;
           case ')':
               closeCurrentAccount();
//               closeAction();
           break;
           case 'q':
               this->quit();
               return;
           default:
           break;
        }
     }

}


bool Machine::shouldQuit()
{
    m_mutex.lock();
    auto result = m_shouldQuit;
    m_mutex.unlock();
    return result;
}


void Machine::quit()
{
    m_mutex.lock();
    m_shouldQuit = true;
    m_mutex.unlock();
    bufferCoinsIntroduction.put(0);
    bufferArticleIntroduction.put((ARTICLE)0);

    m_openAccount.release();
    m_closeAccount.release();
    m_key.release();
    m_coin.release();
    m_article.release();
}

