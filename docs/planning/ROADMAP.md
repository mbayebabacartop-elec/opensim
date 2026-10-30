# Plan de développement détaillé

Les étapes sont livrées comme incréments compilables et vérifiables. L’étape 2 est en cours ; aucun moteur scientifique n’est commencé.

## Étape 0 — audit (terminée)

Windows x64, i5-6300U, 16 Gio, Intel HD 520. Initialement Git/CMake/MSVC/Qt absents. Le prototype CircuitLab3D reste séparé.

## Étape 1 — architecture (terminée, documents initiaux)

AGENTS, architecture, ADR frontières/moteurs/format, dépendances/licences, plan et validation scientifique créés.

## Étape 2 — squelette natif (premier incrément vérifié, QML restant à faire)

Réalisé : Git, CMake, MSVC/SDK ; Core C++ indépendant de Qt ; application Qt Widgets ; journalisation ; presets ; test initial du Core ; build Release et lancement Windows.

À compléter : récupérer assez d’espace disque, installer la totalité du module Qt Quick/QML dans une version prise en charge, puis décider si le shell doit intégrer QML ou conserver Widgets. Le lecteur C: n’avait qu’environ 85 Mio libres après l’installation. Éviter d’ajouter des dépendances ou de lancer de gros builds avant d’avoir résolu cette contrainte.

Aucun solveur du moteur CC n’est inclus dans cette étape.

## Étape 3 — Core et format de projet

Types de composants/ports, connexions, quantités SI, événements, résultats, JSON Schema, format `.olsim` sûr, migrations, sauvegarde/restauration et tests de round-trip.

## Étape 4 — moteur électrique et modèle CC

Écrire et réviser la fiche scientifique avant le code ; tester d’abord un cas RL analytique puis le modèle électromécanique couplé. Étudier stabilité, pas de temps et convergence.

## Étape 5 — laboratoire moteur

Banc d’essai, paramètres, acquisition des résultats réels, multimètre/oscilloscope/tachymètre et export documenté.

## Étape 6 — visualisation synchronisée

Choix de backend selon licence, scène adaptée au matériel modeste, rendu lié à l’état du moteur et benchmark.

## Étape 7 — plugins et bibliothèque

Composants déclaratifs validés, manifestes versionnés, provenance, permissions et isolation.

## Étape 8 — extension scientifique

Nouveaux domaines par modèles documentés et cas validés. FMI après étude d’interopérabilité.

## Étape 9 — apprentissage et cloud optionnel

Cours/TP, évaluations, rapports et collaboration facultative ; fonctionnement hors ligne maintenu.

## Critère de sortie de chaque étape

Fonctionnalité vérifiable, build reproductible, tests réellement exécutés, documentation à jour, licences répertoriées, limites connues et démonstration minimale.
