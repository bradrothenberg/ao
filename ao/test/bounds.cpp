/*
Ao: a CAD kernel for modeling with implicit functions
Copyright (C) 2017  Matt Keeter

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "catch.hpp"

#include "ao/tree/tree.hpp"
#include "ao/solve/bounds.hpp"

#include "util/shapes.hpp"

using namespace Kernel;

TEST_CASE("findBounds")
{
    SECTION("Simple sphere")
    {
        auto s = sphere(0.5);
        auto r = findBounds(s);
        CAPTURE(r.lower);
        CAPTURE(r.upper);
        REQUIRE(r.lower.x() == Approx(-0.5).epsilon(0.001));
        REQUIRE(r.lower.y() == Approx(-0.5).epsilon(0.001));
        REQUIRE(r.lower.z() == Approx(-0.5).epsilon(0.001));

        REQUIRE(r.upper.x() == Approx(0.5).epsilon(0.001));
        REQUIRE(r.upper.y() == Approx(0.5).epsilon(0.001));
        REQUIRE(r.upper.z() == Approx(0.5).epsilon(0.001));
    }

    SECTION("Rotated shape")
    {
        auto s = rotate2d(circle(0.5), M_PI/4);
        auto r = findBounds(s);

        CAPTURE(r.lower);
        CAPTURE(r.upper);

        REQUIRE(!std::isinf(r.lower.x()));
        REQUIRE(!std::isinf(r.lower.y()));
        REQUIRE(r.lower.x() <= -0.5);
        REQUIRE(r.lower.y() <= -0.5);
        REQUIRE(r.lower.x() > -1);
        REQUIRE(r.lower.y() > -1);
        REQUIRE(std::isinf(r.lower.z()));

        REQUIRE(!std::isinf(r.upper.x()));
        REQUIRE(!std::isinf(r.upper.y()));
        REQUIRE(r.upper.x() >= 0.5);
        REQUIRE(r.upper.y() >= 0.5);
        REQUIRE(r.lower.x() < 1);
        REQUIRE(r.lower.y() < 1);
        REQUIRE(std::isinf(r.upper.z()));
    }

    SECTION("Reflected shape")
    {
        auto s = circle(0.5).remap(Tree::Y(), Tree::X(), Tree::Z());
        auto r = findBounds(s);

        CAPTURE(r.lower);
        CAPTURE(r.upper);

        REQUIRE(!std::isinf(r.lower.x()));
        REQUIRE(!std::isinf(r.lower.y()));
        REQUIRE(r.lower.x() <= -0.5);
        REQUIRE(r.lower.y() <= -0.5);
        REQUIRE(r.lower.x() > -1);
        REQUIRE(r.lower.y() > -1);
        REQUIRE(std::isinf(r.lower.z()));

        REQUIRE(!std::isinf(r.upper.x()));
        REQUIRE(!std::isinf(r.upper.y()));
        REQUIRE(r.upper.x() >= 0.5);
        REQUIRE(r.upper.y() >= 0.5);
        REQUIRE(r.lower.x() < 1);
        REQUIRE(r.lower.y() < 1);
        REQUIRE(std::isinf(r.upper.z()));
    }

    SECTION("Tiny transform with numerical implications")
    {
        // Rotate by very close to 90 degrees
        auto s = circle(1).remap(Tree::X() - Tree::Y()*0.0001,
                                   Tree::Y() + Tree::X()*0.0001, Tree::Z());
        auto r = findBounds(s);

        CAPTURE(r.lower);
        CAPTURE(r.upper);

        // TODO: make this better
        REQUIRE(!std::isinf(r.lower.x()));
        REQUIRE(!std::isinf(r.lower.y()));
        REQUIRE(r.lower.x() <= -0.5);
        REQUIRE(r.lower.y() <= -0.5);
        //REQUIRE(r.lower.x() > -1);
        //REQUIRE(r.lower.y() > -1);
        REQUIRE(std::isinf(r.lower.z()));

        REQUIRE(!std::isinf(r.upper.x()));
        REQUIRE(!std::isinf(r.upper.y()));
        REQUIRE(r.upper.x() >= 0.5);
        REQUIRE(r.upper.y() >= 0.5);
        //REQUIRE(r.lower.x() < 1);
        //REQUIRE(r.lower.y() < 1);
        REQUIRE(std::isinf(r.upper.z()));
    }
}
