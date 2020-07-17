#include <iostream>
#include <set>
#include <fstream>
/*
using namespace std;
bool yesNo() {
    while (true) {
        string input;
        cin >> input;
        switch (stoi(input)) {
            case 'y':
                return true;
            case 'n':
                return false;
            default:
                break;
        }
    }
}

int main(int argc, char* argv[]) try {
    string path;
    if (argc > 1)
        path = argv[1];
    else {
        cout << "Hello, type or drag a text file into a window to remove duplicate lines" << endl;
        getline(cin, path);
    }
    cout << "The path is " << path << endl;
    ifstream f(path);
    if (!f) throw runtime_error("Error opening input file.");
    ofstream output(path.append("_dedup"));
    if (!output) throw runtime_error("Error creating output file.");
    cout << "Remove comments (everything after) \"#\" ? y/n" << endl;
    bool com = yesNo();
    size_t cnt = 0, comms = 0;
    set<string> set;
    cout << "Please stand by..." << endl;
    while (f) {
        string line;
        getline(f, line);
        ++cnt;
        if (com) {
            auto it = line.find('#');
            if (it != string::npos) {
                line.erase(it);
                ++comms;
                if (line.empty()) continue;
            }
        }
        set.insert(line);
        if (cnt > (set.max_size() - 1000)) throw runtime_error("Too large file.");
        if (cnt % 10000 == 0) //every 5000th line, output a message
            cout << "Processed " << cnt << " elements\n";
    }
    if (cnt < 2) throw invalid_argument("No data?");
    for (auto& el: set)
        output << el << "\n";
    cout << "Processed " << cnt
         << " elements. Removed " << cnt - set.size() << " lines"
         << (com ? "and removed " + to_string(comms) + " comments" : "") << endl;
    system("pause");
    return EXIT_SUCCESS;
}
catch (const exception& exc) {
    cerr << exc.what() << endl;
    system("pause");
    return (EXIT_FAILURE);
}
catch (...) {
    cerr << "Unknown error. Unable to continue." << endl;
    system("pause");
    return (EXIT_FAILURE);
}
 */