#include <unistd.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <queue>

using namespace std;

struct Node {
    int offset;
    int length;
    string name;
    char* data;

    vector<Node*> children;
    Node* parent = nullptr;

    Node(int off, int len, string n) {
        offset = off;
        length = len;
        name = n;
    }
};

class Wad {
    Node* root = nullptr;
    char magic[5];
    int numDesc;
    int offset;

public:
    Wad(const string &path);
    ~Wad();
    static Wad* loadWad(const string &path);
    string getMagic();
    bool isContent(const string &path);
    bool isDirectory(const string &path);
    int getSize(const string &path);
    int getContents(const string &path, char *buffer, int length, int offset = 0);
    int getDirectory(const string &path, vector<string> *directory);
    Node* getNode(const string &path);
    Node* levelOrderSearch(string target);
    void deleteTree(Node* node);
    Node* getRoot();
};