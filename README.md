# OpenLab Simulator

OpenLab Simulator est un prototype natif Windows de laboratoire numerique pour l'apprentissage scientifique et technique. Son architecture separe l'interface Qt Quick, le Core C++ et les moteurs de simulation.

## Etat actuel

- Etapes 0 a 3 : audit, architecture, Core SI, ports/composants et format de projet `.olsim`.
- Etape 4 : solveur RL serie ideal, reference analytique et verification de convergence selon le pas.
- Etape 5 : banc RL desktop avec parametres reglables, calcul en arriere-plan annulable, courbe, sauvegarde/rechargement `.olsim` et export CSV complet.
- Les trois tests automatises passent. Voir le [rapport de l'etape 5](docs/planning/STEP-5-STATUS.md) et la [feuille de route](docs/planning/ROADMAP.md).
- Qt 6.10.3 est une dependance provisoire du poste ; une version maintenue est requise avant distribution.

## Lancement sous Windows

Double-cliquer sur le raccourci `OpenLab Simulator.lnk` du bureau, ou executer `run-local.bat` depuis le dossier du projet. Qt doit etre installe dans `C:/Qt/6.10.3/msvc2022_64`. C'est un lanceur de developpement, pas encore un installateur redistribuable.

## Compilation et verification

```powershell
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

## Limites scientifiques

Seul un circuit RL serie ideal soumis a un echelon DC est simule. Ce logiciel n'est pas encore un editeur SPICE general, un simulateur de moteur, une scene 3D ou un outil d'analyse de securite. Chaque modele doit documenter equations, unites SI, hypotheses, donnees de validation et limites.

## Licences

Consulter les [dependances et licences](docs/DEPENDENCIES-LICENCES.md) et les notices tierces avant redistribution. Aucun installateur ni SBOM de release n'a ete produit.