#include "formant.h"
#include <Eigen/Dense>

using namespace Analysis::Formant;
using Analysis::FormantResult;

using namespace Eigen;

struct Analysis::Formant::KarmaState
{
    int numF;
    VectorXd y;
    MatrixXd F;
    MatrixXd Q;
    MatrixXd R;
    VectorXd m_up;
    VectorXd P_up;
};

constexpr int ncep = 15;
constexpr int numF = 3;

Karma::Karma()
    : state(new KarmaState)
{
    state->F.setIdentity(2 * numF, 2 * numF);

    state->Q.setZero(2 * numF, 2 * numF);
    state->Q.diagonal().head(numF).setConstant(320 * 320);
    state->Q.diagonal().tail(numF).setConstant(100 * 100);
    
    state->R.setZero(ncep, ncep);
    for (int i = 0; i < ncep; ++i) {
        state->R(i, i) = 1.0 / (double) (i + 1);
    }

    VectorXd x0(2 * numF);
    x0 << 500, 1500, 2500,
           80,  120,  160;

    state->m_up = x0;
    state->P_up = state->Q;
}

Karma::~Karma()
{
    delete state;
}

static VectorXd calcCepstrumCoefs(const double *lpc, int lpcOrder, int ncep);
static MatrixXd calcMatrixH(const VectorXd& m, int cepOrder, double Fs);
static VectorXd calcCepstrumMapping(const VectorXd& m, int cepOrder, double Fs);

FormantResult Karma::solve(const double *lpc, int lpcOrder, double sampleRate)
{
    auto& F  = state->F;
    auto  Ft = state->F.transpose();
    auto& Q  = state->Q;
    auto& R  = state->R;

    auto m_pred = F * state->m_up;
    auto P_pred = F * state->P_up * Ft + Q;

    auto H  = calcMatrixH(m_pred, ncep, sampleRate);
    auto Ht = H.transpose();
   
    auto S = H * P_pred * Ht + R;
    auto K = (P_pred * Ht) * S.colPivHouseholderQr().inverse();

    auto y_pred = calcCepstrumMapping(m_pred, ncep, sampleRate);
    auto y      = calcCepstrumCoefs(lpc, lpcOrder, ncep);

    state->m_up = m_pred + K * (y - y_pred);
    state->P_up = P_pred - K * H * P_pred;

    std::vector<FormantData> formants(numF);
    for (int i = 0; i < numF; ++i) {
        formants[i] = {
            .frequency = state->m_up(i),
            .bandwidth = state->m_up(numF + i),
        };
    }

    return { .formants = formants };
}

VectorXd calcCepstrumCoefs(const double *lpc, int lpcOrder, int ncep)
{
    VectorXd C(ncep);

    for (int n = 1; n <= ncep; ++n) {
        if (n == 1) {
            C(n - 1) = lpc[n - 1];
        }
        else if (n <= lpcOrder) {
            C(n - 1) = lpc[n - 1];
            for (int i = 1; i <= ncep - 1; ++i) {
                C(n - 1) += (double) i / (double) n * lpc[n - i - 1] * C(i - 1);
            }
        }
        else {
            C(n - 1) = 0.0;
            for (int i = ncep - lpcOrder; i <= ncep - 1; ++i) {
                C(n - 1) += (double) i / (double) n * lpc[n - i - 1] * C(i - 1);
            }
        }
    }

    return C;
}

MatrixXd calcMatrixH(const VectorXd &m, int cepOrder, double Fs)
{
    const int numF = m.size() / 2;
    auto freq = m.segment(0, numF);
    auto band = m.segment(numF, numF);

    MatrixXd H(cepOrder, 2 * numF);

    for (int i = 0; i < cepOrder; ++i) {
        for (int j = 0; j < numF; ++j) {
            const double bwTerm = exp((-M_PI * (i + 1) * band(j)) / Fs);

            H(i, j)        = -4.0 * M_PI / Fs * bwTerm * sin((2.0 * M_PI * (i + 1) * freq(j)) / Fs);
            H(i, numF + j) = -2.0 * M_PI / Fs * bwTerm * cos((2.0 * M_PI * (i + 1) * freq(j)) / Fs);
        }
    }

    return H;
}

VectorXd calcCepstrumMapping(const VectorXd& m, int cepOrder, double Fs)
{
    const int numF = m.size() / 2;
    auto freq = m.segment(0, numF);
    auto band = m.segment(numF, numF);

    VectorXd C_int(numF);
    VectorXd C(cepOrder);

    for (int i = 0; i < cepOrder; ++i) {
        for (int p = 0; p < numF; ++p) {
            const double bwTerm = (2.0 / (double) (i + 1)) * exp((-M_PI * (i + 1) * band(p)) / Fs);
            C_int(p) = bwTerm * cos((2.0 * M_PI * (i + 1) * freq(p)) / Fs);
        }
        C(i) = C_int.sum();
    }

    return C;
}
