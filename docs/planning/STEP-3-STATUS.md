# Etape 3 - Core et format `.olsim`

Date : 2026-10-30. Increment Core compile et verifie sur Windows x64 / MSVC Release.

## Livrables

- Type generique `Result<T>` et erreurs structurees, sans dependance a Qt.
- Dimensions SI a sept exposants et quantites finies exprimees en SI.
- Definitions de composants versionnees, parametres bornes, instances, ports de conservation/signaux et connexions compatibles.
- Horloge de simulation a pas fixe ou variable et file d'evenements deterministe.
- Persistance `.olsim` v1 : manifeste et modele JSON, limites de taille, chargement en memoire sans extraire de chemin, ecriture temporaire puis remplacement atomique.
- Dependances vendoriees hors ligne : nlohmann/json 3.12.0 et miniz 3.1.2, avec notices et ADR 0005.

## Verification

- Build Release complet MSVC reussi.
- CTest : 1/1 test de fumee passe. Il verifie les dimensions, les parametres, une connexion, l'ordre d'evenements, le round-trip `.olsim` et le rejet d'une archive malformee.

## Limites

1. Aucun moteur physique n'est encore defini dans cette etape.
2. Le format v1 conserve le graphe et ses definitions, mais pas les resultats, mesures, ressources 3D ou vues d'interface.
3. Les migrations de schema et les tests d'archives hostiles restent a faire.
4. Les dimensions SI ne constituent pas un convertisseur ou moteur complet de formatage d'unites.