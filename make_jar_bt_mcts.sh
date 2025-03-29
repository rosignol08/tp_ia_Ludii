#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

rm -f BreakthroughMCTSPlayer.jar ;
if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi

# Compiler la classe Java
javac -cp ${LUDII_JAR_FILE} BreakthroughMCTSPlayer.java ;

# Créer le répertoire breakthrough si nécessaire
if [ ! -d "breakthrough" ];then
  mkdir breakthrough ;
fi

# Déplacer le fichier class dans le répertoire
mv BreakthroughMCTSPlayer.class breakthrough/ ;

# Créer le JAR
jar cf BreakthroughMCTSPlayer.jar breakthrough/BreakthroughMCTSPlayer.class ;

echo "Le fichier BreakthroughMCTSPlayer.jar a été créé avec succès."