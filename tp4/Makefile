all: tp4

tp4: tp4.pdf


%.pdf: %.tex
	pdflatex --interaction=nonstopmode $<

clean:
	$(RM) tp4.log tp4.pdf tp4.aux
