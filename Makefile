debug: MODE=debug
release: MODE=release

debug: solve
release: solve

run: all
	./solve

clean:
	rm -f solve

.PHONY: solve
solve:
	./build.sh ${MODE} $@
