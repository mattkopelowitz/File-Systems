#include "Wad.h"

Wad::Wad(const string &path) {
    char buffer[4];

    //open the binary file
    int fd = open(path.c_str(), O_RDONLY);

    //read the first 4 bytes into magic
    read(fd, magic, 4);
    magic[4] = '\0';

    //read the next 4 bytes into numDesc
    read(fd, buffer, 4);
    numDesc = *(int*)buffer;

    //read the next 4 bytes into offset and move the fd to start at the offset
    read(fd, buffer, 4);
    offset = *(int*)buffer;
    lseek(fd, offset, SEEK_SET);

    root = new Node(0, 0, "/");

    //pointer to the current directory/parent
    Node* pwd = &(*root);
    
    int count = -1;
    int fileOffset = 0;

    //parse starting from offset
    for (int i = 0; i < numDesc; i++) {
        int nOffset;
        int nLength;
        char cnName[8];
        
        read(fd, &nOffset, 4);
        read(fd, &nLength, 4);
        read(fd, &cnName, 8);

        fileOffset += 16;

        string nName(cnName);

        //determine what type of node it is
        if (nLength == 0 && nName.find("START") != string::npos) { //namespace beginning
            nName = nName.substr(0, nName.find("_START"));
            Node* node = new Node(nOffset, nLength, nName);
            node->parent = pwd;  
            pwd->children.push_back(node);
            pwd = node;

        } else if (nName.at(0) == 'E' && nName.at(2) == 'M') { //map marker
            count = 10;
            Node* n = new Node(nOffset, nLength, nName);
            pwd->children.push_back(n);
            n->parent = pwd;
            pwd = n;

        } else if (nLength == 0 && nName.find("END") != string::npos) { //namespace end
            pwd = pwd->parent;

        } else if (count <= 10 && count > 1) { //inside map marker
            char* data = new char[nLength];
            
            lseek(fd, nOffset, SEEK_SET);
            read(fd, data, nLength);

            Node* node = new Node(nOffset, nLength, nName);
            node->data = data;
            node->parent = pwd;
            pwd->children.push_back(node);

            lseek(fd, offset + fileOffset, SEEK_SET);
            count--;


        } else if (count == 1) { //last element in map marker
            char* data = new char[nLength];

            lseek(fd, nOffset, SEEK_SET);
            read(fd, data, nLength);

            Node* node = new Node(nOffset, nLength, nName);
            node->data = data;
            node->parent = pwd;
            pwd->children.push_back(node);

            lseek(fd, offset + fileOffset, SEEK_SET);
            pwd = pwd->parent;
            count = -1;

        } else { //files
            char* data = new char[nLength];
            
            lseek(fd, nOffset, SEEK_SET);
            read(fd, data, nLength);

            Node* node = new Node(nOffset, nLength, nName);
            node->data = data;
            node->parent = pwd;
            pwd->children.push_back(node);

            lseek(fd, offset + fileOffset, SEEK_SET);
        }
    }

    close(fd);
}

Wad* Wad::loadWad(const string &path) {
    Wad* wad = new Wad(path);
    return wad;
}

string Wad::getMagic() {
    return (string)magic;
}

bool Wad::isContent(const string &path){
    if (getNode(path) != nullptr && getNode(path)->children.empty()) {
        return true;
    }
    return false;
}

bool Wad::isDirectory(const string &path) {
    if (getNode(path) != nullptr && getNode(path)->length == 0) {
        return true;
    }
    return false;
}

int Wad::getSize(const string &path) {
    if (isContent(path)) {
        return getNode(path)->length;
    }
    return -1;
}

int Wad::getContents(const string &path, char *buffer, int length, int offset) {
    Node* node = getNode(path);
    if (isContent(path)) {
        if (node->length < offset + length) {
            memcpy(buffer, node->data + offset, node->length - offset);
            return node->length - offset;
        }
        memcpy(buffer, node->data + offset, length);
        return length;
    }
    return -1;
}

int Wad::getDirectory(const string &path, vector<string> *directory) {
    if (isDirectory(path)) {
        Node* node = getNode(path);
        for (int i = 0; i < node->children.size(); i++) {
            directory->push_back(node->children[i]->name);
        }
        return directory->size();
    }
    return -1;
}

Node* Wad::getNode(const string &path) {
    Node* curr = root;
    vector<string> components;
    int start = 1;
    int end = path.find('/');

    //separate path into components
    if (path == "/") {
        return curr;
    } else {
        while (end != string::npos) {
            components.push_back(path.substr(start, end-start));
            start = end + 1;
            end = path.find('/', start);
        }

        if (start != path.length()) {
            components.push_back(path.substr(start));
        }

    }

    //search each directory to find the node for the given path
    bool found = false;
    for (string c : components) {
        for (int i = 0; i < curr->children.size(); i++) {
            if (c == curr->children[i]->name) {
                curr = curr->children[i];
                found = true;
                break;
            }
        }
    }
    
    if (found) {
        return curr;
    }
    return nullptr;
}

Wad::~Wad() {
    deleteTree(root);
}

void Wad::deleteTree(Node* node) {
    if (node == nullptr) {
        return;
    }
    
    for (int i = 0; i < node->children.size(); i++) {
        deleteTree(node->children[i]);
    }
    delete[] node->data;
    delete node;
}

Node* Wad::getRoot() {
    return root;
}






// Node* Wad::getNode(const string& path) {
//     if (path == "/") {
//         return root;
//     }

//     string p = path.substr(1, path.size());

//     if (p.find("/")) {
//         p = p.substr(0, p.find("/"));
//     }

//     Node* node = levelOrderSearch(p);

//     return node; 
// }

// Node* Wad::levelOrderSearch(string target) {
//     if (root == nullptr) {
//         return nullptr;
//     }

//     queue<Node*> q;
//     q.push(root);

//     while (!q.empty()) {
//         Node* node = q.front();
//         q.pop();

//         if (node->name == target) {
//             return node;
//         }

//         for (Node* child : node->children) {
//             q.push(child);
//         }
//     }
//     return nullptr;
// }