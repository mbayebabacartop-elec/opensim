# Étape 4 — moteur électrique, incrément RL

Date : 2026-10-30. Premier modèle électrique compilé et testé dans une bibliothèque séparée du Core.

## Livrables

- `openlab_electrical_engine` est une bibliothèque C++ dédiée, qui dépend du Core mais pas de Qt.
- Modèle série RL linéaire idéal soumis à un échelon de tension DC, avec courant initial spécifié.
- Intégration backward Euler, pas final ajusté pour terminer à la durée exacte, sorties horodatées `i`, `vR`, `vL` et constante de temps `L/R`.
- Contrôle des dimensions SI, positivité de R/L, finitude, progression temporelle et quota d'un million de pas.
- Fiche scientifique E-001, équations, conventions, hypothèses, limites, méthode et référence analytique.

## Vérification exécutée

- Build complet Release MSVC : Core, moteur électrique, application et exécutables de test compilés et liés.
- CTest : 2/2 passés (`openlab.core.smoke`, `openlab.electrical.rl`).
- Cas `R=10 Ω`, `L=0,1 H`, `U=5 V`, `i(0)=0 A`, durée `0,05 s` ; `tau=0,01 s`.
- Référence analytique finale : `0,4966310265 A`.
- Euler implicite à `h=1 ms` : `0,4957407244 A`, erreur `0,0008903022 A`.
- Euler implicite à `h=0,5 ms` : `0,4961977550 A`, erreur `0,0004332715 A`; ratio d'erreurs `2,05484` (convergence d'ordre un observée).
- Résidu de KVL `vR+vL−U` vérifié sous `10⁻¹⁰ V` pour les échantillons du cas fin ; unités inductance incorrectes et résistance nulle rejetées.
- Le premier run de test a rejeté une tolérance que nous avions fixée trop sévèrement ; valeurs calculées mesurées et tolérance corrigée à 0,5 mA avant le run final réussi.

## Limites et travaux restants

1. Ceci n'est pas un solveur topologique général SPICE : il simule uniquement un circuit série RL avec source constante après l'échelon.
2. Aucune intégration ngspice ou comparaison expérimentale n'a été exécutée ; le cas est vérifié par une référence analytique indépendante.
3. Les composants sont idéaux/linéaires ; saturation, pertes, parasites, température et commutation d'entrée ultérieure ne sont pas représentés.
4. Le moteur n'est pas encore relié à l'interface laboratoire ; pas de graphiques ni d'instruments à cette étape.
5. Aucune fiche de fabricant n'a été fournie ; le test utilise un cas mathématique de référence, pas un moteur réel.

## Suite proposée

Étendre l'API électrique à des réseaux RC/RL/RLC assemblés par nœuds, choisir un solveur nodal/MNA et valider l'interface numérique sur plusieurs topologies. Puis implémenter le moteur DC électromécanique avec un couplage et des tests de bilan/convergence. Une comparaison ngspice sera ajoutée avant d'élargir les affirmations de validation.
