#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi
javac -classpath ".:${LUDII_JAR_FILE}" RunManyGames.java
java -cp ".:${LUDII_JAR_FILE}" RunManyGames

