all: solve

run: all
	./solve

clean:
	rm -f solve

solve:
	./build.sh $@
