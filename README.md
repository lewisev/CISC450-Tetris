# CISC450-Tetris
This is a version of Tetris which implements a server and client connection over TCP. 
> Original version found here: https://github.com/najibghadri/Tetris200lines


## Info
The server and client communicates on the local machine (**127.0.0.1**) with **port 8080**.

The Tetris game runs and displays on the server and the client sends the keystrokes to the server to move the blocks.

Controls for client are:
- **W:** rotate the block
- **A:** move block left
- **S:** move block down
- **D:** move block right

You can hit **Q** to quit the game, which will stop the client and the server.

## Files
- `client.c`: all the code for the client
- `tetris_server.c`: all the code for the hosted server
- `Makefile`: used to compile the programs in one command


## How to run
1. Have C installed on your computer
2. Run `make tetris` to compile the C files into executables
3. On one terminal window, run `./tetris_server` to start the server
4. On another terminal window, run `./client` to start the client.