#Makefile for debug pcm5242 eq
target=reg_tool
$(target) : diff_pcm5242eq.o
	gcc -o  $(target) diff_pcm5242eq.c
clean :
	rm $(target) *.o