all: solve

run: all
	./solve

clean:
	rm -f solve

.PHONY: solve
solve:
	./build.sh $@
