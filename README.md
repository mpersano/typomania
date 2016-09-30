Huh?
----
A typing game with Japanese songs, inspired on the Flash game Typing Mania 4 (site seems to be down).

Building (Linux) 
----------------
Run `fetchstreams.pl` under `data/` to download some songs from YouTube and convert them to ogg. You'll need jwz's [youtubedown](https://www.jwz.org/hacks/youtubedown) and ffmpeg.

Add a font to `data/` with something like:

    ln -s /usr/share/fonts/truetype/takao-gothic/TakaoGothic.ttf data/font.ttf

Build:

    mkdir build
    cd build
    cmake ..
    make

Gameplay
--------
The romaji at the bottom is just a hint, many characters have multiple valid inputs. For instance, し can be typed "si" or "shi", つ can be typed "tsu" or "tu", じ can be typed "ji" or "zi". Just forget the romaji and focus on the actual Japanese text.
