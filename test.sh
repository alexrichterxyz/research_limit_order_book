#!
rm -f test/test.out
g++ -Ofast -Wall -std=c++20 test/main.cpp -o test/test.out
./test/test.out
