-- n@up8.edu 2025 --
Exemple d'interface d'un joueur IA avec Ludii
Jeu : tictactoe
Joueur cpp : tic_tac_toc_player.cpp
Joueur python : tic_tac_toc_player.py

En supposant que le Ludii-1.3.11.jar est dans le répertoire courant
ou en modifiant la variable LUDII_JAR_FILE dans les scripts bash

Modifier le fichier TicTacTocPlayer.java comme indiqué ci dessous pour jouer avec le joueur cpp ou Python

-- Jouer avec le joueur cpp
Pour compiler le joueur tic_tac_toe_player : sh ./make_bin.sh
Mettre la variable CPP_ACTIVE à true dans TicTacTocPlayer.java (ligne 34)
Vérifer PYTHON_ACTIVE à false (ligne 36)
Pour obtenir un jar pour joueur avec le joueur en utilisant l'IHM Ludii : sh ./make_jar.sh
Pour jouer avec l'IHM Ludii : sh ./make_run.sh
Pour avoir des stats entre des joueurs IA : sh ./make_run_many_games.sh
Pour supprimer les fichiers créés : sh ./make_clean.sh

-- Jouer avec le joueur python
Vérifier également le sha-bang (première ligne spécifiant le chemin vers l'interpréteur python)
Vérifier que les droits en execution sur tic_tac_toc_player.py
Mettre la variable PYTHON_ACTIVE à true dans TicTacTocPlayer.java (ligne 36)
Vérifer CPP_ACTIVE à false (ligne 34)
Pour obtenir un jar pour joueur avec le joueur en utilisant l'IHM Ludii : sh ./make_jar.sh
Pour jouer avec l'IHM Ludii : sh ./make_run.sh
Pour avoir des stats entre des joueurs IA : sh ./make_run_many_games.sh
Pour supprimer les fichiers créés : sh ./make_clean.sh

-- Jouer avec le joueur python remote
SUR LA MACHINE REMOTE
Placer tic_tac_toc_player.py et tic_tac_toc_remote_player.sh
Vérifier les sha-bang et venv associés sur la machine remote
Vérifier que les droits en execution sur tic_tac_toc_remote_player.sh
Tester l'execution distante à partir d'une autre machine : ssh mil-www ttt-2025/tic_tac_toc_remote_player.sh x.x...... x
SUR LA MACHINE LOCALE
Définir la variable remote_player_str dans TicTacTocRemotePlayer.java (ligne 35)
Définir la variable remote_host_str dans TicTacTocRemotePlayer.java (ligne 36)

Pour obtenir un jar pour joueur avec le joueur en utilisant l'IHM Ludii : sh ./make_jar_remote.sh
Pour jouer avec l'IHM Ludii : sh ./make_run.sh
Pour avoir des stats entre des joueurs IA : sh ./make_run_many_games.sh
Pour supprimer les fichiers créés : sh ./make_clean.sh
