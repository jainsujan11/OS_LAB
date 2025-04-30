run: demandpaging.cpp
	g++ -Wall -o runsearch demandpaging.cpp
	./runsearch
vrun: demandpaging.cpp
	g++ -Wall -DVERBOSE -o runsearch demandpaging.cpp
	./runsearch
db: gensearch.c
	gcc -Wall -o gensearch gensearch.c
	./gensearch 30 30 
clean:
	-rm -f runsearch gensearch
