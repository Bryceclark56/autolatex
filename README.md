# AUTOLATEX

A convenient tool I made for myself.

It's designed to monitor a file, using inotify. 
When it detects that I have saved/modified the file, 
	it will run `pdflatex` on the file (in nonstop mode).

Afterwards, it will force mupdf process to update view of the pdf.

## Usage

```bash
./autolatex test.tex
```

## License
[MIT](https://choosealicense.com/licenses/mit/)
