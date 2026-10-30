# Modele E-001 - circuit serie RL soumis a un echelon

- Version : 1.0
- Domaine : electrique, transitoire continu
- Classe : composants ideaux, integration Euler implicite
- Etat : implemente dans `openlab_electrical_engine`; valide par reference analytique.

## Schema et convention

Une source ideale `U(t)` alimente en serie une resistance totale `R` et une inductance ideale `L`. Le courant `i` est positif de la source vers les composants. Les tensions passives sont positives dans le sens du courant. A `t=0`, la tension passe instantanement de zero a `U`; le courant initial `i(0)` est fourni.

## Equations et unites SI

```text
U = R i + L di/dt
di/dt = (U - R i)/L
tau = L/R
```

`U` en volts, `R` en ohms, `i` en amperes, `L` en henrys, `t` et `tau` en secondes. `R>0` et `L>0`.

Pour une tension constante apres l'echelon :

```text
i(t) = U/R + (i(0) - U/R) exp(-t/tau)
vR(t) = R i(t)
vL(t) = U - vR(t)
```

Reference pedagogique : [Engineering LibreTexts, reponse transitoire des circuits RL](https://eng.libretexts.org/Workbench/Introduction_to_Circuit_Analysis/07%3A_1st_Order_RC_RL_Circuit/7.05%3A_Transient_Response_of_RL_Circuits). Le manuel ngspice decrit l'analyse transitoire et son pas maximal : [manuel ngspice, section .TRAN](https://ngspice.sourceforge.io/docs/ngspice-43-manual.pdf).

## Methode numerique

Euler implicite d'ordre un, avec pas `h` :

```text
i[n+1] = (i[n] + h U/L) / (1 + h R/L)
vR[n+1] = R i[n+1]
vL[n+1] = L (i[n+1] - i[n]) / h
```

Le resultat contient l'etat initial et les etats calcules jusqu'a la duree exacte demandee ; le dernier pas peut etre plus court. A `t=0`, `vL=U-R i(0)`. Le solveur limite la sortie a un million de pas. Pour `R,L,h>0`, le schema est stable sur ce systeme lineaire ; la precision reste a verifier en reduisant le pas.

## Hypotheses et limites

Composants concentres, lineaires et invariants dans le temps ; source et commutateur ideaux ; pas de capacites parasites, saturation, hysteresis, couplage magnetique, rayonnement ou dependance thermique. Ce modele ne permet aucune conclusion de securite sur un appareil reel. Aucun parametre constructeur n'est integre.

## Cas de reference

`R=10 ohm`, `L=0.1 H`, `U=5 V`, `i(0)=0 A`, duree `0.05 s`. Donc `tau=0.01 s`, limite DC `0.5 A`, et la solution exacte finale vaut `0.5(1-exp(-5)) = 0.4966310265 A`. A `h=1 ms`, erreur mesuree `0.0008903022 A`; a `h=0.5 ms`, erreur `0.0004332715 A`; ratio `2.05484`, compatible avec une convergence d'ordre un. Les tests verifient aussi la loi des mailles a `1e-10 V` et le rejet des parametres invalides. Ce n'est ni une comparaison ngspice executee ni une mesure experimentale.