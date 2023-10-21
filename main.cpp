
#include <omp.h>
#include <iostream>
#include <random>
#include <stdint.h>
#include <chrono>
#define LENGTH 100000000

using namespace std;

double func(double x) {
    return x*x;
}

random_device rd;   // non-deterministic generator
mt19937 gen(rd());  // to seed mersenne twister.
uniform_int_distribution<> dist(0, 255); // distribute results between 1 and 6 inclusive.

uint8_t A[LENGTH] = {};
uint8_t B[LENGTH] = {};
uint8_t C[LENGTH] = {};

void task1(uint8_t thread_num = 1){
    /***********************min/max********************************/
    uint32_t my_min = UINT8_MAX;

    std::chrono::time_point<std::chrono::system_clock> m_StartTime = std::chrono::system_clock::now(), m_EndTime;
    #pragma omp parallel for shared(A) reduction(min:my_min) num_threads(thread_num) schedule(guided, 100000) //schedule(dynamic, 10000000) //schedule(guided, 1) 

    for (auto i=0; i<LENGTH; ++i) {
        if (A[i] < my_min){
            my_min = A[i];
        }
    }
    m_EndTime = std::chrono::system_clock::now();

    cout << my_min << " " << std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count() << endl;
}


void task2(uint8_t thread_num=1){
    /*********************scalar multiply**************************/
    std::chrono::time_point<std::chrono::system_clock> m_StartTime = std::chrono::system_clock::now(), m_EndTime;
    
    uint64_t sum=0;
    #pragma omp parallel shared(A, B, C, sum)  num_threads(thread_num) // reduction(+:sum)
    {                          //  guided 100000000
        #pragma omp for schedule(dynamic, 100000)
        for (auto i=0; i<LENGTH; ++i) {
            sum += A[i] * B[i];
        //    C[i] = A[i] * B[i];
        }
    
//        #pragma omp for reduction(+:sum) //schedule(dynamic, 1000000)
//        for (uint64_t i=0; i<LENGTH; ++i) {
//            sum+=C[i];
//        }

    }


    
    m_EndTime = std::chrono::system_clock::now();

    cout << std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count();
    //cout << "  " << sum <<endl;
}

void task3(int thread_num) {
    /*******************Numerical integration**********************/
    const uint64_t N = LENGTH; //=10000000000;
    const double a = 0, b = 1;
    const double h = (b-a)/N;
    double sum=0;
    std::chrono::time_point<std::chrono::system_clock> m_StartTime = std::chrono::system_clock::now(), m_EndTime;

    #pragma omp parallel for shared(N, a, h) reduction(+:sum) num_threads(thread_num) schedule(dynamic, 100000) //schedule(guided, 1000000)

    for(uint64_t i=0; i<N; ++i) {
        double xi=a+h*i;
        sum+=func(xi);
    }
    sum*=h;
    m_EndTime = std::chrono::system_clock::now();

    cout << std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count();
    //cout << sum << endl;
}

#undef LENGTH

#define LENGTH 5000

void task4(int thread_num) {
    //                             *min-max*                           //
    auto AA = new uint8_t[LENGTH][LENGTH];

    for (int i=0; i<LENGTH; ++i) {
        for (int j=0; j<LENGTH; ++j) {
            AA[i][j] = i + j;
        }
    }
    
    uint32_t min_max = 0;

    std::chrono::time_point<std::chrono::system_clock> m_StartTime = std::chrono::system_clock::now(), m_EndTime;

    #pragma omp parallel for shared(AA) reduction(max:min_max) num_threads(thread_num) // schedule(guided, 100)
    
    for (int i=0; i<LENGTH; ++i) {
        uint32_t my_min = UINT32_MAX;
        for (int j=0; j<LENGTH; ++j) {
            if (AA[i][j] < my_min){
                my_min = AA[i][j];
            }
        }
        if (min_max < my_min){
            min_max = my_min;
        }
    }

    m_EndTime = std::chrono::system_clock::now();

    cout << std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count();

    delete AA;
}

void task5(const uint16_t chunk_size = 240, const uint8_t thread_number = 15){
    //                      *triangle-max*                         //
    
    uint8_t min_max;
    auto AA = new uint8_t[LENGTH][LENGTH];

    for (int i=0; i<LENGTH; ++i) {
        for (int j=0; j<LENGTH; ++j) {
            AA[i][j] = i + j;
        }
    }

    std::chrono::time_point<std::chrono::system_clock> m_StartTime = std::chrono::system_clock::now(), m_EndTime;

    //for (uint16_t ulala=0; ulala<5000; ulala++)
    {
        min_max = 0;
        #pragma omp parallel for shared(AA) reduction(max:min_max) num_threads(thread_number) schedule(static, chunk_size)//schedule(dynamic, chunk_size) // default(private) 
        for (int i=0; i<LENGTH; ++i) {
            uint8_t my_min = UINT8_MAX;
            
            //#pragma omp parallel for shared(AA, i, my_min) schedule(dynamic, 1) num_threads(2)
            for (int j=i; j<LENGTH; ++j) {
                if (AA[i][j] < my_min) {
                    my_min = AA[i][j];
                }
            }

            if (min_max < my_min){
                min_max = my_min;
            }
        }
    }

    m_EndTime = std::chrono::system_clock::now();

    //cout << "  " << uint16_t(min_max) << "  "<< std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count() << endl;
    cout << std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count();
    delete AA;
}

