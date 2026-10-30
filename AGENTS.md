# AGENTS.md — règles de travail OpenLab Simulator

## Périmètre
- OpenLab Simulator est un nouveau projet natif C++20 multiplateforme. Le prototype `CircuitLab3D` est indépendant : ne pas le modifier ni le déplacer sans demande explicite.
- Ce dépôt vise d’abord Windows x64, puis Linux. Toute décision doit préserver une séparation nette entre interface, noyau, moteurs et ressources pédagogiques.
- Respecter l’étape en cours du plan `docs/planning/ROADMAP.md`. Ne pas développer un moteur scientifique avant son ADR, sa fiche de modèle et son plan de validation.

## Règles scientifiques
- SI partout dans les données internes ; conversions explicites à la frontière UI/import/export.
- Chaque modèle documente équations, unités, conventions de signe, hypothèses, plage de validité, provenance des paramètres et limites.
- Distinguer idéal, réaliste et avancé. Une animation doit être pilotée par des résultats calculés, jamais par une séquence décorative présentée comme une simulation.
- Toute valeur illustrative est marquée comme telle. Ne pas annoncer une validation, une mesure ou un test sans résultat reproductible consigné.
- Prévoir tests analytiques, dimensionnels, de stabilité, de convergence et d’intégration adaptés au moteur ; comparer à des références identifiées.

## Architecture et code
- Le Core ne dépend d’aucun moteur, Qt, format natif de moteur ou rendu.
- Pas de dépendance à des classes Qt dans les modèles métier/numériques centraux. Les adaptateurs font la traduction.
- Éviter une ABI C++ de plugin non contrôlée. Aucun code téléchargé ne s’exécute automatiquement ; versions, origine, permissions et isolation sont obligatoires.
- Conserver les frontières et formats documentés. Toute évolution incompatible nécessite une migration explicite et une ADR.
- C++20, CMake, avertissements élevés, builds reproductibles. Aucune dépendance ajoutée sans justification et entrée dans `DEPENDENCIES-LICENCES.md`.
- Ne pas committer les fichiers de build, données privées, clés, binaires ou ressources sans provenance/licence.

## Données et sécurité
- Traiter les projets `.olsim`, modèles importés et archives comme non fiables : limites de taille, chemins confinés, validation stricte des schémas, rejet des chemins traversants et aucune exécution de script implicite.
- Écritures de projets atomiques ; sauvegarde avant migration ; erreurs récupérables et journalisées.
- Les ressources embarquées doivent avoir une provenance et une licence traçables.

## Documentation et livraison
- Les documents de conception et commentaires destinés aux contributeurs sont en français ; identifiants de code en anglais cohérent.
- Mettre à jour README, ADR, matrice de dépendances/licences et notices avec les changements correspondants.
- Avant de conclure une étape : indiquer fichiers changés, commandes réellement exécutées, résultats, limites et travaux restants.
- Ne jamais dire « compilé », « testé » ou « validé » si la commande correspondante n’a pas été exécutée avec succès. Ne pas masquer les avertissements.

## Environnement Windows du premier incrément

- Le build vérifié utilise CMake 4.4.4, MSVC 19.44, SDK 10.0.26100 et Qt Widgets 6.10.3.
- Le build est configuré en Release ; les bibliothèques Qt de debug ne sont pas nécessaires à ce preset.
- L’installation QML de Qt est incomplète et le volume C: a moins de 100 Mio libres. Ne pas lancer d’installation ou build lourd avant d’avoir résolu la capacité disponible.
