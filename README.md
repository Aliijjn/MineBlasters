# MineBlasters 🧨⛏️

Welcome to MineBlaster, my attempt at making a full, polished 2D game. I've been working on this project for a month (since 15-01-2025)

# What I've learned from this project: ✅
- How to procedurally generate levels
- The efficient use of algorithms (DDA, DFS)
- How to increase performance using multithreading (800-2000fps!)
- How to manage a larger code base (~4000 lines)

# How to play the game 🏃
- Download the zip file in the repo
- Unzip the file and enjoy :)
- You move with `WASD` and shoot with `MOUSE1` or `SPACEBAR`
- All other essential controls are shown when needed

# Videos: 📷

### You spawn in an abandoned mine, armed with nothing but a revolver and some ammo
https://github.com/user-attachments/assets/2e4c1ef2-9157-462f-9dcc-9db2b0661544

### Explore the mine to find crates that contain precious ammo
https://github.com/user-attachments/assets/d5075e50-09e3-4493-87e1-bb81b577614f

### Fight enemies to progress further
https://github.com/user-attachments/assets/b7f3a9b2-e808-4a4f-93d6-d2dc6cf05ae3

### Find loot rooms for some juicy loot
https://github.com/user-attachments/assets/9b49e9a4-8503-461f-b505-4a5e2195619f

### Upgrade your old junk for some new toys
https://github.com/user-attachments/assets/e3aecede-144a-48e9-ab13-527ff64e11d5

### And descent further down the cave
https://github.com/user-attachments/assets/157ec0ce-414b-4d56-85d8-8481a6f97ce3

### Buy some new equipment
https://github.com/user-attachments/assets/34cadd47-799a-41bd-aee3-30be27080002

### And fight your way through the hordes
https://github.com/user-attachments/assets/1c6426e8-d679-422e-a3f3-0c9e7339ed3e

### Until you make a mistake...
https://github.com/user-attachments/assets/0d31a399-3d0d-446d-ac51-574363f36056

# Technical specifications: ⚙️

## Map generation:
I'm quite proud of how the map generation turned out. Its design is rather simple, but doesn't break immersion and allows for great replayability. Here's the breakdown:
- **Step 1:**
Create a 2D grid with the width and height of `levelCount + 1` and fill it with 0's (open areas).
- **Step 2:**
Fill random elements of this grid with 1's (walls).
- **Step 3:**
Using a depth-first search, check if it's possible to get from the levels start {0, 0} to the levels exit {1, 1}. If not, go back to step 1.
- **Step 4:**
Assign each element in the grid a value based on which neighbours are open areas. e.g. if the grid has a neighbour to the right it gets +1, below +2, left +4 and above +8.
- **Step 5:**
Load the files from the folder chunks. Each is a unique 20x20 area, with chest and enemy spawns. They are numbered between 0 and 15, which are all possible combinations for neighbouring tiles which they need to connect to.
- **Step 6:**
Look up the values of the 2D array, and place the chunks in a new 2D array, that's 20x wider and higher.

## Map generation examples:

### A basic 2x2 map. One of it's elements got turned into a wall.
```
0 1
0 0
```

### The map, after each element got its value, based on their neighbours.
```
2 0
9 4
```

### The chunk for 4:
```
X X X X X X X X X X X X X X X X X X X X
X . . . . . . . . . . . . . . . . . . X
X . . . . . . . . . . . . . . . . . . X
X . . X X X X X X X X X X X X X X . . X
X . . X X X X X X X X X X X X X X . . X
X . . X X . . X . . X . . X . . X . . X
X . . X X . . . . . . . . . . . . . . X
X . . X X . . . . . . . . . . . . . . X
X . . X X . . X . . X . . X . . X . . X
. . . X X . . X X X X X X X X X X X X X
. . . X X . . X X X X X X X X X X X X X
X X X X X . . X X X X X X . . . . X X X
X X X X X . . X X X X X . . . . . . X X
X . . . . . . X X X X . . . . . . . . X
X . . . . . . X X X X . . . . . . . . X
X . . X X X X X X X X . . . . . . . . X
X . . X X X X X X X X . . . . . . . . X
X . . . . . . . . . . . . . . . . . X X
X . . . . . . . . . . . . . . . . X X X
X X X X X X X X X X X X X X X X X X X X
```

