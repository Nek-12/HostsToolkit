
#include <iostream>
#include <fstream>
#include <forward_list>

using ull = unsigned long long;

int bgproc(std::string& path)
{
    std::ifstream input(path);
    if (!input.good() ) throw std::exception();

    std::forward_list<std::string> list;
    std::forward_list<std::string> lresult;

    ull cnt = 0;
    std::ofstream output (path + "_dedup" );
    if (!output.good() ) throw std::exception();

    while (!input.eof())
    {
        std::string line;
        std::getline(input,line);
        list.push_front(line);
        ++cnt;
        if ( cnt > (list.max_size() - 1000 ) ) throw "Too large file.";
    }

    if (cnt < 2 ) throw std::invalid_argument("No data?");

    //list.reverse();
    ull total = cnt;
    ull i = 0;
    ull removed = 0;
    while (!list.empty() )
    {
        ++i;
        std::string check = *list.begin();
        lresult.push_front(*list.begin());
        list.pop_front();

        auto prev = list.before_begin();
        auto curr = list.begin();
        while ( curr != list.end() )
        {
            if (check == *curr)
            {
                curr = list.erase_after(prev);
                --cnt;
                ++removed;
            }
            else
            {
                prev = curr;
                ++curr;
            }
        }
        if (i%500 == 0)
            std::cout << "Processed " << i << " of " << cnt << " elements" << std::endl;

#ifdef DEBUG
            //system("CLS");
            std::cout << *list.begin() << std::endl;
#endif

    }
    std::cout << "Total " << total << " elements processed. " << std::endl
              << "Removed " << removed << " duplicates." << std::endl;
    std::cout << "Saving a file..." << std::endl;
    for (auto& el: lresult)
        output << el << "\n";
    std::cout << "Done." << std::endl;

    input.close();
    output.close();
    return 0;
}
