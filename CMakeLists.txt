projet (e2sim)
cmake_minimum_required ( VERSION 3.5 )

ensemble (version_majeure "1" )
ensemble (version_mineur "0" )
définir ( patch_level "0" )

définir( racine_d'installation "${CMAKE_INSTALL_PREFIX}" )
définir( install_inc "include/e2sim" )

si( MAN_PREFIX )
        set( install_man ${MAN_PREFIX} ) # existe-t-il une variable cmake pour cela -- je n'en trouve pas
autre()
        set( install_man "/usr/share/man" ) # cela doit être corrigé pour que ce ne soit pas codé en dur
fin si()

inclure ( GNUInstallDirs )

add_definitions("-D ASN_DISABLE_OER_SUPPORT")

si( PAS CMAKE_INSTALL_LIBDIR )
	définir( CMAKE_INSTALL_LIBDIR "lib" )
fin si()

définir( install_lib "${CMAKE_INSTALL_LIBDIR}" )

si( ${CMAKE_SYSTEM_PROCESSOR} CORRESPOND À "x86_64" )
	si( PAS PRESERVE_PTYPE )
		définir( deb_sys_name "amd64" )
	autre()
		définir( nom_système_deb ${CMAKE_SYSTEM_PROCESSOR} )
	fin si()
autre()
	définir( nom_système_deb ${CMAKE_SYSTEM_PROCESSOR} )
fin si()
unset( PRESERVE_PTYPE CACHE ) # nous ne voulons pas que cela persiste

définir( rpm_sys_name ${CMAKE_SYSTEM_PROCESSOR} )

si( DEV_PKG )
    définir( deb_pkg_name "e2sim-dev" )
    définir( rpm_pkg_name "e2sim-devel" )
autre()
    définir( deb_pkg_name "e2sim" )
    définir( rpm_pkg_name "e2sim" )
fin si()

set( pkg_label "ricxfcpp${spoiled_str}-${major_version}.${minor_version}.${patch_level}-${sys_name}" )
set( rpm_pkg_label "${rpm_pkg_name}${spoiled_str}-${major_version}.${minor_version}.${patch_level}-${rpm_sys_name}" )
set( deb_pkg_label "${deb_pkg_name}${spoiled_str}_${major_version}.${minor_version}.${patch_level}_${deb_sys_name}" )
message( "+++ nom du paquet : ${deb_pkg_label}.deb" )

définir( gen_rpm 0 )
find_program( rpm NAMES rpmbuild ) # le package rpm gen nécessite que ceci soit installé
if( "${rpm}" CORRESPOND À "rpm-NOTFOUND" ) # impossible de créer rpm
	définir( pkg_list "DEB" )
	message( "### make package générera uniquement le package deb ; impossible de trouver le support pour générer les packages rpm" )
autre()
	message( "+++ nom du package : ${rpm_pkg_label}.rpm" ) # débogage si nous pensons que nous pouvons également générer des rpm
	définir( pkg_list "DEB;RPM" )
	définir( gen_rpm 1 )
	message( "+++ make package générera à la fois les packages deb et rpm" )
fin si()

# les noms de cmake qui saignent sont des romans courts ; et quand les lignes ne peuvent pas être séparées, elles sont pénibles
définir ( srcd "${CMAKE_CURRENT_SOURCE_DIR}" )

# cela nous permet de contourner le problème de la poule et de l'œuf. Les fichiers d'inclusion n'existent pas tant que make n'est pas exécuté
# mais Cmake insiste pour que ceux-ci existent lorsque nous les ajoutons pour inclure des répertoires à
# activer le code pour les trouver après les avoir construits.
#
include_directories( "asn1c;${srcd}/src/DEF;${srcd}/src/SCTP;${srcd}/src/messagerouting;${srcd}/src/base;${srcd}/src/encoding" )


# Drapeaux du compilateur
#
définir(CMAKE_POSITION_INDEPENDENT_CODE ON)
définir( CMAKE_C_FLAGS "-g " )
définir( CMAKE_CPP_FLAGS "-g " )
définir( CMAKE_CXX_FLAGS "-g " )
if( GPROF ) # si défini, nous définirons l'indicateur de profilage lors des compilations
	message( "+++ le profilage est activé" )
	définir( CMAKE_C_FLAGS "-pg " )
	définir( CMAKE_CPP_FLAGS "-pg " )
autre()
	message( "+++ le profilage est désactivé" )
fin si()
unset( GPROF CACHE ) # nous ne voulons pas que cela persiste

ajouter_sous-répertoire(asn1c)
ajouter_sous-répertoire(src/DEF)
ajouter_sous-répertoire(src/SCTP)
ajouter_sous-répertoire(src/messagerouting)
ajouter_sous-répertoire(src/base)
ajouter_sous-répertoire(src/encodage)
ajouter_sous-répertoire(rc_e2sm/src/rc)

add_library( e2sim_shared PARTAGÉ

"$<OBJETS_CIBLES:objets_asn1>;$<OBJETS_CIBLES:objets_def>;$<OBJETS_CIBLES:objets_sctp>;$<OBJETS_CIBLES:objets_de_routage_de_messages>;$<OBJETS_CIBLES:objets_de_codage>;$<OBJETS_CIBLES:objets_de_base>"

)
# Ajout de l'exécutable principal e2sim
add_executable(e2sim src/base/e2sim.cpp)

target_include_directories(e2sim PRIVÉ
  ${CMAKE_CURRENT_SOURCE_DIR}/asn1c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DEF
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SCTP
  ${CMAKE_CURRENT_SOURCE_DIR}/src/messagerouting
  ${CMAKE_CURRENT_SOURCE_DIR}/src/encoding
  ${CMAKE_CURRENT_SOURCE_DIR}/rc_e2sm/src/rc
  ${CMAKE_CURRENT_SOURCE_DIR}/kpm_e2sm/src/kpm
)

bibliothèques_liens_cibles(e2sim
  e2sim_shared
  pthread
)

# nous construisons/exportons uniquement l'archive statique (.a) si nous générons un package de développement
si( DEV_PKG )
    	add_library( e2sim_static STATIQUE
        	     "$<OBJETS_CIBLES:objets_asn1>;$<OBJETS_CIBLES:objets_def>;$<OBJETS_CIBLES:objets_sctp>;$<OBJETS_CIBLES:objets_de_routage_de_messages>;$<OBJETS_CIBLES:objets_de_codage>;$<OBJETS_CIBLES:objets_de_base>"
	)

	définir_propriétés_cible( e2sim_static
   			       PROPRIÉTÉS
   			       NOM_SORTIE "e2sim"
   			       SOVERSION 1
   			       VERSION ${major_version}.${minor_version}.${patch_level}
	)

	répertoires_include_cible( e2sim_static PUBLIC "asn1c" "src/DEF" "src/SCTP" "src/messagerouting" "src/base" "src/encoding" )	
fin si()


si (DEV_PKG )
   définir (liste_cibles "e2sim_static" )
autre()
   définir (liste_cibles "e2sim_shared" )
fin si()

installer( CIBLES ${target_list} EXPORTER LibraryConfig
       DESTINATION DE LA BIBLIOTHÈQUE ${install_lib}
       DESTINATION DES ARCHIVES ${install_lib}
       DESTINATION DE L'EN-TÊTE_PUBLIQUE ${install_inc}
)


unset( DEV_PKG CACHE ) # empêche d'être un paramètre masqué si l'utilisateur refait les choses

SI( EXISTE "${CMAKE_ROOT}/Modules/CPack.cmake" )
	inclure(InstallRequiredSystemLibraries)

	définir( CPACK_DEBIAN_PACKAGE_NAME ${deb_pkg_name} )
	définir( CPACK_RPM_PACKAGE_NAME ${rpm_pkg_name} )

	définir( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/local;/usr/local/bin;/usr/local/include;/usr/local/share;/usr/local/lib" )

	définir( CPACK_set_DESTDIR "on" )
	définir( CPACK_PACKAGING_INSTALL_PREFIX "${install_root}" )
	définir( CPACK_GENERATOR "${pkg_list}" )

	set( CPACK_PACKAGE_DESCRIPTION "Framework C++ pour RIC xAPPs basé sur RMR." )
	set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bibliothèque de framework C++ xAPP RIC" )
	définir( CPACK_PACKAGE_VENDOR "Aucun" )
	définir( CPACK_PACKAGE_CONTACT "Aucun" )
	définir( CPACK_PACKAGE_VERSION_MAJOR "${major_version}" )
	définir( CPACK_PACKAGE_VERSION_MINOR "${minor_version}" )
	définir( CPACK_PACKAGE_VERSION_PATCH "${patch_level}" )
	set( CPACK_PACKAGE "${pkg_label}" ) # nom générique pour les anciennes versions de cpack
	définir( CPACK_DEBIAN_FILE_NAME "${deb_pkg_label}.deb" )
	définir( CPACK_RPM_FILE_NAME "${rpm_pkg_label}.rpm" )

	# Futur : définir les dépendances sur RMR et d'autres bibliothèques

	définir( CPACK_DEBIAN_PACKAGE_PRIORITY "facultatif" )
	définir( CPACK_DEBIAN_PACKAGE_SECTION "ric" )
	définir( CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )
	définir( CPACK_RPM_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )

	# cela semble ignoré s'il est inclus
	#set( CPACK_COMPONENTS_ALL Bibliothèques ApplicationData )

	INCLURE( CPack )
ENDIF()