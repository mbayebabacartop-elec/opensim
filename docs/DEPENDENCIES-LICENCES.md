# Dépendances et licences — matrice préliminaire

État au 2026-10-30. Cette matrice est une décision d’architecture, pas une clearance juridique. La licence exacte doit être contrôlée sur la version réellement intégrée, dans ses fichiers de licence, composants transitifs et options de compilation ; générer ensuite SBOM et THIRD_PARTY_NOTICES à chaque distribution.

| Composant | Rôle proposé | Licence / enjeu | Décision provisoire |
|---|---|---|---|
| C++20 / MSVC 2022 | langage et toolchain Windows | MSVC Build Tools gratuits dans des conditions Microsoft applicables ; vérifier les droits de redistribution du runtime MSVC et les conditions du SDK | Cible native initiale. Absent dans l’environnement audité. |
| Qt 6 / Qt Quick / QML | application UI | Qt Community sous LGPLv3/GPLv3 selon module, ou commerciale. Modules Community uniquement GPL : notamment Qt Quick 3D et Qt Graphs (au jour de la source consultée). LGPL a des obligations de redistribution/relinking et notices. | Privilégier modules LGPL et vérifier chaque module avant usage. Le choix de 3D commerciale ou backend alternatif doit être tranché avant intégration. Aucune installation. |
| CMake | build et packaging | BSD-3-Clause | Requis ; prendre version stable et minimum maintenable. Projet natif à partir de 3.25 envisagé ; Qt 6.12 source build demande CMake 3.25+ mais Qt binaire ne signifie pas que compiler Qt est nécessaire. |
| Ninja | générateur rapide | Apache-2.0 | Optionnel ; utile CI/builds, pas obligatoire si générateur VS. |
| Eigen | algèbre linéaire | MPL-2.0 ; quelques fonctionnalités optionnelles s’appuient sur licences tierces/LGPL | Optionnel, introduire uniquement là où nécessaire ; considérer `EIGEN_MPL2_ONLY` si on veut borner les fonctionnalités sous MPL2/permissives. |
| SQLite | métadonnées et index local | domaine public pour la bibliothèque principale | Candidat léger. Exclure extensions propriétaires éventuelles. |
| ngspice | solveur circuits | licence Modified BSD dans l’ensemble, exceptions/fichiers distincts : LGPL, MPL, domaine public et exception GPL pour certaines parties d’XSPICE, selon fichiers | Candidat via adaptateur isolé. Auditer version exacte et options désactivées/utilisées. Aucun linking ou bundling avant revue du manifeste de licence. |
| GoogleTest | tests C++ | BSD-3-Clause | Dépendance de développement seulement ; à intégrer en étape 2, après installation de Git/build. |
| JSON Schema validator | validation composants/projets | dépend de la bibliothèque choisie | Pas sélectionné. Comparer licence, maturité, support C++20 et validation hors ligne avant ajout. |
| ZIP | conteneur `.olsim` | dépend de l’implémentation | Pas sélectionné. Comparer sécurité des archives, zlib/minizip/libarchive et obligations de distribution ; tests contre archives hostiles requis. |
| FMI | échange de modèles | spécification et SDK à étudier | Hors dépendance initiale ; étude interopérabilité ultérieure. |

## Versions et compatibilité

- Documentation Qt en ligne consultée le 2026-10-30 : Qt 6.12 LTS est la version mise en avant (septembre 2026) ; Windows x64 pris en charge avec MSVC 2022 sur Windows 10 1809+ et Windows 11. Le build du poste expose actuellement un libellé Windows 10 Pro avec build 22000, correspondant à la génération Windows 11. Une version mineure moins récente ne doit pas être choisie sans raison de compatibilité/licence.
- CMake : 4.3.5 annoncé comme dernière stable dans l’annonce consultée de septembre 2026 ; 4.5.0 est encore en release candidate au 28 octobre. Utiliser une stable, et éviter de verrouiller le projet à la RC. Le minimum CMake du projet sera choisi après build Qt/C++ pilote et compatibilité de CI.
- ngspice : version de distribution exacte à décider au moment du prototype ; la documentation du manuel décrit capacités et exceptions de licence. Ne pas tirer HEAD automatiquement.
- Eigen, SQLite et GoogleTest : versions non encore épinglées. Aucun paquet n’a été téléchargé.

## Gouvernance des licences

1. Vérifier le `LICENSE` de chaque version épinglée, les notices des sous-composants et options compilées.
2. Conserver une copie des licences, auteurs, version/hash, URL, mode (runtime/dev), modifications et obligations dans `THIRD_PARTY_NOTICES/`.
3. Générer un SBOM au format SPDX ou CycloneDX pour chaque release.
4. Réévaluer les modules Qt lorsqu’une fonctionnalité ou le modèle commercial change ; l’EULA d’un module commercial et les règles Community ne sont pas interchangeables.
5. Cette analyse ne constitue pas un avis juridique.

## Sources primaires

- [Qt licensing](https://doc.qt.io/qt-6/licensing.html), [Qt 6.12 Windows support](https://doc.qt.io/qt-6/windows.html), [Qt release lifecycle](https://doc.qt.io/qt-6/qt-releases.html)
- [CMake release announcements](https://discourse.cmake.org/c/announcements/8)
- [Eigen licence et dépendances](https://libeigen.gitlab.io/)
- [SQLite copyright](https://www.sqlite.org/copyright.html)
- [GoogleTest LICENSE](https://github.com/google/googletest/blob/main/LICENSE)
- [ngspice developer licensing](https://ngspice.sourceforge.io/devel.html), [ngspice manual, copyright and licenses](https://ngspice.sourceforge.io/docs/ngspice-manual.pdf)


## Résultat de la mise en place — étape 2

| Composant réellement utilisé | Version | Statut et licence à surveiller |
|---|---|---|
| Git for Windows | 2.56.0.3 | installé ; GPL-2.0 pour Git et notices propres aux composants distribués |
| CMake | 4.4.4 | installé depuis le ZIP officiel ; BSD-3-Clause |
| MSVC Build Tools | 17.14.42 / compilateur 19.44.35229 | installé avec SDK 10.0.26100 ; conditions Microsoft et redistribuable à documenter pour les releases |
| Qt Widgets | 6.10.3 MSVC x64 | utilisé par le shell ; Qt open-source LGPLv3/GPLv3 selon modules et obligations de redistribution à respecter |
| Qt Quick/QML | installation incomplète | non lié au binaire ; installation interrompue faute d’espace disque |

Qt 6.10.3 est une version provisoire de développement et ne doit pas devenir une cible de distribution sans examen de son statut de maintenance. L’outil aqtinstall n’a pas pu récupérer les métadonnées 6.12 sur le dépôt officiel lors de cette opération ; l’archive Qt Quick/QML restante est aussi bloquée par l’espace disque. Revalider les versions prises en charge et choisir un installateur officiel approprié après récupération d’espace.
