#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi
java -jar ${LUDII_JAR_FILE}