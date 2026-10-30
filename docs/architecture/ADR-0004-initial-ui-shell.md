# ADR-0004 — shell natif Qt Widgets et interface QML différée

- **Statut :** accepté pour le premier incrément, révision requise avant UI avancée
- **Date :** 2026-10-30

## Contexte

L’architecture vise Qt 6/QML. Le poste dispose de moins de 100 Mio libres après l’installation MSVC, SDK et QtBase. Le paquet Qt Quick/QML n’a pas pu terminer son installation. La compilation de Qt Widgets fonctionne.

## Décision

Le shell de démarrage utilise Qt Widgets pour prouver le cycle application, fenêtre, journalisation et build natif. Il ne simule rien et ne présente aucun composant comme disponible. L’architecture Core/services reste indépendante de la UI. Le choix entre Widgets, QML ou une combinaison sera revu après récupération d’espace et installation complète/licence validée de Qt.

## Conséquences

- Le shell actuel ne valide pas l’architecture QML visée pour les panneaux et visualisations avancées.
- Pas d’API QML, Qt Quick, Qt Graphs ou Qt Quick 3D dans le build actuel.
- Une migration UI sera nécessaire si le choix final reste QML ; elle ne doit pas entraîner de dépendance Qt dans le Core.
- La cible Qt 6.10.3 reste temporaire et doit être mise à jour vers une version maintenue avant distribution.
