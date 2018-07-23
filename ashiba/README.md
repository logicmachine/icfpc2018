# READ ME
* compile
```
g++ -std=c++11 SOLVER_NAME.cpp
```

* execute
```
./a.out INPUT_FILENAME.mdl
```

## solver.cp
* Using 1 NanoBots
* Backward
* Eat from leaves

## near_solver.cpp
* Using 1 NanoBots
* Backward
* Eat from leaves
* Continuing to eat when new leaves appears by eating

## multi_solver.cpp
* Using many NanoBots( N = min(40, max((R-1)/3*3+1,4)) )
* Forward
* Eat from leaves
* Each bot responsible each leaves
