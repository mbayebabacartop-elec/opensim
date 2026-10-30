# Étape 2 — état du squelette natif

Date : 2026-10-30. L’étape est partiellement réalisée et le premier incrément Windows est compilé et lancé. La migration vers QML et le packaging redistribuable restent ouverts.

## Livrables

- Application C++20 construite avec CMake et MSVC 2022.
- Bibliothèque `openlab_core` distincte et sans dépendance Qt ; elle ne contient pour l’instant que l’identification de version, aucun modèle physique.
- Fenêtre Qt Widgets native avec menu Quitter fonctionnel, À propos, panneau bibliothèque explicitement vide et état de démarrage honnête.
- Journalisation Qt vers `%LOCALAPPDATA%\OpenLab\OpenLab Simulator\openlab.log`.
- Presets Windows x64 Release et test ; CTest et test de fumée du Core.
- Script `run-local.bat` pour ajouter les DLL Qt à la recherche locale et lancer le build courant.

## Dépendances installées pendant l’étape

| Outil | Version / état |
|---|---|
| Git for Windows | 2.56.0.3 |
| CMake | 4.4.4, archive officielle extraite dans `%LOCALAPPDATA%\Programs\CMake` |
| Visual Studio Build Tools | 2022 17.14.42 ; MSVC 19.44.35229 ; SDK 10.0.26100 |
| Qt | 6.10.3 MSVC 2022 x64, Qt Widgets utilisable ; installation QML incomplète |
| Qt Test / GoogleTest | non installés ; test initial fait sans framework tiers |

L’installateur Visual Studio a demandé un redémarrage pour finaliser, mais aucun redémarrage n’a été fait. Le cache des paquets VS a été désactivé pour récupérer de l’espace. Le système de fichiers C: est descendu à environ 85 Mio libres.

## Vérifications effectuées

- CMake configure : réussi, SDK 10.0.26100 sélectionné, MSVC 19.44.35229 détecté.
- Build Visual Studio Release : réussi pour `openlab_core`, `openlab_core_smoke` et `openlab_app`.
- CTest : `openlab.core.smoke` — 1/1 réussi.
- Lancement de `openlab_app.exe` : processus resté actif ; journal créé et alimenté.
- Aucun test physique ou de simulation exécuté.

## Limites et problèmes

1. Le paquet Qt 6.10.3 QML a échoué par manque d’espace pendant l’installation. Les répertoires QML incomplets ne sont pas utilisés par l’application courante.
2. La version 6.10.3 est provisoire ; la cible de licence et de maintenance Qt doit être revalidée avant toute distribution.
3. La vue centrale n’édite encore aucun système et la bibliothèque ne contient aucun composant. Ce n’est pas un simulateur fonctionnel à ce stade.
4. L’application n’est pas encore empaquetée avec ses DLL et plugins Qt ; `run-local.bat` n’est qu’un lanceur de développement.
5. La configuration actuelle est Release seulement ; les import libraries Qt de debug ne sont pas une cible de ce build.
6. Git est installé ; l’initialisation et le commit seront faits après documentation finale de l’étape.

## Suite nécessaire

Récupérer de l’espace disque sans toucher aux données personnelles, reprendre l’installation QML, revoir les ADR de l’interface, et établir un lancement autonome avec `windeployqt` avant de clore l’étape 2. Le moteur CC reste réservé à une étape ultérieure.


