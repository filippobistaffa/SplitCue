SplitCue
===================
SplitCue is a GTK3 program to split an audio image into the single songs given the CUE file.

<p align="center"><img src="http://i.imgur.com/kZaEYfQ.png" width="530" /></p>

Execution
----------
SplitCue receives the path of the input CUE file as its unique command-line argument, i.e.,
```
splitcue CUEFILE
```

Input
----------
In order to decode a specific input format, the corresponding decoder executable must be installed on the system (e.g., `mac` for [Monkey's Audio](http://www.monkeysaudio.com) files).

Output
----------
By default, SplitCue creates one [FLAC](https://xiph.org/flac) file per song, each with the path
```
DESTINATION/ARTIST/ARTIST - YEAR - ALBUMTITLE/ARTIST - TRACKNUMBER - SONGTITLE.flac
```
where `DESTINATION` is the root directory of all artists. 
`DESTINATION` is defined inside [`split.h`](split.h).
If you want to change the output filename pattern, dig into the `split_file` function inside [`shntool/mode_split.c`](shntool/mode_split.c).

Genres
----------
SplitCue reads the genres from the `GENRES_FILE` file defined inside [`split.h`](split.h), which must contain one genre per line, e.g.,

    Atmospheric Black Metal
    Black Metal
    Blues
    Brutal Death Metal
    Cascadian Black Metal
    Crossover
    Death Metal
    Death 'n' Roll
    Djent
    Doom Metal
    Extreme Progressive Metal
    Folk Metal
    Glam Rock
    Grindcore
    Groove Metal
    Grunge
    Hardcore Punk
    Hard Rock
    Heavy Metal
    Indie
    Industrial Metal
    Mathcore
    Melodic Black Metal
    Melodic Death Metal
    Metalcore
    Oi!
    Power Metal
    Power Speed Metal
    Progressive Metal
    Progressive Death Metal
    Progressive Rock
    Punk
    Sludge Metal
    Ska
    Speed Metal
    Stoner Metal
    Technical Death Metal
    Thrash Black Metal
    Thrash Metal
    Viking Metal
