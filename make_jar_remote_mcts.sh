#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

rm -f DHAIRCPlayerRem.jar ;
if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi
javac -cp ${LUDII_JAR_FILE} DHAIRCPlayerRem.java ;
if [ ! -d "breakthrough" ];then
  mkdir breakthrough ;
fi
mv DHAIRCPlayerRem.class breakthrough/ ;
jar cf DHAIRCPlayerRem.jar breakthrough/DHAIRCPlayerRem.class ;
