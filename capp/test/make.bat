g++ -c uart.cpp -o uart.o -std=c++0x
g++ -c main.cpp -o main.o -std=c++0x

g++ -o main.exe main.o uart.o