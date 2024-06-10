# Definire variabili per i compilatori e le opzioni
CC = gcc

# Percorsi dei file sorgente e degli eseguibili
CLIENT_SRC_DIR = client
SERVER_SRC_DIR = server
BUILD_DIR = build

# File sorgenti e di output
CLIENT_SOURCES = $(CLIENT_SRC_DIR)/client.c $(CLIENT_SRC_DIR)/login.c $(CLIENT_SRC_DIR)/room.c $(CLIENT_SRC_DIR)/ncursesUI.c $(CLIENT_SRC_DIR)/chat.c
SERVER_SOURCES = $(SERVER_SRC_DIR)/server.c $(SERVER_SRC_DIR)/database.c
CLIENT_EXEC = client
SERVER_EXEC =  server

# Librerie necessarie
CLIENT_LIBS = -lncurses
SERVER_LIBS = -lsqlite3

# Regole per la compilazione
.PHONY: all clean

all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_SOURCES)
	$(CC) $(CLIENT_SOURCES) $(CLIENT_LIBS) -o $(CLIENT_SRC_DIR)/$(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_SOURCES)
	$(CC) $(SERVER_SOURCES) $(SERVER_LIBS) -o $(SERVER_SRC_DIR)/$(SERVER_EXEC)