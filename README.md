# LSO_2324 - Chat Translator

## Descrizione
Questo progetto consiste in un'applicazione Client-Server di chat a stanze, dove gli utenti possono registrarsi, autenticarsi e chattare in diverse lingue. I messaggi inviati vengono tradotti nella lingua della stanza.

## Requisiti
- Docker
- Docker Compose

## Struttura del Progetto
- **Client**: Applicazione per terminale scritta in C.
- **Server**: Applicazione scritta in C con SQLite per la gestione del database.

## Istruzioni per l'Esecuzione

### 1. Build
```bash
docker compose build
```

### 2. Avviare il Server
```bash
docker compose up server
```

### 3. Eseguire il Client
```bash
docker compose run Client
```

## Autori
Antonio Legnante

Leo Cucurachi
