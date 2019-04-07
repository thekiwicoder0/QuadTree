// QuadTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

using namespace std;

struct Point
{
	Point(float x, float y) 
		: x(x), 
		y(y) 
	{}

	float x;
	float y;
};

struct Bounds
{
	Bounds(const Point& min, const Point& max) 
		: min(min)
		, max(max) 
	{}

	Point min;
	Point max;
};

struct QuadTree
{
	QuadTree(float minX, float minY, float maxX, float maxY, unsigned int maxCapacity)
		: maxCapacity(maxCapacity)
		, bounds(Point(minX, minY), Point(maxX, maxY))
	{}

	unsigned int maxCapacity;
	Bounds bounds;
	vector<Point> points;
	unique_ptr<QuadTree> topLeft;
	unique_ptr<QuadTree> topRight;
	unique_ptr<QuadTree> bottomLeft;
	unique_ptr<QuadTree> bottomRight;
};

bool Contains(const Bounds& bounds, const Point& point)
{
	return (point.x >= bounds.min.x) && 
		(point.y >= bounds.min.y) &&
		(point.x <= bounds.max.x) && 
		(point.y <= bounds.max.y);
}

bool Intersects(const Bounds& first, const Bounds& second)
{
	if (second.max.x < first.min.x) return false;
	if (second.max.y < first.min.y) return false;
	if (second.min.x > first.max.x) return false;
	if (second.min.y > first.max.y) return false;
	return true;
}

void Split(unique_ptr<QuadTree>& tree)
{
	if (tree->topLeft != nullptr)
	{
		return;
	}

	Point center((tree->bounds.min.x + tree->bounds.max.x) / 2, (tree->bounds.min.y + tree->bounds.max.y) / 2);
	Point min = tree->bounds.min;
	Point max = tree->bounds.max;

	tree->topLeft = make_unique<QuadTree>(min.x, min.y, center.x, center.y, tree->maxCapacity);
	tree->topRight = make_unique<QuadTree>(center.x, min.y, max.x, center.y, tree->maxCapacity);
	tree->bottomLeft = make_unique<QuadTree>(min.x, center.y, center.x, max.y, tree->maxCapacity);
	tree->bottomRight = make_unique<QuadTree>(center.x, center.y, max.x, max.y, tree->maxCapacity);
}

bool Insert(unique_ptr<QuadTree>& tree, Point point)
{
	// Reject it if it's outside tree bounds
	if (!Contains(tree->bounds, point))
	{
		return false;
	}

	// If we have capacity just add it here and we're done.
	if (tree->points.size() < tree->maxCapacity)
	{
		tree->points.push_back(point);
		return true;
	}

	// Otherwise split the tree
	Split(tree);

	// And then add the point into an individual subtree
	if (Insert(tree->topLeft, point)) return true;
	if (Insert(tree->topRight, point)) return true;
	if (Insert(tree->bottomLeft, point)) return true;
	if (Insert(tree->bottomRight, point)) return true;

	// This should never happen
	return false;
}

void OverlapQuery(unique_ptr<QuadTree>& tree, const Bounds& region, vector<Point>& output)
{
	if (!tree)
	{
		return;
	}

	if (!Intersects(tree->bounds, region))
	{
		return;
	}

	for (const auto& p : tree->points)
	{
		if (Contains(region, p))
		{
			output.push_back(p);
		}
	}

	OverlapQuery(tree->topLeft, region, output);
	OverlapQuery(tree->topRight, region, output);
	OverlapQuery(tree->bottomLeft, region, output);
	OverlapQuery(tree->bottomRight, region, output);
}

int main()
{
	// Test parameters
	const int treeCapacity = 4;
	const int numPoints = 10000;
	const int maxPointRange = 1000;

	// Create the tree and fill it with data
	std::cout << "Initialising tree..." << endl;
	unique_ptr<QuadTree> tree = make_unique<QuadTree>(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX, treeCapacity);
	for (int i = 0; i < numPoints; ++i)
	{
		float x = (rand() / (float)RAND_MAX) * maxPointRange;
		float y = (rand() / (float)RAND_MAX) * maxPointRange;
		Insert(tree, Point(x, y));
	}

	vector<Point> result;
	Bounds queryRegion(Point(10, 10), Point(100, 100));

	// Perform the query
	auto begin = chrono::steady_clock::now();
	OverlapQuery(tree, queryRegion, result);
	auto end = chrono::steady_clock::now();
	auto elapsedTime = chrono::duration_cast<chrono::nanoseconds>(end - begin).count();

	std::cout << "Query took: " << elapsedTime << "ns" << endl;
	std::cout << "Overlapping points: " << result.size() << endl;
}
