.RECIPEPREFIX +=
CC = g++
CFLAGS = -Wall -lm -lrt -lwiringPi -lpthread

PROG = bin/*
OBJS = obj/*

default:
    mkdir -p bin obj
    $(CC) $(CFLAGS) -c src/miniProject.cpp -o obj/miniProject
    $(CC) $(CFLAGS) obj/miniProject -o bin/miniProject

run:
    sudo ./bin/miniProject

clean:
    rm $(PROG) $(OBJS)
