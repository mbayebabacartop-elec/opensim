# Audit système — 2026-10-30

## Résumé

L’environnement est un Windows x64 avec 16 Gio de RAM et un processeur mobile 2 cœurs / 4 fils. Il est utilisable pour un poste de développement et des simulations pédagogiques modestes ; le rendu 3D détaillé et les grands systèmes devront être adaptatifs. Les outils de compilation natifs recommandés ne sont pas détectés. Aucun logiciel n’a été installé pendant cet audit.

## Machine et système

| Élément | Observation |
|---|---|
| Système | Windows 10 Pro, version 2009, build 22000 (marketing Windows 11), x64 |
| Processeur | Intel Core i5-6300U @ 2,40 GHz, 2 cœurs, 4 processeurs logiques |
| Mémoire | 16 972 673 024 octets installés (≈15,8 Gio) ; environ 8,5 Gio libres lors de la première mesure |
| Graphique | Intel HD Graphics 520, pilote 31.0.101.2111, mémoire déclarée 1 Gio |
| Python | Python 3.14.6 détecté via `py`; pip 26.1.2 |
| Paquets Python observés | NumPy 2.5.4, OpenCV contrib 5.0.0.93, Pillow 13.0.0 |

Le build 22000 correspond à Windows 11, malgré le libellé retourné `Windows 10 Pro` par la classe système ; éviter de conclure à un OS Windows 10 à partir du seul nom produit. Qt 6.12 annonce Windows 10 1809+ et Windows 11 avec MSVC 2022, mais la configuration réellement installée doit être validée au moment du setup.

## Outils de développement

| Outil | Résultat d’audit |
|---|---|
| Git | absent du `PATH` et emplacement standard non trouvé |
| CMake | absent du `PATH` et emplacement standard non trouvé |
| MSVC `cl` | absent du `PATH`; aucune installation VS 2022 standard trouvée |
| Clang / GCC | absents du `PATH` |
| Ninja | absent du `PATH` |
| Qt (`qtpaths`, `qmake`, `windeployqt`) | absents du `PATH`; `C:\Qt` non trouvé |
| Windows SDK | répertoire standard `Windows Kits\10\Include` non trouvé |
| GitHub CLI | présent (`gh.exe`), ce n’est pas Git |
| winget | présent; `winget list` montre Arduino IDE 2.3.10 notamment |
| Arduino IDE | installé, utile plus tard pour comparer les cartes, ne fournit pas à lui seul la chaîne Qt/MSVC du projet |

Un ancien répertoire Visual Studio 14.0 partiel existe mais ne contient pas `VC\bin\cl.exe`. L’inventaire `winget list` est une photographie du catalogue et ne remplace pas une inspection de chaque installateur ; le constat fiable ici est l’absence des outils requis du PATH et des emplacements standards vérifiés.

## Dépôt et prototype existant

Le dossier `C:\Users\User\Desktop\CircuitLab3D` contient un prototype web (HTML/CSS/JavaScript) : `index.html`, `app.js`, `simulator.js`, `asset_store.js`, `interop.js`, `lab_assistant.js`, `style.css`, `run.bat`, `README.md` et documents de conception. Il ne constitue pas un dépôt OpenLab natif C++/Qt et aucun dossier `.git` n’a été observé dans les emplacements inspectés. Le prototype comprend des fonctions exploratoires et un solveur simple ; il n’a pas été considéré ici comme moteur validé.

Le nouveau projet documentaire est créé dans `C:\Users\User\Desktop\OpenLabSimulator`, sans déplacer ni modifier CircuitLab3D. Git n’étant pas disponible, aucun commit n’a pu être créé. Installer Git sera un prérequis de l’étape 2, avant l’ajout du code.

## Impacts

1. La prochaine étape de développement devra installer/configurer une seule chaîne cohérente : Visual Studio 2022 Build Tools avec C++ desktop et Windows SDK, CMake récent, Ninja si utile, Qt Desktop MSVC x64 ciblé.
2. Ne pas installer tous les modules Qt : vérifier modules réellement LGPL/GPL et besoins de rendu avant sélection. Qt Quick 3D est distribué sous GPL dans la Community Edition, ce qui pose une incompatibilité potentielle avec une stratégie Open Core propriétaire ; il ne doit donc pas être retenu sans étude de licence dédiée. On peut d’abord garder la vue 3D derrière une interface de rendu et envisager une bibliothèque permissive ou modèle d’édition commerciale adapté.
3. Prévoir des paramètres qualité bas pour Intel HD 520 et garder le solveur hors du thread UI.
4. Réaliser un build de référence et établir les prérequis système exacts avant d’annoncer Windows/Linux comme officiellement supportés.

## Méthode et limites

Audit par PowerShell, `Get-ComputerInfo`, CIM, recherche des exécutables et répertoires standards, puis inventaire `winget list`. Il est en lecture seule. Les versions de dépendances mentionnées dans les documents sont des observations ou des pages officielles consultées le 2026-10-30 ; elles devront être revérifiées avant intégration. Aucune compilation, installation, simulation ou test logiciel n’a été exécuté.

## Installation initiale des outils — 2026-10-30 (avant récupération d’espace)

- Git for Windows 2.56.0.3 installé.
- CMake 4.4.4 installé depuis l’archive officielle dans `%LOCALAPPDATA%\Programs\CMake` ; le cache winget a présenté un verrou, l’archive avec hachage vérifié a été extraite manuellement.
- Visual Studio Build Tools 2022 17.14.42 installé avec MSVC 19.44.35229 et SDK Windows 10.0.26100. L’installateur recommande un redémarrage, non effectué.
- Qt Widgets 6.10.3 MSVC x64 installé en premier. Le paquet Qt Quick/QML s’est initialement arrêté faute d’espace ; l’utilisateur a ensuite demandé la désinstallation de KiCad, ce qui a permis de terminer Qt Declarative.
- Le cache VS a été désactivé via l’installateur. Lors de cette première étape, le volume C: est descendu sous 100 Mio libres ; KiCad a ensuite été désinstallé explicitement à la demande de l’utilisateur pour récupérer de l’espace.
- Build CMake/MSVC Release, test de fumée Core, CTest et lancement du shell ont été vérifiés. Voir [STEP-2-STATUS.md](planning/STEP-2-STATUS.md).

## Désinstallation KiCad et reprise Qt — 2026-10-30

KiCad 10.0 a été désinstallé à la demande de l’utilisateur après avoir constaté qu’il occupait environ 4,46 Gio dans `C:\Program Files\KiCad\10.0`. `winget list` ne le détecte plus ; environ 4,5 Gio ont été récupérés. Les dossiers `Documents\KiCad` et `Documents\kicad_projects` ont été vérifiés et sont toujours présents.

L’espace récupéré a permis de terminer le paquet Qt Declarative/QML 6.10.3. Le shell Qt Quick/QML a été compilé, lancé et observé sans nouvel avertissement QML dans le journal après correction du dialogue À propos. Le build CMake Release et le test de fumée Core ont repassé.

