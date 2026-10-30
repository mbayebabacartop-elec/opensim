# OpenLab Simulator

OpenLab Simulator vise un laboratoire numérique scientifique : construction de systèmes, moteurs de simulation spécialisés, instruments, vues 2D/3D et expériences pédagogiques.

## État du projet

- Étapes 0–1 : audit, architecture, ADR, licences et validation scientifique documentés.
- Étape 2 : squelette C++20/CMake compilé et lancé sur Windows avec MSVC 2022 et Qt Quick/QML.
- Test initial du Core : réussi.
- Aucun moteur de simulation ni composant physique n’est encore inclus. L’interface annonce explicitement cet état.
- Qt 6.10.3 est la version installée pour le prototype local ; elle doit être remplacée par une version maintenue avant distribution.

Voir [le rapport de l’étape 2](docs/planning/STEP-2-STATUS.md), [l’audit système](docs/SYSTEM-AUDIT.md), [l’architecture](docs/architecture/ARCHITECTURE.md) et [le plan](docs/planning/ROADMAP.md).

## Architecture

Qt Quick/QML fournit l’interface ; le Core C++ reste indépendant de Qt et des solveurs. Les moteurs spécialisés seront intégrés derrière des contrats versionnés. Les instruments et vues devront consommer des résultats calculés, typés et exprimés en unités SI.

Les décisions sont consignées dans les [ADR](docs/architecture) et la matrice [dépendances/licences](docs/DEPENDENCIES-LICENCES.md).

## Build Windows

Depuis le dossier du projet :

```powershell
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
.\run-local.bat
```

Le preset cible MSVC x64 et Qt dans `C:/Qt/6.10.3/msvc2022_64`. `run-local.bat` est un lanceur de développement, pas un installateur redistribuable.

## Règles scientifiques

Chaque modèle devra documenter équations, unités SI, conventions, hypothèses, plage de validité, provenance des paramètres et limites. Aucun résultat ne sera présenté comme simulé sans calcul et validation reproductibles. Voir [VALIDATION.md](docs/planning/VALIDATION.md).
