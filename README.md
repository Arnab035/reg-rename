# cap-reg-rename
This is the stripped workflow describing how arch registers get renamed into physical registers in Gem5.

# to compile and run
```
g++ -std=c++17 main.cc debug.cc rename_map.cc regfile_o3.cc reg_class.cc -o ./cap-reg-rename

./cap-reg-rename
```
