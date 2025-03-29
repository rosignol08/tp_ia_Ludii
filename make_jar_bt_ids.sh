#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

rm -f BreakthroughIDSPlayer.jar ;
if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi

# Compiler la classe Java
javac -cp ${LUDII_JAR_FILE} BreakthroughIDSPlayer.java ;

# Créer le répertoire breakthrough si nécessaire
if [ ! -d "breakthrough" ];then
  mkdir breakthrough ;
fi

# Déplacer le fichier class dans le répertoire
mv BreakthroughIDSPlayer.class breakthrough/ ;

# Créer le JAR
jar cf BreakthroughIDSPlayer.jar breakthrough/BreakthroughIDSPlayer.class ;

echo "Le fichier BreakthroughIDSPlayer.jar a été créé avec succès."