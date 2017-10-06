# Data-Flow-Piggy

A multidirectional data flow program that transmits and recieves messages by using tcp connections. The program integrates ncurses for a better viewing experience. 

With ncurses the terminal windows are displayed in 8 seperate regions of the screen. Narrow slices of the full width of the screen along the bottom will be used to show the errrors, while middle bottom will shot the input and log, and the top-bottom section will show the commands. While the remainder of the screen will be sub-divided into four equal sized boxes to show the data entering and leaving the program. 

Below is how the terminal screen will look like.
![terminalview](https://user-images.githubusercontent.com/30418138/31292292-b9e90988-aa87-11e7-8cfd-64db3f11ac09.png)


When running the piggy porgrams the data can flow from either direction. The designed structure is described as hooking each other in a of chain. 

Below is a simple visual of three piggies, but not limit to, on how the program is to behave.
![piggy_visual](https://user-images.githubusercontent.com/30418138/31292741-670207f4-aa89-11e7-9be9-a258bfb4b4e8.png)

