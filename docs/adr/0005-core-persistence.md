# ADR 0005 : modele Core et persistance `.olsim`

- Statut : adopte pour les fondations de l'etape 3
- Date : 2026-10-30

## Contexte

Le modele projet doit rester independant de Qt et des moteurs physiques. Il faut des definitions de composants types, des dimensions SI, un temps/evenements deterministes et un format versionne qui ne decompresse pas de chemins non fiables sur le disque.

## Decision

- Utiliser des types valeur C++20 et `Result<T>` dans `openlab::core`.
- Encoder les dimensions SI par sept exposants signes dans l'ordre m, kg, s, A, K, mol, cd ; stocker les grandeurs en SI.
- Separer definitions, instances, ports et connexions des moteurs.
- Utiliser un conteneur ZIP `.olsim` avec `manifest.json` et `project.json`. La version 1 refuse les versions inconnues ; aucune migration n'est encore enregistree.
- Utiliser nlohmann/json 3.12.0 et miniz 3.1.2 vendories avec notices pour permettre un build hors ligne.
- Ecrire dans un fichier temporaire voisin puis remplacer atomiquement quand la plateforme le permet. Lire l'archive en memoire, verifier noms, nombre, tailles et sommes de controle, sans extraire les chemins.

## Consequences

- Le Core pourra etre utilise par une interface, une CLI et des tests sans Qt.
- Les limites d'archive reduisent les erreurs accidentelles et certains risques de traversal, mais ne remplacent pas un audit securite complet.
- La v1 ne conserve pas encore resultats, ressources, migrations de schema ni provenance cryptographique.