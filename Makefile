debug: MODE=debug
release: MODE=release

debug: solve
release: solve

run: golden.txt
	cat golden.txt

clean:
	rm -f solve

golden.txt: solve
	./solve | sed -E 's/ in [0-9]+[um]s$$//' | head -n -1 > golden.txt 

solve:
	./build.sh ${MODE} $@
