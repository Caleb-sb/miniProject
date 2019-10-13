.RECIPEPREFIX +=
CC = g++
CFLAGS = -Wall -lm -lrt -lwiringPi -lpthread -I ./src/ -DLINUX -DRASPBERRY

PROG = bin/*
OBJS = obj/*

default:
    mkdir -p bin obj
    $(CC) $(CFLAGS) -c src/miniProject.cpp -o obj/miniProject
    $(CC) $(CFLAGS) -c src/currentTime.c -o obj/currentTime
    $(CC) $(CFLAGS) -c src/utility/BlynkDebug.cpp -o obj/BlynkDebug
    $(CC) $(CFLAGS) -c src/utility/BlynkHandlers.cpp -o obj/BlynkHandlers
    $(CC) $(CFLAGS) -c src/utility/BlynkTimer.cpp -o obj/BlynkTimer
    $(CC) $(CFLAGS) obj/BlynkDebug obj/BlynkHandlers obj/BlynkTimer obj/miniProject obj/currentTime -o bin/miniProject

run:
    sudo ./bin/miniProject

clean:
    rm $(PROG) $(OBJS)