### The finished map after all the chunks got inserted in the right place:
```
X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
X . . . . . . X X . . . . . . . . . . X X X X X X X X X X X X X X X X X X X X X
X . . . . . . X X . . . . . . . . . . X X X X X X X X X X X X X X X X X X X X X
X . . X X . . X X X X X X X X X X . . X X X X X X X X X X X X X X X X X X X X X
X . . X X . . X X X X X X X X X X . . X X X X X X X X X X X X X X X X X X X X X
X . . X X . . . . . . . . . . . . . . X X X X X X X X X X X X X X X X X X X X X
X . . X X . . . . . . . . . . . . . . X X X X X X X X X X X X X X X X X X X X X
X . . X X X X X X . . X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
X . . X X X X X X . . X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
X . . X X . . . . . . X X . . . . . . X X X X X X X X X X X X X X X X X X X X X
X . . X X . . . . . . X X . . . . . . X X X X X X X X X X X X X X X X X X X X X
X X X X X . . X X . . X X . . X X . . X X X X X X X X X X X X X X X X X X X X X
X X X X X . . X X . . X X . . X X . . X X X X X X X X X X X X X X X X X X X X X
X . . . . . . X X . . . . . . X X . . X X X X X X X X X X X X X X X X X X X X X
X . . . . . . X X . . . . . . X X . . X X X X X X X X X X X X X X X X X X X X X
X . . X X X X X X X X X X X X X X . . X X X X X X X X X X X X X X X X X X X X X
X . . X X X X X X X X X X X X X X . . X X X X X X X X X X X X X X X X X X X X X
X . . . . . . . . . . . . . . . . . . X X X X X X X X X X X X X X X X X X X X X
X . . . . . . . . . . . . . . . . . . X X X X X X X X X X X X X X X X X X X X X
X X X X X X X X X . . X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
X X X X X X X X X . . X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
X . . . . . . . . . . . . . . . . . . X X . . . . . . . . . . . . . . . . . . X
X . . . . . . . . . . . . . . . . . . X X . . . . . . . . . . . . . . . . . . X
X . . X X X X X X . . X X X X X X . . X X . . X X X X X X X X X X X X X X . . X
X . . X X X X X X . . X X X X X X . . X X . . X X X X X X X X X X X X X X . . X
X . . X X X . . . . . . . . X X X . . X X . . X X . . X . . X . . X . . X . . X
X . . X X . . . . . . . . . . X X . . X X . . X X . . . . . . . . . . . . . . X
X . . X X . . . X X X X . . . X X . . X X . . X X . . . . . . . . . . . . . . X
X . . X X . . X X X X X X . . X X . . X X . . X X . . X . . X . . X . . X . . X
X . . . . . . X X X X X X . . . . . . . . . . X X . . X X X X X X X X X X X X X
X . . . . . . X X X X X X . . . . . . . . . . X X . . X X X X X X X X X X X X X
X . . X X . . X X X X X X . . X X . . X X X X X X . . X X X X X X . . . . X X X
X . . X X . . . X X X X . . . X X . . X X X X X X . . X X X X X . . . . . . X X
X . . X X . . . . . . . . . . X X . . X X . . . . . . X X X X . . . . . . . . X
X . . X X X . . . . . . . . X X X . . X X . . . . . . X X X X . . . . . . . . X
X . . X X X X X X . . X X X X X X . . X X . . X X X X X X X X . . . . . . . . X
X . . X X X X X X . . X X X X X X . . X X . . X X X X X X X X . . . . . . . . X
X . . . . . . . . . . . . . . . . . . X X . . . . . . . . . . . . . . . . . X X
X . . . . . . . . . . . . . . . . . . X X . . . . . . . . . . . . . . . . X X X
X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
```



