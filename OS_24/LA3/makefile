compile: schedule.c
	gcc -Wall -o schedule schedule.c -lm
run: compile
	./schedule > myoutput.txt
vcompile: schedule.c
	gcc -Wall -o schedule -DVERBOSE schedule.c -lm
vrun: vcompile
	./schedule > myverboseoutput.txt
db: genproc.c
	gcc -Wall -o genproc genproc.c
clean:
	-rm -f genproc schedule proc.txt myoutput.txt myverboseoutput.txt
