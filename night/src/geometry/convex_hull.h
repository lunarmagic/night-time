#pragma once

#include "math/Math.h"
#include <algorithm>
#include <iostream>
#include <vector>

namespace night
{
	namespace place_holder
	{
		//struct point {
		//	float x;
		//	float y;

		//	point(float xIn, float yIn) : x(xIn), y(yIn) {}
		//};

		//using point = vec2;
		// The z-value of the cross product of segments 
		// (a, b) and (a, c). Positive means c is ccw
		// from (a, b), negative cw. Zero means its collinear.
		inline float ccw(const vec2& a, const vec2& b, const vec2& c) {
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}

		// Returns true if a is lexicographically before b.
		inline bool isLeftOf(const vec2& a, const vec2& b) {
			return (a.x < b.x || (a.x == b.x && a.y < b.y));
		}

		// Used to sort vec2s in ccw order about a pivot.
		struct ccwSorter {
			const vec2& pivot;

			ccwSorter(const vec2& inPivot) : pivot(inPivot) {}

			bool operator()(const vec2& a, const vec2& b) {
				return ccw(pivot, a, b) < 0;
			}
		};

		// The length of segment (a, b).
		inline float len(const vec2& a, const vec2& b) {
			return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
		}

		// The unsigned distance of p from segment (a, b).
		inline float dist(const vec2& a, const vec2& b, const vec2& p) {
			return fabs((b.x - a.x) * (a.y - p.y) - (b.y - a.y) * (a.x - p.x)) / len(a, b);
		}

		// Returns the index of the farthest vec2 from segment (a, b).
		inline size_t getFarthest(const vec2& a, const vec2& b, const std::vector<vec2>& v) {
			size_t idxMax = 0;
			float distMax = dist(a, b, v[idxMax]);

			for (size_t i = 1; i < v.size(); ++i) {
				float distCurr = dist(a, b, v[i]);
				if (distCurr > distMax) {
					idxMax = i;
					distMax = distCurr;
				}
			}

			return idxMax;
		}


		// The gift-wrapping algorithm for convex hull.
		// https://en.wikipedia.org/wiki/Gift_wrapping_algorithm
		inline std::vector<vec2> giftWrapping(std::vector<vec2> v) {
			// Move the leftmost vec2 to the beginning of our vector.
			// It will be the first vec2 in our convext hull.
			std::swap(v[0], *std::min_element(v.begin(), v.end(), isLeftOf));

			std::vector<vec2> hull;
			// Repeatedly find the first ccw vec2 from our last hull vec2
			// and put it at the front of our array. 
			// Stop when we see our first vec2 again.
			do {
				hull.push_back(v[0]);
				std::swap(v[0], *std::min_element(v.begin() + 1, v.end(), ccwSorter(v[0])));
			} while (v[0].x != hull[0].x && v[0].y != hull[0].y);

			return hull;
		}


		// The Graham scan algorithm for convex hull.
		// https://en.wikipedia.org/wiki/Graham_scan
		inline std::vector<vec2> GrahamScan(std::vector<vec2> v) {
			// Put our leftmost vec2 at index 0
			std::swap(v[0], *std::min_element(v.begin(), v.end(), isLeftOf));

			// Sort the rest of the vec2s in counter-clockwise order
			// from our leftmost vec2.
			std::sort(v.begin() + 1, v.end(), ccwSorter(v[0]));

			// Add our first three vec2s to the hull.
			std::vector<vec2> hull;
			auto it = v.begin();
			hull.push_back(*it++);
			hull.push_back(*it++);
			hull.push_back(*it++);

			while (it != v.end()) {
				// Pop off any vec2s that make a convex angle with *it
				while (ccw(*(hull.rbegin() + 1), *(hull.rbegin()), *it) >= 0) {
					hull.pop_back();
				}
				hull.push_back(*it++);
			}

			return hull;
		}


		// The monotone chain algorithm for convex hull.
		inline std::vector<vec2> monotoneChain(std::vector<vec2> v) {
			// Sort our vec2s in lexicographic order.
			sort(v.begin(), v.end(), isLeftOf);

			// Find the lower half of the convex hull.
			std::vector<vec2> lower;
			for (auto it = v.begin(); it != v.end(); ++it) {
				// Pop off any vec2s that make a convex angle with *it
				while (lower.size() >= 2 && ccw(*(lower.rbegin() + 1), *(lower.rbegin()), *it) >= 0) {
					lower.pop_back();
				}
				lower.push_back(*it);
			}

			// Find the upper half of the convex hull.
			std::vector<vec2> upper;
			for (auto it = v.rbegin(); it != v.rend(); ++it) {
				// Pop off any vec2s that make a convex angle with *it
				while (upper.size() >= 2 && ccw(*(upper.rbegin() + 1), *(upper.rbegin()), *it) >= 0) {
					upper.pop_back();
				}
				upper.push_back(*it);
			}

			std::vector<vec2> hull;
			hull.insert(hull.end(), lower.begin(), lower.end());
			// Both hulls include both endvec2s, so leave them out when we 
			// append the upper hull.
			hull.insert(hull.end(), upper.begin() + 1, upper.end() - 1);
			return hull;
		}


		// Recursive call of the quickhull algorithm.
		inline void quickHull(const std::vector<vec2>& v, const vec2& a, const vec2& b,
			std::vector<vec2>& hull) {
			if (v.empty()) {
				return;
			}

			vec2 f = v[getFarthest(a, b, v)];

			// Collect vec2s to the left of segment (a, f)
			std::vector<vec2> left;
			for (auto p : v) {
				if (ccw(a, f, p) > 0) {
					left.push_back(p);
				}
			}
			quickHull(left, a, f, hull);

			// Add f to the hull
			hull.push_back(f);

			// Collect vec2s to the left of segment (f, b)
			std::vector<vec2> right;
			for (auto p : v) {
				if (ccw(f, b, p) > 0) {
					right.push_back(p);
				}
			}
			quickHull(right, f, b, hull);
		}

		// QuickHull algorithm. 
		// https://en.wikipedia.org/wiki/QuickHull
		inline std::vector<vec2> quickHull(const std::vector<vec2>& v)
		{
			std::vector<vec2> hull;

			// Start with the leftmost and rightmost vec2s.
			vec2 a = *std::min_element(v.begin(), v.end(), isLeftOf);
			vec2 b = *std::max_element(v.begin(), v.end(), isLeftOf);

			// Split the vec2s on either side of segment (a, b)
			std::vector<vec2> left, right;
			for (auto p : v) {
				ccw(a, b, p) > 0 ? left.push_back(p) : right.push_back(p);
			}

			// Be careful to add vec2s to the hull
			// in the correct order. Add our leftmost vec2.
			hull.push_back(a);

			// Add hull vec2s from the left (top)
			quickHull(left, a, b, hull);

			// Add our rightmost vec2
			hull.push_back(b);

			// Add hull vec2s from the right (bottom)
			quickHull(right, b, a, hull);

			return hull;
		}

#if 0
		std::vector<vec2> getPoints()
		{
			std::vector<vec2> v;

			const float lo = -100.0;
			const float hi = 100.0;

			for (int i = 0; i < 100; ++i) {
				float x = lo +
					static_cast<float>(
						rand()) / static_cast<float>(RAND_MAX / (hi - lo));

				float y = lo +
					static_cast<float>(
						rand()) / static_cast<float>(RAND_MAX / (hi - lo));

				v.push_back(vec2(x, y));
			}

			return v;
		}
#endif
	}
}