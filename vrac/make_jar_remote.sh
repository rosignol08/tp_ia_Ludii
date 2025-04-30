#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

rm -f TicTacTocRemotePlayer.jar ;
if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi
javac -cp ${LUDII_JAR_FILE} BreakthroughIDSPlayer.java ;
if [ ! -d "ttt" ];then
  mkdir ttt ;
fi
mv BreakthroughIDSPlayer.class breakthrough/ ;
jar cf BreakthroughIDSPlayer.jar breakthrough/BreakthroughIDSPlayer.class ;
