# Etape 5 - premier laboratoire de bureau

Date : 2026-10-30. Application Windows native Qt Quick lancee depuis le bureau.

## Fonctionnalites

- Banc de test RL interactif : resistance, inductance, tension, courant initial, duree et pas reglables.
- Calcul hors interface dans un thread de travail ; annulation utilisateur ; interface non bloquee.
- Courbe de courant tracee a partir des valeurs calculees. Le tracage est limite a 5000 points representatifs ; les valeurs brutes sont conservees pour export.
- Affichage de la constante de temps, du courant final, du nombre exact d'echantillons et de l'etat du solveur.
- Enregistrement et ouverture de la configuration en `.olsim`; export CSV de tous les echantillons calcules.
- Menus Fichier, Simulation, Aide ; lanceur `run-local.bat` et raccourci Windows `OpenLab Simulator.lnk` sur le bureau.

## Verification executee

- Build complet Release MSVC reussi ; application demarree et fenetre native observee.
- CTest : 3/3 tests passes : persistance Core, convergence RL et controleur desktop.
- Le test du controleur calcule le cas de reference, verifie `tau=0.01 s` et `i(0.05 s)=0.4961977550 A`, enregistre/recharge un `.olsim`, puis verifie que le CSV contient l'en-tete et les 101 echantillons bruts.
- Le journal du dernier lancement ne contient pas d'erreur de chargement QML.
- L'espace libre C: depassait 5 Go au debut de cet increment ; aucun fichier personnel ni cache n'a ete supprime.

## Comment lancer

Double-cliquer sur `C:\Users\User\Desktop\OpenLab Simulator.lnk`, ou executer `run-local.bat` depuis le dossier `OpenLabSimulator`.

## Limites actuelles

1. Le logiciel est un prototype desktop utilisable, pas encore un installateur redistribuable ; Qt doit etre installe a l'emplacement du lanceur.
2. Qt 6.10.3 est un choix local provisoire, a remplacer par une version maintenue avant distribution.
3. Seul le circuit RL serie ideal est simule. Pas de schema general, solveur MNA, moteur DC, composants non lineaires, instruments dedies ou scene 3D.
4. Les parametres initiaux sont pedagogiques, pas les valeurs garanties d'un composant reel.
5. Des avertissements MSVC C4702 restent emis depuis des en-tetes Qt pendant l'application ; le build et les tests passent.

## Suite

Ajouter au bureau des courbes de tension et de puissance, puis le solveur de reseaux nodaux (MNA), une comparaison ngspice et le moteur CC. Preparer ensuite un package portable ou un installateur avec ses DLL et dependances QML licenciees.