# ADR-0002 — moteurs spécialisés et co-simulation

- **Statut :** accepté comme architecture proposée, à confirmer avant l’étape 2
- **Date :** 2026-10-30

## Décision

Chaque domaine garde son solveur approprié et publie les variables d’échange avec unités/conventions explicites. Un ordonnanceur applicatif coordonne les solveurs et gère l’horloge de simulation, les événements, la pause, l’arrêt et les diagnostics. Les capacités de pas fixe/variable et d’événements sont déclarées, pas supposées.

Pour le premier circuit électrique, évaluer ngspice comme backend spécialisé via un adaptateur. Garder le Core indépendant et isoler le backend derrière un protocole/adaptateur. Au premier prototype, l’exécution séparée par processus est la solution privilégiée pour maîtriser stabilité, versions et surface d’intégration ; mesurer ensuite le coût par rapport à l’API partagée.

Le couplage du moteur CC doit échanger le courant/tension électrique et la vitesse/couple mécaniques de manière définie. Choix d’algorithme (co-simulation explicite/itérative, pas commun ou adaptatif) dépendra de l’analyse de stabilité/convergence documentée dans la fiche du modèle. FMI est une piste d’interopérabilité à évaluer plus tard, pas une dépendance initiale.

## Contrat proposé

`EngineDescriptor` décrit version, domaines, variables, capacité temporelle et limites. `SimulationEngine` accepte un `SimulationConfig`, initialise un état, avance jusqu’à un temps/événement demandé, retourne résultats horodatés et diagnostics, puis se réinitialise. L’annulation et toute erreur numérique sont remontées explicitement. Pas d’écriture directe dans la UI.

## Pourquoi pas un moteur universel

Un solveur nodal SPICE, une intégration mécanique, un réseau thermique et un automate ont des variables, événements et stratégies numériques différents. Les uniformiser dans une seule abstraction pourrait masquer stabilité, précision et unités. Le contrat commun concerne orchestration/résultats, pas l’algorithme interne.

## Validation ultérieure

Chaque modèle doit avoir des références, tests dimensionnels, stabilité et convergence. Le premier banc moteur ne sera implémenté qu’après sa fiche d’équations, de conventions et son protocole de validation.
