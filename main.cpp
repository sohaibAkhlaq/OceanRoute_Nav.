#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include "H/Helper.h"
#include "H/Graph.h"
using namespace std;


int main()
{
    Graph g("Routes.txt", "PortLocations.txt", "PortCharges.txt");
    g.printGraph();
    return 0;
}
