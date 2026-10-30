# OpenLab Simulator

OpenLab Simulator vise un laboratoire numérique scientifique : construction de systèmes, moteurs de simulation spécialisés, instruments, vues 2D/3D et expériences pédagogiques.

## État

- Étapes 0 et 1 : audit, architecture et ADR documentés.
- Étape 2 : premier squelette natif créé, compilé et lancé avec MSVC 2022 et Qt Widgets 6.10.3 ; test Core de fumée réussi.
- Limite importante : Qt Quick/QML n’a pas pu finir de s’installer faute d’espace disque. Le shell de ce premier incrément utilise Qt Widgets. La migration vers l’interface QML reste à faire après rétablissement de l’espace et réévaluation de la version Qt.
- Aucun moteur de simulation n’est implémenté. Les équations du moteur CC ne sont pas exécutées.

Voir [le rapport de l’étape 2](docs/planning/STEP-2-STATUS.md), [l’audit système](docs/SYSTEM-AUDIT.md) et [le plan](docs/planning/ROADMAP.md).

## Architecture

Une interface pilote un Core indépendant, qui orchestre des moteurs spécialisés derrière des contrats versionnés. Les instruments et vues consomment des résultats calculés et typés en unités SI. Le Core n’intègre pas Qt ni un moteur physique particulier.

Les décisions sont consignées dans [ARCHITECTURE.md](docs/architecture/ARCHITECTURE.md), les [ADR](docs/architecture) et [DEPENDENCIES-LICENCES.md](docs/DEPENDENCIES-LICENCES.md).

## Build Windows

Après installation de la chaîne native, depuis le dossier du projet :

```powershell
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
.\run-local.bat
```

Le preset attend CMake 3.25+ et Qt 6 MSVC x64. Il pointe pour l’instant vers `C:/Qt/6.10.3/msvc2022_64`. Les scripts ne déploient pas encore une version redistribuable de l’application.

## Règles scientifiques

Les modèles doivent documenter équations, unités SI, conventions, hypothèses, plage de validité, provenance des paramètres et limites. Aucun résultat de simulation ne doit être fabriqué ou annoncé sans calcul et validation reproductibles. Voir [VALIDATION.md](docs/planning/VALIDATION.md).

