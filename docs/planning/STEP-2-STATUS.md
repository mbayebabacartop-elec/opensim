# Étape 2 — squelette natif

Date : 2026-10-30. Premier incrément Windows compilé, testé et lancé. Le dépôt Git est initialisé.

## Livrables

- Application C++20 construite avec CMake et MSVC 2022.
- Core statique indépendant de Qt, limité pour l’instant à l’identification de version.
- Interface Qt Quick/QML intégrée aux ressources de l’application : fenêtre principale, bibliothèque vide explicitement signalée, menus Quitter et À propos, état de simulation inactif.
- Journalisation Qt dans `%LOCALAPPDATA%\OpenLab\OpenLab Simulator\openlab.log`.
- Presets Windows x64 Release ; test de fumée Core sous CTest.
- Lanceur local `run-local.bat` configurant la recherche des DLL et plugins Qt.

## Dépendances installées

| Outil | Version / état |
|---|---|
| Git for Windows | 2.56.0.3 |
| CMake | 4.4.4 |
| Visual Studio Build Tools | 2022 17.14.42 ; MSVC 19.44.35229 ; SDK 10.0.26100 |
| Qt | 6.10.3 MSVC 2022 x64 ; Qt Quick/QML et Quick Controls installés |
| GoogleTest / Qt Test | non installés ; test initial minimal sans framework tiers |

Qt 6.10.3 est provisoire et sa période de support standard est échue. Les essais aqtinstall 3.3.0 pour Qt 6.11/6.12 n’ont pas permis de récupérer les index nécessaires. Ne pas distribuer ce build avant d’avoir sélectionné et validé une version Qt maintenue et sa conformité de licence. Qt Quick 3D et Qt Graphs ne sont pas utilisés.

KiCad 10.0 a été désinstallé à la demande de l’utilisateur afin de récupérer de l’espace. Les projets dans Documents ont été préservés ; le volume C: dispose maintenant d’environ 4 Gio libres après l’installation QML et le build.

## Vérifications effectuées

- Configuration CMake : réussie avec le SDK 10.0.26100 et MSVC 19.44.35229.
- Build Release : réussi pour `openlab_core`, `openlab_core_smoke` et `openlab_app`.
- CTest : `openlab.core.smoke` — 1/1 réussi.
- Lancement par `run-local.bat` : fenêtre détectée et processus actif ; journal créé.
- QML : ressource compilée et chargée ; aucun avertissement QML nouveau après correction de la boîte À propos.
- Liens Markdown internes : vérifiés.

## Limites

1. Aucun composant ni moteur physique n’est implémenté.
2. Le Core n’a ni graphe de projet, ni unités/ports, ni sauvegarde/chargement ; cela constitue l’étape suivante.
3. La fenêtre est un shell utilisable, pas encore un éditeur de systèmes.
4. Le binaire n’est pas empaqueté pour distribution ; `run-local.bat` requiert Qt présent localement.
5. Qt 6.10.3 ne doit pas être considéré comme une base maintenue pour une release.

## Prochaine étape

Étape 3 : contrat de composants/ports et quantités SI, graphe de projet, horloge/événements et stratégie de persistance `.olsim`, avec migrations et tests de round-trip avant le premier moteur physique.
