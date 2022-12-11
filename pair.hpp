#pragma once

#include <cstdio>
#include <iostream>

using namespace std;

#define CSI "\x1b["

struct Pair
{
	int x, y;
	Pair(int x, int y) : x(x), y(y) {}
	Pair(int x) : Pair(x, x) {}

	//explicit operator bool() { return x != -1 && y != -1; }
	Pair& operator += (Pair p) { x += p.x, y += p.y; return *this; }
	Pair operator + (Pair p) const { Pair res = *this; return res += p; }
	Pair& operator -= (Pair p) { x -= p.x, y -= p.y; return *this; }
	Pair operator - (Pair p) const { Pair res = *this; return res -= p; }
	Pair operator - () const { return Pair{ 0, 0 } - *this; }
	Pair& operator *= (Pair p) { x *= p.x, y *= p.y;return *this; }
	Pair operator * (Pair p) const { Pair res = *this; return res *= p; }
	Pair operator /= (Pair p) { x /= p.x, y /= p.y; return *this; }
	Pair operator / (Pair p) const { Pair res = *this;  return res /= p; }
	auto operator <=> (const Pair& p) const = default;

	static void go(Pair pos)
	{
		printf(CSI "%d;%dH", pos.x, pos.y);
	}
	template<typename... Ts>
	static void print(Pair&& pos, Ts ... args)
	{
		go(pos);
		(cout << ... << args);
	}
	template<typename... Ts>
	static void print(Pair& pos, Ts ... args)
	{
		go(pos);
		(cout << ... << args);
		pos.y += ((int)string(args).size() + ...); // TODO: wrong adding length of vt sequence
	}
	template<typename... Ts>
	static void println(Pair& pos, Ts ... args)
	{
		go(pos);
		(cout << ... << args);
		pos.x++;
	}
};
