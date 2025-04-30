#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

rm -f BreakthroughIDSPlayer.jar ;
if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi
javac -cp ${LUDII_JAR_FILE} BreakthroughIDSPlayer_remote.java ;
if [ ! -d "ttt" ];then
  mkdir ttt ;
fi
mv TicTacTocRemotePlayer.class ttt/ ;
jar cf TicTacTocPlayer.jar ttt/TicTacTocRemotePlayer.class ;
