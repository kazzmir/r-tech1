# R Tech 1
A set of C++ wrappers around existing graphical, music and other misc. libraries. 
The library historically used to wrap allegro4, SDL 1.2 and subsequently [Allegro5](https://github.com/liballeg/allegro5). This allowed for using SDL to build  [Paintown](https://www.github.com/kazzmir/paintown) on platforms such as the Wii. 
Since separating R Tech 1 from [Paintown](https://www.github.com/kazzmir/paintown) it now only depends on [Allegro5](https://github.com/liballeg/allegro5) as its backend. 
*In the future SDL 2 or other libraries may be supported.*

---

## Projects that use R Tech 1

| Project Name        | GitHub               | Author |
| :-------------      | :------------------: |:--------:|
| Paintown            | [view](https://www.github.com/kazzmir/paintown)| [kazzmir](https://www.github.com/kazzmir) |
| Asteroids            | [view](https://www.github.com/kazzmir/asteroids)| [kazzmir](https://www.github.com/kazzmir) |
| Platform Engine      | [view](https://www.github.com/juvinious/platform-engine)| [juvinious](https://www.github.com/juvinious) |

## Requirements
* [SCons](http://scons.org/)
* [Allegro 5](https://github.com/liballeg/allegro5)
* [Freetype2](https://freetype.org/freetype2/docs/index.html)
* [zlib](https://zlib.net/)

## Libraries that R Tech 1 includes or wraps:

| Library        | Provides             | In build |
| :------------- | :------------------: |:--------:|
| Allegro5       | graphics/sound/input |*External |
| Freetypo2      | fonts                |*External |
| zlib           | compression          |*External |
| 7zip           | compression          |Internal  |
| LZ4            | compression          |Internal  |
| ZIP            | compression          |Internal  |
| Hawknl         | network              |Internal  |
| SFL            | filesystem/os        |Internal  |
| PCRE           | regular expressions  |Internal  |
| HQX/XBR        | pixel scaling        |Internal  |
| DUMB           | mod/s3m/xm/it        |Internal  |
| GME            | console sound formats, e.g: NES, SNES, etc |
*\* External libraries are linked in using pkg-config after install*

## How to Build, install and link to R Tech 1
```bash
 # Build, append verbose=1 to print verbosely
 $ scons
 
 # Debug build
 $ export DEBUG=1
 $ scons
 
 # To install (set prefix if desired defaults to /usr/local)
 $ export PREFIX=/usr/local
 $ sudo scons install
 
 # To get package information use pkg-config to use in your own application
 $ pkg-config r-tech1 --version
 $ pkg-config r-tech1-debug --cflags --libs
```

---
> Originally written for the game [Paintown](http://paintown.org)