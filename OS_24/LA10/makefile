run: LRU.cpp
	g++ -Wall -o runsearch LRU.cpp
	./runsearch > out.txt
vrun: LRU.cpp
	g++ -Wall -DVERBOSE -o runsearch LRU.cpp
	./runsearch > vout.txt
db: gensearch.c
	gcc -Wall -o gensearch gensearch.c
	./gensearch
clean:
	-rm -f runsearch gensearch out.txt vout.txt