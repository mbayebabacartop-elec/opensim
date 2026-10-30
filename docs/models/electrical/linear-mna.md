# Modele E-003 — reseaux lineaires RLC par analyse nodale modifiee

## Portee

Reseaux lineaires concentres composes de resistances, condensateurs, inductances et sources ideales de tension DC independantes. Noeud 0 est la masse. Les noeuds sont numerotes de 1 a N-1. Le solveur retourne les tensions nodales apres chaque pas d'integration ; les courants de branches ne sont pas encore publies.

## Formulation

Inconnues de chaque pas : tensions nodales non referencees et courants des branches contenant une source ideale de tension ou une inductance. Cette extension de l'analyse nodale modifiee (MNA) produit le systeme lineaire stampé element par element.

- Resistance : conductance `G=1/R` entre les noeuds.
- Condensateur, Euler implicite : `i[n]=C/h·(v[n]-v[n-1])`. Norton equivalent `G=C/h` avec source historique `-G·v[n-1]`. Tension initiale fournie par l'element.
- Inductance, Euler implicite : `v[n]-(L/h)i[n]=-(L/h)i[n-1]`. Le courant de branche est une inconnue MNA ; le courant initial est fourni.
- Source ideale : contrainte de branche `v_p-v_n=U`.

Un pivot partiel de Gauss resout le systeme dense. Les topologies singulieres (par exemple ilot flottant sans reference) retournent une erreur explicite. Limites actuelles : 256 inconnues et 1 000 000 pas ; pas adaptatif, sources dependantes, diodes/transistors et courants de branches ne sont pas disponibles.

## Validation et limites

`mna_smoke` compare le RC du premier ordre avec `v_C(t)=U(1-exp(-t/RC))`, verifie la reduction d'erreur en diminuant le pas, verifie une branche inductive sous tension et rejette les reseaux singuliers et les dimensions invalides. Euler implicite est dissipatif et d'ordre un ; les fortes disparites d'echelle et matrices mal conditionnees ne sont pas encore diagnostiquees finement. Le solveur ne se presente pas comme SPICE.
