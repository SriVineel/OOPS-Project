// VectorUtils.h
// Small inline overloads to make vector math terse and readable in interaction code.
// Note: Division by zero returns the left-hand vector unchanged as a defensive fallback.

#pragma once
#include <SFML/Graphics.hpp>

inline sf::Vector2f operator+(const sf::Vector2f& l, const sf::Vector2f& r) { return {l.x + r.x, l.y + r.y}; }
inline sf::Vector2f operator-(const sf::Vector2f& l, const sf::Vector2f& r) { return {l.x - r.x, l.y - r.y}; }
inline sf::Vector2f operator*(const sf::Vector2f& l, float s) { return {l.x * s, l.y * s}; }
inline sf::Vector2f operator/(const sf::Vector2f& l, float s) { return s == 0.f ? l : sf::Vector2f{l.x / s, l.y / s}; }
inline sf::Vector2f& operator+=(sf::Vector2f& l, const sf::Vector2f& r) { l.x += r.x; l.y += r.y; return l; }
inline bool operator==(const sf::Vector2f& l, const sf::Vector2f& r) { return l.x == r.x && l.y == r.y; }
inline bool operator!=(const sf::Vector2f& l, const sf::Vector2f& r) { return !(l == r); }
