CXX = g++
CXXFLAGS = -Wall -Wextra -O2

# Nom des exécutables
EXEC1 = DH_RC_AI_ids_player
EXEC2 = DH_RC_AI_mcts_player

# Fichiers sources
SOURCES1 = DH_RC_AI_ids_player.cpp
SOURCES2 = DH_RC_AI_mcts_player.cpp

# Compilation des exécutables
$(EXEC1) $(EXEC2): $(SOURCES1) $(SOURCES2)
	$(CXX) $(CXXFLAGS) -o $(EXEC1) $(SOURCES1)
	$(CXX) $(CXXFLAGS) -o $(EXEC2) $(SOURCES2)

# Règle par défaut (all)
all: $(EXEC1) $(EXEC2)

# Nettoyage des fichiers objets et exécutables
clean:
	rm -f $(EXEC1) $(EXEC2)