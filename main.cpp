#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <sstream>
#include <vector>

#define VERSION "2.1";
#define BUILD 4;

using namespace std;

mutex print_mutex;
const int size = 400;
const float threshold = 0.001;
float pow_offset;


void randomize_sp(float *sp) {
    for (int i = 0; i < size; ++i) {
        sp[i] = rand() / (float) RAND_MAX * 2 - 1;
    }
}

void randomize_mat(float *mat) {
    for (int i = 0; i < size; ++i) {
        for (int j = i + 1; j < size; ++j) {
            float v = 2 * (float) rand() / (float) RAND_MAX - 1;
            mat[i * size + j] = v;
            mat[j * size + i] = v;
        }
        mat[i * size + i] = 0;
    }
}

float hamiltonian(float *mat, float *sp) {
    float ham = 0;
    for (int i = 0; i < size; i++)
        for (int j = i + 1; j < size; j++)
            ham += sp[i] * sp[j] * mat[i * size + j];
    return ham;
}

float meanfield(float *mat, float *sp, int ind) { // Returns /Phi_ind
    float mf = 0;
    for (int i = 0; i < size; ++i)
        mf += mat[ind * size + i] * sp[i];
    return mf;
}

float iprob(float *x, float *y, int ind) { // Returns P_ind
    return (1 + x[ind] * y[ind]) / 2.;
}

void corr_float(float *mantissa, int *exponent) {
    if (*mantissa == 0) {
        *exponent = 0;
        return;
    }
    while (abs(*mantissa) > 10) {
        *mantissa /= 10;
        *exponent += 1;
    }
    while (abs(*mantissa) < 1) {
        *mantissa *= 10;
        *exponent -= 1;
    }
}

float prob(float *x, float *y, int *expon) { // Returns P
    float pr = 1;
    *expon = 0;
    for (int i = 0; i < size; ++i) {
        pr *= iprob(x, y, i);
        corr_float(&pr, expon);
    }
    return pr == 0 ? 0 : pr * exp10f((*expon + pow_offset));
}

float diprob(float *x, float *y, int ind, int *expon) { // Returns dP / dx_ind
    if (1 + x[ind] * y[ind] == 0)
        return 0;
    return prob(x, y, expon) * y[ind] / (1 + x[ind] * y[ind]);
}

bool iterate(float *mat, float *sp_block, int run_active, vector<int> run_linked, int spin_index, float t, int *expon) {
    float sf = 0;
    sf += meanfield(mat, &(sp_block[run_active * size]), spin_index);
    for (int interaction : run_linked)
        sf -= diprob(&(sp_block[run_active * size]), &(sp_block[interaction * size]), spin_index, expon);

    float old = sp_block[run_active * size + spin_index];
    sp_block[run_active * size + spin_index] = t > 0 ? tanhf(-sf / t) :
                                               sf > 0 ? -1 : 1;
    return (sp_block[run_active * size + spin_index] - old > 0)
           ? (sp_block[run_active * size + spin_index] - old > threshold)
           : (old - sp_block[run_active * size + spin_index] > threshold);
}

void anneal(float *mat, float *sp_block, int run_count, float temp, float step,
            bool *thr_inactive, int *expon) {
    int counter = 0;
    float t = temp;
    do {
        t -= step;
        bool cont = true;
        while (cont) {
            cont = false;
            for (int i = 0; i < run_count; ++i) {
                sp_block[i * size] = 1;
            }
            for (int spin_index = 1; spin_index < size; ++spin_index) {
                for (int run_index = 0; run_index < run_count; ++run_index) {
                    if (iterate(mat, sp_block, run_index, run_index == 0 ? vector<int>{} : vector<int>{0},
                                spin_index, t, expon))
                        cont = true;
                }
            }
            counter++;
        }
    } while (t > 0);
    print_mutex.lock();
    cout << temp;
    for (int run_index = 0; run_index < run_count; ++run_index)
        cout << " " << hamiltonian(mat, &(sp_block[run_index * size]));
    cout << " [" << counter << " iterations]"
         << endl;
    print_mutex.unlock();
    *thr_inactive = true;
}

string getTimeString(double time) {
    if (time <= 0)
        return "0 h 0 m 0 s";
    ostringstream oss;
    int d = (int) (time / (24 * 3600));
    int h = (int) ((time - d * 24 * 3600) / 3600);
    int m = (int) ((time - d * 24 * 3600 - h * 3600) / 60);
    int s = (int) (time - d * 24 * 3600 - h * 3600 - m * 60);
    if (d != 0) {
        oss << d << " d ";
    }
    oss << h << " h " << m << " m " << s << " s";
    return oss.str();
}

int main() {
    cout << "MARS analysis by Yxbcvn410, CPU edition, version " << VERSION;
    cout << ", build " << BUILD;
    cout << endl;
    float temp = 10;
    float temp_f = 10;
    cout << "Lambda decimal log?" << endl;
    cin >> pow_offset;
    cout << "Start temp?" << endl;
    cin >> temp;
    cout << "End temp?" << endl;
    cin >> temp_f;
    float step = 0.01;
    cout << "Step?" << endl;
    cin >> step;
    cout << "Thread quantity?" << endl;
    int threads;
    cin >> threads;
    cout << "Group size?" << endl;
    int gsize;
    cin >> gsize;
    cout << "Group quantity?" << endl;
    int quan;
    cin >> quan;
    auto *spinset_blocks = new float[size * gsize * threads];
    auto *J = new float[size * size];
    int *expon = new int[threads];
    bool *f = new bool[threads];
    for (int i = 0; i < threads; ++i)
        f[i] = true;
    srand(10);
    randomize_mat(J);
    int launched = 0;
    double start_time = time(0);
    while (launched < quan)
        for (int i = 0; i < threads; i++)
            if (f[i] && launched < quan) {
                f[i] = false;
                for (int j = 0; j < gsize; ++j)
                    randomize_sp(&(spinset_blocks[size * gsize * i + size * j]));

                thread(anneal, J, &(spinset_blocks[size * gsize * i]), gsize,
                       temp + (launched / (float) quan) * (temp_f - temp),
                       step, &(f[i]), &(expon[i])).detach();
                launched++;
            }

    bool flag_wait = true;
    while (flag_wait) {
        flag_wait = false;
        for (int i = 0; i < threads; i++)
            flag_wait = (flag_wait || !f[i]);
    }
    cout << "Calculations complete in " << getTimeString(time(0) - start_time) << endl;
}
