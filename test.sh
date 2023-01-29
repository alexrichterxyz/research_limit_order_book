#!
rm -f test/test.out
g++ -Ofast -Wall -std=c++17 test/main.cpp -o test/test.out
./test/test.out
