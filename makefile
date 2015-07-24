all: fsm.png

%.svg: %.dot
	dot -Tsvg $< > $@

%.png: %.svg
	convert -resize 1000 $< $@

clean:
	rm -f fsm.png

