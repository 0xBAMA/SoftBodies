#include <iostream>
#include <thread>
#include <vector>

using std::cout;
using std::endl;

// minimal example of using threads in the style of the multithreaded softbody update
// compile and run with: g++ -Wall -std=c++17 -pthread threadtest.cc && ./a.out

void set(std::vector<int> &intvec, int offset, int stride)
{
    for(unsigned int i = 0; offset+i*stride < intvec.size(); i++)
        intvec[offset+i*stride] = 1;
}

int main(int argc, char *argv[])
{
    /* cout << "test" << endl; */ 
    
    std::vector<int> intvec;
    intvec.resize(100);
    

    cout << "Original List: ";
    for(unsigned int i = 0; i < intvec.size(); i++)
        cout << intvec[i] << " ";

    cout << endl;

    std::thread t1(set, std::ref(intvec), 0, 8);
    std::thread t2(set, std::ref(intvec), 1, 8);
    std::thread t3(set, std::ref(intvec), 2, 8);
    std::thread t4(set, std::ref(intvec), 3, 8);
    std::thread t5(set, std::ref(intvec), 4, 8);
    std::thread t6(set, std::ref(intvec), 5, 8);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    cout << "Changed List:  ";
    for(unsigned int i = 0; i < intvec.size(); i++)
        cout << intvec[i] << " ";

    cout << endl;

    return 0;
}
