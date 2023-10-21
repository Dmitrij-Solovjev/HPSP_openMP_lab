#include <omp.h>
#include <iostream>
#include <random>
#include <fstream>
#define VECTOR_LEN 100

using namespace std;

class Vect {
public:
    int LENGTH;
    bool finished = false;
    uint8_t *VECT = nullptr;


    Vect(int length): LENGTH(length) {
        VECT = new uint8_t[LENGTH];
    }
    
    void Read(ifstream &file){
        for (int i=0; i<100; i++) {
            string s;
            getline(file, s);
            VECT[i] = atoi(s.c_str());
        }
    }
    void Show(){
        for (int i=0; i<100; i++) {
            cout << int(VECT[i]) << " ";
        }
        cout << endl;
    }
    
    ~Vect() {
        delete VECT;
    }
};
int operator* (Vect &x, Vect& y) {
    int sum=0;
    for(int i=0; i< x.LENGTH; i++) {
        sum += x.VECT[i]*y.VECT[i];
    }
    return sum;
}



int main(){
    // https://www.openmp.org/wp-content/uploads/openmp-examples-4.0.2.pdf
    // стр. 13

    /*
    random_device rd;
    ofstream myfile;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 255);
    myfile.open ("example.txt");
    for (int i=0; i<1000; ++i) {
       myfile << dist(gen) << endl;
    }
    myfile.close();
    */

    ifstream myfile("example.txt", std::ifstream::binary);
    myfile.seekg(0, myfile.end);
    int length = myfile.tellg();
    myfile.seekg(0, myfile.beg);
    
    
    vector <Vect *> vect_arr(0);    
    
    uint16_t index = 0;
    #pragma omp parallel shared(vect_arr, index)
    {
        #pragma omp sections nowait
        {
            #pragma omp section
                for (int i=0; i<10; ++i){
                    vect_arr.push_back(new Vect(100));
                    vect_arr.back()->Read(myfile);
                    cout << "readok"<<endl;
                    if (i%2 == 1) index+=2;
                    #pragma omp flush(index)
                }

            #pragma omp section
                for (int i=0; i<10; i+=2){
                    while (index < i+2) {
                        #pragma omp flush(index)
                        //cout << index << endl;
                    }
                    cout << (*(vect_arr[i])) * (*(vect_arr[i+1])) << endl;

                }
                    
        } // end of sections
    } // end of parallel section
    return 0;
}
