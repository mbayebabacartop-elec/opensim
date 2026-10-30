# ADR-0004 — premier shell Qt Quick/QML

- **Statut :** implémenté pour le premier incrément ; le choix de version Qt reste provisoire
- **Date :** 2026-10-30

## Contexte

L’architecture initiale visait Qt 6/QML. Le premier build, fait avant l’installation de Qt Declarative, utilisait Widgets. Après désinstallation de KiCad à la demande de l’utilisateur et récupération d’espace disque, Qt Quick/QML a pu être installé.

## Décision actuelle

Le shell natif utilise Qt Quick/QML avec ressource QML intégrée au binaire. Le Core reste indépendant de Qt. La bibliothèque et les états affichés signalent explicitement l’absence de moteur physique.

La version locale est Qt 6.10.3. Elle est uniquement un outil de développement pour ce squelette ; le projet doit passer à une version maintenue et revérifier licences/modules avant distribution.

## Conséquences

- Le shell QML est compilé par `qt_add_qml_module` et lancé par `QQmlApplicationEngine`.
- Pas de Qt Quick 3D, Qt Graphs ni simulation dans ce build.
- `run-local.bat` configure le chemin des DLL/plugins mais ne produit pas de package autonome.
- Toute dépendance Qt demeure hors du Core.
