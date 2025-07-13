#include "parser/parser.h"
#include "evaluator/evaluator.h"
#include "ui/ui.h"


int main() {
    runUI();  
    return 0;
}



/*g++ main.cpp ui/ui.cpp parser/parser.cpp evaluator/evaluator.cpp -o desmos -I/mingw64/include -L/mingw64/lib -lraylib -lwinmm -lgdi32 -std=c++17 -Wall -Wextra -O2
*/

/*cd /c/Users/Asus/Desktop/DSA-Project/desmos-clone
*/