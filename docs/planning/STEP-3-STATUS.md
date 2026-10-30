# Étape 3 — Core et format `.olsim`

Date : 2026-10-30. Incrément Core compilé et vérifié sur Windows x64 / MSVC Release.

## Livrables

- `Result<T>` et erreurs structurées sans dépendance Qt.
- Dimensions SI à sept exposants et quantités finies en unités SI.
- Définitions de composants versionnées, paramètres bornés, instances, ports conservatifs/signaux et connexions compatibles.
- Horloge de simulation à pas fixe borné par la fin, ou progression explicite à pas variable ; file d'événements triée par temps puis ordre d'insertion.
- API de persistance `.olsim` v1, ZIP contenant `manifest.json` et `project.json` : sérialisation JSON, validation à la lecture, limite d'archive de 32 MiB et de chaque entrée de 16 MiB, aucune extraction de chemin, écriture temporaire et remplacement atomique.
- Dépendances vendoriées hors ligne : nlohmann/json 3.12.0 et miniz 3.1.2, avec notices et ADR 0005.

## Vérifications

- Build Release complet MSVC : `openlab_core`, test Core et `openlab_app` compilés et liés.
- CTest : `openlab.core.smoke` — 1/1 réussi. Il exerce cohérence dimensionnelle V = Ω·A, validation/paramètre d'instance, connexion conservative, ordre déterministe des événements et sauvegarde/rechargement `.olsim`.
- Le premier build a détecté une erreur d'accès à l'erreur de `Result<void>` ; elle a été corrigée avant le build final.
- L'application déjà lancée a verrouillé son exécutable lors d'un relink ; fermeture normale, puis build final réussi.

## Limites connues

1. Il ne s'agit pas encore d'un solveur physique : aucun moteur électrique ni moteur DC n'existe.
2. `.olsim` v1 sauvegarde le graphe et ses définitions embarquées ; il ne conserve pas encore résultats, mesures, ressources 3D, vues d'interface ni état de simulation.
3. Version inconnue refusée ; aucune migration n'est encore implémentée.
4. Les dimensions sont exposants SI, pas un moteur complet de conversion/formatage des unités.
5. Le test passe par la sauvegarde puis le rechargement d'une archive valide ; les archives hostiles, CRC endommagés, quotas, répétitions, écriture interrompue et migrations n'ont pas encore de tests dédiés. Les limites de taille ne remplacent pas un audit de sécurité.
6. La définition des domaines ne suffit pas à imposer des lois de conservation multiphysiques ; c'est à la charge des moteurs spécialisés futurs.

## Prochaine étape

Faire relire le contrat de données et le format v1, puis étape 4 : fiche scientifique sourcée et solveur électrique simple avec cas analytique RL, avant le modèle électromécanique du moteur DC.
