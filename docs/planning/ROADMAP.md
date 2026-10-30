# Plan de développement détaillé

## Étape 0 — audit (terminée)

Windows x64, i5-6300U, 16 Gio, Intel HD 520. Prototype CircuitLab3D séparé. Outils natifs installés lors de l’étape 2.

## Étape 1 — architecture (terminée)

AGENTS, architecture, ADR frontières/moteurs/format, matrice de dépendances/licences, plan et validation scientifique.

## Étape 2 — squelette natif (terminée pour le premier incrément)

C++20/CMake, MSVC 2022, Qt Quick/QML, Core indépendant, journalisation, preset Windows, test initial, fenêtre lancée et commit Git. Le choix Qt 6.10.3 est temporaire : sélectionner une version maintenue avant distribution. Détails : [STEP-2-STATUS.md](STEP-2-STATUS.md).

## Étape 3 — Core et format de projet (premier incrément réalisé)

Ajout des dimensions/quantités SI, définitions et instances de composants, ports typés, connexions validées, horloge à pas fixe/variable et file d'événements déterministe. Le conteneur `.olsim` v1 stocke manifeste + modèle JSON dans une archive ZIP, avec limites de taille, chargement en mémoire et remplacement atomique. Le test couvre dimensions, paramètres, connexion, ordre d'événements et round-trip. Restent les cas d'archives hostiles, les migrations, les résultats de simulation et les actifs. Voir ADR 0005 ; la validation scientifique ne commence qu'avec l'étape 4.

## Étape 4 — moteur électrique et modèle CC

Écrire et réviser la fiche scientifique avant le code ; tester un circuit RL analytique puis le modèle électromécanique couplé. Étudier stabilité, pas de temps et convergence.

## Étape 5 — laboratoire moteur

Banc d’essai, paramètres, acquisition réelle des résultats, multimètre/oscilloscope/tachymètre et export documenté.

## Étape 6 — visualisation synchronisée

Choisir le backend selon licences, construire une scène adaptée au matériel modeste, lier le rendu aux états numériques et réaliser un benchmark.

## Étape 7 — plugins et bibliothèque

Composants déclaratifs validés, manifestes versionnés, provenance, permissions et isolation.

## Étape 8 — extension scientifique

Ajouter des domaines avec modèles documentés et cas validés. Étudier FMI pour l’interopérabilité.

## Étape 9 — apprentissage et cloud optionnel

Cours/TP, évaluations, rapports et collaboration facultative ; le fonctionnement hors ligne reste complet.

## Critère de sortie de chaque étape

Fonctionnalité vérifiable, build reproductible, tests réellement exécutés, documentation et licences à jour, limites connues et démonstration minimale.
