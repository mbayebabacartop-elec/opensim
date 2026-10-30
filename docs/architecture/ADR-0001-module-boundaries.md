# ADR-0001 — frontières Core, application et interface

- **Statut :** accepté comme architecture proposée, à confirmer avant l’étape 2
- **Date :** 2026-10-30

## Contexte

Le logiciel doit accueillir plusieurs domaines physiques, des UI natives et des moteurs remplaçables sans lier les données scientifiques à Qt ou à un fournisseur de solveur. L’Open Core demande aussi une sélection prudente des modules tiers.

## Décision

1. Langage du domaine/moteurs : C++20.
2. Interface : Qt 6/QML, isolée derrière services applicatifs et adaptateurs.
3. Core en bibliothèque C++ sans dépendance Qt et sans dépendance directe de moteur.
4. Moteurs spécialisés implémentent des contrats communs versionnés. Pas de « solveur universel » artificiel.
5. Rendu derrière des interfaces ; les vues et animations consomment les résultats calculés.
6. CMake organise les targets et dépendances ; le modèle scientifique reste utilisable dans tests sans lancer l’UI.

## Conséquences

- Les objets Qt ne traversent pas les frontières du Core ; échanges de valeurs et messages sérialisables.
- Le démarrage initial ajoute un coût d’adaptateurs mais rend les moteurs testables/remplaçables.
- La sélection exacte de Qt 6 et modules n’est pas verrouillée avant audit de licence et build pilote.
- Open Core peut utiliser des modules LGPL si les obligations applicables sont respectées ; exclure les modules Community Edition GPL de l’application non-GPL sauf révision commerciale/licence.

## Options rejetées

- Tout écrire dans QML/JavaScript : peu adapté au noyau numérique typé et au long terme C++.
- Lier directement le Core à ngspice/Qt : remplacements et tests plus coûteux.
- Dépendre d’OpenGL brut partout : limite la portabilité et mélange rendu avec métier.
