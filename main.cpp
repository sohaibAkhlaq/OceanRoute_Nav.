#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include "Helper.h"
#include "Graph.h"
using namespace std;


int main()
{
    Graph g("Routes.txt", "PortLocations.txt", "PortCharges.txt");
    g.printGraph();
    return 0;
}
