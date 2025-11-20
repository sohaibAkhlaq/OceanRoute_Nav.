#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include "Helper.h"
using namespace std;

class Edge
{
public:
    string dest;
    string date;
    string dep;
    string arr;
    int cost;
    string company;
    Edge *next;

    Edge(string d, string da, string de, string a, int c, string co)
    {
        dest = d;
        date = da;
        dep = de;
        arr = a;
        cost = c;
        company = co;
        next = nullptr;
    }
};

class Port
{
public:
    float lat, lon;
    bool left = false;
    float charges = 0;

    Port() {}
    Port(float la, float lo, bool l = false, float ch = 0)
    {
        lat = la;
        lon = lo;
        left = l;
        charges = ch;
    }
};

class PortNode
{
public:
    string name;
    Edge *head;
    PortNode *next;
    Port portData;

    PortNode(string n)
    {
        name = n;
        head = nullptr;
        next = nullptr;
    }
};

class Graph
{
private:
    PortNode *vertices;

    PortNode *findVertex(string name)
    {
        PortNode *temp = vertices;
        while (temp != nullptr)
        {
            if (temp->name == name)
                return temp;
            temp = temp->next;
        }
        return nullptr;
    }

    PortNode *addVertex(string name)
    {
        PortNode *v = findVertex(name);
        if (v != nullptr)
            return v;
        PortNode *nv = new PortNode(name);
        nv->next = vertices;
        vertices = nv;
        return nv;
    }

    void addEdge(string src, string dest, string date, string dep, string arr, int cost, string company)
    {
        PortNode *v = addVertex(src);
        addVertex(dest);

        Edge *e = new Edge(dest, date, dep, arr, cost, company);
        e->next = v->head;
        v->head = e;
    }

    void sortEdges(PortNode *v)
    {
        if (!v || !v->head)
            return;

        bool swapped = true;
        while (swapped)
        {
            swapped = false;
            Edge *curr = v->head;
            Edge *prev = nullptr;

            while (curr->next != nullptr)
            {
                if (curr->dest > curr->next->dest)
                {
                    Edge *temp = curr->next;
                    curr->next = temp->next;
                    temp->next = curr;

                    if (prev == nullptr)
                        v->head = temp;
                    else
                        prev->next = temp;

                    swapped = true;
                    prev = temp;
                }
                else
                {
                    prev = curr;
                    curr = curr->next;
                }
            }
        }
    }

    void sortVertices()
    {
        if (!vertices)
            return;

        bool swapped = true;
        while (swapped)
        {
            swapped = false;
            PortNode *curr = vertices;
            PortNode *prev = nullptr;

            while (curr->next != nullptr)
            {
                if (curr->name > curr->next->name)
                {
                    PortNode *temp = curr->next;
                    curr->next = temp->next;
                    temp->next = curr;

                    if (prev == nullptr)
                        vertices = temp;
                    else
                        prev->next = temp;

                    swapped = true;
                    prev = temp;
                }
                else
                {
                    prev = curr;
                    curr = curr->next;
                }
            }
        }
    }

    void loadLocations(string file)
    {
        ifstream in(file);
        if (!in)
            return;

        string name;
        float lat, lon;
        string flag;

        while (in >> name >> lat >> lon)
        {
            in >> ws;
            getline(in, flag);
            bool leftFlag = (flag.find("L") != string::npos);

            PortNode *v = addVertex(name);
            v->portData.lat = lat;
            v->portData.lon = lon;
            v->portData.left = leftFlag;
        }
        in.close();
    }

    void loadCharges(string file)
    {
        ifstream in(file);
        if (!in)
            return;

        string name;
        float charge;
        while (in >> name >> charge)
        {
            PortNode *v = findVertex(name);
            if (v)
                v->portData.charges = charge;
        }
        in.close();
    }

public:
    Graph(string routesFile, string locationsFile, string chargesFile)
    {
        vertices = nullptr;
        loadRoutes(routesFile);
        loadLocations(locationsFile);
        loadCharges(chargesFile);

        sortVertices();
        PortNode *v = vertices;
        while (v != nullptr)
        {
            sortEdges(v);
            v = v->next;
        }
    }

    void printGraph(bool vertices_only = false)
    {
        if (vertices_only)
        {
            PortNode *v = vertices;
            int i = 1;
            while (v != nullptr)
            {
                cout << i++ << " " << colorText(v->name, BLUE)
                     << " (Lat=" << v->portData.lat
                     << ", Lon=" << v->portData.lon
                     << ", Left=" << v->portData.left
                     << ", Charges=" << v->portData.charges << ")\n";
                v = v->next;
            }
        }
        else
        {
            PortNode *v = vertices;
            while (v != nullptr)
            {
                cout << "[" << colorText(v->name, BLUE)
                     << " (Lat=" << v->portData.lat
                     << ", Lon=" << v->portData.lon
                     << ", Left=" << v->portData.left
                     << ", Charges=" << v->portData.charges << ")] ->\n";

                Edge *e = v->head;
                while (e != nullptr)
                {
                    cout << "    "
                         << setw(25) << left << colorText(e->dest, GREEN)
                         << setw(15) << left << e->date
                         << setw(15) << left << (e->dep + "-" + e->arr)
                         << "$" << setw(10) << left << e->cost
                         << colorText(e->company, RED)
                         << "\n";
                    e = e->next;
                }
                cout << "\n";
                v = v->next;
            }
        }
    }

    void loadRoutes(string file)
    {
        ifstream in(file.c_str());
        if (!in)
            return;

        string src, dest, date, dep, arr, company;
        int cost;

        while (in >> src >> dest >> date >> dep >> arr >> cost >> company)
        {
            addEdge(src, dest, date, dep, arr, cost, company);
        }

        in.close();
    }

    PortNode *getVertices() { return vertices; }
    PortNode *findVertexByName(string name) { return findVertex(name); }
};
