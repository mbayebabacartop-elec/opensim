# Etape 4 - moteur electrique, increment RL

Date : 2026-10-30. Premier modele electrique compile et teste dans une bibliotheque independante du Core.

## Livrables

- Bibliotheque C++ `openlab_electrical_engine`, sans dependance Qt.
- Modele RL serie ideal soumis a un echelon de tension continue, avec courant initial specifie.
- Integration Euler implicite, dernier pas ajuste pour terminer exactement a la duree demandee. Sorties horodatees du courant, de la tension resistive et de la tension inductive.
- Controle des dimensions SI, positivite de R/L, finitude, progression temporelle et quota d'un million de pas.
- Fiche E-001 avec equations, conventions, hypotheses, limites, methode et reference analytique.

## Verification executee

- Build Release complet MSVC : Core, moteur, application et tests compiles et lies.
- CTest : 2/2 tests passes a la fin de l'increment RL (`openlab.core.smoke`, `openlab.electrical.rl`).
- Cas `R=10 ohm`, `L=0.1 H`, `U=5 V`, `i(0)=0 A`, duree `0.05 s`; constante de temps `0.01 s`.
- Reference analytique finale : `0.4966310265 A`.
- Pas de 1 ms : `0.4957407244 A`, erreur `0.0008903022 A`.
- Pas de 0.5 ms : `0.4961977550 A`, erreur `0.0004332715 A`; ratio des erreurs `2.05484`, conforme a la convergence d'ordre un.
- Residus de la loi des mailles sous `1e-10 V`. Dimensions invalides, resistance nulle et annulation sont testees.

## Limites

1. Ce n'est pas encore un solveur topologique SPICE : il calcule uniquement un RL serie avec une source constante apres l'echelon.
2. Pas d'integration ngspice ni de comparaison experimentale executee ; la reference est analytique.
3. Composants ideaux et lineaires : pas de saturation, pertes, parasites, thermique ou commutation ulterieure.
4. Aucun moteur reel ni parametre fabricant n'est integre.

## Suite

Etendre l'API a des reseaux RC/RL/RLC via MNA, comparer a ngspice, puis implementer le moteur CC electromecanique avec des tests de bilan et convergence.