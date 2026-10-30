# ADR-0003 — format projet `.olsim`

- **Statut :** accepté comme proposition initiale
- **Date :** 2026-10-30

## Décision

`.olsim` sera un conteneur ZIP ouvert, versionné et documenté, comprenant au minimum un `manifest.json` versionné et des documents JSON de projet. Le manifest indique versions de format/application, ressources, dépendances de composants, empreintes et entrées. Les grands résultats de simulation pourront être absents, inclus ou référencés selon une politique explicite.

Schéma initial proposé :

```text
project.olsim (ZIP)
├── manifest.json
├── project.json          # composants, connexions, paramètres
├── simulation.json       # configuration, pas de temps, backend/version
├── learning/             # TP et annotations si présents
├── results/              # export/résultats optionnels et identifiés
└── assets/               # ressources référencées, licence/provenance
```

Le Core ne s’appuie pas sur une base SQLite embarquée pour les données de circuit. Les documents utilisent JSON Schema et migrations pures et séquencées. L’enregistrement se fait dans un fichier temporaire suivi d’un remplacement atomique ; sauvegardes avant migration.

## Sécurité et compatibilité

Les archives peuvent être malveillantes : contrôler taille compressée/décompressée, nombre de fichiers, chemins, collisions, encodage, schémas et limites numériques. Refuser traversal de chemin et archives ambiguës. Les ressources ne contiennent jamais du code exécutable auto-lancé. En cas de dépendance manquante, préserver les données et afficher une erreur/placeholder explicite.

Toute version de format annonce sa politique : versions majeures incompatibles avec migration explicite ; nouvelles propriétés optionnelles doivent avoir des défauts documentés. Tests de sauvegarde/rechargement et fixtures d’anciennes versions requis dès l’étape Core.

## Conséquences

L’utilisateur peut partager un seul fichier hors ligne et les outils tiers peuvent documenter le format. La stabilité du conteneur dépendra des tests de migration et de l’absence de sérialisation de classes C++ natives.
