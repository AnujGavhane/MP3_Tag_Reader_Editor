# MP3 Tag Reader

## Project overview

MP3 Tag Reader is a command-line program written in C. It reads common ID3 metadata stored in an MP3 file and can update one selected tag. It displays the ID3 version and these fields using their frame IDs:

| Frame ID | Meaning | Edit modifier |
|---|---|---|
| `TIT2` | Title | `-t` |
| `TPE1` | Artist | `-a` |
| `TALB` | Album | `-A` |
| `TYER` | Year | `-y` |
| `TCON` | Genre / content | `-m` |
| `COMM` | Comment | `-c` |

The project supports ID3v1 and ID3v2.2/v2.3 for reading. ID3v2.4 is not supported. The editor writes an ID3v2 tag, preserves other ordinary frames, and copies the MP3 audio data into the updated file.

## Project files

- `main.c` reads the command-line options and calls the reader, editor, or display functions.
- `mp3_tag.c` reads ID3 headers and frames, extracts supported values, and checks for ID3v1 tags.
- `edit.c` replaces the selected tag frame and copies the remaining file data.
- `view.c` prints the help menu and tag values.
- `types.h` defines the shared tag fields and data structure.
- `edit.h` and `view.h` declare functions used by the editor and display modules.
- `original.mp3` is the sample MP3 file included with the project.

## Requirements

Use a Linux terminal, WSL, or another environment with GCC installed. The commands below assume the project files are in the same directory.

## Open the project in WSL

If the project is in the Windows folder shown below, open WSL and change to it. Keep the quotes because the path may contain spaces:

```sh
cd "/mnt/c/Users/Anuj/Desktop/MP3_Tag_Reader_Code"
```

Confirm that the source files are present:

```sh
pwd
ls
```

## Compile

Compile all C source files in the current directory:

```sh
gcc -std=c99 -Wall -Wextra *.c -o mp3_tag_reader
```

If the command finishes without errors, GCC has created an executable named `mp3_tag_reader`. The `*.c` pattern includes `main.c`, `mp3_tag.c`, `view.c`, and `edit.c`. Header files are included by the C source and should not be listed as compile inputs.

## Run commands

Print the help menu:

```sh
./mp3_tag_reader --help
```

View metadata from the sample file:

```sh
./mp3_tag_reader -v original.mp3
```

The output includes the ID3 version and the supported frame IDs. A missing value is shown as `(not set)`.

## Edit a tag

The edit command has this form:

```sh
./mp3_tag_reader -e <modifier> "new value" <file.mp3>
```

For example, make a copy of the sample first, then update and view its title:

```sh
cp original.mp3 sample_copy.mp3
./mp3_tag_reader -e -t "A new title" sample_copy.mp3
./mp3_tag_reader -v sample_copy.mp3
```

Other examples:

```sh
./mp3_tag_reader -e -t "A new title" sample_copy.mp3
./mp3_tag_reader -e -a "Artist name" sample_copy.mp3
./mp3_tag_reader -e -A "Album name" sample_copy.mp3
./mp3_tag_reader -e -y 2026 sample_copy.mp3
./mp3_tag_reader -e -m Rock sample_copy.mp3
./mp3_tag_reader -e -c "A short comment" sample_copy.mp3
```

Put quotation marks around a value containing spaces so the shell passes it as one argument. The modifier is case-sensitive: `-a` edits Artist and uppercase `-A` edits Album. Make a copy or backup before editing an MP3 you want to preserve.

## Command-Line Screenshots

### 1. Build and Help

![Build and help output](https://github.com/AnujGavhane/MP3_Tag_Reader_Editor/blob/main/OP_Demo_Screenshots/Build_%26_Help.png)

### 2. View Tags

![View tags output](https://github.com/AnujGavhane/MP3_Tag_Reader_Editor/blob/main/OP_Demo_Screenshots/View_Tags.png)

### 3. Edit Tags

![Edit tags and updated output](https://github.com/AnujGavhane/MP3_Tag_Reader_Editor/blob/main/OP_Demo_Screenshots/Edit_Tags.png)

## Common errors

- **`Invalid arguments`**: run `./mp3_tag_reader --help` and check the command format.
- **`Could not open MP3 file`**: check the file name and current directory. Use `ls` to see the files here, or pass the full file path.
- **No ID3 tag found**: the file may not contain an ID3v1 or supported ID3v2.2/v2.3 tag.
- **ID3v2.4 not supported**: this version is outside the project requirements.
- **Compiler reports missing source files**: make sure you are in the project directory and that all `.c` files are present.

## Current limitations

The reader handles ordinary, uncompressed ID3v2 frames and displays text as basic single-byte characters. It does not fully convert Unicode text or process advanced frame features such as compression, encryption, or unsynchronization. Embedded picture details are not shown in this simplified interface.
