# AUTOLATEX

A convenient tool I made for myself.

It's designed to monitor a file, using inotify, and when it detects that I have saved/modified the file, it will run `pdflatex` on the file (in nonstop mode).

## Usage

```bash
./autolatex test.tex
```

## License
[MIT](https://choosealicense.com/licenses/mit/)
