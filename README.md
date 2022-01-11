# pickle

## bash commands

```sh
make
./pickle
make clean
```

## error detection

### redefinition errors
- global variables 🥒
- types 🥒
- functions 🥒
- members 🥒
- arguments 🥒

### cycle of dependencies from type definitions 🥒
```
A { B b; }
B { A a; }
```

### `break`/`continue` not inside loop 🥒
```
void fun() {
  if (true) {
    break;
  }
}
```

### undefined type
```
X x = { }; 🥒
Type { X x; } 🥒
void fun1(X x) { } 🥒
void fun2() { X x = { }; } 🥒
X fun3() { } 🥒
```

### `void` errors
- `void[]` 🥒
- `void` elsewhere than in function return type 🥒

### variable errors
- undefined variable
- redefined variable
- constant inside `rvalue` of assignment

### undefined function
```
int x = fun1();
void fun2() { int x = fun1(); }
void fun3() { int x = fun2(10); }
```

### type errors
- in expressions
- in function calls
- in `return` statements
- in declarations
- in assignations
- in `if`/`while`/`for` expressions (check that they are `bool`/`int`)
- in member access and object literals (check that the member name exists in the given type)
- in element access (check that `lvalue` is an array)

### runtime errors
- index out of bounds
