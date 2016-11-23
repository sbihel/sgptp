all: tp5

tp5: tp5.pdf


%.pdf: %.tex
	pdflatex --interaction=nonstopmode $<

clean:
	$(RM) tp5.log tp5.pdf tp5.aux
