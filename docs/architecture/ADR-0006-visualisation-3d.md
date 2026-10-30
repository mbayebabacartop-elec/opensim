# ADR-0006 — Visualisation 3D initiale et moteur CC

- Statut : accepte pour le prototype
- Date : 2026-10-30

## Contexte

L'application doit montrer une machine dont le mouvement suit les etats d'un modele multiphysique. Qt Quick 3D n'est pas installe sur ce poste. La documentation Qt indique le module sous GPLv3 ou licence commerciale. Le projet envisage une offre professionnelle proprietaire ; introduire le renderer GPL par defaut serait incompatible avec cette strategie sans adaptation de licence.

## Decision

1. Ajouter d'abord le moteur de moteur CC comme modele numerique C++ autonome, avec equations, unites SI et validation propres.
2. Construire la premiere visualisation par maillage triangule procedural en Qt Quick Canvas, projete en logiciel. Le modele comporte carter, ailettes, arbre, ventilateur, socle, boitier de bornes et fixations.
3. Synchroniser l'angle du rotor sur l'integrale temporelle de la vitesse angulaire calculee. Le renderer n'invente aucun resultat physique.
4. Indiquer que la vue est un visualiseur pedagogique, pas un modele constructeur, un outil CAD ou un renderer spatial general.
5. N'ajouter un backend graphique que derriere une interface qui permet de garder le simulateur sans ce module. Evaluer licences de redistribution, performances Intel HD 520, GPU/CPU et portabilite Windows/Linux.
6. Tout espace de travail incomplet indique ce qui est deja utilisable.

## Consequences

- Aucun nouveau module ou paquet Qt n'est requis pour cet increment.
- Le tri painter simplifie la projection, mais ne resout pas tous les chevauchements geometriques. Ajouter un z-buffer ou un backend GPU apres mesure et tests visuels.
- Les maillages generes n'ont aucune provenance fabricant et ne doivent pas suggerer une precision constructeur.
- Une distribution commerciale future doit refaire l'audit des modules Qt et de la licence de chaque backend.

## References

- [Documentation officielle Qt Quick 3D](https://doc.qt.io/qt-6/qtquick3d-index.html)
- [Licences officielles Qt](https://doc.qt.io/qt-6/licensing.html)
