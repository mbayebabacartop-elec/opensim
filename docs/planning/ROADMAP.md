# Feuille de route de developpement

## Etape 0 - audit environnement (terminee)

Windows x64, Intel i5-6300U, 16 Gio de RAM, Intel HD 520. Le prototype distinct CircuitLab3D n'a pas ete modifie.

## Etape 1 - architecture (terminee)

Audit, architecture modulaire, ADR, licences, plan de validation scientifique et developpement par etapes.

## Etape 2 - coquille native (terminee)

C++20/CMake, MSVC 2022, Qt Quick/QML, journalisation et lancement Windows natif.

## Etape 3 - Core et format projet (premier increment realise)

Dimensions/quantites SI, composants, ports et connexions types, horloge, evenements et format `.olsim` v1. Restent les migrations, resultats et protections avancees d'archives.

## Etape 4 - moteur electrique (deux increments)

Circuit RL serie ideal sous echelon DC, integration backward Euler, reference analytique et convergence. Moteur CC a aimants permanents, couplage implicite electrique/mecanique et tests d'equilibre, de convergence et d'unites. Restent le solveur MNA general, composants non lineaires, AC et comparaison ngspice.

## Etape 5 - laboratoire desktop (premier banc RL realise)

Banc RL, parametres, calcul annulable, graphe, sauvegarde `.olsim` et export CSV complet.

## Etape 6 - application de laboratoire multiphysique (terminee)

Espaces Atelier/Laboratoire/Cours, banc moteur CC et visualiseur maillage 3D procedural anime par la vitesse calculee. Voir [ADR-0006](../architecture/ADR-0006-visualisation-3d.md) et [rapport Etape 6](STEP-6-STATUS.md).

## Etape 7 - editeur de schema et composants

Placement libre, ports graphiques et routage de fils ; modele de netlist commun, composants electriques reutilisables, validation des connexions et projets RL/moteur persistants.

## Etape 8 - MNA lineaire (terminee)

R/C/L lineaires et sources de tension DC sous Euler implicite ; couverture analytique RC, RL et singularite, banc RC integre au laboratoire. Voir [rapport Etape 8](STEP-8-STATUS.md).

## Etape 9 - Prototype 3D immersif banc moteur CC (terminee)

Banc d'essai multiphysique complet et interactif pour moteur CC conformement a l'exigence 3D majeure :
- Moteur CC 3D avec carcasse, ailettes, flasques et boite a bornes cablee.
- Vues 3D (Exterieure, Coupe Cutaway montrant le rotor bobine et le stator, Eclatee), orbite et zoom.
- Organes internes visibles en mouvement : rotor avec spires cuivre, collecteur a lames, balais graphite et poles inducteurs.
- Arbre de transmission acier et manchon d'accouplement elastique.
- Charge mecanique : banc de frein dynamometrique avec disque ventile/volant d'inertie et reglage de couple resistant Cr.
- Alimentation DC reglable avec afficheurs numeriques et interrupteur marche/arret physique interactif.
- Multimetre numerique de banc (DMM) a ecran LCD (U, I, P, rendement).
- Tachymetre optique numerique (RPM, rad/s, bargraphe dynamique et sens de rotation).
- Oscilloscope numerique DSO 2 voies (CH1 courant i(t), CH2 vitesse w(t)) avec curseur de balayage temporel temps reel.
- Inspecteur de pieces techniques avec fiches contextuelles et mise en surbrillance.
Voir [rapport Etape 9](STEP-9-STATUS.md).

## Etape 10 - editeur interactif et circuit general

Placement libre, ports, netlist editable, routage de fils et persistance de configuration ; puis analyse DC/AC, sources pilotees, composants non lineaires documentes, comparaison ngspice.

## Etape 10 - machines et instruments

Moteurs, transformateurs, convertisseurs, mesures synchronisees, oscilloscope, alimentation et analyse puissance avec fiches de parametres et validations identifiees.

## Etape 11 - renderer 3D et extensions

Evaluer backend maillage GPU interchangeable, performances et licences de distribution ; bibliotheques de composants avec provenance, versionnage et isolation des extensions.

## Etape 12 - mecanique, automatisme et apprentissage

Couplage moteur-charge, mecanismes, asservissement, cours, TP, evaluations et rapports.

## Etape 13 - distribution

Migration vers version Qt maintenue, audit licences, SBOM, installateur Windows, paquet Linux, validation multi-materiel et documentation de release.

## Criteres de sortie

Chaque etape exige un build reproductible, des tests reellement executes, une documentation, des licences auditees, les limites connues et une demonstration observable.
