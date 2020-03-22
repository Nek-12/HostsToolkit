#include <iostream>
#include <set>
#include <fstream>
using ull = unsigned long long;

int main(int argc, char *argv[])
{
    std::string path;

    if (argc > 1)
        path = argv[1];
    else
    {
        path = "";
        std::cout << "Hello, type or drag a text file into a window to remove duplicate lines" << std::endl;
        std::getline(std::cin, path);
    }
    try
    {
        std::cout << "The path is " << path << std::endl;
        std::cout << "Please stand by" << std::endl;

        std::ifstream input(path);
        if (!input) throw std::runtime_error("Error opening input file.");

        ull cnt = 0;
        std::ofstream output(path + "_dedup");
        if (!output) throw std::runtime_error("Error creating output file.");

        std::set<std::string> set;
        while (input)
        {
            std::string line;
            std::getline(input, line);
            set.insert(line);
            ++cnt;
            if (cnt > (set.max_size() - 1000)) throw std::runtime_error("Too large file.");
            if (cnt % 5000 == 0)
                std::cout << "Processed " << cnt << " elements" << std::endl;
        }
        if (cnt < 2) throw std::invalid_argument("No data?");
        std::cout << "Saving a file..." << std::endl;
        for (auto& el: set)
            output << el << "\n";
        std::cout << "Processed " << cnt
        << " elements. Removed " << cnt - set.size()  << " lines." << std::endl;
        system("pause");
        return 0;
    }
    catch (const std::exception& exc)
    {
        std::cerr << exc.what() << std::endl;
        system("pause");
        return (-1);
    }
    catch (...)
    {
        std::cerr << "Unknown error. Restarting." << std::endl;
        system("pause");
        return(-1);
    }
}