void task6(int thread_num=15){
    
    auto AA = new uint8_t[LENGTH][LENGTH];

    for (int i=0; i<LENGTH; ++i){
        for (int j=0; j<LENGTH; ++j) {
            AA[i][j] = i+j; //1 //dist(gen);
        }
    }

    omp_lock_t lock;
    omp_init_lock(&lock);
    uint64_t SUM=0;
    std::chrono::time_point<std::chrono::system_clock> m_StartTime = std::chrono::system_clock::now(), m_EndTime;

    //for (uint16_t ulala=0; ulala<1; ulala++)
    {
        SUM=0;
        #pragma omp parallel for shared(AA) schedule(guided, 240) num_threads(thread_num) reduction(+:SUM)
        for (int i=0; i<LENGTH; ++i) {
            for (int j=0; j<LENGTH; ++j) {
                //#pragma omp atomic
                //#pragma omp critical
                SUM += AA[i][j];
                
                //Удивительно, но факт: работает только с -Ox где x>0
                //часть записей не выполняет
                //omp_set_lock(&lock);
                //SUM += AA[i][j];
                //omp_unset_lock(&lock);
            }
        }
    }

    m_EndTime = std::chrono::system_clock::now();
    cout << std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count();
    omp_destroy_lock (&lock);

    delete AA;
}


int main() {
    for (int i=0; i<LENGTH; ++i){
        A[i]=dist(gen);
        B[i]=dist(gen);
    }

    cout << omp_get_max_threads() <<endl;

    
    for (int i = 1; i<=18; i++){
        task2(i);
        cout << ", ";
    }
    // 226751, 113489, 76021, 56735, 45965, 39324, 34680, 30595, 30757, 30684, 31506, 33098, 31916, 31471, 31462, 31848, 39624, 38240, 107926
    // 12, 174, 111, 147, 111, 167, 124, 165, 381, 181, 147, 106, 115, 134, 138, 4412, 8155, 16146, 37984

    task2(32);


    //task2();
    //task3();
    //task4();

    //cout << 1 << " ";
    //task5();
    //for (int i = 2; i<=31250; i*=5){
    //    cout << i << " ";
    //    task5(i, 15);
    //}

    //task6();
}

/** №5
 * Матрица 5000x5000 верхнетреугольная
 * 
 * static thread number 16
 * chunk        time
 * 1            643708
 * 2            693850
 * 10           831951
 * 50           789384
 * 250          739394
 * 1250         1822448
 * 6250         5555513
 * 31250        5771466
 * 
 * 
 * dynamic thread number 16
 * chunk        time
 * 1            467052
 * 2            457531
 * 10           404752
 * 50           429625
 * 250          524537
 * 1250         1688234
 * 6250         4665428
 * 31250        5033924
 * 
 * 
 * guided thread number 16
 * chunk        time
 * 1            452123
 * 2            438033
 * 10           407178
 * 50           384513
 * 250          332619
 * 1250         1702167
 * 6250         4630411
 * 31250        4694093
 * 
 * 
 * guided chunk 250 
 * thread number    time
 * 1                3815261
 * 2                1958921
 * 3                1249759
 * 4                947278
 * 5                771785
 * 6                655577
 * 7                587205
 * 8                532971
 * 9                513935
 * 10               422458
 * 11               370889
 * 12               340186
 * 13               322646
 * 14               327211
 * 15               309297
 * 16               321575
 * 17               516487
 * 18               725633
 * 
 * 
 * */

/** №6 10 repeat
 * 1594121020
 * reduction
 * 10581
 * 11312
 * 9789
 * 
 * atomic
 * 1338659
 * 
 * lock
 * 12021902
 * 
 * critical 
 * 10763483
*/

/**
 * Вложенный параллелизм:
 * guided chunk 10 thread 7*2
 * 4805958
 * vs 
 * без
 * guided chunk 240 thread 15
 * 309714
*/



/*************тестирование задачи с равной нагрузкой****************/

/** guided thread 15
 * chunk        time
 * 1            1672007
 * 2            1566995
 * 10           1560740
 * 50           1589816
 * 250          1576753
 * 1250         1914489
 * 6250         5718762
 * 31250        5712262
 * 
 * 
 * static thread 15
 * chunk        time
 * 1            1658263
 * 2            1576829
 * 10           1696350
 * 50           1593564
 * 250          1617713
 * 1250         1825921
 * 6250         5772659
 * 31250        5784947
 * 
 * 
 * dynamic thread 15
 * chunk        time
 * 1            1621774
 * 2            1594301
 * 10           1633300
 * 50           1633300
 * 250          1631086
 * 1250         1980129
 * 6250         5716976
 * 31250        5668896
 * 
 * 
*/
