# Exclusion mutuelle et partage d’information

## Nelson JEANRENAUD - Théo MIRABILE

# Problématique

L'objectif de ce laboratoire est de réaliser le logiciel d'un distributeur de marchandise afin de mettre en pratique les notions d'exclusion mutuelle vues en cours.

Le distributeur permet d'acheter 4 variétés d'articles différents avec du crédit introduit pièce par pièce. Les utilisateurs peuvent sauvegarder leur crédit en ouvrant des comptes. La machine doit gérer l'inventaire des pièces disponibles et rendre la monnaie après l'achat d'un article si possible. Dans le cas où le rendu n'est pas possible, l'utilisateur a le choix d'annuler son achat ou d'acheter l'article en acceptant de ne pas recevoir la monnaie exacte.

# Choix d'implémentation

## Ressources partagées à protéger

### Variable partagée crédit

Pour pouvoir gérer le montant inséré par un utilisateur lorsqu'aucun compte n'est sélectionné, nous avons décidé d'utiliser une variable de type entier. La valeur est donc modifiée par le thread `Money` lorsqu'une pièce est insérée et le thread `Merchandise` lorsqu'un article est acheté.

Cette variable étant partagée entre les deux threads, sa lecture et modification sont donc des sections critiques. Nous avons donc utilisé un mutex comme mécanisme d'exclusion mutuelle.

### Appels aux méthodes `getCreditOpenAccount` et `updateOpenAccount`

Les méthodes de récupération et mise à jour du crédit d'un compte ouvert sont utilisées dans nos deux threads, ce qui implique qu'il s'agit d'une ressource partagée.

Après avoir étudié les méthodes, il s'avère qu'elles ne sont pas *thread-safe* (protégées contre des accès concurrents). Il faut donc protéger les sections critiques à l'aide d'un mutex.

## Découpage du code

Nous avons décidé d'implémenter les fonctionnalités suivantes sous forme de fonctions :

- `showBalance` pour l'affichage du solde restant (pièces insérées + montant du compte si ouvert),
- `buyArticle` pour l'achat d'un article avec vérification du solde et rendu de la monnaie le cas échéant,
- `returnChange` pour le retour effectif des pièces à l'utilisateur ("éjection" des pièces)

Ces fonctions sont complémentaires à celles qui étaient déjà proposées, soit

- `amountToReturn` pour le calcul du montant à rendre
- `displayArticle` pour l'affichage de l'article

# Scénarios de test

Pour vérifier le bon fonctionnement de l'application, nous avons préparé une batterie de tests.

Chaque test est un scénario d'utilisation de la machine et comporte un ensemble d'actions avec chacune un résultat attendu.

Un test est alors réussi uniquement si l'ensemble des résultats attendus sont obtenus.

## Tests des fonctionnalités sans avoir de compte ouvert

### Acheter un article avec la monnaie exacte

[Untitled](https://www.notion.so/204a6e2940f34b3688d25b684e29984f)

### Acheter un article qui n'est plus en stock

Pour faciliter ce test, le tableau `InventoryItem` dans le fichier *machine.cpp* a été modifié.
Le nombre de l'article *Chocolate* disponible dans la machine a été passé à 0.

[Untitled](https://www.notion.so/345d51bd1761492aa08c6445098162bc)

### Acheter un article avec un crédit insuffisant

[Untitled](https://www.notion.so/fd312d8699194f818fd145cf826c2613)

### Acheter un article avec plus de monnaie que nécessaire

Ce test est séparé en deux parties, selon si la machine possède les pièces nécessaire pour pouvroir rendre le change.

### La machine possède les pièces nécessaires

[Untitled](https://www.notion.so/d810fa468cd8411d8a67a01df30739aa)

### La machine ne possède pas les pièces nécessaires

Pour ce test, le tableau `InventoryCoins` du fichier machine.cpp a été modifié pour que la machine ne contienne aucune pièce au départ.

[Untitled](https://www.notion.so/26202ca21bed4f9f8e914a0c0d4702f2)

## Test des fonctionnalitées avec un compte

### Acheter un article avec la monnaie exacte

[Untitled](https://www.notion.so/c6eaf62cf95d4961a5a1c0468e0379a9)

### Acheter un article qui n'est plus en stock

Pour faciliter ce test, le tableau `InventoryItem` dans le fichier machine.cpp a été modifié.
Le nombre de l'article *Chocolate* disponible dans la machine a été passé à 0.

[Untitled](https://www.notion.so/58f4375c5bac4ff3b8cb45a248875338)

### Acheter un article avec un crédit insuffisant

[Untitled](https://www.notion.so/a569b5abb05541d58208180c8069c419)

### Acheter un article avec plus de monnaie que nécessaire

[Untitled](https://www.notion.so/47293ace52384f2180cae550b1b7552d)

### Acheter un article avec un credit suffisant

[Untitled](https://www.notion.so/d7974cf377e847238e3653275b94d376)

### Acheter un article avec un credit insuffisant

[Untitled](https://www.notion.so/ef7a941569824ab8baa95b8f7a436102)

### Ouvrir un compte avec du credit puis le refermer sans l'avoir dépensé

[Untitled](https://www.notion.so/7a32a913251a4be49c3d1e9c425a16ec)

## Test des cas limites

### Acheter un article et insérer une pièce en même temps avec et sans compte

[Untitled](https://www.notion.so/85d29071a82f4025a1aac9784a9461ce)

### Acheter 2 articles simultanément avec un crédit insuffisant

[Untitled](https://www.notion.so/8bd90c6f8d7a470a922dd4022ccf7ea3)