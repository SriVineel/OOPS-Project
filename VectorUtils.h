//=============================================================================
// VectorUtils.h
//=============================================================================
// PURPOSE:
//   Provides operator overloads for sf::Vector2f to enable intuitive vector math.
//   Makes interaction code more readable and less verbose.
//
// USAGE:
//   Include this header wherever you need vector arithmetic:
//   - Addition: position + offset
//   - Subtraction: endPos - startPos
//   - Scaling: velocity * deltaTime
//   - Division: sum / count (with divide-by-zero protection)
//
// SAFETY:
//   Division by zero returns the original vector unchanged (defensive fallback)
//
// WHERE TO MODIFY:
//   Add more operators here if you need dot product, magnitude, normalization, etc.
//=============================================================================

#pragma once

#include <SFML/System/Vector2.hpp>

// Vector addition: combines two vectors component-wise
inline sf::Vector2f operator+(const sf::Vector2f& l, const sf::Vector2f& r) { 
    return {l.x + r.x, l.y + r.y}; 
}

// Vector subtraction: computes difference between two vectors
inline sf::Vector2f operator-(const sf::Vector2f& l, const sf::Vector2f& r) { 
    return {l.x - r.x, l.y - r.y}; 
}

// Scalar multiplication: scales a vector by a scalar value
inline sf::Vector2f operator*(const sf::Vector2f& l, float s) { 
    return {l.x * s, l.y * s}; 
}

// Scalar division: divides vector by scalar (returns original if scalar is zero)
inline sf::Vector2f operator/(const sf::Vector2f& l, float s) { 
    return s == 0.f ? l : sf::Vector2f{l.x / s, l.y / s}; 
}

// Compound addition: adds right vector to left and returns reference
inline sf::Vector2f& operator+=(sf::Vector2f& l, const sf::Vector2f& r) { 
    l.x += r.x; 
    l.y += r.y; 
    return l; 
}

// Equality comparison: checks if two vectors are exactly equal
inline bool operator==(const sf::Vector2f& l, const sf::Vector2f& r) { 
    return l.x == r.x && l.y == r.y; 
}

// Inequality comparison: checks if two vectors differ
inline bool operator!=(const sf::Vector2f& l, const sf::Vector2f& r) { 
    return !(l == r); 
}