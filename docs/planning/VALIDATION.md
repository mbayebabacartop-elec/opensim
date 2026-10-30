# Stratégie de validation scientifique

Aucune simulation physique n’est implémentée dans l’étape 1. Ce document fixe les critères nécessaires avant de présenter un résultat comme scientifiquement fiable.

## Fiche modèle exigée

Chaque modèle possède : identifiant/version, auteur/source, équations, variables et unités SI, conventions de signe et d’orientation, hypothèses, conditions initiales, paramètres et leur provenance, domaine/plage de validité, limites, méthode numérique et références de validation. Qualifier le modèle idéal/réaliste/avancé.

## Pyramide de validation

1. **Contrat et dimensions** : vérifier unités d’entrée/sortie, conservation des dimensions, limites des paramètres et conventions.
2. **Cas analytiques** : comparer à des solutions fermées connues ; tolérances absolue et relative consignées.
3. **Stabilité/convergence** : pas de temps successivement réduit, solveur tolérances variées, états bornés, détection d’échec et de non-convergence.
4. **Tests d’intégration** : ports/connectivité, démarrage/arrêt, erreurs et résultat monotone/cas de référence attendu.
5. **Référence indépendante** : comparer un cas documenté à un solveur de référence (par ex. ngspice ou calcul indépendant) en précisant versions/configurations.
6. **Validation expérimentale** : lorsque banc disponible, protocole, instrument/incertitude, calibration, conditions ambiantes et données anonymisées documentées.
7. **Régression** : préserver cas/résultats, versions moteur et tolérances, détecter les changements de comportement.

## Instrumentation

Les instruments lisent les échantillons horodatés du moteur et conservent l’unité, le canal, le taux d’échantillonnage, la configuration et l’horloge. Les valeurs affichées doivent pouvoir être reliées au résultat source ; aucune valeur simulée décorative ou remplacée par zéro en cas d’erreur.

## Modèle CC, à valider ultérieurement

Équations demandées : `U = R i + L di/dt + Ke ω`, `C = Kt i`, `J dω/dt = C − Cr − f ω`.

Avant implémentation, préciser SI : U en V, i en A, R en Ω, L en H, Ke en V·s/rad, ω en rad/s, Kt en N·m/A, C et Cr en N·m, J en kg·m², f en N·m·s/rad, temps en s. Positiver les références et établir la cohérence Ke/Kt selon unités SI et hypothèses de conversion électromécanique. Expliquer si Cr est constant, dépend de ω ou défini par une charge externe. Les valeurs par défaut seront marquées « illustratives » tant qu’une fiche constructeur n’est pas fournie.

Cas à comparer : moteur à vide et régime établi (si paramètres et charge définis), circuit R-L avec force contre-électromotrice, réponse à échelon, bilan de puissance/pertes sous hypothèses compatibles. Étudier convergence du couplage électromécanique et vérifier que le pas temporel résout la constante L/R et la dynamique mécanique. Ces cas ne sont pas encore codés ni exécutés.

## Rapports et honnêteté

Un rapport distingue valeur calculée, valeur mesurée, référence, tolérance, erreur, logiciel/version et limites. Les tests sont horodatés et reproductibles. L’absence de données de fabricant ou d’expérience est annoncée, jamais comblée par des paramètres présentés comme vérifiés.
