# IGADPiGameServer
Game server created for educational purposes for IGAD (Breda University of Applied Sciences, the Netherlands)

<img src="https://insight.nhtv.nl/wp-content/uploads/2018/01/English-banner.jpg" />

## How to use

In order to learn how to use the server, check out the documentation. There you will find help files for how to communicate with the server.
### Dependencies

- CMake 3.9.9
- RakNet build as static lib
### Building dependencies

This little bash script will help you build RakNet:
```bash
#!/bin/bash

# RakNet installation:
# Commands below provided by Bert Heesakkers (NHTV) and modified by Zandor Smith.
sudo apt-get install cmake -y

git clone https://github.com/facebookarchive/RakNet.git
cd RakNet
cmake .
make
make install

echo Removing old RakNet installation if it exists.
sudo rm -rf /usr/local/include/raknet

echo Moving RakNet to the correct directory.
sudo mv [RakNet path]/RakNet/Lib/LibStatic/libRakNetLibStatic.a /usr/local/lib
sudo mv [RakNet path]/RakNet/include/raknet /usr/local/include/

echo Cleaning up...
rm -rf [RakNet path]/RakNet

echo RakNet installed.
```

## Build

```
$ cd /server_root
$ cmake .
$ make install
```

## Run
You have to make sure that there is a log folder and that the app has the premission to write in it:
```
$ cd /server/path/bin/
$ mkdir -p log/Games
$ chmod 666 -R log
```
The you can run the application:
```
$ /server/path/bin/ServerApp
```

### Supported compiler

- GCC 4.9 - 7.2 (and possibly later) (because of the [json.hpp](https://github.com/nlohmann/json#supported-compilers)) (because of the [json.hpp](https://github.com/nlohmann/json#supported-compilers))
- Clang 3.4 - 5.0 (and possibly later) (because of the [json.hpp](https://github.com/nlohmann/json#supported-compilers))
- Intel C++ Compiler 17.0.2 (and possibly later) (because of the [json.hpp](https://github.com/nlohmann/json#supported-compilers))
- Microsoft Visual C++ 2015 / Build Tools 14.0.25123.0 (and possibly later) (because of the [json.hpp](https://github.com/nlohmann/json#supported-compilers))
- Microsoft Visual C++ 2017 / Build Tools 15.5.180.51428 (and possibly later) (because of the [json.hpp](https://github.com/nlohmann/json#supported-compilers))
