#!/bin/bash
LUDII_JAR_FILE="Ludii-1.3.11.jar"

if [ ! -f ${LUDII_JAR_FILE} ];then
  echo "Ludii jar file is missing" ;
  exit 0 ;
fi

# Vérifier si les JAR des joueurs existent
if [ ! -f BreakthroughIDSPlayer.jar ];then
  echo "BreakthroughIDSPlayer.jar file is missing, please run make_jar_bt_ids.sh first" ;
  exit 0 ;
fi

if [ ! -f BreakthroughMCTSPlayer.jar ];then
  echo "BreakthroughMCTSPlayer.jar file is missing, please run make_jar_bt_mcts.sh first" ;
  exit 0 ;
fi

# Vérifier que les programmes C++ sont compilés
if [ ! -f DH_RC_AI_ids_player ];then
  echo "DH_RC_AI_ids_player executable is missing, please compile it first" ;
  exit 0 ;
fi

if [ ! -f DH_RC_AI_mcts_player ];then
  echo "DH_RC_AI_mcts_player executable is missing, please compile it first" ;
  exit 0 ;
fi

# Lancer Ludii
java -jar ${LUDII_JAR_FILE}

echo "Pour utiliser les joueurs Breakthrough, ouvrez le jeu Breakthrough dans Ludii"
echo "puis sélectionnez les joueurs dans le menu déroulant 'AI', en choisissant"
echo "BT_IDS ou BT_MCTS."