# 🎵 Verse

A simple command-line tool that fetches lyrics for your music collection and prints a random lyric line — like the classic `fortune` command, but for your songs.

---

![Options](https://github.com/rexept/verse/blob/master/assets/options.png?raw=true)

## ✨ Features

- 🔍 Automatically fetches lyrics for your local music directory (optional).
- 🎲 Picks a random lyric line and prints it to the console.
- 🎤 Optional flags to show artist and/or song title (currently parses filename, metadata support coming soon).

---

## 📦 Installation

### Clone the repository

```bash
git clone https://github.com/rexept/verse.git
cd verse
```

### Dependencies required:
`Taglib` (C), `Mutagen`(Python)

Install with your local package manager.

#### To use the lyrics downloader:
Edit these lines in the python script to your desired directories (defaults shown):

```python
MUSIC_DIR = os.path.expanduser("~/music")
LYRICS_DIR = os.path.expanduser("~/.lyrics")
```

### Compile verse locally

```bash
make LYRICS_DIR={your lyrics directory here, defaults to $HOME/.lyrics}
```
### Or install globally

```bash
sudo make LYRICS_DIR={your lyrics directory here, defaults to $HOME/.lyrics} install
```
## 🚀 Usage

```bash
verse [FLAGS]
```

### Basic Example

```bash
verse --show-artist
```

Output:
```
The Beatles
And in the end, the love you take is equal to the love you make.
```

---

Thanks to [termshot](https://github.com/homeport/termshot) for the amazing command screenshot!
