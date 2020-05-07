#include <iostream>
#include <set>
#include <fstream>

int wmain(int argc, wchar_t *argv[])
{
    std::wstring path;
    if (argc > 1)
        path = argv[1];
    else
    {
        std::wcout << "Hello, type or drag a text file into a window to remove duplicate lines" << std::endl;
        std::getline(std::wcin, path);
    }
    try
    {
        std::wcout << "The path is " << path << std::endl;
        std::wifstream f(path.c_str());
        if (!f) throw std::runtime_error("Error opening input file.");
        size_t cnt = 0;
        std::wofstream output(path.append(L"_dedup").c_str());
        if (!output) throw std::runtime_error("Error creating output file.");
        std::set<std::wstring> set;
        std::cout << "Please stand by..." << std::endl;
        while (f)
        {
            std::wstring line;
            std::getline(f, line);
            set.insert(line);
            ++cnt;
            if (cnt > (set.max_size() - 1000)) throw std::runtime_error("Too large file.");
            if (cnt % 10000 == 0) //every 5000th line, output a message
                std::cout << "Processed " << cnt << " elements" << std::endl;
        }
        if (cnt < 2) throw std::invalid_argument("No data?");
        std::cout << "Saving a file..." << std::endl;
        for (auto& el: set)
            output << el << "\n";
        std::cout << "Processed " << cnt
                  << " elements. Removed " << cnt - set.size()  << " lines." << std::endl;
        system("pause");
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exc)
    {
        std::cerr << exc.what() << std::endl;
        system("pause");
        return (EXIT_FAILURE);
    }
    catch (...)
    {
        std::cerr << "Unknown error. Unable to continue." << std::endl;
        system("pause");
        return(EXIT_FAILURE);
    }
}
