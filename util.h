// Manav Gupta
// UIC Spring 2022
// CS251 Project 6 - File Compression II
//

#include <iostream>
#include <fstream>
#include <map>
#include <queue>          // std::priority_queue
#include <vector>         // std::vector
#include <functional>     // std::greater
#include <string>
#include "bitstream.h"
#include "hashmap.h"
#include "mymap.h"
#pragma once

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};


//
// *This method frees the memory allocated for the Huffman tree.
// Post order traversal to delete nodes...
void freeTree(HuffmanNode* node) {
  if (node == nullptr) {
    return;
  }
  // go left to right, bottom to up, root deleted last
  freeTree(node->zero);
  freeTree(node->one);
  delete node;
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmap &map) {
  if (isFile) {
    ifstream inFS(filename);
    char c;
    // get input from file character by character
    while (inFS.get(c)) {
      // if character not in map...
      if (map.containsKey(c) == false) {
        // put in map with 1 occurence
        map.put(c, 1);
      } else {
        // if in map...
        // increase that characters frequency by 1
        map.put(c, map.get(c) + 1);
      }
    }
  } else {
    // same as above but for string
    for (char c : filename) {
      if (map.containsKey(c) == false) {
        map.put(c, 1);
      } else {
        map.put(c, map.get(c) + 1);
      }
    }
  }
  // adding PSEUDO_EOF manually
  map.put(PSEUDO_EOF, 1);
}

// prioritize priority queue of HuffmanNodes based on their count values
class prioritize {
    public:
        bool operator()(const HuffmanNode* p1, const HuffmanNode* p2) {
          return p1->count > p2->count;
        }
};
//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmap &map) {
  vector<int> keys = map.keys();
  priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> g;
  for (auto e : keys) {
    int key = e;
    int value = map.get(e);
    HuffmanNode* newnode = new HuffmanNode();
    newnode->character = key;
    newnode->count = value;
    newnode->zero = nullptr;
    newnode->one = nullptr;
    // create node of key, value pairs
    // and add to priority queue
    // priority based on value/count
    g.push(newnode);
  }
  while (g.size() > 1) {
    HuffmanNode* nonCharNode = new HuffmanNode();
    // pointer to first node from queue
    HuffmanNode* first = g.top();
    // pop it out of queue
    g.pop();
    // pointer to second node from queue
    // technically first after popping previous first
    HuffmanNode* second = g.top();
    g.pop();
    // assign NOT_A_CHAR to node's character value
    nonCharNode->character = NOT_A_CHAR;
    // node count = sum of top 2 nodes,
    // previously, in priority queue
    nonCharNode->count = first->count + second->count;
    // node's left/zero = first (top of queue previously)
    nonCharNode->zero = first;
    // node's right/one = second (2nd in queue previously)
    nonCharNode->one = second;
    // push node back to queue,
    // where it is compared with other values
    // and is positioned accordingly
    g.push(nonCharNode);
  }
  // returns pointer to tree root
  return g.top();
}

// helper function to input string into encoding map
// recursively appends a string, which is huf-bin for that char
void stringIntoMap(string& s, HuffmanNode* node, mymap <int, string> &map) {
  // if goes past a leaf, return
  if (node == nullptr) {
    return;
  }
  // if character is not 257, therefore hitting a leaf
  // and a character that's important
  if (node->character != 257) {
    // put that string and character into map
    map.put(node->character, s);
  }
  // new str for different cases, eg. 010 and 011
  string str = s;
  // recursive calls to reach each node leaf
  // while recursively appending string
  stringIntoMap(s += "0", node->zero, map);
  stringIntoMap(str += "1", node->one, map);
}

//
// *This function builds the encoding map from an encoding tree.
//
mymap <int, string> buildEncodingMap(HuffmanNode* tree) {
  mymap <int, string> encodingMap;
  string s;
  // calls helper function to input data from tree
  // to encodingMap
  stringIntoMap(s, tree, encodingMap);
  // return the map
  return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, mymap <int, string> &encodingMap,
ofbitstream& output, int &size, bool makeFile) {
  string str;
  char ch;
  // similar char by char read from input file
  while (input.get(ch)) {
    // append huf-bin code to string
      str += encodingMap.get(ch);
  }
  // adding PSEUDO_CODE manually
  str += encodingMap.get(256);
  // size pass-by-reference gets updated
  size = str.length();
  // if makeFile...
  if (makeFile == true) {
    // read each char in str
    for (char c : str) {
      // since char is either '0' or '1'
      // only two cases needed, and...
      if (c == '0') {
        // 0 added if '0'
        output.writeBit(0);
      } else {
        // 1 added if not '0', ie: if '1'
        output.writeBit(1);
      }
    }
  }
  // returns the string of huf-bin code
  return str;
}



//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
  string str;
  // pointer to encoding tree root
  HuffmanNode* start = encodingTree;
  // while not end of file
  while (!input.eof()) {
    // read file bit by bit, as ints
    int bit = input.readBit();
    // if bit is 0 go left/node->zero
    if (bit == 0) {
      start = start->zero;
    } else {
    // if bit is 1 go right/node->one
      start = start->one;
    }
    // if PSEUDO_CODE char is met, break outh of loop
    if (start->character == 256) {
      break;
    } else if (start->character != 257 && start->character != 256) {
      // else if node character isn't PSEUDO_CODE or NOT_A_CHAR
      // ie: if node character is an actual character
      str += start->character;
      start = encodingTree;
    }
  }
  // for char in string
  for (char ch : str) {
    // output each char to outfile
    output.put(ch);
  }
  // return the decoded string
  return str;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
  hashmap map;
  // build frequency map
  buildFrequencyMap(filename, true, map);
  // node pointer to encoding tree from frequency map
  HuffmanNode* node = buildEncodingTree(map);
  // map of chars and their huf-bin code
  // from encoding tree root
  mymap <int, string> mine = buildEncodingMap(node);
  // output filename is filename + .huf at the end
  ofbitstream output(filename + ".huf");
  // put the frequency map in outfile to decompress it
  output << map;
  ifstream input(filename);
  int size;
  // encode the output file with huf-bin version of input file
  string s = encode(input, mine, output, size, true);
  // free memory (tree)
  freeTree(node);
  // return string version of huf-bin code
  return s;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
  // find ".huf" in filename
  size_t pos = filename.find(".huf");
  // if their, update filename to not contain it
  if ((int)pos >= 0) {
      filename = filename.substr(0, pos);
  }
  // find first fullstop
  pos = filename.find(".");
  // extract pos till end of filename and stor in ext
  string ext = filename.substr(pos, filename.length() - pos);
  // update filename to only have its name
  filename = filename.substr(0, pos);
  // input will have "filename.txt.huf"
  ifbitstream input(filename + ext + ".huf");
  // output will have "filename_unc.txt"
  ofstream output(filename + "_unc" + ext);
  hashmap map;
  // get map from file to decode/encoded file
  input >> map;
  // pointer to encoding tree
  HuffmanNode* node = buildEncodingTree(map);
  // decode the file and store answer in string
  string s = decode(input, node, output);
  // free memory (tree)
  freeTree(node);
  // return the decoded string
  return s;
}
