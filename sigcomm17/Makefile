DOCUMENT=main

default: $(DOCUMENT).pdf

$(DOCUMENT).toc: $(DOCUMENT).tex
	pdflatex $(DOCUMENT).tex

$(DOCUMENT).pdf: $(DOCUMENT).toc
	bibtex $(DOCUMENT)
	pdflatex $(DOCUMENT).tex
	pdflatex $(DOCUMENT).tex

#--------------------------------------------------
# $(DOCUMENT).ps: $(DOCUMENT).dvi
# 	dvips -t letter -Ppdf -G0 -o $@ $<
# 
# $(DOCUMENT).pdf: $(DOCUMENT).ps
# 	cat $(DOCUMENT).ps | ps2pdf - > $(DOCUMENT).pdf 
#-------------------------------------------------- 

all: $(DOCUMENT).pdf

clean:
	rm -rf *.aux $(DOCUMENT).log $(DOCUMENT).toc \
     $(DOCUMENT).bbl $(DOCUMENT).blg *.bak $(DOCUMENT).fdb_latexmk $(DOCUMENT).fls \
		 $(DOCUMENT).out *.log *.gz\
      $(DOCUMENT).pdf 


open:
	open $(DOCUMENT).pdf
