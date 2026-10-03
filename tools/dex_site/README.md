# Emerald Dex

Pokédex offline del hack, generato dai sorgenti della ROM. Nessun server: basta aprire
`index.html` nel browser.

## Aggiornare il sito

Dalla cartella del repository, in WSL:

```bash
python3 tools/dex_site/build.py
```

Lo script:
1. lancia i tre export (`export_pokemon_json`, `export_moves_json`, `export_abilities_json`);
2. legge la tabella dei tipi da `src/battle_main.c` e i numeri delle MT/MN da `include/constants/tms_hms.h`;
3. scrive `js/data.js` con tutti i dati;
4. copia gli sprite da `graphics/pokemon/*/front.png` in `sprites/pokemon/N.png` (serve Pillow:
   `sudo apt install python3-pil`).

Opzioni: `--no-export` usa i JSON già generati, `--no-sprites` non ricopia gli sprite.

## File

| File | |
|---|---|
| `index.html` | la pagina |
| `css/style.css` | stile (base presa da "my dex") |
| `js/app.js` | logica del sito |
| `js/data.js` | generato da `build.py`, non modificarlo a mano |
| `sprites/pokemon/` | generati da `build.py` |
