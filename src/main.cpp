#include <SFML/Graphics.hpp>
#include <iostream>
#include "classes.h"

int main(){
    Game g("resources/config.txt");
    g.run();
    return 0;
}
