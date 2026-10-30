# Modèle E-001 — Circuit série RL soumis à un échelon

- Version : 1.0
- Domaine : électrique, transitoire continu
- Classe : composants idéaux, intégration numérique backward Euler
- État : implémenté dans `openlab_electrical_engine`, validé par référence analytique (cas de test indiqué ci-dessous)

## Schéma et convention

Une source idéale `U(t)` alimente en série une résistance totale `R` et une inductance idéale `L`. Le courant `i` est positif de la source vers les composants. Les chutes passives sont positives dans le sens du courant. À `t = 0`, la tension passe instantanément de zéro à la valeur constante `U`; le courant initial `i(0)` est la condition fournie au modèle.

## Équations et unités SI

La loi des mailles et les lois constitutives donnent

```text
U = R i + L di/dt
di/dt = (U - R i)/L
tau = L/R
```

`U` en volts (V), `R` en ohms (Ω), `i` en ampères (A), `L` en henrys (H = V·s/A), `t` et `tau` en secondes (s). `R > 0`, `L > 0`; la dimension de `tau` est le temps.

Pour `U` constant après l'échelon, la solution analytique utilisée comme référence est

```text
i(t) = U/R + (i(0) - U/R) exp(-t/tau)
vR(t) = R i(t)
vL(t) = U - vR(t)
```

Ces équations résultent de la loi des mailles et de la loi de l'inductance linéaire idéale. Référence pédagogique indépendante pour la réponse au pas et `tau = L/R` : [Engineering LibreTexts, Transient Response of RL Circuits](https://eng.libretexts.org/Workbench/Introduction_to_Circuit_Analysis/07%3A_1st_Order_RC_RL_Circuit/7.05%3A_Transient_Response_of_RL_Circuits). Le manuel ngspice décrit l'analyse transitoire et le pas de calcul maximal (`tmax`) : [ngspice manual, section .TRAN](https://ngspice.sourceforge.io/docs/ngspice-43-manual.pdf).

## Méthode numérique

Le courant est avancé par Euler implicite (backward Euler), d'ordre un :

```text
i[n+1] = (i[n] + h U/L) / (1 + h R/L)
vR[n+1] = R i[n+1]
vL[n+1] = L (i[n+1] - i[n]) / h
```

La sortie contient l'état initial à `t=0` et chaque état calculé jusqu'à la durée demandée ; le dernier pas peut être plus court pour finir exactement à cette durée. `vL` au temps zéro est la valeur instantanée issue de `U - R i(0)`. Une limite de 1 000 000 pas empêche des allocations démesurées. Pour `R,L,h > 0`, le dénominateur est strictement positif et le schéma est stable pour ce système linéaire; la précision reste limitée et doit être étudiée en réduisant le pas.

## Hypothèses et limites

Composants concentrés, linéaires, invariants dans le temps; source et interrupteur idéaux; aucune capacité parasite, résistance série d'inductance distincte, saturation, hystérésis, couplage magnétique, rayonnement ou dépendance thermique. Ce modèle ne permet pas de conclure à la sécurité ou au comportement d'un appareil réel. La valeur `R` doit représenter la résistance série totale modélisée. Aucun paramètre de fabricant n'est intégré.

## Cas de référence et validation

`R = 10 Ω`, `L = 0,1 H`, `U = 5 V`, `i(0)=0 A`, durée `0,05 s`; donc `tau = 0,01 s` et la limite DC `0,5 A`. À `t=0,05 s`, la référence exacte est `0,5 (1 − exp(−5)) A = 0,4966310265 A`. Pour `h=1 ms`, Euler implicite donne `0,4957407244 A` (erreur `0,0008903022 A`); pour `h=0,5 ms`, il donne `0,4961977550 A` (erreur `0,0004332715 A`). Le ratio d'erreurs est environ `2,055`, conforme à la convergence d'ordre un attendue. Les tests vérifient aussi la loi des mailles à chaque échantillon avec une tolérance absolue de `10⁻¹⁰ V`, le temps caractéristique et le rejet d'unités/paramètres invalides. Cette validation analytique n'est ni une comparaison ngspice exécutée, ni une mesure expérimentale.
