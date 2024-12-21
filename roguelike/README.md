
## Ubuntu setup ##

Install sdl2-dev:
```sh
$ apt-get install libsdl2-dev
```

Build PDCurses:
```sh
$ git clone git@github.com:wmcbrine/PDCurses.git
$ cd PDCurses/sdl2/ && make
```

To build app:
```sh
$ PDCURSESPATH=<PDCURSESLIB PATH> make
```

To run app:
```sh
$ PDCURSESPATH=<PDCURSESLIB PATH> make run
```
