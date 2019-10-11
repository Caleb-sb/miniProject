.RECIPEPREFIX +=
CC = g++
CFLAGS = -Wall -lm -lrt -lwiringPi -lpthread

PROG = bin/*
OBJS = obj/*

default:
    mkdir -p bin obj
    $(CC) $(CFLAGS) -c src/miniProject.c -o obj/miniProject
    $(CC) $(CFLAGS) -c src/currentTime.c -o obj/currentTime
    $(CC) $(CFLAGS) obj/miniProject obj/currentTime -o bin/miniProject

run:
    sudo ./bin/miniProject

clean:
    rm $(PROG) $(OBJS)
