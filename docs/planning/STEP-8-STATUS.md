# Etape 8 - MNA lineaire et banc RC integre

- Solveur C++ MNA transitoire pour R/C/L lineaires et sources DC, sous Euler implicite.
- Banc RC accessible depuis Laboratoire > Reseau RC, avec U, R et C reglables, courbe de tension du condensateur, constante RC et echantillons.
- Le solveur et le controleur tournent hors du thread UI ; annulation partagee avec les autres bancs.
- Les tests comparent RC a la solution analytique, verifient la convergence au pas affine, la branche inductive, la singularite de topologie et l'incompatibilite dimensionnelle. Le test du controleur verifie 101 points jusqu'a 10 tau et sauvegarde/restauration RL existante.
- Limites : 256 inconnues denses, 1 million de pas, uniquement R/C/L lineaires, sources idealles DC independantes, pas de courants de branche publishes, pas de persistance MNA dans `.olsim`, editeur general encore a faire.
- CTest total a ce jour : 5 tests. Build Windows Release MSVC reussi. Les avertissements C4702 du code Qt fourni restent visibles.
- Suite : exposer le reste des resultats MNA (courants et tensions de composants), ajouter enregistrement du banc RC, puis relier un schema libre a une netlist validee.
