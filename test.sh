#!
rm -f src/test/test.out
g++ -Ofast -Wall -std=c++20 src/test/main.cpp -o src/test/test.out
./src/test/test.out
