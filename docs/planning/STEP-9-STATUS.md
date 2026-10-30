# Étape 9 — Prototype banc d'essai Moteur CC 3D réaliste et interactif

## 1. Objectif réalisé

Conformément à l'exigence majeure (Section 10 — Premier prototype obligatoire), un véritable laboratoire virtuel interactif pour le moteur CC a été développé et intégré dans l'espace Laboratoire :

- **Moteur CC modélisé en 3D** : carcasse cylindrique rainurée en fonte d'aluminium, ailettes de refroidissement radiales, flasques d'extrémités, boîte à bornes supérieure avec presse-étoupes et câblage bicolore (rouge $U+$, noir $GND$), pieds de montage boulonnés sur profilé.
- **Rotor et Stator visibles** : mode *Vue en coupe* (Cutaway) découvrant l'intérieur de la machine pendant la rotation :
  - **Stator (inducteur)** : carcasse coupée révélant les pièces polaires Nord et Sud et leurs bobinages d'excitation en cuivre.
  - **Rotor (induit multipolaire)** : paquet de tôles magnétiques au silicium avec 8 encoches longitudinales garnies de spires de cuivre émaillé en rotation continue synchrone avec $\omega(t)$.
  - **Collecteur à lames & balais** : bague segmentée en cuivre tournant sur l'arbre en contact direct avec deux balais électrographite (charbons) et leurs porte-balais en laiton avec ressorts de pression.
  - **Flasques & roulements à billes**.
- **Arbre de sortie et accouplement mécanique** : arbre en acier rectifié C45E entraînant la charge via un manchon d'accouplement élastique anodisé rouge avec vis de bridage.
- **Charge mécanique (Frein dynamométrique)** : banc de freinage monté sur le rail commun à droite, avec disque ventilé / volant d'inertie en rotation synchrone, mâchoires d'étriers et capteur d'effort dynamométrique réglable de 0 à 120 mN·m.
- **Alimentation réglable DC** : pupitre PSU avec afficheurs LED de tension et de courant débité, consigne de tension réglable (0 à 48 V) et boutons de présélection rapide (5 V, 12 V, 24 V, 48 V).
- **Interrupteur physique On/Off** : commutateur marche/arrêt avec témoin LED vert/rouge, permettant de démarrer ou couper l'alimentation en direct (avec arrêt physique progressif selon inertie et frottements).
- **Multimètre numérique de banc (DMM)** : grand afficheur LCD rétroéclairé avec sélecteur de mode rotatif mesurant Tension $U_a$ (V), Courant $I_a$ (A), Puissance électrique $P_e = U \cdot I$ (W) et Rendement électromécanique $\eta = P_{méc} / P_e$ (%).
- **Tachymètre optique numérique** : vitesse instantanée en tr/min (RPM) et en rad/s ($\omega$), bargraphe analogique tricolore et indicateur de sens de rotation (CW / CCW).
- **Oscilloscope numérique de banc (DSO)** : écran CRT à grille 8x10 divisions avec balayage temporel temps réel, tracé simultané des voies CH1 (Courant d'induit $i(t)$, cyan) et CH2 (Vitesse angulaire $\omega(t)$, ambre), et curseur temporel synchronisé avec les points lumineux instantanés.
- **Navigation 3D & Inspection** : orbite 3D fluide à la souris (azimut / élévation), zoom interactif (+ / -), et sélecteur d'inspection technique mettant en surbrillance chaque organe (Rotor, Stator, Collecteur, Arbre, Frein) avec fiche technique contextuelle.

## 2. Validation et tests

- Compilation native réussie sous MSVC 2022 (x64 Release).
- 5 tests unitaires et d'intégration validés à 100% via CTest :
  - `openlab.core.smoke`
  - `openlab.electrical.rl`
  - `openlab.electrical.dc_motor`
  - `openlab.electrical.mna`
  - `openlab.app.controller` (incluant la vérification des nouvelles propriétés physiques : puissance électrique, puissance mécanique, rendement, f.é.m. et comportement hors tension).
