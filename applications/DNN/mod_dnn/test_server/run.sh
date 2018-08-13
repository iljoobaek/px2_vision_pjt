g++ -shared -c -fPIC server.cpp -o server.o && g++ -shared -Wl,-soname,library.so -o server.so server.o
