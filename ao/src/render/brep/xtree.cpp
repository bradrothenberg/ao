#include "ao/render/brep/xtree.hpp"

namespace Kernel {

template <unsigned N>
XTree<N>::XTree(Evaluator* eval, Region<N> region)
    : region(region)
{
    // Do a preliminary evaluation to prune the tree
    eval->eval(region.lower3(), region.upper3());
    eval->push();

    if (!findVertex(eval))
    {
        auto rs = region.subdivide();
        for (unsigned i=0; i < (1 << N); ++i)
        {
            if (!rs[i].empty())
            {
                children[i].reset(new XTree<N>(eval, rs[i]));
            }
        }
    }
    eval->pop();
}

/*  Used for compile-time checking of array bounds in findVertex */
constexpr unsigned _pow(unsigned x, unsigned y)
{
    return y ? x * _pow(x, y - 1) : 1;
}

template <unsigned N>
template <unsigned R>
bool XTree<N>::findVertex(Evaluator* eval)
{
    constexpr unsigned num = _pow(R, N);
    static_assert(num < Result::N, "Bad resolution");

    // Evaluation takes place in 3-space regardless of tree dimensionality
    // because evaluators need X / Y / Z coordinates
    auto lower = region.lower;
    auto upper = region.upper;
    auto center = (region.lower + region.upper) / 2;
    Eigen::Array<float, R, N> pts;

    // Pre-compute per-axis grid positions
    for (unsigned i=0; i < R; ++i)
    {
        const float frac = (i + 0.5) / R;
        pts.row(i) = lower * (1 - frac) + upper * frac;
    }

    // Load all sample points into the evaluator
    Eigen::Array<float, num, N> positions;
    for (unsigned i=0; i < num; ++i)
    {
        // Unpack from grid positions into the position vector
        for (unsigned j=0; j < N; ++j)
        {
            positions(i, j) = pts((i % _pow(R, j + 1)) / _pow(R, j), j);
        }

        Eigen::Vector3f pos; // the evaluator works in 3-space
        pos.template head<N>() = positions.row(i);
        pos.tail<3 - N>() = region.perp;
        eval->set(pos, i);
    }

    // Get derivatives!
    auto ds = eval->derivs(num);

    // Load data into QEF arrays here
    Eigen::Matrix<float, num, N + 1> A;
    Eigen::Matrix<float, num, 1> b;
    for (unsigned i=0; i < num; ++i)
    {
        Eigen::Array3f deriv(ds.dx[i], ds.dy[i], ds.dz[i]);

        // Load this row of A matrix
        A.row(i).template head<N>() = deriv.head<N>();
        A(i, N) = -1;

        // Temporary variable for dot product
        Eigen::Matrix<float, 1, N + 1> n;
        n.template head<N>() = positions.row(i) - center.transpose();
        n(N) = ds.v[i];

        b(i) = A.row(i).dot(n);
    }

    // Solve QEF (least-squares)
    auto sol = A.jacobiSvd(Eigen::ComputeThinU |
                           Eigen::ComputeThinV).solve(b);

    // Store vertex location
    vert = sol.template head<N>().array() + center;

    std::cout << "vert:\n" << vert << "\n";

    auto err = A * sol - b;
    std::cout << "err: " << err.dot(err) << '\n';

    // Check error
    return true;
}

// Explicit initialization of templates
template class XTree<2>;
template class XTree<3>;

}   // namespace Kernel
