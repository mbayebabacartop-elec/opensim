# Feuille de route de developpement

## Etape 0 - audit environnement (terminee)

Windows x64, Intel i5-6300U, 16 Gio de RAM, Intel HD 520. Le prototype distinct CircuitLab3D n'a pas ete modifie.

## Etape 1 - architecture (terminee)

Audit, architecture modulaire, ADR, licences, plan de validation scientifique et developpement par etapes.

## Etape 2 - coquille native (terminee)

C++20/CMake, MSVC 2022, Qt Quick/QML, journalisation, preset Windows, test initial et fenetre native lancee. Qt 6.10.3 reste provisoire ; choisir une version maintenue et conforme aux licences avant distribution. Voir [STEP-2-STATUS.md](STEP-2-STATUS.md).

## Etape 3 - Core et format projet (premier increment realise)

Dimensions/quantites SI, definitions et instances versionnees, ports et connexions types, horloge et file d'evenements deterministes. `.olsim` v1 contient un manifeste et le modele JSON dans un conteneur ZIP. Restent les tests d'archives hostiles, migrations, resultats et ressources. Voir ADR 0005 et le rapport d'etape 3.

## Etape 4 - moteur electrique (increment RL realise)

Circuit RL serie ideal sous echelon DC, integration backward Euler, reference analytique et convergence lors de la reduction du pas. Suite : reseaux RC/RL/RLC par analyse nodale modifiee (MNA), comparaison ngspice, puis moteur DC couple a la mecanique. Voir la [fiche E-001](../models/electrical/series-rl-step.md) et le [rapport etape 4](STEP-4-STATUS.md).

## Etape 5 - laboratoire desktop (premier banc RL realise)

Banc natif RL, parametres, calcul annulable en arriere-plan, courbe du courant, sauvegarde `.olsim` et export CSV complet. Les instruments et le moteur DC restent a ajouter. Voir [STEP-5-STATUS.md](STEP-5-STATUS.md).

## Etape 6 - visualisation synchronisee

Choisir un moteur de rendu sous licence compatible, construire une scene legere, la piloter par les etats calcules et mesurer les performances.

## Etape 7 - plugins et bibliotheque

Composants declaratifs valides, manifestes versionnes, provenance, permissions et isolation.

## Etape 8 - extension scientifique

Ajouter des domaines avec modeles documentes et cas valides. Etudier l'interoperabilite FMI.

## Etape 9 - apprentissage et cloud facultatif

Cours, travaux pratiques, evaluations, rapports et collaboration optionnelle. L'utilisation hors ligne reste disponible.

## Criteres de sortie

Chaque etape doit avoir un build reproductible, des tests reellement executes, une documentation et des licences a jour, les limites connues et une demonstration minimale.