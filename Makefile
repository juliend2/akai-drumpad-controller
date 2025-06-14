clean:
	rm ./read_midi

read_midi: clean
	gcc main.c -o read_midi -lasound

.PHONY: run
run: read_midi
	./read_midi
