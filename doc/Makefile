SOURCES = $(wildcard *.txt)
DIAGRAMS = $(patsubst %.txt,%.png,$(SOURCES))

all: $(DIAGRAMS)

$(DIAGRAMS): %.png: %.txt
	plantuml -verbose $(^)
