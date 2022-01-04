# pickle

```sh
flex pickle.l && bison -d pickle.y && gcc lex.yy.c pickle.tab.c -o pickle && ./pickle && rm lex.yy.c && rm pickle.tab.h && rm pickle.tab.c && rm pickle
```
