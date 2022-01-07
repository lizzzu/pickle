#include "scanner.h"
#include "parser.hpp"
#include "interpreter.h"

using namespace std;
using namespace Pickle;

int main() {
    Interpreter driver;
    ifstream file("input.txt");
    driver.switchInputStream(file);
    const int res = driver.parse();
    file.close();
    return res;
}
