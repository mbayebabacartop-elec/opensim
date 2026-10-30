# OpenLab Simulator

OpenLab Simulator est une application native Windows de laboratoire numerique pour l'apprentissage scientifique et technique. Son architecture separe l'interface Qt Quick, le Core C++ et les moteurs de simulation.

## Etat actuel

- Etapes 0 a 3 : audit, architecture, Core SI, ports/composants et format de projet `.olsim`.
- Etape 4 : solveur transitoire RL serie ideal, reference analytique et verification de convergence.
- Etape 5 : banc RL desktop, calcul annulable en arriere-plan, courbe, sauvegarde/rechargement `.olsim` et export CSV.
- Etape 6 (premier increment) : espaces Atelier, Laboratoire et Cours ; solveur couple d'un moteur CC ; visualiseur 3D pedagogique pilote par la vitesse simulee.
- Etape 8 (moteur initial) : solveur MNA transitoire lineaire R/C/L et sources DC, couvert par des cas RC et RL analytiques.
- Cinq tests automatises couvrent la persistance, le RL, le moteur CC, MNA et le controleur desktop. Voir le [rapport de l'etape 6](docs/planning/STEP-6-STATUS.md), le [rapport du moteur MNA](docs/planning/STEP-8-STATUS.md), et la [feuille de route](docs/planning/ROADMAP.md).
- Qt 6.10.3 est une dependance provisoire du poste ; une version maintenue est requise avant distribution.

## Lancement sous Windows

Double-cliquer sur le raccourci `OpenLab Simulator.lnk` du bureau ou executer `run-local.bat` depuis le dossier du projet. Qt doit etre installe dans `C:/Qt/6.10.3/msvc2022_64`. C'est un lanceur de developpement, pas encore un installateur redistribuable.

## Compilation et verification

```powershell
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

## Limites scientifiques et fonctionnelles

Les modeles actifs incluent RL serie, moteur CC ideal couple et un moteur MNA lineaire R/C/L utilisable par l'API mais pas encore par un banc graphique. Le moteur 3D est un maillage procedural en projection logiciel pedagogique, pas un modele constructeur, un outil CAD, ni un renderer temps reel general. L'Atelier affiche le schema fonctionnel du moteur, mais le placement libre et le routage de fils ne sont pas encore implantes. Pas encore d'analyse AC, sources pilotees, composants non lineaires, hydraulique ou mecanique generale. Ne pas utiliser les resultats comme validation de securite ou dimensionnement industriel.

## Licences

Consulter les [dependances et licences](docs/DEPENDENCIES-LICENCES.md) et les notices tierces avant redistribution. Aucun installateur ni SBOM de release n'a ete produit.
