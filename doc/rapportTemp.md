## Appel à la méthode updateOpenAccount
La modification à la méthode de mise à jour du compte 
est également une variable partagée entre le thread money et le thread merchandise.

Après avoir étudier la méthode, il s'avère qu'elle ne soit pas thread-safe.
Il faut donc protéger cette section critique à l'aide d'un mutex.

# Tests
Pour vérifier le bon fonctionnement de l'application, nous avons préparer une batterie de tests.

Chaque test est un scénario d'utilisation 
de la machine et comporte un ensemble d'actions avec chacune un resultat attendu.

Un test est alors réussi, uniquement si l'ensemble des resultats attendus sont obtenus.

## Tests des fonctionnalités sans compte
### Acheter un article avec la monnaie exacte

| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 2 | Le solde du crédit insérée est augmenté de 2
| Demander l'article Candy cane | L'article sors et le solde du crédit insérée passe à 0

### Acheter un article qui n'est plus en stock
Pour faciliter ce test, le tableau *InventoryItem* dans le fichier machine.cpp a été modifié.
Le nombre de l'article *Chocolate* disponible dans la machine a été passé à 0.

| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 1 | Le solde du crédit insérée est augmenté de 1
| Demander l'article Chocolate | la transaction n'est pas effectuée et l'utilisateur est prevenu que l'article n'est plus disponible

### Acheter un article avec pas assez de crédit
| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 1 | Le solde du crédit insérée est augmenté de 1
| Demander l'article Candy cane | La transaction n'est pas effectuée et l'utilisateru est prevenu que son solde est insuffisant.

### Acheter un article avec plus de monnaie que nécessaire
Ce test est séparé en 2 parties, selon si la machine possède les pièces nécessaire pour pouvroir rendre le change.

#### La machine possède les pièces nécessaires
| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 2 | Le solde du crédit insérée est augmenté de 2
| Demander l'article Chocolate | La machine sors l'article et une pièce de 1, le solde du crédit insérée est mis à 0.

#### La machine ne possède pas les pièces nécessaires
Pour ce test, le tableau *InventoryCoins*, du fichier machine.cpp à été modifié pour que la machine ne contienne aucune pièce au départ.

| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 2 | Le solde du crédit insérée est augmenté de 2
| Demander l'article Chocolate | La machine previent l'utilisateur qu'elle ne pourra pas rendre le change et demande si il veut poursuivre l'achat.
| 1. L'utilisateur accepte | La machine sors l'article et le credit de l'utilisateur passe à 0
| 2. L'utilisateur refuse | La transaction est annulée

## Test des fonctionnalitées avec un compte
### Acheter un article avec la monnaie exacte

| Action | Résultat attendu |
|---|---| 
| Demander l'ouverture d'un compte | La machine demande à l'utilisateur un nom de compte
| Tapper compteTest | Le compte compteTest est crée et ouvert 
| Insérer une pièce de 2 | Le solde du compte est augmenté de 2
| Demander l'article Candy cane | L'article sors et le solde du compte passe à 0

### Acheter un article qui n'est plus en stock
Pour faciliter ce test, le tableau *InventoryItem* dans le fichier machine.cpp a été modifié.
Le nombre de l'article *Chocolate* disponible dans la machine a été passé à 0.

| Action | Résultat attendu |
|---|---| 
| Demander l'ouverture d'un compte | La machine demande à l'utilisateur un nom de compte
| Tapper compteTest | Le compte compteTest est crée et ouvert 
| Insérer une pièce de 1 | Le solde du compte est augmenté de 1
| Demander l'article Chocolate | la transaction n'est pas effectuée et l'utilisateur est prevenu que l'article n'est plus disponible

### Acheter un article avec pas assez de crédit
| Action | Résultat attendu |
|---|---| 
| Demander l'ouverture d'un compte | La machine demande à l'utilisateur un nom de compte
| Tapper compteTest | Le compte compteTest est crée et ouvert 
| Insérer une pièce de 1 | Le solde du compte est augmenté de 1
| Demander l'article Candy cane | La transaction n'est pas effectuée et l'utilisateru est prevenu que son solde est insuffisant.

### Acheter un article avec plus de monnaie que nécessaire

| Action | Résultat attendu |
|---|---| 
| Demander l'ouverture d'un compte | La machine demande à l'utilisateur un nom de compte
| Tapper compteTest | Le compte compteTest est crée et ouvert 
| Insérer une pièce de 2 | Le solde du compte est augmenté de 2
| Demander l'article Chocolate | L'article sors et le solde du compte passe à 1

### Acheter un article avec un credit suffisant
| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 2 | Le solde du crédit insérée est augmenté de 2
| Demander l'ouverture d'un compte | La machine demande à l'utilisateur un nom de compte
| Tapper compteTest | Le compte compteTest est crée et ouvert 
| Insérer une pièce de 2 | Le solde du compte est augmenté de 2
| Demander l'article Chocolate | L'article sors et le crédit insérée passe à 1

### Acheter un article avec un credit insuffisant
| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 1 | Le solde du crédit insérée est augmenté de 1
| Demander l'ouverture d'un compte | La machine demande à l'utilisateur un nom de compte
| Tapper compteTest | Le compte compteTest est crée et ouvert
| Insérer une pièce de 2 | Le solde du compte est augmenté de 2
| Demander l'article Candy Cane | L'article sors, le crédit insérée passe à 0 et le solde du compte passe à 1

### Ouvrir un compte avec du credit puis le refermer sans l'avoir dépensé
| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 1 | Le solde du crédit insérée est augmenté de 1
| Demander l'ouverture d'un compte | La machine demande à l'utilisateur un nom de compte
| Tapper compteTest | Le compte compteTest est crée et ouvert
| Insérer une pièce de 2 | Le solde du compte est augmenté de 2
| Fermer le compte | Le compte est fermé et le crédit insérée est de 1


## Test des cas limites
### Acheter un article et insérer une pièce en même temps avec et sans compte
| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 2 | Le solde du crédit insérée est augmenté de 2
| Demander l'article Candy cane en insérant une pièce de 1 | L'article sors et le solde du crédit insérée passe à 1

### Acheter 2 articles simultannément avec pas assez de crédit
| Action | Résultat attendu |
|---|---|
| Insérer une pièce de 1 | Le solde du crédit insérée est augmenté de 1
| Demander l'article Chocolate 2 fois rapidement | La 1ere transaction est effectué, mais pas la 2ème

