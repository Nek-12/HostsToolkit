#include <iostream>
#include <vector>
int bgproc(std::string& );

int main(int argc, char *argv[])
{
    int first = true;
    std::string path;
beginning:

if (argc == 2 && first == 1) { path = argv[1]; first = false; }
else
{
    path = "";
    std::cout << "Hello, type or drag a text file into a window to remove duplicate lines" << std::endl;
    std::getline(std::cin,path);
}
    try
{
    std::cout << "The path is " << path << std::endl;
    std::cout << "Please stand by" << std::endl;
    bgproc(path);
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    system("pause");
    return 0;
}
catch (const char* msg)
    {
    std::cerr << msg << std::endl;
    system("pause");
    goto beginning;
    }
catch (const std::exception &exc)
{
    std::cerr << "Wrong file path or name. Try again. " << std::endl;
    std::cerr << exc.what() << std::endl ;
    system("pause");
    goto beginning;
}
catch (...)
{
    std::cerr << "Unknown error. Restarting." << std::endl;
    system("pause");
    goto beginning;
}

}
