# Modèle E-002 — Moteur à courant continu à aimants permanents

## Objet et niveau

Modèle concentré idéal de premier ordre électriquement et mécaniquement couplé. Il est destiné à l'apprentissage, non au dimensionnement certifié.

## États, entrées et unités

États : courant d'induit `i` en A, vitesse angulaire `ω` en rad/s. Entrées : tension `u` en V et couple résistant `C_r` en N·m (positif lorsqu'il s'oppose à la rotation positive).

- Équation électrique : `L·di/dt = u - R·i - K_e·ω`
- Couple électromagnétique : `C_e = K_t·i`
- Équation mécanique : `J·dω/dt = C_e - C_r - f·ω`

`R` en Ω, `L` en H, `K_e` en V·s/rad, `K_t` en N·m/A, `J` en kg·m², `f` en N·m·s/rad. En SI, le radian est sans dimension. Pour un modèle idéal réciproque, `K_e` et `K_t` ont même valeur numérique SI ; les paramètres sont conservés indépendamment pour représenter des jeux identifiés.

## Intégration numérique

Euler implicite, pas maximal `h` et dernier pas raccourci au temps final :

`(R + L/h)i[n] + K_e·ω[n] = u + (L/h)i[n-1]`

`-K_t·i[n] + (f + J/h)ω[n] = (J/h)ω[n-1] - C_r`

Le système 2×2 est résolu simultanément à chaque pas. Pour `R,L,J > 0`, `f >= 0`, `K_e,K_t >= 0`, le déterminant est positif en précision exacte. Le solveur rejette les valeurs non finies, les pas sans progrès et les calculs au-delà d'un million de pas, et vérifie l'annulation à chaque pas.

## Hypothèses et limites

Flux constant, collecteur idéal, paramètres constants, température fixe, absence de saturation, commutation, pertes fer, frottement sec, jeu, bruit, effets de peau et dynamique du convertisseur. Le frottement est visqueux (`f·ω`) uniquement. Les valeurs initiales de l'application sont illustratives et ne proviennent pas d'une fiche technique.

Ce modèle ne décrit pas les défauts ni la sécurité d'une machine réelle. Il ne doit pas servir à choisir des protections ou valider une installation. Une validation ultérieure comparera l'équilibre analytique, la convergence temporelle et une référence expérimentale ou un solveur indépendant identifié.

## Validation numérique prévue

1. Cas nul (`u=0`, états initiaux nuls) : solution nulle.
2. Cohérence du signe du couple résistant et des états.
3. Pas réduit : erreur décroissante sur les états comparés à une référence fine.
4. Équilibre DC : `i∞=(u-K_e·ω∞)/R`, `ω∞=(K_t·i∞-C_r)/f` si `f>0`.
5. Rejet des paramètres invalides et des limites de ressources.
