#!/usr/bin/env python3
import os
import json
import requests
import re
from mutagen import File

MUSIC_DIR = os.path.expanduser("~/music/tracks/misc")
LYRICS_DIR = os.path.expanduser("~/lyrics")
FAILED_FILE = os.path.join(LYRICS_DIR, "failed.json")
os.makedirs(LYRICS_DIR, exist_ok=True)

API_URL = "https://api.lyrics.ovh/v1/{artist}/{title}"

def sanitize(s):
    return "".join(c for c in s if c.isalnum() or c in " _-").strip()

def normalize_title(title: str) -> str:
    # remove things in brackets: (Remastered), [Clean], {Live}, etc.
    title = re.sub(r'\s*[\(\[\{].*?[\)\]\}]\s*', '', title)

    # remove everything after the last dash — handles " - Single Version", " - Remastered", etc.
    title = re.sub(r'\s*-\s*[^-]+$', '', title)

    # clean spaces
    title = re.sub(r'\s+', ' ', title).strip()

    return title

def get_metadata(path):
    try:
        audio = File(path, easy=True)
        artist = audio.get("artist", [None])[0]
        title = audio.get("title", [None])[0]
        return artist, title
    except Exception:
        return None, None

def get_lyrics(artist, title):
    try:
        r = requests.get(API_URL.format(artist=artist, title=title))
        if r.status_code == 200:
            data = r.json()
            return data.get("lyrics", "").strip()
    except Exception:
        pass
    return None

def load_failed():
    if os.path.exists(FAILED_FILE):
        with open(FAILED_FILE, "r") as f:
            return json.load(f)
    return []

def save_failed(failed):
    with open(FAILED_FILE, "w") as f:
        json.dump(failed, f, indent=2)

def main():
    failed = load_failed()

    for root, _, files in os.walk(MUSIC_DIR):
        for f in files:
            if not f.lower().endswith((".mp3", ".flac", ".m4a", ".ogg", ".wav")):
                continue

            full_path = os.path.join(root, f)
            artist, title = get_metadata(full_path)
            if not artist or not title:
                continue

            filename = f"{artist} - {title}.txt"
            dest = os.path.join(LYRICS_DIR, filename)

            if os.path.exists(dest):
                continue

            lyrics = get_lyrics(artist, normalize_title(title))
            if lyrics:
                with open(dest, "w") as out:
                    out.write(lyrics)
                print(f"✅ {artist} - {title}")
            else:
                failed.append({"artist": artist, "title": title})
                print(f"❌ No lyrics: {artist} - {title}")

    # Remove duplicates
    unique_failed = {f"{f['artist']}|{f['title']}": f for f in failed}.values()
    save_failed(list(unique_failed))
    print(f"\nFailed entries saved to {FAILED_FILE}")

if __name__ == "__main__":
    main()
