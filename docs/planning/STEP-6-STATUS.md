# Etape 6 - premier increment multiphysique et vue 3D

Date : 2026-10-30. Ajout a l'application native existante.

## Fonctionnalites

- Navigation Atelier, Laboratoire et Cours ; sous-laboratoires RL et Moteur CC (le banc RC MNA a ete ajoute ensuite, a l'etape 8).
- Nouveau solveur moteur CC a aimants permanents : EDO couplees electrique/mecanique, Euler implicite simultane, validation des dimensions SI, controles de ressources et annulation.
- Banc moteur avec tension et couple resistant reglables ; autres constantes affichees comme valeurs illustratives documentees.
- Visualiseur de maillage procedural en projection 3D : carter, ailettes, stator, arbre, ventilateur, socle, boitier de bornes et vis ; orbite de camera a la souris.
- Lecture de la trajectoire calculee ; l'angle rotoriel est l'integrale temporelle de la vitesse echantillonnee par le solveur. Trois indicateurs affichent courant, vitesse et couple calcules ; courbe temporelle du courant et de la vitesse.
- Atelier presente le schema fonctionnel du moteur ; Cours ouvre les laboratoires et explique les objectifs pedagogiques.

## Verification executee a la fin de l'etape 6

- Build Windows MSVC Release reussi ; application demarree et fenetre native observee.
- A ce stade, CTest : 4/4 tests passes (Core/persistance, RL, moteur CC et controleur, qui exerce RL et moteur).
- Le test moteur verifie la solution nulle, l'equilibre DC analytique (4 A, 40 rad/s), l'amelioration en diminuant le pas et le rejet de dimensions incorrectes.
- UI Automation a selectionne Moteur CC et actionne Simuler ; le panneau a publie 1001 points, 4.852 A, 485.0 tr/min et 0.2426 N·m a 0.5 s pour des parametres illustratifs.
- Une erreur QML `Canvas.roundRect` a ete corrigee par des primitives compatibles. Le lancement qui a suivi ne rapporte pas d'avertissement QML.
- L'espace C: observe a 4.94 Gio libres ; aucun fichier personnel ni cache n'a ete supprime.

## Limites

1. Le visualiseur est un rendu procedural logiciel avec projection/painter, pas un renderer 3D GPU general ; les faces se chevauchant peuvent necessiter une meilleure gestion de profondeur.
2. Maillage pedagogique sans provenance constructeur, textures, coupe interne ou contraintes CAD.
3. Atelier n'a ni placement libre, ni fils interactifs, ni netlist generale ; source, moteur et charge sont un schema fonctionnel.
4. Le banc moteur ne persiste pas encore ses parametres et ses resultats dans `.olsim` ; cette persistance concerne le RL.
5. Le modele moteur omet saturation, pertes fer, frottements secs, commutation detaillee, thermique et variateur ; il n'est pas destine au dimensionnement.
6. Qt Quick 3D est exclu a cause de son regime GPLv3/licence commerciale sans decision de licence appropriee. Cette contrainte motive le renderer procedural present.
7. La fenetre peut depasser certaines hauteurs logiques d'ecrans a mise a l'echelle elevee ; ajustement responsive a poursuivre.

## Suite

Finir l'editeur de schema, brancher la netlist aux moteurs, persister les bancs moteur/MNA et enrichir les modeles avec des parametres verifies.
