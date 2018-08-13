CC=g++
#used for gprof mode
#CFLAGS = -Wall -pg -I`pkg-config --cflags opencv`
#LDFLAGS = -pg `pkg-config --libs opencv`

#used for normal mode
CFLAGS = -Wall -O3 -I`pkg-config --cflags opencv`
LDFLAGS = `pkg-config --libs opencv`

HEADERS = opticalflow.h features.h morph.h log.h

all: objectDetector clean
	@echo "objectDetector is succefully made"

objectDetector: main.o features.o morph.o opticalflow.o
	$(CC) -Wall -o $@ $^ $(LDFLAGS)

main.o: main.cpp $(HEADERS)
	$(CC) -c $(CFLAGS) $<

features.o: features.cpp features.h
	$(CC) -c $(CFLAGS) $<

morph.o: morph.cpp morph.h
	$(CC) -c $(CFLAGS) $<

opticalflow.o: opticalflow.cpp opticalflow.h
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm -f *.o

cleanest: clean
	rm objectDetector
