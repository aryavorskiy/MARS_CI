#include <iostream>
#include <math.h>
#include <thread>
#include <mutex>
#include <sstream>

#define VERSION "2.0.2";
#define BUILD 2;

using namespace std;

mutex outp_mutex;
const int size = 400;
const float threshold = 0.001;
float pow_offset;


void randsp(float *sp) {
    for (int i = 0; i < size; ++i) {
        sp[i] = rand() / (float) RAND_MAX * 2 - 1;
    }
}

void randmat(float *mat) {
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

void corr_float(float *mant, int *expo) {
    if (*mant == 0) {
        *expo = 0;
        return;
    }
    while (abs(*mant) > 10) {
        *mant /= 10;
        *expo += 1;
    }
    while (abs(*mant) < 1) {
        *mant *= 10;
        *expo -= 1;
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

bool iterate(float *mat, float *x, float *y, int ind, float t, int *expon) {
    float sf = 0;
    sf += meanfield(mat, x, ind);
    sf -= diprob(x, y, ind, expon);
    float old = x[ind];
    if (t > 0)
        x[ind] = tanhf(-sf / t);
    else
        x[ind] = sf > 0 ? -1 : 1;
    return (x[ind] - old > 0) ? (x[ind] - old > threshold) : (old - x[ind] > threshold);
}

void anneal(float *mat, float *x, float *y, float temp, float step,
            bool *f, int *expon) {
    int counter = 0;
    float t = temp;
    do {
        t -= step;
        bool cont = true;
        while (cont) {
            cont = false;
            *x = 1;
            *y = 1;
            for (int i = 1; i < size - 1; i++) {
                if (iterate(mat, x, y, i, t, expon))
                    cont = true;
                if (iterate(mat, y, x, i, t, expon))
                    cont = true;
            }
            counter++;
        }
    } while (t > 0);
    outp_mutex.lock();
    cout << temp << " " << hamiltonian(mat, x) << " " << hamiltonian(mat, y) << " [" << counter << " iterations]"
         << endl;
    outp_mutex.unlock();
    *f = true;
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
    cout << "Pair quantity?" << endl;
    int quan;
    cin >> quan;
    float *x = new float[size * threads];
    float *y = new float[size * threads];
    float *J = new float[size * size];
    int *expon = new int[threads];
    bool *f = new bool[threads];
    for (int i = 0; i < threads; ++i)
        f[i] = true;
    srand(10);
    randmat(J);
    int launched = 0;
    double start_time = time(0);
    while (launched < quan)
        for (int i = 0; i < threads; i++)
            if (f[i] && launched < quan) {
                f[i] = false;
                randsp(&(x[size * i]));
                randsp(&(y[size * i]));
                thread(anneal, J, &(x[size * i]), &(y[size * i]), temp + (launched / (float) quan) * (temp_f - temp),
                       step,
                       &(f[i]), &(expon[i])).detach();
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
